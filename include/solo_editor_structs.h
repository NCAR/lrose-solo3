/* 	$Id$	 */
#ifndef solo_editor_structs_h
#define solo_editor_structs_h

#include <vector>

#define             SE_FRAME  SOLO_MAX_WINDOWS+1
#define         SE_MAX_GATES  2048
#define        SE_MAX_STRING  256
#define   SE_INSIDE_BOUNDARY  1
#define  SE_OUTSIDE_BOUNDARY -1
#define     MAX_SETUP_INPUTS  7

#include <sed_shared_structs.h>
#include <solo_window_structs.h>
#include <DataManager.hh>
#include <ForEachRayCmd.hh>
#include <OneTimeOnlyCmd.hh>

// Defines for BB_init_on values

#define         BB_USE_FGG 0
#define     BB_USE_AC_WIND 1
#define  BB_USE_LOCAL_WIND 2

// Defines for histo_output_key values

#define   SE_HST_NOCOPY 0
#define     SE_HST_COPY 1

/* c------------------------------------------------------------------------ */

struct zmap_points {
    struct zmap_points *next_list;
    struct zmap_points *next;
    double lat;
    double lon;
    float src_val;
    float curr_val;
    float azm;
    float rng;
    float x;
    float y;
    int number;
    char id[48];
    char list_id[16];
};

/* c------------------------------------------------------------------------ */

struct prev_bnd_sets {
  struct prev_bnd_sets *last;
  struct prev_bnd_sets *next;
  int sizeof_set;
  char *at;    // It looks like this is a generic_descriptor pointer
};
/* c------------------------------------------------------------------------ */

struct se_pairs {
    int low;
    int high;
    int sum;
    float f_low;
    float f_high;
    float f_sum;
    struct se_pairs *last;
    struct se_pairs *next;
};
/* c------------------------------------------------------------------------ */

struct solo_edit_stuff
{
  std::vector< std::string > radar_stack;
  
  int num_radars;
  struct swp_file_input_control *sfic;
  int se_frame;		/* frame number to use for editing
			 */
  
  /* BB unfolding */
  float ew_wind;
  float ns_wind;
  float ud_wind;
  int BB_avg_count;
  int BB_init_on;
  int BB_max_neg_folds;
  int BB_max_pos_folds;
  int num_unfolded;
  std::string BB_unfold_field;

  /* despeckling */
  int a_speckle;
  std::string despeckle_field;
  int num_despeckled;

  /* forced unfolding */
  float forced_unfold_center;
  std::string forced_unfolding_field;

  /* simple thresholding */
  float threshold_val;
  int threshold_where;	/* 0 => "below" */
  int num_thresholded;
  std::string threshold_target;
  std::string threshold_field;

  /* defreckling */
  float freckle_threshold;
  int dual_average_offset;
  int freckle_avg_count;
  int num_defreckled;
  std::string defreckle_field;

  /* remove sidelobe ring */
  int sidelobe_ring_gate_count;
  std::string ac_sidelobe_ring_field;

  /* remove ac motion */
  std::string remove_ac_motion_field;

  /* histogram stuff */
#define      H_BIN_MAX 100
#define         H_BINS 0x01
#define        H_AREAS 0x02
#define          H_REG 0x04
#define        H_IRREG 0x08
  
  double histo_start_time;
  double histo_stop_time;
  struct se_pairs *h_pairs;

  int histo_key;
  int histo_num_bins;
  int histo_output_key;
  int histo_output_lines;

  int low_count;
  int medium_count;
  int high_count;
  int histo_missing_count;
  int num_irreg_bins;
  int *counts_array;		/* low, high and between are tacked
				 * onto the end of this array */
  float *areas_array;		/* same here */
  float low_area;
  float high_area;
  float histo_low;
  float histo_high;
  float histo_increment;
  float histo_missing_area;
  float histo_sum;
  float histo_sum_sq;

  std::string histogram_field;
  std::string histo_directory;
  std::string histo_filename;
  std::string histo_comment;
  std::vector< std::string > h_output;

  /* denotching */
  float notch_max;
  float notch_shear;
  int num_denotched;
  std::string denotch_field;

  float optimal_beamwidth;

  float add_constant;
  float multiply_constant;

#ifdef USE_RADX
#else
  int finish_up;		/* every operation may have a final step
				 * which this flag should trigger
				 */
#endif
  int check_command;		/* runs the command(s) through the ui
				 * for syntax checking but does not
				 * execute them
				 */
  int punt;
  int batch_mode;
  int ac_data;

  int bad_flags_initialized;
  int boundary_exists;
  int boundary_use;
  int first_good_gate;
  int gate_diff_interval;
  int initialize_on_fgg;
  int min_bad_count;
  int modified;
  int num_bins_averaged;
  int num_deleted;
  int num_segments;
  int process_ray_count;
  int sweep_count;
  int sweep_ray_count;
  int use_boundary;
  int use_bad_flag_mask;
  int volume_count;
  int volume_ray_count;

  unsigned short *bad_flag_mask;
  unsigned short *boundary_mask;
  unsigned short *condition_mask;

  unsigned short     *all_zeroes_array;
  unsigned short       *all_ones_array;
  unsigned short  *bad_flag_mask_array;
  unsigned short  *boundary_mask_array;

  struct solo_click_info *click;

  std::vector< std::string > cmdz; /* linked list of all commands */

  std::vector< std::string > fer_cmds; /* points to start of FER commands */
  std::vector< std::string> once_cmds; /* points to commands executed only once */
  int num_bnd_cmds;

  int num_setup_input_cmds;
  int setup_input_ndx;	/* set to zero when the process begins */
  std::vector< std::string > setup_inputs[MAX_SETUP_INPUTS];
  std::vector< std::string > edit_summary;

  // List of commands for use by widgets

  std::vector< std::string > current_cmds;
  std::vector< std::string > help_info;
  std::vector< std::string > list_ed_cmd_files;
  std::vector< std::string > list_winfo_files;
  std::vector< std::string > list_sweeps;
  std::vector< std::string > list_radars;

  std::string help_directory;
  std::string last_directory;
  std::vector< std::string > last_radar_stack;
  int last_new_version_flag;
  int32_t time_modified;

  std::string histo_radar_name;
  float histo_fixed_angle;

  int surface_gate_shift;
  float surface_shift;

  int popup_frame;
  int focus_frame;
  int manifest_editing;	/* sweep file input control info (sfic)
			 * matches frame + replot after pass */
  int num_prev_bnd_sets;
  struct prev_bnd_sets *last_pbs;
  struct prev_bnd_sets *pbs;

  float nyquist_velocity;

  int max_gates;
  float *old_scale;
  float *old_bias;
  struct zmap_points *top_zmap_list;
  struct zmap_points *curr_zmap_list;

  int gates_of_shift;
  int deglitch_min_bins;
  int deglitch_radius;
  float deglitch_threshold;

  std::vector< OneTimeOnlyCmd* > one_time_only_cmds;
  std::vector< ForEachRayCmd* > for_each_ray_cmds;
//  std::vector< UiCommand* > all_cmds; /* exhaustive list of commands */

  int check_cmd_flag;
  std::string error;

  std::string fer_lines;
  std::string oto_lines;
    
  int histo_flush;
//  std::vector< std::string > all_templates;
};

/* c------------------------------------------------------------------------ */

#endif
