/* 	$Id$	 */

#include <ctype.h>
#include <fcntl.h>
#include <glib.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef USE_RADX
#include <Radx/RadxGeoref.hh>
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxVol.hh>
#endif

#include <dd_math.h>
#include <DataInfo.hh>
#include <DataManager.hh>
#include <DateTime.hh>
#include "se_proc_data.hh"
#include "se_utils.hh"
#include "se_wgt_stf.hh"
#include <seds.h>
#include <sii_exam_widgets.hh>
#include <sii_perusal.hh>
#include <sii_utils.hh>
#include <solo_editor_structs.h>
#include <solo_list_widget_ids.h>
#include <solo_window_structs.h>
#include <solo2.hh>
#include <soloii.hh>
#include <sp_basics.hh>
#include <sp_clkd.hh>
#include "sxm_examine.hh"

extern GString *gs_complaints;

# define PRESERVATION_FILE_NAME "preserved_sweep_files_list.txt"
static const char *preservation_file_name=PRESERVATION_FILE_NAME;

/*
 * log files are time stamped at the time they are opened
 * data displays are always preceeded by a "stat" line to identify the data
 * 
 * you can toggle between logging the display and not logging
 * the display by typing "xlog"
 * "xlog" close will cause the file to be closed
 */
# define     LOG_CLOSED 0
# define     LOG_ACTIVE 1
# define  LOG_SUSPENDED 2

static FILE *log_stream=NULL;
static int log_state=LOG_CLOSED;
static char log_directory[128];
static char log_file_name[80];
static char *log_dir_ptr=NULL;

static int click_frame=0, focus_frame=0;
static struct se_changez *sxm_spair_changez=NULL;
static struct examine_widget_info *ewi=NULL;

/* c------------------------------------------------------------------------ */

void remove_duplicates(std::vector< std::string > &list)
{
  if (list.size() <= 1)
    return;
  
  sort(list.begin(), list.end());

  std::vector< std::string >::iterator prev_entry = list.begin();
  std::vector< std::string >::iterator curr_entry = prev_entry + 1;
	  
  while (curr_entry != list.end())
  {
    if (*curr_entry == *prev_entry)
    {
      list.erase(curr_entry);
      curr_entry = prev_entry + 1;
    }
    else
    {
      prev_entry = curr_entry;
      curr_entry = prev_entry + 1;
    }
  }
}

/* c------------------------------------------------------------------------ */

int sxm_append_to_log_stream(const char *stuff, const int nchar)
{
    int nn;

    if(log_state != LOG_ACTIVE){
	return(0);
    }

    sxm_open_log_stream();

    if(!log_stream){
	return(0);
    }
    
    char line[4096];
    sprintf(line,"%s\n",stuff);

    return(nn = fwrite(line, sizeof(char), strlen(line), log_stream));
}

/* c------------------------------------------------------------------------ */

void sxm_change_cell_in_list(struct se_changez *chz,
			     const int data_row_num, const double val)
{
  WW_PTR wwptr = solo_return_wwptr(chz->window_num);
  struct examine_control *ecs = wwptr->examine_control;
  int entry_num = data_row_num + ecs->heading_row_count;

  strcpy(ecs->data_line, wwptr->examine_list[entry_num].c_str());

  int col = chz->col_num;

  std::string display_string;
  
  if (val == ecs->bad_data[col])
  {
    display_string = ecs->del_str;
  }
  else
  {
  char tmp_val[16];
    sprintf(tmp_val, ecs->actual_data_format.c_str(), val);
    if ((int)strlen(tmp_val) > ecs->num_chars_per_cell)
    {
      tmp_val[ecs->num_chars_per_cell-1] = '|';
      tmp_val[ecs->num_chars_per_cell] = '\0';
    }

    display_string = tmp_val;
  }
  strncpy(ecs->col_ptrs[col], display_string.c_str(), ecs->num_chars_per_cell);

  wwptr->examine_list[entry_num] = std::string(ecs->data_line);
}

/* c------------------------------------------------------------------------ */

void 
sxm_clear_changes (int frme)
{
    WW_PTR wwptr;

    wwptr = solo_return_wwptr(frme);

    for(; wwptr->changez_list;) {
	sxm_undo_last(frme);
    }
}

/* c------------------------------------------------------------------------ */

void sxm_click_in_data(struct solo_click_info *sci)
{
  WW_PTR wwptr;

  wwptr = solo_return_wwptr(sci->frame);

  wwptr->examine_control->ctr_on_click = YES;
  wwptr->examine_control->click_angle = wwptr->clicked_angle;

  if (wwptr->view->type_of_plot & SOLO_TIME_SERIES)
    wwptr->examine_control->click_range = KM_TO_M(wwptr->field_vals->rng);
  else
    wwptr->examine_control->click_range = KM_TO_M(wwptr->clicked_range);
  sxm_get_widget_info(sci->frame);
  if (wwptr->examine_info->whats_selected ==  EX_RADAR_DATA)
    sxm_update_examine_data(sci->frame, 0L);
}

/* c------------------------------------------------------------------------ */

void sxm_close_log_stream (void)
{
    if(!log_stream)
	  return;
    fclose(log_stream);
    log_stream = NULL;
}
/* c------------------------------------------------------------------------ */

void sxm_flush_log_stream (void)
{
    if(!log_stream){
	printf("sxm_flush_log_stream: log_stream is NULL\n");
	return;
    }
    fflush(log_stream);
}
/* c------------------------------------------------------------------------ */

void sxm_set_log_dir(const std::string &dir)
{
  char str[2048];
  sprintf(str, "Setting log directory to: %s\n", dir.c_str());
  sii_message(str);
  sxm_close_log_stream();
  if (dir[dir.size()-1] == '/')
    strcpy(log_directory, dir.c_str());
  else
    sprintf(log_directory, "%s/", dir.c_str());
  log_dir_ptr = log_directory;
  log_state = LOG_ACTIVE;
}

/* c------------------------------------------------------------------------ */

void sxm_toggle_log_dir(const bool active)
{
  char str[2048];
  log_state = (active) ? LOG_ACTIVE : LOG_SUSPENDED;

  sprintf (str, "Toggled log state to: %s!\n"
	   , (active) ? "log active" : "log suspended");
  sii_message (str);
}

/* c------------------------------------------------------------------------ */

void sxm_gen_all_files_list(int window_index, std::vector< std::string > &file_list)
{
  // NOTE: Can we clean this up?  Which window pointer to we really want?
  // If we can't clean this up, we need to figure this out and put in a comment.

  WW_PTR wwptr = solo_return_wwptr(window_index);
  int frame_index = wwptr->lead_sweep->window_num;
  wwptr = solo_return_wwptr(frame_index);

  DataManager::getInstance()->getFileList(frame_index,
					  wwptr->sweep->directory_name,
					  wwptr->sweep->radar_num,
					  file_list);
}

/* c------------------------------------------------------------------------ */

void 
sxm_gen_delete_lists (struct solo_click_info *sci)
{
    size_t ii;
    size_t jj;
    size_t kk;
    WW_PTR wwptr;
    struct solo_perusal_info *spi;

    wwptr = solo_return_wwptr(sci->frame);
    wwptr = wwptr->lead_sweep;
    spi = solo_return_winfo_ptr();

    spi->list_select_files.clear();
    
    /*
     * produce a list of all the files for this radar
     * in the current directory
     */
    sxm_gen_all_files_list(wwptr->window_num, spi->list_all_files);
    /*
     * locate the current file in this list
     */
    for (jj = 0; jj < spi->list_all_files.size(); ++jj)
    {
      if (spi->list_all_files[jj].compare(wwptr->sweep->file_name) == 0)
	break;
    }
    kk = sci->ival0 >= 0 ? sci->ival0 : 0; /* sweep count comes in ival0 */

    for(; wwptr; wwptr = wwptr->next_sweep) { /* broadcast the sweep count */
	wwptr->sweep->sweep_keep_count = kk;
    }

    // Copy all the previous file name except for a few into the select list

    if (jj < spi->list_all_files.size() && jj > kk)
    {
      jj -= kk;
      spi->list_select_files.clear();
      
      for (ii = 0; ii <= jj; ii++)
	spi->list_select_files.push_back(spi->list_all_files[ii].c_str());
    }
}
/* c------------------------------------------------------------------------ */

void 
sxm_get_widget_info (int frme)
{
    int width, frac;
    float f;
    WW_PTR wwptr;
    struct examine_control *ecs;

    if (ewi == 0)
    {
      ewi = new struct examine_widget_info;

      ewi->window_num = 0;
      ewi->whats_selected = 0;
      ewi->scroll_increment = 0;
      ewi->ray_num = 0;
      ewi->at_cell = 0;
      ewi->ray_count = 0;
      ewi->cell_count = 0;
      ewi->change_count = 0;
      ewi->typeof_change = 0;
      ewi->row_annotation = 0;
      ewi->col_annotation = 0;
    }

    /* nab the user modifiable contents of the examine widget
     */
    se_dump_examine_widget(frme, ewi);
    wwptr = solo_return_wwptr(frme);
    ecs = wwptr->examine_control;

    if(ewi->ray_num >= 0)
	  wwptr->examine_info->ray_num = ewi->ray_num;
    if(ewi->at_cell >= 0)
	  wwptr->examine_info->at_cell = ewi->at_cell;
    if(ewi->ray_count > 0)
	  wwptr->examine_info->ray_count = ewi->ray_count;
    if(ewi->cell_count > 0)
	  wwptr->examine_info->cell_count = ewi->cell_count;
    if(ewi->scroll_increment > 0)
	  wwptr->examine_info->scroll_increment = ewi->scroll_increment;

    switch(ewi->typeof_change) {
    case EX_MINUS_FOLD:
    case EX_PLUS_FOLD:
    case EX_DELETE:
    case EX_REPLACE:
	wwptr->examine_info->typeof_change = ewi->typeof_change;
	break;
    default:
	break;
    }
    switch(ewi->row_annotation) {
    case EX_VIEW_RANGES:
    case EX_VIEW_CELL_NUMS:
	wwptr->examine_info->row_annotation = ewi->row_annotation;
	break;
    default:
	break;
    }
    switch(ewi->col_annotation) {
    case EX_VIEW_ROT_ANGS:
    case EX_VIEW_RAY_NUMS:
	wwptr->examine_info->col_annotation = ewi->col_annotation;
	break;
    default:
	break;
    }
    wwptr->examine_info->row_annotation =
      (ewi->row_annotation == EX_VIEW_CELL_NUMS) ? YES : NO;
    wwptr->examine_info->col_annotation =
      (ewi->col_annotation == EX_VIEW_RAY_NUMS) ? YES : NO;

    strcpy(wwptr->examine_info->fields_list, ewi->fields_list.c_str());
    if (sscanf(ewi->modify_val.c_str(), "%f", &f) == 1) {
	wwptr->examine_info->modify_val = f;
    }

    // Now do the display format

    std::size_t digit_pos = ewi->display_format.find_first_of("0123456789");
    
    if (digit_pos != std::string::npos)
    {
      std::string digit_string = ewi->display_format.substr(digit_pos);
      
      if (sscanf(digit_string.c_str(), "%d.%d", &width, &frac) == 2)
      {
	if (frac >= 0 && width > frac + 2)
	  sprintf(wwptr->examine_info->display_format, "%d.%df",
		  width, frac);
      }
      
    }
    
    return;
}

/*********************************************************************
 * print_radx_status_line()
 */

#ifdef USE_RADX

void print_radx_status_line(const bool verbose,
			    const std::string &preamble,
			    const std::string &postamble,
			    const RadxVol &vol,
			    const RadxSweep &sweep,
			    const RadxRay &ray,
			    char *line)
{
  // Get a pointer into the output line to use for constructing the line

  char *aa = line;
  *aa = '\0';

  char radar_name[12];
  char str[2048];

  DateTime ray_time(ray.getTimeSecs(), ray.getNanoSecs());
  
  sprintf(aa+strlen(aa), "%s ", preamble.c_str());
  sprintf(aa+strlen(aa), "%s ", ray_time.toString().c_str());
  // NOTE: Don't know if the radar name is stored in the instrument name
  // or the site name
  sprintf(aa+strlen(aa), "%s ", vol.getInstrumentName().substr(0, 8).c_str());
//  sprintf(aa+strlen(aa), "%s ", vol.getSiteName().substr(0, 8).c_str());
  aa += strlen(aa);

  // NOTE: Need to double-check on the use of the platform types

  const RadxGeoref *georef = ray.getGeoreference();
  
  switch (vol.getPlatformType())
  {
  case Radx::PLATFORM_TYPE_AIRCRAFT_AFT :
  case Radx::PLATFORM_TYPE_AIRCRAFT_TAIL :
  case Radx::PLATFORM_TYPE_AIRCRAFT_BELLY :
  case Radx::PLATFORM_TYPE_AIRCRAFT_ROOF :
    sprintf(aa+strlen(aa), "rt:%6.1f ", FMOD360(georef->getRotation()));
    sprintf(aa+strlen(aa), "t:%5.1f ", georef->getTilt());
    sprintf(aa+strlen(aa), "rl:%5.1f ", georef->getRoll());
    sprintf(aa+strlen(aa), "h:%6.1f ", georef->getHeading());
    sprintf(aa+strlen(aa), "al:%6.3f ", georef->getAltitudeKmMsl());
    break;
    
  case Radx::PLATFORM_TYPE_AIRCRAFT :
  case Radx::PLATFORM_TYPE_AIRCRAFT_FORE :
  case Radx::PLATFORM_TYPE_AIRCRAFT_NOSE :
    sprintf(aa+strlen(aa), "az:%6.1f ", ray.getAzimuthDeg());
    sprintf(aa+strlen(aa), "el:%6.2f ", ray.getElevationDeg());
    sprintf(aa+strlen(aa), "h:%6.1f ", georef->getHeading());
    sprintf(aa+strlen(aa), "rl:%5.1f ", georef->getRoll());
    sprintf(aa+strlen(aa), "p:%5.1f ", georef->getPitch());
    break;
    
  default:
    sprintf(aa+strlen(aa), "fx:%6.1f ", ray.getFixedAngleDeg());
    sprintf(aa+strlen(aa), "az:%6.1f ", ray.getAzimuthDeg());
    sprintf(aa+strlen(aa), "el:%6.2f ", ray.getElevationDeg());
  }
  aa += strlen(aa);

  sprintf(aa+strlen(aa), "swp: %2d ", sweep.getSweepNumber());
  sprintf(aa+strlen(aa), "%s ", postamble.c_str());

  if(verbose)
  {
    sprintf(aa+strlen(aa), "\n");

    sprintf(aa+strlen(aa), "la:%9.4f ", georef->getLatitude());
    sprintf(aa+strlen(aa), "lo:%9.4f ", georef->getLongitude());

    if (vol.getPlatformType() == Radx::PLATFORM_TYPE_AIRCRAFT_AFT ||
	vol.getPlatformType() == Radx::PLATFORM_TYPE_AIRCRAFT_TAIL ||
	vol.getPlatformType() == Radx::PLATFORM_TYPE_AIRCRAFT_BELLY ||
	vol.getPlatformType() == Radx::PLATFORM_TYPE_AIRCRAFT_ROOF ||
	vol.getPlatformType() == Radx::PLATFORM_TYPE_AIRCRAFT ||
	vol.getPlatformType() == Radx::PLATFORM_TYPE_AIRCRAFT_FORE ||
	vol.getPlatformType() == Radx::PLATFORM_TYPE_AIRCRAFT_NOSE)
    {
      sprintf(aa+strlen(aa), "p:%5.1f ", georef->getPitch());
      sprintf(aa+strlen(aa), "d:%5.1f ", georef->getDrift());
      sprintf(aa+strlen(aa), "ag:%6.3f ", georef->getAltitudeKmAgl());
      sprintf(aa+strlen(aa), "\n");
      sprintf(aa+strlen(aa), "ve:%6.1f ", georef->getEwVelocity());
      sprintf(aa+strlen(aa), "vn:%6.1f ", georef->getNsVelocity());
      sprintf(aa+strlen(aa), "vv:%6.1f ", georef->getVertVelocity());
      sprintf(aa+strlen(aa), "we:%6.1f ", georef->getEwWind());
      sprintf(aa+strlen(aa), "wn:%6.1f ", georef->getNsWind());
      sprintf(aa+strlen(aa), "wv:%6.1f ", georef->getVertWind());
    }
    else
    {
      sprintf(aa+strlen(aa), "al:%6.3f ", georef->getAltitudeKmMsl());
    }

    sprintf(aa+strlen(aa), "\n");

    const std::vector< RadxField* > fields = ray.getFields();
    
    sprintf(aa+strlen(aa), "Num parameters: %d  ",
	    fields.size());

    for (std::size_t ii = 0; ii < fields.size(); ii++)
    {
      sprintf(aa+strlen(aa), "%s ", fields[ii]->getName().substr(0, 8).c_str());
    }
    sprintf(aa+strlen(aa), "\n");
  }
}

#endif


/* c------------------------------------------------------------------------ */

void sxm_list_beams(const int frame_index)
{
  static const std::string method_name = "sxm_list_beams()";
  
  WW_PTR wwptr = solo_return_wwptr(frame_index);
  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);

  wwptr->examine_list.clear();

  if (wwptr->lead_sweep->sweep_file_modified)
  {
    // If it's been edited, reopen the file

    DataManager::getInstance()->nabNextFile(wwptr->lead_sweep->window_num,
					    DataManager::TIME_NEAREST,
					    DataManager::LATEST_VERSION,
					    YES);
    wwptr->lead_sweep->sweep_file_modified = NO;
  }

#ifdef USE_RADX

  const RadxVol &radx_vol = data_info->getRadxVolume();
  const RadxSweep *sweep = data_info->getRadxSweep();
  if (sweep == 0)
  {
    std::cerr << "ERROR: " << method_name << std::endl;
    std::cerr << "No current sweep -- cannot list beams" << std::endl;
  }
  else
  {

    std::vector< RadxRay* > rays = radx_vol.getRays();
    
    for (std::size_t ray_index = sweep->getStartRayIndex();
	 ray_index <= sweep->getEndRayIndex(); ++ray_index)
    {
      char str[2048];
    
      print_radx_status_line(false, "", "",
			     radx_vol, *sweep, *rays[ray_index],
			     str);
      wwptr->examine_list.push_back(str);
    }
  }
  
#else
  data_info->rewindBuffer();
  data_info->loadHeaders();

  for (int i = 0; i < data_info->getNumRays(); ++i)
  {
    if (!data_info->loadNextRay())
      break;

    char str[2048];
    
    data_info->printStatusLine(false, "", "", str);
    wwptr->examine_list.push_back(str);
  }
#endif

  se_refresh_examine_widgets(frame_index, wwptr->examine_list);
}

/* c------------------------------------------------------------------------ */

void sxm_list_descriptors(const int frame_index)
{
  WW_PTR wwptr = solo_return_wwptr(frame_index);
  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);

  wwptr->examine_list.clear();
#ifdef USE_RADX
#else
  data_info->listDescriptors(wwptr->examine_list);
#endif
  se_refresh_examine_widgets(frame_index, wwptr->examine_list);
}

/* c------------------------------------------------------------------------ */

void sxm_list_edit_hist(const int frame_index)
{
  WW_PTR wwptr = solo_return_wwptr(frame_index);
  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);

  wwptr->examine_list.clear();
  
  if (!data_info->haveCommands())
  {
    wwptr->examine_list.push_back(" ");
    wwptr->examine_list.push_back("No edit history for this sweep");
    wwptr->examine_list.push_back(" ");
    return;
  }

  char *aa = data_info->getCommandPtr();
  char *bb = aa;
  char *zz = aa + data_info->getCommandSize();

  for (; bb < zz; bb++)
  {
    // Find the next line feed

    char str[2048];
    char *cc = str;

    for (aa = bb; bb < zz && *bb != '\n'; *cc++ = *bb++);
    *cc++ = '\0';

    if (strlen(str) != 0)
      wwptr->examine_list.push_back(str);

    if (bb >= zz)
      break;
  }

  se_refresh_examine_widgets(frame_index, wwptr->examine_list);
}
/* c------------------------------------------------------------------------ */

void sxm_list_to_log(const std::vector< std::string > &list)
{
  if (log_state != LOG_ACTIVE)
    return;

  sxm_open_log_stream();
  if (!log_stream)
    return;
    
  for (size_t i = 0; i < list.size(); ++i)
    fprintf(log_stream, "%s\n", list[i].c_str());
}
/* c------------------------------------------------------------------------ */

#ifdef USE_RADX

void sxm_log_radx_stat_line(const DataInfo &data_info,
			    const std::size_t ray_num)
{
  static const std::string method_name = "sxm_log_radx_stat_line()";
  
  char str[2048];

  if (log_state != LOG_ACTIVE)
    return;

  const RadxVol &radx_vol = data_info.getRadxVolume();
  const RadxSweep *sweep = data_info.getRadxSweep();

  if (sweep == 0)
  {
    std::cerr << "ERROR: " << method_name << std::endl;
    std::cerr << "No current sweep -- cannot log status line" << std::endl;
    return;
  }
  
  std::vector< RadxRay* > rays = radx_vol.getRays();
  
  sxm_open_log_stream();

  print_radx_status_line(true, "", "",
			 radx_vol, *sweep,
			 *rays[sweep->getStartRayIndex() + ray_num],
			 str);

  fprintf(log_stream, "%s", str);
}

#else

void sxm_log_stat_line(const DataInfo &data_info)
{
  char str[2048];

  if (log_state != LOG_ACTIVE)
    return;

  sxm_open_log_stream();

  data_info.printStatusLine(true, "", "", str);

  fprintf(log_stream, "%s", str);
}

#endif

/* c------------------------------------------------------------------------ */

void 
sxm_open_log_stream (void)
{
    char str[2048], *aa;

    if(log_stream)
	  return;

    if(!log_dir_ptr) {
      if ((aa = getenv("LOG_DIRECTORY")) != 0) {
	    strcpy(log_directory, aa);
	}
	else {
	    strcpy(log_directory, "./");
	}
	log_dir_ptr = log_directory;
    }
    strcpy(str, log_dir_ptr);
    aa = str + strlen(str);
    std::string file_name =
      DataManager::getInstance()->getFileBaseName("log", time(0), "SED_LOG", 0);
    strcpy(aa, file_name.c_str());
    g_string_sprintfa (gs_complaints,"Opening display log file: %s\n", str);
    if(!(log_stream = fopen(str, "w"))) {
	g_string_sprintfa (gs_complaints,"**** Unable to open this file! ****\n");
	return;
    }
    strcpy(log_file_name, aa);
}
/* c------------------------------------------------------------------------ */

struct se_changez *
sxm_pop_change (struct se_changez **topptr)
{
    /* pops a change struct off a stack
     */
    struct se_changez *top=(*topptr);

    if(!top)
	  return(NULL);

    if(top->next) {
	top->next->last = top->last;
    }
    *topptr = top->next;
    return(top);
}
/* c------------------------------------------------------------------------ */

struct se_changez *
sxm_pop_spair_change (void)
{
    /* pops a changez struct off the spairs stack
     */
    struct se_changez *chz=sxm_spair_changez;

    if(!chz) {
	chz = (struct se_changez *)malloc(sizeof(struct se_changez));
	memset(chz, 0, sizeof(struct se_changez));
    }
    else {
	sxm_spair_changez = chz->next; 
	memset(chz, 0, sizeof(struct se_changez));
    }
    return(chz);
}

/* c------------------------------------------------------------------------ */

void 
sxm_push_change (struct se_changez *chz, struct se_changez **topptr)
{
    struct se_changez *top=(*topptr);

    if(!chz)
	  return;

    if(!top) {
	chz->last = chz;
	chz->next = NULL;
    }
    else {
	chz->next = top;
	chz->last = top->last;
	top->last = chz;
    }
    *topptr = chz;
    return;
}
/* c------------------------------------------------------------------------ */

void 
sxm_push_spair_change (struct se_changez *chz)
{
    /* pushes an individual struct on to the spairs stack
     */
    if(!chz)
	  return;
    chz->next = sxm_spair_changez;
    sxm_spair_changez = chz;
    return;
}
/* c------------------------------------------------------------------------ */

void sxm_refresh_list(const int frame_index)
{
  WW_PTR wwptr = solo_return_wwptr(frame_index);

  switch (wwptr->examine_info->whats_selected)
  {
  case EX_RADAR_DATA:
    sxm_get_widget_info(frame_index);
    sxm_update_examine_data(frame_index, 0L);
    break;
  case EX_BEAM_INVENTORY:
    sxm_list_beams(frame_index);
    break;
  case EX_EDIT_HIST:
    sxm_list_edit_hist(frame_index);
    break;
  case EX_DESCRIPTORS:
    sxm_list_descriptors(frame_index);	
    break;
  default:
    break;
  }
}
/* c------------------------------------------------------------------------ */

struct se_changez *
sxm_remove_change (struct se_changez **topptr, struct se_changez *this_se)
{
    /* pops a change struct off a stack
     */
    struct se_changez *top=(*topptr);

    if(!top || !this_se)
	  return(NULL);

    if(this_se == top) {
	if(top->next) {
	    top->next->last = top->last;
	}
	*topptr = top->next;
    }
    else {
	this_se->last->next = this_se->next;
	if(this_se->next) {
	    this_se->next->last = this_se->last;
	}
	else { /* at the end */
	    top->last = this_se->last;
	}
    }
    return(this_se);
}

/* c------------------------------------------------------------------------ */

void 
sxm_set_click_frame (int frme, double theta, double range)
{
    WW_PTR wwptr;
    struct solo_edit_stuff *seds;
    char *aa, str[2048];

    if(frme >= 0 && frme < SOLO_MAX_WINDOWS) {
	seds = return_sed_stuff();
	aa = str;
	seds->focus_frame = 
	      focus_frame = click_frame = frme;
	wwptr = solo_return_wwptr(frme);
	sprintf(aa,
		"Click at angle, range, and frame | %7.2f | %8.3f | %d |\n"
		, theta, range, frme);
	sxm_append_to_log_stream(aa, strlen(aa));
    }
}

/* c------------------------------------------------------------------------ */

void 
sxm_undo_last (int frme)
{
    int ii, nn;
    struct se_changez *chz;
    struct examine_control *ecs;
    WW_PTR wwptr;
    float *fptr;

    wwptr = solo_return_wwptr(frme);
    ecs = wwptr->examine_control;
    if(!(chz = sxm_pop_change(&wwptr->changez_list)))
	  return;
    /* decrement the changes count
     */
    wwptr->examine_info->change_count--;

    switch(chz->typeof_change) {

    case EX_MINUS_FOLD:
    case EX_PLUS_FOLD:
    case EX_DELETE:
    case EX_REPLACE:
    case EX_REMOVE_AIR_MOTION:
	fptr = ecs->data_ptrs[chz->col_num] + chz->row_num;
	*fptr = chz->f_old_val;
	sxm_change_cell_in_list(chz, chz->row_num, *fptr);
	break;

    case EX_RAY_PLUS_FOLD:
    case EX_RAY_MINUS_FOLD:
    case EX_GT_PLUS_FOLD:
    case EX_GT_MINUS_FOLD:
    case EX_RAY_IGNORE:

	if(chz->second_cell_num) { /* it's a run */
	    ii = chz->row_num;
	    nn = ii + chz->second_cell_num - chz->cell_num;
	}
	else if (chz->typeof_change == EX_GT_PLUS_FOLD ||
		chz->typeof_change == EX_GT_MINUS_FOLD ) {
	    ii = chz->row_num; nn = wwptr->examine_control->actual_num_cells;
	}
	else {
	    ii = 0; nn = wwptr->examine_control->actual_num_cells;
	}
	fptr = ecs->data_ptrs[chz->col_num] + ii;

	for(; ii < nn; ii++, fptr++) {
	    if(chz->typeof_change != EX_RAY_IGNORE &&
	       *fptr != ecs->bad_data[chz->col_num])
		  *fptr -= chz->f_new_val;
	    sxm_change_cell_in_list(chz, ii, *fptr);
	}
	break;
    }
    /*
     * pop this change back onto spair changes
     */
    sxm_push_spair_change(chz);
}
/* c------------------------------------------------------------------------ */

void sxm_unlink_files_list(const std::vector< std::string > &file_list,
			   const char *dir, const int frame_index)
{
  // Removes the files listed.

  int nn;
  char *aa, *bb, name[256], line[256];
  struct solo_perusal_info *spi;
  FILE *stream;

  if ((nn = file_list.size()) <= 0 || !dir || !strlen(dir))
    return;

  // Read in the preserved files list

  std::vector< std::string > preserve_list;
    
  if (dir[strlen(dir)-1] == '/')
    sprintf(name, "%s%s", dir, preservation_file_name);
  else
    sprintf(name, "%s/%s", dir, preservation_file_name);
  if ((stream = fopen(name, "r")) != 0)
  {
    for (; (aa = read_file_lines(line, 88, stream)) != 0;)
    {
      preserve_list.push_back(aa);
    }
    fclose(stream);
  }

  aa = name;
  if (dir[strlen(dir)-1] == '/')
    strcpy(aa, dir);
  else
    sprintf(aa, "%s/", dir);
  bb = aa + strlen(aa);

  for (size_t jj = 0; jj < file_list.size(); jj++)
  {
    size_t ii;
    
    for (ii = 0; ii < preserve_list.size(); ++ii)
    {
      if (preserve_list[ii].compare(file_list[jj]) == 0)
	break;
    }

    if (ii == preserve_list.size())
    {
      strcpy(bb, file_list[jj].c_str());
      g_string_sprintfa (gs_complaints,"Unlinking %s\n", aa);
      nn = unlink(name);
    }
    else
    {
      g_string_sprintfa(gs_complaints,
			"**** %s is in the preserved file list!\n",
			file_list[jj].c_str());
    }
  }
  spi = solo_return_winfo_ptr();
  sxm_gen_all_files_list(frame_index, spi->list_all_files);
}

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX

int create_sweep_ray_list(WW_PTR wwptr, const int button,
			  struct ts_ray_table *tsrt)
{
  struct solo_examine_info *sei = wwptr->examine_info;

  if (button == EX_SCROLL_LEFT)
  {
    sei->ray_num--;
  }
  else if (button == EX_SCROLL_RIGHT)
  {
    sei->ray_num++;
  }

  struct examine_control *ecs = wwptr->examine_control;
  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);

  if (ecs->ctr_on_click)
  {
    // If we get here, the user has clicked in the frame data while the
    // examine window was open.  We are centering the data in the examine
    // window on the location of the click.  ray_num is the index of the
    // first ray in the examine window after the click.  at_cell is the index
    // of the first cell.

    sei->ray_num =
      data_info->getAngleIndex(ecs->click_angle) - (sei->ray_count / 2);
    int gate_index =
      data_info->getRadxVolume().getGateIndex(ecs->click_range);
    sei->at_cell = gate_index - (sei->cell_count / 2);
    ecs->ctr_on_click = NO;
  }

  int rays_in_sweep = data_info->getRadxSweep()->getNRays();

  ecs->actual_ray_count = sei->ray_count > rays_in_sweep ?
    rays_in_sweep : sei->ray_count;
  
  if (sei->ray_num + sei->ray_count >= rays_in_sweep)
    sei->ray_num = rays_in_sweep - ecs->actual_ray_count;

  if (sei->ray_num < 0)
    sei->ray_num = 0;

  ecs->actual_ray_num = sei->ray_num;

  // The sweep info was placed in tsrt->tssi_top by solo_nab_next_file()

  struct ts_ray_info *tsri = tsrt->tsri_sxm;

  for (int ii = 0; ii < ecs->actual_ray_count; ii++, tsri++)
  {
    tsri->ray_num = ecs->actual_ray_num + ii;
    tsri->sweep_num = 1;
  }

  return ecs->actual_ray_count;
}


int create_centered_time_series_ray_list(WW_PTR wwptr, const int button,
					 struct ts_ray_table *tsrt,
					 const int frme)
{
  // Assemble a list of rays based on the clicked x pixel

  int nn = sp_ts_ray_list(frme, (int)wwptr->clicked_x_pxl, (int)3);
  struct ts_ray_info *tsrix =
    *(tsrt->tsri_list + (nn - 1) / 2); /* center ray info struct */

  // See how many rays we can actually do

  struct solo_examine_info *sei = wwptr->examine_info;

  int mm = (sei->ray_count - 1) / 2; /* in case ray count is an even number */
  nn = sei->ray_count / 2;

  // Put the current center in the center of the new list for now

  sp_copy_tsri(tsrix, tsrt->tsri_sxm + sei->ray_count - 1); 
  tsrix = tsrt->tsri_sxm + sei->ray_count - 1;

  // See if we can find the number of rays we need to the left

  int sweep_num = tsrix->sweep_num;
  int ray_num = tsrix->ray_num -1;
  struct ts_sweep_info *tssi = tsrt->tssi_top + sweep_num;
  struct ts_ray_info *tsri = tsrix -1;

  int nurl;
    
  for (nurl = 0; nurl < sei->ray_count - 1; )
  {
    if (ray_num < 0)
    {
      if (--sweep_num < 1)
	break;

      tssi--;
      ray_num = tssi->ray_count-1;
    }

    nurl++;
    tsri->sweep_num = sweep_num;
    tsri->ray_num = ray_num--;
    tsri--;
  }

  // Now look to the right

  sweep_num = tsrix->sweep_num;
  ray_num = tsrix->ray_num +1;
  tssi = tsrt->tssi_top +sweep_num;
  tsri = tsrix +1;

  int nurr;
    
  for (nurr = 0; nurr < sei->ray_count - 1; )
  {
    if (ray_num >= tssi->ray_count)
    {
      if (++sweep_num > tsrt->sweep_count)
	break;
      
      tssi++;
      ray_num = 0;
    }

    nurr++;
    tsri->sweep_num = sweep_num;
    tsri->ray_num = ray_num++;
    tsri++;
  }

  struct examine_control *ecs = wwptr->examine_control;
  ecs->actual_ray_count = sei->ray_count;
  ray_num = sei->ray_count-1 -mm;

  // Move them over to the start of the array

  if ((nurl + 1 + nurr) < sei->ray_count)
  {
    ray_num = sei->ray_count - 1 - nurl;
    ecs->actual_ray_count = nurl + 1 + nurr;
  }
  else if (nurl < mm)
  {
    ray_num = sei->ray_count - 1 - nurl;
  }
  else if (nurr < nn)
  {
    ray_num -= nn - nurr;
  }
	
  tsri = tsrt->tsri_sxm;
  for (int ii = 0; ii++ < ecs->actual_ray_count; tsri++)
  {
    sp_copy_tsri(tsri + ray_num, tsri);
  }

  sei->ray_num = tsrt->tsri_sxm->ray_num;
  sei->sweep_num = tsrt->tsri_sxm->sweep_num;
  float rng = KM_TO_M(wwptr->field_vals->rng);

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);

  sei->at_cell =
    data_info->getRadxVolume().getGateIndex(rng) - sei->cell_count/2;
  ecs->ctr_on_click = NO;

  return ecs->actual_ray_count;
}


int create_time_series_ray_list(WW_PTR wwptr, const int button,
				struct ts_ray_table *tsrt, const int frme)
{
  // Time series data!

  struct ts_ray_info *tsri_right = tsrt->tsri_top + wwptr->view->width_in_pix;
  struct examine_control *ecs = wwptr->examine_control;
  struct solo_examine_info *sei = wwptr->examine_info;

  // If this is just a refresh or a first time display, make sure the
  // starting sweep and ray numbers make sense.  i.e. see if we can assemble
  // a list based on the starting ray number and sweep number

  struct ts_ray_info *tsri;
  
  if (sei->sweep_num < 1)
  {
    int incre;
    
    if (wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT)
    {
      tsri = tsrt->tsri_top + wwptr->view->width_in_pix - 1;
      incre = -1;
    }
    else
    {
      tsri = tsrt->tsri_top;
      incre = 1;
    }

    // Find the first ray plotted

    int mm = wwptr->view->width_in_pix;
    for (; mm-- ; tsri += incre)
    {
      if (tsri->ray_num >= 0 && tsri->sweep_num > 0)
	break;
    }

    sei->sweep_num = tsri->sweep_num;
    sei->ray_num = tsri->ray_num;
  }

  if (sei->sweep_num > tsrt->sweep_count)
  {
    // Give it the last ray number of the last sweep

    sei->sweep_num = tsrt->sweep_count;
    struct ts_sweep_info *tssi = tsrt->tssi_top + sei->sweep_num;
    sei->ray_num = tssi->ray_count - 1;
  }

  int blip = 0;
  
  if (button == EX_SCROLL_LEFT)
    blip = -1;
  else if (button == EX_SCROLL_RIGHT)
    blip = 1;
  else
    blip = 0;

  if (wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT)
    blip *= -1;

  sei->ray_num += blip;

  // Get to a legitimate starting sweep and ray number

  struct ts_sweep_info *tssi = tsrt->tssi_top +sei->sweep_num;

  if (sei->ray_num >= tssi->ray_count)
  {
    if (sei->sweep_num < tsrt->sweep_count)
    {
      sei->sweep_num++;
      sei->ray_num = 0;
      tssi = tsrt->tssi_top +sei->sweep_num;
    }
    else
    {
      sei->ray_num = tssi->ray_count -1;
    }
  }
  else if (sei->ray_num < 0)
  {
    if (sei->sweep_num > 1)
    {
      sei->sweep_num--;
      tssi = tsrt->tssi_top + sei->sweep_num;
      sei->ray_num = tssi->ray_count - 1;
    }
    else
    {
      sei->ray_num = 0;
    }
  }

  int incre;
  
  if (wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT)
  {
    tsri = tsrt->tsri_sxm +sei->ray_count -1;
    incre = -1;
  }
  else
  {
    tsri = tsrt->tsri_sxm;
    incre = 1;
  }

  int sweep_num = sei->sweep_num;
  int ray_num = sei->ray_num;
  tssi = tsrt->tssi_top + sweep_num;

  int mm;
  
  for (mm = 0; mm < sei->ray_count && sweep_num <= tsrt->sweep_count;)
  {
    if (ray_num >= tssi->ray_count)
    {
      sweep_num++;
      tssi++;
      ray_num = 0;
      continue;
    }

    mm++;
    tsri->sweep_num = sweep_num;
    tsri->ray_num = ray_num++;
    tsri += incre;
  }

  ecs->actual_ray_count = mm;

  return ecs->actual_ray_count;
}

int sxm_ray_list(const int frme, const int button)
{
  // Assemble a list of ts_ray_info structs representing the requisite
  // number of rays to be displayed

  WW_PTR wwptr = solo_return_wwptr(frme);
  struct ts_ray_table *tsrt = wwptr->lead_sweep->tsrt;
  struct examine_control *ecs = wwptr->examine_control;
  struct solo_examine_info *sei = wwptr->examine_info;

  // Make sure there is space the list the requisite number of rays

  if (tsrt->max_sxm_entries < 2 * sei->ray_count)
  {
    if (tsrt->tsri_sxm != 0)
      free(tsrt->tsri_sxm);

    tsrt->max_sxm_entries = 2 * sei->ray_count;
    tsrt->tsri_sxm =
      (struct ts_ray_info *)malloc(tsrt->max_sxm_entries *
				   sizeof(struct ts_ray_info));
    memset(tsrt->tsri_sxm, 0,
	   tsrt->max_sxm_entries * sizeof(struct ts_ray_info));
  }

  // Create the ray list depending on the type of plot

  if (wwptr->view->type_of_plot & SOLO_TIME_SERIES)
  {
    if (ecs->ctr_on_click)
      return create_centered_time_series_ray_list(wwptr, button, tsrt, frme);
    else
      return create_time_series_ray_list(wwptr, button, tsrt, frme);
  }
  else
  {
    return create_sweep_ray_list(wwptr, button, tsrt);
  }
}

#else

int sxm_ray_list (int frme, int button)
{
  std::cerr << "Entering sxm_examine::sxm_ray_list(" << frme << ", " << button << ")" << std::endl;
  
  // Assemble a list of ts_ray_info structs representing the requisite
  // number of rays to be displayed

  WW_PTR wwptr = solo_return_wwptr(frme);
  struct ts_ray_table *tsrt = wwptr->lead_sweep->tsrt;
  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);
  struct examine_control *ecs = wwptr->examine_control;
  struct solo_examine_info *sei = wwptr->examine_info;
  bool time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES;

  // Make sure there is space the list the requisite number of rays

  if (tsrt->max_sxm_entries < 2 *sei->ray_count)
  {
    if (tsrt->tsri_sxm) free(tsrt->tsri_sxm);
    tsrt->max_sxm_entries = 2 * sei->ray_count;
    tsrt->tsri_sxm = (struct ts_ray_info *)
      malloc(tsrt->max_sxm_entries * sizeof(struct ts_ray_info));
    memset(tsrt->tsri_sxm, 0,
	   tsrt->max_sxm_entries * sizeof(struct ts_ray_info));
  }

  if (!time_series)
  {
    std::cerr << "    Not a time series" << std::endl;
    std::cerr << "    Starting ray_num = " << sei->ray_num << std::endl;
    
    if (button == EX_SCROLL_LEFT)
    {
      sei->ray_num--;
    }
    else if (button == EX_SCROLL_RIGHT)
    {
      sei->ray_num++;
    }

    std::cerr << "    ray_num after button check = " << sei->ray_num << std::endl;
    
    if (ecs->ctr_on_click)
    {
      std::cerr << "    Inside ctr_on_click condition" << std::endl;
      std::cerr << "    click_angle = " << ecs->click_angle << std::endl;
      std::cerr << "    getAngleIndex() = " << data_info->getAngleIndex(ecs->click_angle) << std::endl;
      std::cerr << "    ray_count = " << sei->ray_count << std::endl;
      
      sei->ray_num = data_info->getAngleIndex(ecs->click_angle)
	- sei->ray_count/2;
      sei->at_cell = data_info->getGateNum(ecs->click_range) -
	sei->cell_count/2;
      ecs->ctr_on_click = NO;

      std::cerr << "    Ending ray_num = " << sei->ray_num << std::endl;
      std::cerr << "           at_cell = " << sei->at_cell << std::endl;
      
    }

    int rays_in_sweep = data_info->getNumRaysRAT();

    ecs->actual_ray_count = sei->ray_count > rays_in_sweep ?
      rays_in_sweep : sei->ray_count;
    
    if (sei->ray_num + sei->ray_count >= rays_in_sweep)
      sei->ray_num = rays_in_sweep - ecs->actual_ray_count;

    if (sei->ray_num < 0)
      sei->ray_num = 0;

    ecs->actual_ray_num = sei->ray_num;
    struct ts_ray_info *tsri = tsrt->tsri_sxm;

    // The sweep info was placed in tsrt->tssi_top by solo_nab_next_file()

    for (int ii = 0; ii < ecs->actual_ray_count; ii++, tsri++)
    {
      tsri->ray_num = ecs->actual_ray_num + ii;
      tsri->sweep_num = 1;
    }

    return ecs->actual_ray_count;
  }

  // Time series data!

  struct ts_ray_info *tsri_right = tsrt->tsri_top + wwptr->view->width_in_pix;

  if (ecs->ctr_on_click)
  {
    // Assemble a list of rays based on the clicked x pixel

    int nn = sp_ts_ray_list(frme, (int)wwptr->clicked_x_pxl, (int)3);
    struct ts_ray_info *tsrix =
      *(tsrt->tsri_list + (nn - 1) / 2); /* center ray info struct */

    // See how many rays we can actually do

    int mm = (sei->ray_count - 1) / 2; /* in case ray count is an even number */
    nn = sei->ray_count / 2;

    // Put the current center in the center of the new list for now

    sp_copy_tsri(tsrix, tsrt->tsri_sxm + sei->ray_count - 1); 
    tsrix = tsrt->tsri_sxm + sei->ray_count - 1;

    // See if we can find the number of rays we need to the left

    int sweep_num = tsrix->sweep_num;
    int ray_num = tsrix->ray_num -1;
    struct ts_sweep_info *tssi = tsrt->tssi_top +sweep_num;
    struct ts_ray_info *tsri = tsrix -1;

    int nurl;
    
    for (nurl = 0; nurl < sei->ray_count - 1; )
    {
      if (ray_num < 0)
      {
	if (--sweep_num < 1)
	  break;

	tssi--;
	ray_num = tssi->ray_count-1;
      }

      nurl++;
      tsri->sweep_num = sweep_num;
      tsri->ray_num = ray_num--;
      tsri--;
    }

    // Now look to the right

    sweep_num = tsrix->sweep_num;
    ray_num = tsrix->ray_num +1;
    tssi = tsrt->tssi_top +sweep_num;
    tsri = tsrix +1;

    int nurr;
    
    for (nurr = 0; nurr < sei->ray_count - 1; )
    {
      if (ray_num >= tssi->ray_count)
      {
	if (++sweep_num > tsrt->sweep_count)
	  break;

	tssi++;
	ray_num = 0;
      }

      nurr++;
      tsri->sweep_num = sweep_num;
      tsri->ray_num = ray_num++;
      tsri++;
    }

    ecs->actual_ray_count = sei->ray_count;
    ray_num = sei->ray_count-1 -mm;

    // Move them over to the start of the array

    if ((nurl + 1 + nurr) < sei->ray_count)
    {
      ray_num = sei->ray_count - 1 - nurl;
      ecs->actual_ray_count = nurl + 1 + nurr;
    }
    else if (nurl < mm)
    {
      ray_num = sei->ray_count - 1 - nurl;
    }
    else if (nurr < nn)
    {
      ray_num -= nn - nurr;
    }
	
    tsri = tsrt->tsri_sxm;
    for (int ii = 0; ii++ < ecs->actual_ray_count; tsri++)
    {
      sp_copy_tsri(tsri + ray_num, tsri);
    }

    sei->ray_num = tsrt->tsri_sxm->ray_num;
    sei->sweep_num = tsrt->tsri_sxm->sweep_num;
    float rng = KM_TO_M(wwptr->field_vals->rng);

    sei->at_cell = data_info->getGateNum(rng) - sei->cell_count/2;
    ecs->ctr_on_click = NO;

    return ecs->actual_ray_count;
  }

  // If this is just a refresh or a first time display, make sure the
  // starting sweep and ray numbers make sense.  i.e. see if we can assemble
  // a list based on the starting ray number and sweep number

  struct ts_ray_info *tsri;
  
  if (sei->sweep_num < 1)
  {
    int incre;
    
    if (wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT)
    {
      tsri = tsrt->tsri_top + wwptr->view->width_in_pix - 1;
      incre = -1;
    }
    else
    {
      tsri = tsrt->tsri_top;
      incre = 1;
    }

    // Find the first ray plotted

    int mm = wwptr->view->width_in_pix;
    for (; mm-- ; tsri += incre)
    {
      if (tsri->ray_num >= 0 && tsri->sweep_num > 0)
	break;
    }

    sei->sweep_num = tsri->sweep_num;
    sei->ray_num = tsri->ray_num;
  }

  if (sei->sweep_num > tsrt->sweep_count)
  {
    // Give it the last ray number of the last sweep

    sei->sweep_num = tsrt->sweep_count;
    struct ts_sweep_info *tssi = tsrt->tssi_top + sei->sweep_num;
    sei->ray_num = tssi->ray_count - 1;
  }

  int blip = 0;
  
  if (button == EX_SCROLL_LEFT)
    blip = -1;
  else if (button == EX_SCROLL_RIGHT)
    blip = 1;
  else
    blip = 0;

  if (wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT)
    blip *= -1;

  sei->ray_num += blip;

  // Get to a legitimate starting sweep and ray number

  struct ts_sweep_info *tssi = tsrt->tssi_top +sei->sweep_num;

  if (sei->ray_num >= tssi->ray_count)
  {
    if (sei->sweep_num < tsrt->sweep_count)
    {
      sei->sweep_num++;
      sei->ray_num = 0;
      tssi = tsrt->tssi_top +sei->sweep_num;
    }
    else
    {
      sei->ray_num = tssi->ray_count -1;
    }
  }
  else if (sei->ray_num < 0)
  {
    if (sei->sweep_num > 1)
    {
      sei->sweep_num--;
      tssi = tsrt->tssi_top + sei->sweep_num;
      sei->ray_num = tssi->ray_count - 1;
    }
    else
    {
      sei->ray_num = 0;
    }
  }

  int incre;
  
  if (wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT)
  {
    tsri = tsrt->tsri_sxm +sei->ray_count -1;
    incre = -1;
  }
  else
  {
    tsri = tsrt->tsri_sxm;
    incre = 1;
  }

  int sweep_num = sei->sweep_num;
  int ray_num = sei->ray_num;
  tssi = tsrt->tssi_top + sweep_num;

  int mm;
  
  for (mm = 0; mm < sei->ray_count && sweep_num <= tsrt->sweep_count;)
  {
    if (ray_num >= tssi->ray_count)
    {
      sweep_num++;
      tssi++;
      ray_num = 0;
      continue;
    }

    mm++;
    tsri->sweep_num = sweep_num;
    tsri->ray_num = ray_num++;
    tsri += incre;
  }

  ecs->actual_ray_count = mm;

  return ecs->actual_ray_count;
}

#endif

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX

void sxm_update_examine_data (int frme, int button)
{
  static const string method_name = "sxm_update_examine_data()";
  
  // Get pointers to needed information

  WW_PTR wwptr = solo_return_wwptr(frme);
  WW_PTR wwptrx = wwptr->lead_sweep;

  // Check for outstanding changes

  if (wwptr->examine_info->change_count > 0)
  {
    char str[80];
    sprintf(str, "You have %d changes outstanding!\n",
	    wwptr->examine_info->change_count);

    std::string message = str;
    message += "You cannot change the display\n";
    message += "until you either commit or clear them\n";
    sii_message(message);

    return;
  }

  struct examine_control *ecs = wwptr->examine_control;
  int mm = sxm_ray_list(frme, button);

  // See if we are displaying a time series.

  bool time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES;
  double d_start = 0.0;
  
  if (time_series)
    d_start = wwptrx->sweep->start_time;

  // Change the cursor to let the user know we are working

  main_window->changeCursor(false);

  // Generate a dummy label so we can calculate how many characters are in
  // the range label.
  // NOTE: It seems like since we are hard-coding the rng_fmt here, then we
  // already know the number of characters.

  char sample_line[1024];
  
  ecs->rng_fmt = "%7.2f_|";
  sprintf(sample_line, ecs->rng_fmt.c_str(), 1.0);
  ecs->num_chars_rng_label = strlen(sample_line);
  ecs->col_lims[0] = strlen(sample_line);

  // Then the size of a data column

  char buffer[1024];
  sprintf(buffer, "%%%s|", wwptr->examine_info->display_format);
  ecs->actual_data_format = buffer;
  
  sprintf(sample_line, ecs->actual_data_format.c_str(), 1.0);
  ecs->num_chars_per_cell = strlen(sample_line);

  // If the sweep file's been edited, reopen the file

  if (wwptr->lead_sweep->sweep_file_modified)
  {
    DataManager::getInstance()->nabNextFile(wwptr->lead_sweep->window_num,
					    DataManager::TIME_NEAREST,
					    DataManager::LATEST_VERSION,
					    YES);
    wwptr->lead_sweep->sweep_file_modified = NO;
  }

  // Nab the first ray
  // NOTE: I'm not sure what we're doing here.  These are time series
  // structures so shouldn't they be inside of a time series condition?

  struct ts_ray_table *ray_table = wwptr->lead_sweep->tsrt;
  struct ts_ray_info *ray_info = ray_table->tsri_sxm;
  int incre = 1;
  struct ts_sweep_info *sweep_info = ray_table->tssi_top + ray_info->sweep_num;

  // NOTE: I don't think we need to reread the file here.  We should just be
  // able to use the data that's already read in.

//  char sweep_dir[2048];
//  if (sweep_info->directory[strlen(sweep_info->directory)-1] == '/')
//    sprintf(sweep_dir, "%s%s", sweep_info->directory, sweep_info->filename);
//  else
//    sprintf(sweep_dir, "%s/%s", sweep_info->directory, sweep_info->filename);
//
//  if (wwptrx->file_id > 0)
//    close(wwptrx->file_id);
//
//  if ((wwptrx->file_id = open(sweep_dir, O_RDONLY, 0)) < 0)
//  {
//    g_string_sprintfa (gs_complaints, "Unable to open sweep %s\n", sweep_dir);
//    return;
//  }

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);

  // NOTE: Don't know why we reset the directory.  Would it get changed
  // somewhere before this?

//  data_info->setDir(sweep_info->directory);

  sxm_log_radx_stat_line(*data_info, ray_info->ray_num);

  // Create the list of fields to display.  First, see if we can use
  // sei->fields_list.  If that string is empty or doesn't contain any valid
  // field names, then we'll use our default field list instead.  (Note that
  // if the field list is empty then num_params will be 0 and we won't go into
  // the for loop, so we don't have to check for this case explicitly.

  struct solo_examine_info *sei = wwptr->examine_info;

  // Compile the list of fields to examine

  ecs->fields_list = sei->fields_list;
  tokenize(ecs->fields_list, ecs->fld_ptrs);
  int num_params = ecs->fld_ptrs.size();
  
  ecs->actual_num_fields = 0;
  for (int i = 0; i < num_params; ++i)
  {
    int param_num = data_info->getParamIndex(ecs->fld_ptrs[i]);
    if (param_num >= 0)
      ecs->actual_field_num[ecs->actual_num_fields++] = param_num;
  }
  
  // If we don't have the fields the user asked for, uuse the default
  // ones

  if (ecs->actual_num_fields == 0)
  {
    char default_fields[32];
    default_fields[0] = '\0';
    int num_params = 0;
    
    for (std::size_t param_num = 0;
	 param_num < 2 && param_num < data_info->getNumParams(); param_num++)
    {
      std::string param_name = data_info->getParamName(param_num).substr(0, 8);
      strcat(default_fields, param_name.c_str());
      strcat(default_fields, " ");
      num_params++;
    }

    strcpy(sei->fields_list, default_fields);
    ecs->fields_list = default_fields;
    
    for (int i = 0; i < num_params; ++i)
      ecs->actual_field_num[i] = i;
    ecs->actual_num_fields = num_params;
  }

  ecs->num_data_cols = ecs->actual_ray_count * ecs->actual_num_fields;
  if (ecs->num_data_cols >= MAX_EXM_COLUMNS)
    ecs->num_data_cols = MAX_EXM_COLUMNS-1;
  ecs->num_cols = ecs->num_data_cols + 1;

  // Now that we know how many fields we will be displaying,  get the number
  // of fields per ray

  int max_char_per_line = 256;
  int remaining_chars = max_char_per_line - ecs->num_chars_rng_label;

  int max_rays =
    remaining_chars / (ecs->actual_num_fields * ecs->num_chars_per_cell);
  
  if (max_rays < sei->ray_count)
    sei->ray_count = max_rays;

  // Now we can start accumulating the list.  Set some initial values and
  // get a pointer to the list.

  ecs->actual_at_cell = 0;
  ecs->actual_num_cells = data_info->getMaxNGates();
    
  wwptr->examine_list.clear();

  // Pointers to the beginning of each data column

  ecs->col_ptrs[0] = ecs->data_line + ecs->num_chars_rng_label;

  // Generate the array of range values

  if (ecs->actual_num_cells > ecs->max_rngs)
  {
    delete [] ecs->ranges;
    ecs->max_rngs = ecs->actual_num_cells;
    ecs->ranges = new float[ecs->max_rngs];
    memset(ecs->ranges, 0, ecs->max_rngs * sizeof(float));
  }

  for (int ii = ecs->actual_at_cell;
       ii < ecs->actual_at_cell + ecs->actual_num_cells; ii++)
  {
    ecs->ranges[ii] = data_info->getGateRangeKm(ii);
  }

  // Set up the delete pattern for the display

  ecs->del_str = "";
  for (int i = 0; i < ecs->num_chars_per_cell - 1; ++i)
    ecs->del_str += "-";
  ecs->del_str += "|";
  
  // Calculate column limits and pointers

  for (int ii = 1; ii < ecs->num_cols; ii++)
  {
    ecs->col_lims[ii] = ecs->col_lims[ii-1] + ecs->num_chars_per_cell;
    ecs->col_ptrs[ii] = ecs->col_ptrs[ii-1] + ecs->num_chars_per_cell;
  }
  ecs->non_data_col_count = 1; // First col contains range

  // Set up the field names line.  We start with the row annotation which must
  // be the same size as the range label on each of the data lines.

  std::string names_header;
  
  int chars_in_label = ecs->num_chars_rng_label - 2;
  std::string row_annot = " ";
  if (sei->row_annotation == 0)
    row_annot = "KM.";
  int row_annot_size = row_annot.size();
  int padding_chars = chars_in_label - row_annot_size;
  for (int ii = 0; ii < padding_chars; ii++)
    names_header += " ";
  names_header += row_annot + " |";
  int chars_per_cell = ecs->num_chars_per_cell - 1;
  
  std::string param_names_string;
  
  for (int ii = 0; ii < ecs->actual_num_fields; ii++)
  {
    // Get the paramter name

    std::string param_name =
      data_info->getParamName(ecs->actual_field_num[ii]).substr(0, 8);

    // Make sure the parameter name will fit in the cell.  Pad it with
    // blanks if it is smaller than the cell.

    int param_name_chars = param_name.size();
    if (param_name_chars > chars_per_cell)
      param_name_chars = chars_per_cell;
    int num_blanks = chars_per_cell - param_name_chars;
    int num_blanks_end = num_blanks / 2;
    int num_blanks_start = num_blanks_end;
    if (num_blanks % 2 != 0)
      num_blanks_start++;
    
    param_name = param_name.substr(0, param_name_chars);
    param_name.insert(0, num_blanks_start, ' ');
    param_name.insert(param_name.size(), num_blanks_end, ' ');
    
    // Add the parameter name to the full string

    param_names_string += param_name + "|";
  }
  
  for (int ii = 0; ii < ecs->actual_ray_count; ii++)
    names_header += param_names_string;

  // Check arrays that hold data

  int ncells =
    ecs->actual_ray_count * ecs->actual_num_fields * ecs->actual_num_cells;
  
  if (ecs->max_cells < ncells)
  {
    delete [] ecs->data_space;
    ecs->data_space = new float[ncells];
    memset(ecs->data_space, 0, ncells * sizeof(float));
    ecs->max_cells = ncells;
  }

  float *fptr = ecs->data_space;
  for (int ii = 0; ii < ecs->num_data_cols; ii++, fptr += ecs->actual_num_cells)
  {
    ecs->data_ptrs[ii] = fptr;
  }

  // We now know what ray to start at and how many rays to display,
  // what cell to start at and how many cells to display,
  // the number fields and which fields to display.
  // Set up the rotation angle header.

  std::string rotang_header;
  
  rotang_header.insert(0, ecs->num_chars_rng_label - 1, ' ');
  rotang_header += "|";
  
  // Get access to the ray data

  const RadxVol &radx_vol = data_info->getRadxVolume();
  const RadxSweep *sweep = data_info->getRadxSweep();
  if (sweep == 0)
  {
    std::cerr << "ERROR: " << method_name << std::endl;
    std::cerr << "No current sweep -- cannot update examine info" << std::endl;
    return;
  }
  
  std::vector< RadxRay* > rays = radx_vol.getRays();

  std::size_t ray_count = 0;
  
  for (std::size_t ray_index = sweep->getStartRayIndex();
       ray_index <= sweep->getEndRayIndex(); ++ray_index, ++ray_count)
  {
    const RadxRay *ray = rays[ray_index];
    
    // Rotation angles...first blank out the space

    int chars_in_col = ecs->actual_num_fields * ecs->num_chars_per_cell - 1;
    char tmp_val[32];

    if (sei->col_annotation)
    {
      sprintf(tmp_val, "%d", ecs->actual_ray_num + ray_count + 1);
    }
    else if (time_series)
    {
      double et = fmod(data_info->getTime() - d_start, (double)1000.0);
      sprintf(tmp_val, "%.2f", et);
      if ((mm = strlen(tmp_val)) > chars_in_col)
      {
	int jj = mm - chars_in_col;
	char *ee = tmp_val;
	for (mm = chars_in_col + 1; mm--; ee++)
	{
	  // don't forget the null

	  *ee = *(ee +jj);
	}
      }
    }
    else
    {
      double angle;

      angle = data_info->getRadxSweep()->getFixedAngleDeg();
      sprintf(tmp_val, "%.2f", angle);
    }

    int display_chars = strlen(tmp_val);
    if (display_chars > chars_in_col)
      display_chars = chars_in_col;
    
    int num_blanks = chars_in_col - strlen(tmp_val);
    int num_blanks_end = num_blanks / 2;
    int num_blanks_start = num_blanks_end;
    if (num_blanks % 2 != 0)
      num_blanks_start++;
    
    rotang_header.insert(rotang_header.size(), num_blanks_start, ' ');
    rotang_header += std::string(tmp_val).substr(0, display_chars);
    rotang_header.insert(rotang_header.size(), num_blanks_end, ' ');
    rotang_header += "|";
    
    // Nab the data for each ray and keep it around

    int ndx = ray_count * ecs->actual_num_fields;
    
    for (int ii = 0; ii < ecs->actual_num_fields; ii++)
    {
      const RadxField *field = ray->getField(ecs->actual_field_num[ii]);
      if (field == 0)
      {
	std::cerr << "ERROR: " << method_name << std::endl;
	std::cerr << "Error getting pointer to field number: " <<
	  ecs->actual_field_num[ii] << std::endl;
	
	return;
      }
      
      const Radx::fl32 *data = field->getDataFl32();
      if (data == 0)
      {
	std::cerr << "ERROR: " << method_name << std::endl;
	std::cerr << "Error getting data for field: " << field->getName() << std::endl;
	
	return;
      }
      
      memcpy(ecs->data_ptrs[ndx + ii], &data[ecs->actual_at_cell + 1],
	     ecs->actual_num_cells * sizeof(Radx::fl32));
      
      ecs->bad_data[ndx+ii] = field->getMissingFl32();
    }

    ecs->ac_vel[ray_count] = data_info->getAircraftVelocity();

    if (ray_count >= ecs->actual_ray_count - 1 ||
	ray_count >= (ecs->num_data_cols / ecs->actual_num_fields) - 1 ||
	ray_count >= MAX_EXM_COLUMNS / ecs->actual_num_fields)
      break;

    ray_info += incre;
//    if (ray_info->sweep_num != (ray_info - incre)->sweep_num)
//    {
//      // Need to open a new file
//
//      struct ts_sweep_info *sweep_info =
//	ray_table->tssi_top + ray_info->sweep_num;
//
//      char sweep_file_path[2048];
//
//      if (sweep_info->directory[strlen(sweep_info->directory)-1] == '/')
//	sprintf(sweep_file_path, "%s%s",
//		sweep_info->directory, sweep_info->filename);
//      else
//	sprintf(sweep_file_path, "%s/%s",
//		sweep_info->directory, sweep_info->filename);
//
//      if (wwptrx->file_id > 0)
//	close(wwptrx->file_id);
//
//      if ((wwptrx->file_id = open(sweep_file_path, O_RDONLY, 0)) < 0)
//      {
//	g_string_sprintfa(gs_complaints, "Unable to open sweep %s\n",
//			  sweep_file_path);
//	return;
//      }
//
//      data_info->setDir(sweep_info->directory);
//      data_info->setInputFileId(wwptrx->file_id);
//      data_info->rewindBuffer();
//      data_info->loadHeaders();
//      data_info->loadRay(ray_info->ray_num);
//    }
//    else
//    {
//      data_info->loadNextRay();
//    }
  } /* endfor - ray_index */
  
  wwptr->examine_list.clear();
  
  wwptr->examine_list.push_back(rotang_header);
  sxm_append_to_log_stream(rotang_header.c_str(),
			   rotang_header.size());

  // Second row: field names

  wwptr->examine_list.push_back(names_header);
  sxm_append_to_log_stream(names_header.c_str(), names_header.size());
  
  ecs->heading_row_count = 2;

  for (int cell_num = 0; cell_num < ecs->actual_num_cells; cell_num++)
  {
    // First do the range value

    char tmp_val[32];

    if (sei->row_annotation)
    {
      int jj;
      char *cc;
      char *aa;
      
      for (aa = cc = ecs->data_line, jj = 0;
	   jj++ < ecs->num_chars_rng_label - 2;
	   *cc++ = ' ');
      *cc++ = '_';
      *cc++ = '|';
      *cc++ = '\0';
      sprintf(tmp_val, "%4d", cell_num + ecs->actual_at_cell + 1);
      strncpy(ecs->data_line + 1, tmp_val, strlen(tmp_val));
    }
    else
    {
      sprintf(tmp_val, ecs->rng_fmt.c_str(), *(ecs->ranges + cell_num));
      if ((int)strlen(tmp_val) > ecs->num_chars_rng_label)
	strcpy(tmp_val + ecs->num_chars_rng_label -1, "|");
      strcpy(ecs->data_line, tmp_val);
    }

    // Now do the data for this line

    for (int col = 0; col < ecs->num_data_cols; col++)
    {
      float val = *(ecs->data_ptrs[col] + cell_num);
      if (val != ecs->bad_data[col])
      {
	sprintf(tmp_val, ecs->actual_data_format.c_str(), val);
	if ((int)strlen(tmp_val) > ecs->num_chars_per_cell)
	{
	  tmp_val[ecs->num_chars_per_cell-1] = '|';
	  tmp_val[ecs->num_chars_per_cell] = '\0';
	}
	strcpy(ecs->col_ptrs[col], tmp_val);
      }
      else
      {
	strcpy(ecs->col_ptrs[col], ecs->del_str.c_str());
      }
    }

    // Send this line back to the list management tool

    wwptr->examine_list.push_back(ecs->data_line);
    sxm_append_to_log_stream(ecs->data_line,strlen(ecs->data_line));
  }

  se_refresh_examine_widgets(frme, wwptr->examine_list);
  main_window->changeCursor(true);

  return;
}

#else

void sxm_update_examine_data (int frme, int button)
{
  // Get pointers to needed information

  WW_PTR wwptr = solo_return_wwptr(frme);
  WW_PTR wwptrx = wwptr->lead_sweep;

  // Check for outstanding changes

  if (wwptr->examine_info->change_count > 0)
  {
    char str[80];
    sprintf(str, "You have %d changes outstanding!\n",
	    wwptr->examine_info->change_count);

    std::string message = str;
    message += "You cannot change the display\n";
    message += "until you either commit or clear them\n";
    sii_message(message);

    return;
  }

  struct examine_control *ecs = wwptr->examine_control;
  int mm = sxm_ray_list(frme, button);

  // See if we are displaying a time series.

  bool time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES;
  double d_start = 0.0;
  
  if (time_series)
    d_start = wwptrx->sweep->start_time;

  // Change the cursor to let the user know we are working

  main_window->changeCursor(false);

  // Generate a dummy label so we can calculate how many characters are in
  // the range label.
  // NOTE: It seems like since we are hard-coding the rng_fmt here, then we
  // already know the number of characters.

  char sample_line[1024];
  
  ecs->rng_fmt = "%7.2f_|";
  sprintf(sample_line, ecs->rng_fmt.c_str(), 1.0);
  ecs->num_chars_rng_label = strlen(sample_line);
  ecs->col_lims[0] = strlen(sample_line);

  // Then the size of a data column

  char buffer[1024];
  sprintf(buffer, "%%%s|", wwptr->examine_info->display_format);
  ecs->actual_data_format = buffer;
  
  sprintf(sample_line, ecs->actual_data_format.c_str(), 1.0);
  ecs->num_chars_per_cell = strlen(sample_line);

  // If the sweep file's been edited, reopen the file

  if (wwptr->lead_sweep->sweep_file_modified)
  {
    DataManager::getInstance()->nabNextFile(wwptr->lead_sweep->window_num,
					    DataManager::TIME_NEAREST,
					    DataManager::LATEST_VERSION,
					    YES);
    wwptr->lead_sweep->sweep_file_modified = NO;
  }

  // Nab the first ray
  // NOTE: I'm not sure what we're doing here.  These are time series
  // structures so shouldn't they be inside of a time series condition?

  struct ts_ray_table *ray_table = wwptr->lead_sweep->tsrt;
  struct ts_ray_info *ray_info = ray_table->tsri_sxm;
  int incre = 1;
  struct ts_sweep_info *sweep_info = ray_table->tssi_top + ray_info->sweep_num;

  char str[2048];
  if (sweep_info->directory[strlen(sweep_info->directory)-1] == '/')
    sprintf(str, "%s%s", sweep_info->directory, sweep_info->filename);
  else
    sprintf(str, "%s/%s", sweep_info->directory, sweep_info->filename);

  if (wwptrx->file_id > 0)
    close(wwptrx->file_id);

  if ((wwptrx->file_id = open(str, O_RDONLY, 0)) < 0)
  {
    g_string_sprintfa (gs_complaints, "Unable to open sweep %s\n", str);
    return;
  }

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);

  data_info->setDir(sweep_info->directory);
  data_info->setInputFileId(wwptrx->file_id);

  data_info->rewindBuffer();
  data_info->loadHeaders();
  data_info->loadRay(ray_info->ray_num);

  sxm_log_stat_line(*data_info);

  // Create the list of fields to display.  First, see if we can use
  // sei->fields_list.  If that string is empty or doesn't contain any valid
  // field names, then we'll use our default field list instead.  (Note that
  // if the field list is empty then num_params will be 0 and we won't go into
  // the for loop, so we don't have to check for this case explicitly.

  struct solo_examine_info *sei = wwptr->examine_info;

  ecs->fields_list = sei->fields_list;
  tokenize(ecs->fields_list, ecs->fld_ptrs);
  int num_params = ecs->fld_ptrs.size();
  
  ecs->actual_num_fields = 0;
  for (int i = 0; i < num_params; ++i)
  {
    int param_num = data_info->getParamIndex(ecs->fld_ptrs[i]);
    if (param_num >= 0)
      ecs->actual_field_num[ecs->actual_num_fields++] = param_num;
  }
  
  if (ecs->actual_num_fields == 0)
  {
    char default_fields[32];
    default_fields[0] = '\0';
    int num_params = 0;
    
    for (std::size_t param_num = 0;
	 param_num < 2 && param_num < data_info->getNumParams(); param_num++)
    {
      std::string param_name = data_info->getParamNameClean(param_num, 8);
      strcat(default_fields, param_name.c_str());
      strcat(default_fields, " ");
      num_params++;
    }

    strcpy(sei->fields_list, default_fields);
    ecs->fields_list = default_fields;
    
    for (int i = 0; i < num_params; ++i)
      ecs->actual_field_num[i] = i;
    ecs->actual_num_fields = num_params;
  }

  ecs->num_data_cols = ecs->actual_ray_count * ecs->actual_num_fields;
  if (ecs->num_data_cols >= MAX_EXM_COLUMNS)
    ecs->num_data_cols = MAX_EXM_COLUMNS-1;
  ecs->num_cols = ecs->num_data_cols + 1;

  // Now that we know how many fields we will be displaying,  get the number
  // of fields per ray

  int max_char_per_line = 256;
  int remaining_chars = max_char_per_line - ecs->num_chars_rng_label;

  int max_rays =
    remaining_chars / (ecs->actual_num_fields * ecs->num_chars_per_cell);
  
  if (max_rays < sei->ray_count)
    sei->ray_count = max_rays;

  // Now we can start accumulating the list.  Set some initial values and
  // get a pointer to the list.

  ecs->actual_at_cell = 0;
  ecs->actual_num_cells = data_info->getNumCellsC();
    
  wwptr->examine_list.clear();

  // Pointers to the beginning of each data column

  ecs->col_ptrs[0] = ecs->data_line + ecs->num_chars_rng_label;

  // Generate the array of range values

  if (ecs->actual_num_cells > ecs->max_rngs)
  {
    delete [] ecs->ranges;
    ecs->max_rngs = ecs->actual_num_cells;
    ecs->ranges = new float[ecs->max_rngs];
    memset(ecs->ranges, 0, ecs->max_rngs * sizeof(float));
  }

  for (int ii = ecs->actual_at_cell;
       ii < ecs->actual_at_cell + ecs->actual_num_cells; ii++)
  {
    ecs->ranges[ii] = M_TO_KM(data_info->getGateRangeC(ii));
  }

  // Set up the delete pattern for the display

  ecs->del_str = "";
  for (int i = 0; i < ecs->num_chars_per_cell - 1; ++i)
    ecs->del_str += "-";
  ecs->del_str += "|";
  
  // Calculate column limits and pointers

  for (int ii = 1; ii < ecs->num_cols; ii++)
  {
    ecs->col_lims[ii] = ecs->col_lims[ii-1] + ecs->num_chars_per_cell;
    ecs->col_ptrs[ii] = ecs->col_ptrs[ii-1] + ecs->num_chars_per_cell;
  }
  ecs->non_data_col_count = 1; // First col contains range

  // Set up the field names line.  We start with the row annotation which must
  // be the same size as the range label on each of the data lines.

  std::string names_header;
  
  int chars_in_label = ecs->num_chars_rng_label - 2;
  std::string row_annot = " ";
  if (sei->row_annotation == 0)
    row_annot = "KM.";
  int row_annot_size = row_annot.size();
  int padding_chars = chars_in_label - row_annot_size;
  for (int ii = 0; ii < padding_chars; ii++)
    names_header += " ";
  names_header += row_annot + " |";
  int chars_per_cell = ecs->num_chars_per_cell - 1;
  
  std::string param_names_string;
  
  for (int ii = 0; ii < ecs->actual_num_fields; ii++)
  {
    // Get the paramter name

    std::string param_name =
      data_info->getParamNameClean(ecs->actual_field_num[ii], 8);

    // Make sure the parameter name will fit in the cell.  Pad it with
    // blanks if it is smaller than the cell.

    int param_name_chars = param_name.size();
    if (param_name_chars > chars_per_cell)
      param_name_chars = chars_per_cell;
    int num_blanks = chars_per_cell - param_name_chars;
    int num_blanks_end = num_blanks / 2;
    int num_blanks_start = num_blanks_end;
    if (num_blanks % 2 != 0)
      num_blanks_start++;
    
    param_name = param_name.substr(0, param_name_chars);
    param_name.insert(0, num_blanks_start, ' ');
    param_name.insert(param_name.size(), num_blanks_end, ' ');
    
    // Add the parameter name to the full string

    param_names_string += param_name + "|";
  }
  
  for (int ii = 0; ii < ecs->actual_ray_count; ii++)
    names_header += param_names_string;

  // Check arrays that hold data

  int ncells =
    ecs->actual_ray_count * ecs->actual_num_fields * ecs->actual_num_cells;
  
  if (ecs->max_cells < ncells)
  {
    delete [] ecs->data_space;
    ecs->data_space = new float[ncells];
    memset(ecs->data_space, 0, ncells * sizeof(float));
    ecs->max_cells = ncells;
  }

  float *fptr = ecs->data_space;
  for (int ii = 0; ii < ecs->num_data_cols; ii++, fptr += ecs->actual_num_cells)
  {
    ecs->data_ptrs[ii] = fptr;
  }

  // We now know what ray to start at and how many rays to display,
  // what cell to start at and how many cells to display,
  // the number fields and which fields to display.
  // Set up the rotation angle header.

  std::string rotang_header;
  
  rotang_header.insert(0, ecs->num_chars_rng_label - 1, ' ');
  rotang_header += "|";
  
  for (int ray_count = 0; ; ray_count++)
  {
    // Rotation angles...first blank out the space

    int chars_in_col = ecs->actual_num_fields * ecs->num_chars_per_cell - 1;
    char tmp_val[32];

    if (sei->col_annotation)
    {
      sprintf(tmp_val, "%d", ecs->actual_ray_num + ray_count + 1);
    }
    else if (time_series)
    {
      double et = fmod(data_info->getTime() - d_start, (double)1000.0);
      sprintf(tmp_val, "%.2f", et);
      if ((mm = strlen(tmp_val)) > chars_in_col)
      {
	int jj = mm - chars_in_col;
	char *ee = tmp_val;
	for (mm = chars_in_col + 1; mm--; ee++)
	{
	  // don't forget the null

	  *ee = *(ee +jj);
	}
      }
    }
    else
    {
      double angle;

      if (data_info->getScanMode() == DD_SCAN_MODE_RHI)
	angle = data_info->getElevationAngleCalc();
      else
	angle = data_info->getRotationAngleCalc();
      sprintf(tmp_val, "%.2f", angle);
    }

    int display_chars = strlen(tmp_val);
    if (display_chars > chars_in_col)
      display_chars = chars_in_col;
    
    int num_blanks = chars_in_col - strlen(tmp_val);
    int num_blanks_end = num_blanks / 2;
    int num_blanks_start = num_blanks_end;
    if (num_blanks % 2 != 0)
      num_blanks_start++;
    
    rotang_header.insert(rotang_header.size(), num_blanks_start, ' ');
    rotang_header += std::string(tmp_val).substr(0, display_chars);
    rotang_header.insert(rotang_header.size(), num_blanks_end, ' ');
    rotang_header += "|";
    
    // Nab the data for each ray and keep it around

    int ndx = ray_count * ecs->actual_num_fields;
    
    for (int ii = 0; ii < ecs->actual_num_fields; ii++)
    {
      float bad_val;
      
      data_info->getParamDataUnscaled(ecs->actual_field_num[ii],
				      ecs->actual_at_cell + 1,
				      ecs->actual_num_cells,
				      ecs->data_ptrs[ndx+ii], bad_val);
      ecs->bad_data[ndx+ii] = bad_val;
    }

    ecs->ac_vel[ray_count] = data_info->getAircraftVelocity();

    if (ray_count >= ecs->actual_ray_count - 1 ||
	ray_count >= (ecs->num_data_cols / ecs->actual_num_fields) - 1 ||
	ray_count >= MAX_EXM_COLUMNS / ecs->actual_num_fields)
      break;

    ray_info += incre;
    if (ray_info->sweep_num != (ray_info - incre)->sweep_num)
    {
      // Need to open a new file

      struct ts_sweep_info *sweep_info =
	ray_table->tssi_top + ray_info->sweep_num;

      char sweep_file_path[2048];

      if (sweep_info->directory[strlen(sweep_info->directory)-1] == '/')
	sprintf(sweep_file_path, "%s%s",
		sweep_info->directory, sweep_info->filename);
      else
	sprintf(sweep_file_path, "%s/%s",
		sweep_info->directory, sweep_info->filename);

      if (wwptrx->file_id > 0)
	close(wwptrx->file_id);

      if ((wwptrx->file_id = open(sweep_file_path, O_RDONLY, 0)) < 0)
      {
	g_string_sprintfa(gs_complaints, "Unable to open sweep %s\n",
			  sweep_file_path);
	return;
      }

      data_info->setDir(sweep_info->directory);
      data_info->setInputFileId(wwptrx->file_id);
      data_info->rewindBuffer();
      data_info->loadHeaders();
      data_info->loadRay(ray_info->ray_num);
    }
    else
    {
      data_info->loadNextRay();
    }
  } /* endfor - ray_count */
  
  wwptr->examine_list.clear();
  
  wwptr->examine_list.push_back(rotang_header);
  sxm_append_to_log_stream(rotang_header.c_str(),
			   rotang_header.size());

  // Second row: field names

  wwptr->examine_list.push_back(names_header);
  sxm_append_to_log_stream(names_header.c_str(), names_header.size());
  
  ecs->heading_row_count = 2;

  for (int cell_num = 0; cell_num < ecs->actual_num_cells; cell_num++)
  {
    // First do the range value

    char tmp_val[32];

    if (sei->row_annotation)
    {
      int jj;
      char *cc;
      char *aa;
      
      for (aa = cc = ecs->data_line, jj = 0;
	   jj++ < ecs->num_chars_rng_label - 2;
	   *cc++ = ' ');
      *cc++ = '_';
      *cc++ = '|';
      *cc++ = '\0';
      sprintf(tmp_val, "%4d", cell_num + ecs->actual_at_cell + 1);
      strncpy(ecs->data_line + 1, tmp_val, strlen(tmp_val));
    }
    else
    {
      sprintf(tmp_val, ecs->rng_fmt.c_str(), *(ecs->ranges + cell_num));
      if ((int)strlen(tmp_val) > ecs->num_chars_rng_label)
	strcpy(tmp_val + ecs->num_chars_rng_label -1, "|");
      strcpy(ecs->data_line, tmp_val);
    }

    // Now do the data for this line

    for (int col = 0; col < ecs->num_data_cols; col++)
    {
      float val = *(ecs->data_ptrs[col] + cell_num);
      if (val != ecs->bad_data[col])
      {
	sprintf(tmp_val, ecs->actual_data_format.c_str(), val);
	if ((int)strlen(tmp_val) > ecs->num_chars_per_cell)
	{
	  tmp_val[ecs->num_chars_per_cell-1] = '|';
	  tmp_val[ecs->num_chars_per_cell] = '\0';
	}
	strcpy(ecs->col_ptrs[col], tmp_val);
      }
      else
      {
	strcpy(ecs->col_ptrs[col], ecs->del_str.c_str());
      }
    }

    // Send this line back to the list management tool

    wwptr->examine_list.push_back(ecs->data_line);
    sxm_append_to_log_stream(ecs->data_line,strlen(ecs->data_line));
  }

  se_refresh_examine_widgets(frme, wwptr->examine_list);
  main_window->changeCursor(true);

  return;
}

#endif
