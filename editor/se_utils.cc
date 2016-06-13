/* 	$Id$	 */

#include <algorithm>
#include <glib.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

#include <AssignValueCmd.hh>
#include <se_add_mult.hh>
#include <se_BB_unfold.hh>
#include <se_bnd.hh>
#include <se_catch_all.hh>
#include <se_defrkl.hh>
#include <se_flag_ops.hh>
#include <se_for_each.hh>
#include <se_histog.hh>
#include <se_proc_data.hh>
#include <se_utils.hh>
#include <SeBoundaryList.hh>
#include <solo_editor_structs.h>
#include <solo_list_mgmt.hh>
#include <solo2.hh>
#include <sp_basics.hh>

// Forward declarations

int se_malloc_flagged_arrays(struct solo_edit_stuff *seds, int max_gates);
//void se_all_fer_cmds(std::vector< std::string > &which);
//void se_all_other_commands(std::vector< std::string > &which);

extern GString *gs_complaints;

static struct solo_edit_stuff *seds=NULL;
static struct sed_command_files *scf=NULL;
static struct solo_frame_state_files *sfsf=NULL;

/* c------------------------------------------------------------------------ */

std::vector< std::string> se_absorb_strings(const std::string &path_name)
{
  std::vector< std::string > strings;
  
  FILE *stream;
  
  if ((stream = fopen(path_name.c_str(), "r")) == 0)
  {
    printf("Unable to open %s", path_name.c_str());
    return strings;
  }

  // Read in the new strings

  for (int nn = 0;; nn++)
  {
    char *a;
    
    char buffer[88];
    
    if ((a = fgetz(buffer, 88, stream)) == 0)
      break;

    strings.push_back(buffer);
  }

  fclose(stream);

  return strings;
}

/* c------------------------------------------------------------------------ */

void se_fix_comment(std::string &comment)
{
  // Remove blanks and slashes

  for (std::size_t i = 0; i < comment.size(); ++i)
    if (comment[i] == ' ' || comment[i] == '/')
      comment[i] = '_';
}

/* c------------------------------------------------------------------------ */

struct sed_command_files *se_return_cmd_files_struct(void)
{
  if (scf == 0)
  {
    scf = new struct sed_command_files;
    scf->comment_text = "no_comment";
    scf->omit_source_file_info = YES;

    struct swp_file_input_control *sfic = return_swp_fi_in_cntrl();
    scf->directory_text = sfic->directory_text;
  }

  return scf;
}
/* c------------------------------------------------------------------------ */

struct solo_frame_state_files *se_return_state_files_struct (void)
{
  if (sfsf == 0)
  {
    sfsf = new struct solo_frame_state_files;
    sfsf->comment_text = "no_comment";
    sfsf->omit_sweep_info = NO;
  }

  return sfsf;
}

/* c------------------------------------------------------------------------ */

struct solo_click_info *clear_click_info (void)
{
    struct solo_click_info *sci;

    sci = return_solo_click_ptr();
    memset(sci, 0, sizeof(struct solo_click_info));
    return(sci);
}

/* c------------------------------------------------------------------------ */

char *fgetz(char *buffer, int size, FILE *stream)
{
  char *c = buffer;

  for (int ii, jj = 0; size--; jj++,*c++ = ii)
  {
    ii = fgetc(stream);
    if (ii == EOF)
    {
      if (jj == 0)
	return 0;
      return buffer;
    }

    if (ii == '\n')
    {
      // Don't include the \n

      *c = '\0';
      return buffer;
    }
  }

  return buffer;
}

/* c------------------------------------------------------------------------ */

struct solo_click_info *return_solo_click_ptr(void)
{
    struct solo_edit_stuff *seds;

    seds = return_sed_stuff();
    if(!seds->click) {
	seds->click = (struct solo_click_info *)
	      malloc(sizeof(struct solo_click_info));
	memset(seds->click, 0, sizeof(struct solo_click_info));
    }
    return(seds->click);
}

/* c------------------------------------------------------------------------ */

int se_malloc_flagged_arrays (struct solo_edit_stuff *seds, int max_gates)
{
    int ii, nn=max_gates;
    unsigned short *us;

    if(max_gates < seds->max_gates)
	  return(seds->max_gates);

    if(seds->all_zeroes_array) free(seds->all_zeroes_array);
    if(seds->all_ones_array) free(seds->all_ones_array);
    if(seds->bad_flag_mask_array) free(seds->bad_flag_mask_array);
    if(seds->boundary_mask_array) free(seds->boundary_mask_array);

    if(!(seds->all_zeroes_array =
	 (unsigned short *)malloc(max_gates * sizeof(unsigned short)))) {
	nn = 0;
    }
    else {
	memset(seds->all_zeroes_array, 0, max_gates * sizeof(unsigned short));
    }

    if(!(seds->all_ones_array =
	 (unsigned short *)malloc(max_gates * sizeof(unsigned short)))) {
	nn = 0;
    }
    else {
	for(ii=max_gates, us = seds->all_ones_array; ii--; *us++ = 1);
    }

    if(!(seds->bad_flag_mask_array =
	 (unsigned short *)malloc(max_gates * sizeof(unsigned short)))) {
	nn = 0;
    }
    else {
	memset(seds->bad_flag_mask_array, 0
	       , max_gates * sizeof(unsigned short));
    }

    if(!(seds->boundary_mask_array =
	 (unsigned short *)malloc(max_gates * sizeof(unsigned short)))) {
	nn = 0;
    }
    else {
	memset(seds->boundary_mask_array, 0
	       , max_gates * sizeof(unsigned short));
    }

    if(!nn) {
	printf("Unable to malloc flagged arrays!\n");
    }
    else {
	seds->max_gates = max_gates;
    }
    return(nn);
}

/* c------------------------------------------------------------------------ */

struct solo_edit_stuff *return_sed_stuff (void)
{
  // If the edit stuff hasn't been allocated yet, allocate it now

  if (!seds)
  {
    seds = new struct solo_edit_stuff;
    
    if (seds == 0)
    {
      printf("Unable to malloc solo editor struct!\n");
      yes_exit();
    }

    seds->num_radars = 0;
    seds->sfic = (struct swp_file_input_control *)0;
    seds->se_frame = 0;
    seds->ew_wind = 0.0;
    seds->ns_wind = 0.0;
    seds->ud_wind = 0.0;
    seds->BB_avg_count = 0;
    seds->BB_init_on = 0;
    seds->BB_max_neg_folds = 0;
    seds->BB_max_pos_folds = 0;
    seds->num_unfolded = 0;
    seds->BB_unfold_field[0] = '\0';
    seds->a_speckle = 0;
    seds->despeckle_field[0] = '\0';
    seds->num_despeckled = 0;
    seds->forced_unfold_center = 0.0;
    seds->forced_unfolding_field[0] = '\0';
    seds->threshold_val = 0.0;
    seds->threshold_where = 0;
    seds->num_thresholded = 0;
    seds->threshold_target[0] = '\0';
    seds->threshold_field[0] = '\0';
    seds->freckle_threshold = 0.0;
    seds->dual_average_offset = 0;
    seds->freckle_avg_count = 0;
    seds->num_defreckled = 0;
    seds->defreckle_field[0] = '\0';
    seds->sidelobe_ring_gate_count = 0;
    seds->ac_sidelobe_ring_field[0] = '\0';
    seds->remove_ac_motion_field[0] = '\0';
    seds->histo_start_time = 0.0;
    seds->histo_stop_time = 0.0;
    seds->h_pairs = 0;
    seds->histo_key = 0;
    seds->histo_num_bins = 0;
    seds->histo_output_key = 0;
    seds->histo_output_lines = 0;
    seds->low_count = 0;
    seds->medium_count = 0;
    seds->high_count = 0;
    seds->histo_missing_count = 0;
    seds->num_irreg_bins = 0;
    seds->counts_array = (int *)0;
    seds->areas_array = (float *)0;
    seds->low_area = 0.0;
    seds->high_area = 0.0;
    seds->histo_low = 0.0;
    seds->histo_high = 0.0;
    seds->histo_increment = 0.0;
    seds->histo_missing_area = 0.0;
    seds->histo_sum = 0.0;
    seds->histo_sum_sq = 0.0;
    seds->histogram_field[0] = '\0';
    seds->histo_directory[0] = '\0';
    seds->histo_filename[0] = '\0';
    seds->histo_comment[0] = '\0';
    seds->notch_max = 0.0;
    seds->notch_shear = 0.0;
    seds->num_denotched = 0;
    seds->denotch_field[0] = '\0';
    seds->optimal_beamwidth = 0.0;
    seds->add_constant = 0.0;
    seds->multiply_constant = 0.0;
#ifdef USE_RADX
#else
    seds->finish_up = 1;
#endif
    seds->check_command = 0;
    seds->punt = 0;
    seds->batch_mode = 0;
    seds->ac_data = 0;
    seds->bad_flags_initialized = 0;
    seds->boundary_exists = 0;
    seds->boundary_use = 0;
    seds->first_good_gate = 0;
    seds->gate_diff_interval = 0;
    seds->initialize_on_fgg = 0;
    seds->min_bad_count = 0;
    seds->modified = 0;
    seds->num_bins_averaged = 0;
    seds->num_deleted = 0;
    seds->num_segments = 0;
    seds->process_ray_count = 0;
    seds->sweep_count = 0;
    seds->sweep_ray_count = 0;
    seds->use_boundary = 0;
    seds->use_bad_flag_mask = 0;
    seds->volume_count = 0;
    seds->volume_ray_count = 0;
    seds->bad_flag_mask = (unsigned short *)0;
    seds->boundary_mask = (unsigned short *)0;
    seds->condition_mask = (unsigned short *)0;
    seds->all_zeroes_array = (unsigned short *)0;
    seds->all_ones_array = (unsigned short *)0;
    seds->bad_flag_mask_array = (unsigned short *)0;
    seds->boundary_mask_array = (unsigned short *)0;
    seds->click = (struct solo_click_info *)0;
    seds->num_bnd_cmds = 0;
    seds->num_setup_input_cmds = 0;
    seds->setup_input_ndx = 0;
    seds->help_directory[0] = '\0';
    seds->last_directory[0] = '\0';
    seds->last_new_version_flag = 0;
    seds->time_modified = 0;
    seds->histo_radar_name[0] = '\0';
    seds->histo_fixed_angle = 0.0;
    seds->surface_gate_shift = 0;
    seds->surface_shift = 0.0;
    seds->popup_frame = 0;
    seds->focus_frame = 0;
    seds->manifest_editing = 0;
    seds->num_prev_bnd_sets = 0;
    seds->last_pbs = (struct prev_bnd_sets *)0;
    seds->pbs = (struct prev_bnd_sets *)0;
    seds->nyquist_velocity = 0.0;
    seds->max_gates = 0;
    seds->old_scale = new float[DataManager::MAX_PARAMS];
    seds->old_bias = new float[DataManager::MAX_PARAMS];
    for (int i = 0; i < DataManager::MAX_PARAMS; ++i)
    {
      seds->old_scale[i] = 0.0;
      seds->old_bias[i] = 0.0;
    }
    seds->top_zmap_list = (struct zmap_points *)0;
    seds->curr_zmap_list = (struct zmap_points *)0;
    seds->gates_of_shift = 0;
    seds->deglitch_min_bins = 0;
    seds->deglitch_radius = 0;
    seds->deglitch_threshold = 0.0;
    seds->check_cmd_flag = 0;
    seds->error[0] = '\0';
    seds->fer_lines = "";
    seds->oto_lines = "";
    seds->histo_flush = 0;
    
    if (!(se_malloc_flagged_arrays(seds, SE_MAX_GATES)))
    {
      yes_exit();
    }
 
    seds->sfic = new struct swp_file_input_control;
    seds->sfic->start_time = 0.0;
    seds->sfic->stop_time = 0.0;
    seds->sfic->frame = 0;
    seds->sfic->version = 99999;
    seds->sfic->radar_num = 0;
    seds->sfic->new_version_flag = 0;
    seds->sfic->first_sweep_text = " ";
    seds->sfic->last_sweep_text = " ";
    seds->sfic->version_text = "last";
    seds->sfic->clicked_frame = 0;
    seds->se_frame = SE_FRAME;

    // Set default parameters

    seds->BB_avg_count = 4;
    seds->BB_max_neg_folds = 999;
    seds->BB_max_pos_folds = 999;
    seds->BB_unfold_field = "VU";

    seds->a_speckle = 3;
    seds->despeckle_field = "VD";

    seds->threshold_val = 0.33;
    seds->threshold_target = "VD";
    seds->threshold_field = "NCP";
	
    seds->freckle_threshold = 5.0;
    seds->dual_average_offset = 1;
    seds->freckle_avg_count = 5;
    seds->defreckle_field = "DZ";

    seds->sidelobe_ring_gate_count = 3;
    seds->ac_sidelobe_ring_field = "VU";

    seds->optimal_beamwidth = 0;

    seds->remove_ac_motion_field = "VG";
    
    seds->histo_comment = "no_comment";

//    se_all_fer_cmds(seds->all_fer_cmds);
//    se_all_other_commands(seds->all_other_cmds);
    
    seds->surface_gate_shift = -3;

  }

  return seds;
}

/* c------------------------------------------------------------------------ */

struct swp_file_input_control *return_swp_fi_in_cntrl (void)
{
    struct solo_edit_stuff *seds;

    seds = return_sed_stuff();
    return(seds->sfic);
}

/* c------------------------------------------------------------------------ */

//void se_all_fer_cmds(std::vector< std::string > &which)
//{
//  which.push_back("absolute-value of <field>");
//  which.push_back("BB-unfolding of <field>");
//  which.push_back("add-field <field> to <field> put-in <field>");
//  which.push_back("add-value <real> to <field> put-in <field>");
//  which.push_back("and-bad-flags with <field> <where> <real>");
//  which.push_back("assert-bad-flags in <field>");
//  which.push_back("assign-value <real> to <field>");
//  which.push_back("clear-bad-flags");
//  which.push_back("complement-bad-flags");
//  which.push_back("copy <field> to <field>");
//  which.push_back("copy-bad-flags from <field>");
//  which.push_back("despeckle <field>");
//  which.push_back("do-histogram");
//  which.push_back("dont-use-boundary");
//  which.push_back("duplicate <field> in <field>");
//  which.push_back("establish-and-reset <field> to <field>");
//  which.push_back("exponentiate <field> by <real> put-in <field>");
//  which.push_back("fix-vortex-velocities in <field>");
//  which.push_back("flag-freckles in <field>");
//  which.push_back("flag-glitches in <field>");
//  which.push_back("flagged-add of <real> in <field>");
//  which.push_back("flagged-assign of <real> in <field>");
//  which.push_back("flagged-copy <field> to <field>");
//  which.push_back("flagged-multiply by <real> in <field>");
//  which.push_back("forced-unfolding in <field> around <real>");
//  which.push_back("header-value <name> is <real>");
//  which.push_back("ignore-field <field>");
//  which.push_back("merge-field <field> with <field> put-in <field>");
//  which.push_back("mult-fields <field> by <field> put-in <field>");
//  which.push_back("multiply <field> by <real> put-in <field>");
//  which.push_back("or-bad-flags with <field> <where> <real>");
//  which.push_back("radial-shear in <field> put-in <field>");
//  which.push_back("rain-rate <field> by <real> put-in <field>");
//  which.push_back("remove-aircraft-motion in <field>");
//  which.push_back("remove-only-surface in <field>");
//  which.push_back("remove-only-second-trip-surface in <field>");
//  which.push_back("remove-ring in <field> from <real> to <real> km.");
//  which.push_back("remove-storm-motion in <field> of <real> deg <real> mps");
//  which.push_back("remove-surface in <field>");
//  which.push_back("rescale-field <field> <real> <real> scale and bias");
//  which.push_back("rewrite");
//  which.push_back("set-bad-flags when <field> <where> <real>");
//  which.push_back("shift-field <field> put-in <field>");
//  which.push_back("subtract <field> from <field> put-in <field>");
//  which.push_back("threshold <field> on <field> <where> <real>");
//  which.push_back("unconditional-delete in <field>");
//  which.push_back("use-boundary");
//  which.push_back("xor-bad-flags with <field> <where> <real>");
//  which.push_back("xy-listing of <field> and  <field>");
//}

/* c------------------------------------------------------------------------ */

//void se_all_input_commands (std::vector< std::string > &which)
//{
//  which.push_back("first-sweep date-time");
//**  which.push_back("generate-sweep-list for radar-name");
//  which.push_back("last-sweep date-time");
//  which.push_back("new-version");
//**  which.push_back("process-data");
//**  which.push_back("source-version-number is version-indicator");
//  which.push_back("sweep-directory is directory-name");
//}

/* c------------------------------------------------------------------------ */

//void se_all_other_commands(std::vector< std::string > &which)
//{
//  which.push_back("BB-gates-averaged is <integer> gates");
//  which.push_back("BB-max-neg-folds is <integer>");
//  which.push_back("BB-max-pos-folds is <integer>");
//  which.push_back("BB-use-ac-wind");
//  which.push_back("BB-use-first-good-gate");
//  which.push_back("BB-use-local-wind");
//  which.push_back("a-speckle is <integer> gates");
//
//  which.push_back("append-histogram-to-file");
//  which.push_back("area-histogram on <field>");
//  which.push_back("count-histogram on <field>");
//  which.push_back("deglitch-min-gates is <integer>");
//  which.push_back("deglitch-radius is <integer>");
//  which.push_back("deglitch-threshold is <real>");
//  which.push_back("dont-append-histogram-to-file");
//  which.push_back("ew-wind is <real>");
//  which.push_back("first-good-gate is <integer>");
//  which.push_back("freckle-average is <integer> gates");
//  which.push_back("freckle-threshold is <real>");
//  which.push_back("gates-shifted is <integer>");
//  which.push_back("histogram-comment <comment>");
//  which.push_back("histogram-directory <directory>");
//  which.push_back("histogram-flush");
//  which.push_back("irregular-histogram-bin from <real> to <real>");
//  which.push_back("min-bad-count is <integer> gates");
//  which.push_back("min-notch-shear is <real>");
//  which.push_back("notch-max is <real>");
//  which.push_back("new-histogram-file");
//  which.push_back("ns-wind is <real>");
//  which.push_back("nyquist-velocity is <real>");
//  which.push_back("optimal-beamwidth is <real> degrees");
//  which.push_back("offset-for-radial-shear is <integer> gates");
//  which.push_back("regular-histogram-parameters low <real> high <real> increment <real>");
//  which.push_back("surface-gate-shift is <integer> gates");
//  which.push_back("vert-wind is <real>");
//  which.push_back("xy-directory <directory>");
//}

/* c------------------------------------------------------------------------ */

void se_print_strings(const std::vector< std::string > &strings)
{
  for (size_t i = 0; i < strings.size(); ++i)
  {
    std::string print_string = strings[i];
    
    // Remove the trailing CR

    if (print_string[print_string.size()-1] == '\n')
      print_string = print_string.substr(0, print_string.size()-1);
    
    g_message(print_string.c_str());
  }
}

/* c------------------------------------------------------------------------ */

std::string se_unquote_string(const std::string &quoted_string)
{
  std::string unquoted_string = quoted_string;
  
  // Remove the double quotes from either end of the string

  size_t quote_pos;
  while ((quote_pos = unquoted_string.find("\"")) != std::string::npos)
    unquoted_string.erase(quote_pos);
  
  return unquoted_string;
}

/* c---------------------------------------------------------------------- */

void tokenize(const std::string &line,
	      std::vector< std::string > &tokens)
{
  tokenize(line, tokens, " \t\n");
}

void tokenize(const std::string &line,
	      std::vector< std::string > &tokens,
	      const std::string &delimiters)
{
  tokens.clear();
  
  char *line_string = new char[line.size()+1];
  strcpy(line_string, line.c_str());
  char *line_ptr = line_string;
  
  for (line_ptr = strtok(line_ptr, delimiters.c_str()); line_ptr != 0;
       line_ptr = strtok(NULL, delimiters.c_str()))
  {
    tokens.push_back(line_ptr);
  }

  delete [] line_string;
}

/* c---------------------------------------------------------------------- */

double angle_diff(double a1, double a2)
{
    double d=a2-a1;

    if( d < -180. )
	  return(d+360.);
    if( d > 180. )
	  return(d-360.);
    return(d);
}

/* c---------------------------------------------------------------------- */

char *read_file_lines(char *line, int nn, FILE *stream)
{
    /* read in lines from a file and remove leading blanks
     * trailing blanks and line feeds
     */
    int ii, jj;
    unsigned char *aa, *cc;

    for(aa=cc=(unsigned char*)line ;; aa=cc=(unsigned char*)line) {
	for(jj=1 ;;) {
	    ii = fgetc(stream);
	    if(ii == EOF || ii == '\n') {
		*cc = '\0';
		break;
	    }
	    if(jj < nn) {
		*cc++ = ii;
		jj++;
	    }
	}
	for(; aa < cc && *aa == ' '; aa++); /* leading blanks */
	for(; aa < cc && *(cc-1) == ' '; *(--cc) = '\0'); /* trailing blanks */
	if(aa != cc)
	      return( (char *) aa);
	if(ii == EOF)
	      return(NULL);
    }
}
