/* 	$Id$	 */

#include <fcntl.h>
#include <fstream>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>

#include <DataInfo.hh>
#include <DataManager.hh>
#include <DateTime.hh>
#include <se_utils.hh>
#include <sed_shared_structs.h>
#include <seds.h>
#include <solo_editor_structs.h>
#include <solo_list_widget_ids.h>
#include <solo_window_structs.h>
#include "solo2.hh"
#include "sp_accepts.hh"
#include "sp_basics.hh"

# ifndef SOLO_HALT
# define SOLO_EXIT 0x1
# define SOLO_HALT 0x2
# endif

static int Solo_Busy_Signal=NO;
static struct solo_perusal_info *static_solo_perusal_info = 0;

/* c------------------------------------------------------------------------ */

void solo_clear_busy_signal (void)
{
    Solo_Busy_Signal = NO;
}

/* c------------------------------------------------------------------------ */

void solo_set_busy_signal (void)
{
    Solo_Busy_Signal = YES;
}

/* c------------------------------------------------------------------------ */

int solo_busy (void)
{
    int ii;
    ii = Solo_Busy_Signal ? YES : NO;
    return(ii);
}

/* c------------------------------------------------------------------------ */

std::string solo_return_radar_name(int frame_num)
{
  WW_PTR wwptr = solo_return_wwptr(frame_num);
  return wwptr->sweep->radar_name;
}

/* c------------------------------------------------------------------------ */

void solo_activate_frame (int dst_frme)
{
    /* current thinking is just to clone the last active window
     */
    int ww, src_frme;
    WW_PTR wwptrd, wwptrb, wwptrs;
    struct solo_perusal_info *spi;

    wwptrd = solo_return_wwptr(dst_frme);
    spi = solo_return_winfo_ptr();
    spi->active_windows[dst_frme] = YES;
    /*
     * find the last active window
     */
    for(src_frme=dst_frme-1; src_frme >= 0; src_frme--) {
	if((spi->active_windows[src_frme]))
	      break;
    }
    wwptrs = solo_return_wwptr(src_frme);
    /*
     * before copying, fix links to this and other windows
     */
    for(ww=0; ww <= src_frme; ww++) {
	wwptrb = solo_return_wwptr(ww);
	if(wwptrs->parameter.linked_windows[ww]) {
	    wwptrb->parameter.linked_windows[dst_frme] = YES;
	}
	if(wwptrs->view->linked_windows[ww]) {
	    wwptrb->view->linked_windows[dst_frme] = YES;
	}
	if(wwptrs->sweep->linked_windows[ww]) {
	    wwptrb->sweep->linked_windows[dst_frme] = YES;
	}
	if(wwptrs->lock->linked_windows[ww]) {
	    wwptrb->lock->linked_windows[dst_frme] = YES;
	}
	if(wwptrs->landmark_info->linked_windows[ww]) {
	    wwptrb->landmark_info->linked_windows[dst_frme] = YES;
	}
	if(wwptrs->frame_ctr_info->linked_windows[ww]) {
	    wwptrb->frame_ctr_info->linked_windows[dst_frme] = YES;
	}
    }
    /*
     * parameter info
     */
    solo_cpy_parameter_info(src_frme, dst_frme);
    /*
     * view info
     */
    solo_cpy_view_info(src_frme, dst_frme);
    /*
     * sweep info
     */
    solo_cpy_sweep_info(src_frme, dst_frme);
    /*
     * lock info
     */
    solo_cpy_lock_info(src_frme, dst_frme);
    /*
     * landmark info
     */
    sp_cpy_lmrk_info(src_frme, dst_frme);
    /*
     * frame center info
     */
    sp_cpy_ctr_info(src_frme, dst_frme);
}

/* c------------------------------------------------------------------------ */

struct solo_perusal_info *solo_return_winfo_ptr (void)
{
  // Allocate space the first time through

  if (static_solo_perusal_info == 0)
  {
    static_solo_perusal_info = new solo_perusal_info;
    for (int i = 0; i < SOLO_MAX_WINDOWS; ++i)
      static_solo_perusal_info->active_windows[i] = 0;
    for (int i = 0; i < SOLO_TOTAL_WINDOWS; ++i)
      static_solo_perusal_info->solo_windows[i] = 0;
    static_solo_perusal_info->num_locksteps = 0;
    for (int i = 0; i < SOLO_MAX_WINDOWS; ++i)
    {
      static_solo_perusal_info->first_ww_for_lockstep[i] = 0;
      static_solo_perusal_info->sweeps_in_lockstep[i] = 0;
    }
  }
  
  return static_solo_perusal_info;
}

/* c------------------------------------------------------------------------ */

void sp_copy_tsri (struct ts_ray_info *a, struct ts_ray_info *b)
{
    if(a && b) {
	b->ray_num = a->ray_num;
	b->sweep_num = a->sweep_num;
    }
    return;
}

/* c------------------------------------------------------------------------ */

struct solo_window_ptrs *solo_return_wwptr (int ww_num)
{
  struct solo_perusal_info *spi = solo_return_winfo_ptr();

  if (ww_num < 0 || ww_num > SOLO_TOTAL_WINDOWS-1)
    return 0;
  
  if (spi->solo_windows[ww_num] != 0)
    return spi->solo_windows[ww_num];
  
  // Initialize pointers and structs for this window

  int32_t time2 = time(0);

  struct solo_window_ptrs *next = new struct solo_window_ptrs;

  next->window_num = ww_num;
  next->time_modified = time2;

  next->parameter_scale = 0.0;
  next->parameter_bias = 0.0;

  next->active = 0;
  next->changed = 0;
  next->ddir_radar_num = 0;
  next->file_id = 0;
  next->menus_popped_up = 0;
  next->no_more_data = 0;
  next->num_colors_in_table = 0;
  next->parameter_num = 0;
  next->parameter_bad_val = 0;
  next->prev_time_stamp = 0;
  next->selected_sweep_num = 0;

  next->annotation_color_num = 0;
  next->background_color_num = 0;
  next->bnd_alert_num = 0;
  next->bnd_color_num = 0;
  next->bnd_last_num = 0;
  next->emphasis_color_num = 0;
  next->exceeded_color_num = 0;
  next->grid_color_num = 0;
  next->lock_color_num = 0;
  next->missing_data_color_num = 0;
  
  next->lock = (struct solo_plot_lock *)malloc(sizeof(struct solo_plot_lock));
  memset((char *)next->lock, 0, sizeof(struct solo_plot_lock));
  strncpy(next->lock->name_struct, "SPTL", 4);
  next->lock->sizeof_struct = sizeof(struct solo_plot_lock);
  next->lock->window_num = ww_num;
  next->lock->time_modified = time2;
  next->lock->linked_windows[ww_num] = YES;
  
  next->sweep =
    (struct solo_sweep_file *)malloc(sizeof(struct solo_sweep_file));
  memset((char *)next->sweep, 0, sizeof(struct solo_sweep_file));
  strncpy(next->sweep->name_struct, "SSFI", 4);
  next->sweep->sizeof_struct = sizeof(struct solo_sweep_file);
  next->sweep->window_num = ww_num;
  next->sweep->time_modified = time2;
  next->sweep->linked_windows[ww_num] = YES;
  next->sweep->version_num = DataManager::LATEST_VERSION;
  next->sweep->latest_version = YES;
  next->sweep->sweep_skip = 1;
  next->sweep->sweep_keep_count = 7;

  next->radar =
    (struct solo_radar_name *)malloc(sizeof(struct solo_radar_name));
  memset((char *)next->radar, 0, sizeof(struct solo_radar_name));
  strncpy(next->radar->name_struct, "SRDN", 4);
  next->radar->sizeof_struct = sizeof(struct solo_radar_name);
  next->radar->window_num = ww_num;
  next->radar->time_modified = time2;
  next->radar->linked_windows[ww_num] = YES;

  strncpy(next->parameter.name_struct, "SPMI", 4);
  next->parameter.sizeof_struct = sizeof(struct solo_parameter_info);
  next->parameter.window_num = ww_num;
  next->parameter.time_modified = time2;
  next->parameter.changed = NO;
  next->parameter.always_popup = NO;
  for (int i = 0; i < SOLO_MAX_WINDOWS; ++i)
    next->parameter.linked_windows[i] = NO;
  next->parameter.linked_windows[ww_num] = YES;
  next->parameter.parameter_name[0] = '\0';
  next->parameter.palette_name[0] = '\0';
  
  next->palette = 0;

  next->view = (struct solo_view_info *)malloc(sizeof(struct solo_view_info));
  memset((char *)next->view, 0, sizeof(struct solo_view_info));
  strncpy(next->view->name_struct, "SWVI", 4);
  next->view->sizeof_struct = sizeof(struct solo_view_info);
  next->view->window_num = ww_num;
  next->view->time_modified = time2;
  next->view->linked_windows[ww_num] = YES;
  next->view->magnification = 1.0;
  next->view->az_line_int_deg = 30.0;
  next->view->az_annot_at_km = 45.0;
  next->view->rng_ring_int_km = 15.0;
  next->view->rng_annot_at_deg = 45.0;
  next->view->ts_magnification = 1.0;
  next->view->x_tic_mag = 1.0;
  next->view->y_tic_mag = 1.0;
  next->view->angular_fill_pct = 120.0;
  
  next->radar_location.setId("SOL_RAD_LOC");
  next->center_of_view.setId("SOL_WIN_LOC");
  next->landmark.setId("SOL_LANDMARK");

  // Set up for field values

  struct solo_field_vals *last_sfv = 0;
  for (int i = 0; i < 3; ++i)
  {
    struct solo_field_vals *sfv =
      (struct solo_field_vals *)malloc(sizeof(struct solo_field_vals));
    memset(sfv, 0, sizeof(struct solo_field_vals));
    sfv->state = 123;
    sfv->num_vals = 5;
    if (i == 0)
    {
      next->field_vals = sfv;
    }
    else
    {
      sfv->last = last_sfv;
      last_sfv->next = sfv;
    }
    last_sfv = sfv;
    sfv->next = next->field_vals;
    next->field_vals->last = sfv;
  }

//  next->list_colors =
//    (struct solo_list_mgmt *)malloc(sizeof(struct solo_list_mgmt));
//  memset(next->list_colors, 0, sizeof(struct solo_list_mgmt));
//  next->list_colors->sizeof_entries = SIZEOF_COLOR_NAMES;

  // Data coloration parameters

  next->data_color_lut = (u_int32_t *)malloc(65536*sizeof(int32_t));
  if (next->data_color_lut == 0)
  {
    printf("Unable to malloc next->data_color_lut for frame: %d\n", ww_num);
    exit(1);
  }
  memset (next->data_color_lut, 0, 65536*sizeof (int32_t));
  // Now center it
  next->data_color_lut += 32768;

  next->data_cell_lut = (short *)malloc(10000*sizeof(short));
  if (next->data_cell_lut == 0)
  {
    printf("Unable to malloc next->data_cell_lut for frame: %d\n", ww_num);
    exit(1);
  }
  memset (next->data_cell_lut, 0, 10000*sizeof(short));

  next->uniform_cell_spacing = 0.0;
  next->uniform_cell_one = 0.0;
  next->number_cells = 0;

  next->cell_colors = (u_int32_t *)malloc(10000*sizeof(u_int32_t));
  if (next->cell_colors == 0)
  {
    printf("Unable to malloc next->cell_colors for frame: %d\n", ww_num);
    exit(1);
  }
  memset (next->cell_colors, 0, 10000*sizeof(u_int32_t));

  next->color_bar = 0;
  next->clicked_angle = 0.0;
  next->clicked_range = 0.0;
  next->pisp_click = new PointInSpace();
  next->lock_num = 0;
  next->clicked_frame_num = 0;
  next->clicked_list_id = 0;
  next->clicked_list_char_num = 0;
  next->clicked_button_id = 0;
  next->clicked_list_entry_num = 0;
  next->next_lock = 0;
  next->next_sweep = 0;
  next->lead_sweep = 0;
  next->image = 0;
  next->big_image = 0;
  next->landmark_x_offset = 0.0;
  next->landmark_y_offset = 0.0;

  next->landmark_info =
    (struct landmark_info *)malloc(sizeof(struct landmark_info));
  memset(next->landmark_info, 0, sizeof(struct landmark_info));
  strncpy(next->landmark_info->name_struct, "SLMK", 4);
  next->landmark_info->sizeof_struct = sizeof(struct landmark_info);
  next->landmark_info->reference_frame = ww_num +1;
  next->landmark_info->reference_frame = 1;
  next->landmark_info->linked_windows[ww_num] = 1;
  next->landmark_info->window_num = ww_num;

  next->frame_ctr_info =
    (struct frame_ctr_info *)malloc(sizeof(struct frame_ctr_info));
  memset(next->frame_ctr_info, 0, sizeof(struct frame_ctr_info));
  strncpy(next->frame_ctr_info->name_struct, "SCTR", 4);
  next->frame_ctr_info->sizeof_struct = sizeof(struct frame_ctr_info);
  next->frame_ctr_info->reference_frame = ww_num +1;
  next->frame_ctr_info->reference_frame = 1;
  next->frame_ctr_info->linked_windows[ww_num] = 1;
  next->frame_ctr_info->window_num = ww_num;

  next->clicked_ctr_of_frame = 0;
  next->sweep_file_modified = 0;
  next->scan_mode = 0;
  
  next->examine_info =
    (struct solo_examine_info *)malloc(sizeof(struct solo_examine_info));
  memset(next->examine_info, 0, sizeof(struct solo_examine_info));
  strncpy(next->examine_info->name_struct, "SXMN", 4);
  next->examine_info->sizeof_struct = sizeof(struct solo_examine_info);
  next->examine_info->window_num = ww_num;
  next->examine_info->scroll_increment = 19;
  next->examine_info->ray_count = 5;
  next->examine_info->cell_count = 21;
  next->examine_info->whats_selected = EX_RADAR_DATA;
  next->examine_info->typeof_change = EX_DELETE;
  strcpy(next->examine_info->display_format, "6.1f");

  next->changez_list = 0;
  next->num_changez = 0;

  // Examine control

  next->examine_control = new struct examine_control;
  next->examine_control->window_num = 0;
  next->examine_control->actual_ray_num = 0;
  next->examine_control->actual_at_cell = 0;
  next->examine_control->actual_num_cells = 0;
  next->examine_control->actual_ray_count = 0;
  next->examine_control->actual_num_fields = 0;
  for (int i = 0; i < 16; ++i)
    next->examine_control->actual_field_num[i] = 0;
  next->examine_control->num_data_cols = 0;
  next->examine_control->num_cols = 0;
  for (int i = 0; i < MAX_EXM_COLUMNS; ++i)
  {
    next->examine_control->col_lims[i] = 0;
    next->examine_control->col_ptrs[i] = 0;
  }
  next->examine_control->bad_click = 0;
  next->examine_control->heading_row_count = 0;
  next->examine_control->non_data_col_count = 0;
  next->examine_control->num_chars_rng_label = 0;
  next->examine_control->num_chars_per_row = 0;
  next->examine_control->num_chars_per_cell = 0;
  next->examine_control->rays_in_sweep = 0;
  next->examine_control->cells_in_ray = 0;
  next->examine_control->ctr_on_click = 0;
  next->examine_control->tmp_change = 0;
  next->examine_control->click_angle = 0.0;
  next->examine_control->click_range = 0.0;
  for (int i = 0; i < MAX_EXM_COLUMNS; ++i)
  {
    next->examine_control->rotation_angles[i] = 0.0;
    next->examine_control->bad_data[i] = 0.0;
    next->examine_control->ac_vel[i] = 0.0;
    next->examine_control->data_ptrs[i] = 0;
  }
  next->examine_control->data_space = 0;
  next->examine_control->max_cells = 0;
  next->examine_control->ranges = 0;
  next->examine_control->max_rngs = 0;
  for (int i = 0; i < 256; ++i)
    next->examine_control->data_line[i] = '\0';
  
  next->tsrt = (struct ts_ray_table *)malloc(sizeof(struct ts_ray_table));
  memset(next->tsrt, 0, sizeof(struct ts_ray_table));
  next->tsrt->max_sweep_entries = 3;
  int mm = next->tsrt->max_sweep_entries * sizeof(struct ts_sweep_info);
  next->tsrt->tssi_top = (struct ts_sweep_info *)malloc(mm);
  memset(next->tsrt->tssi_top, 0, mm);

  next->clicked_tsri =
    (struct ts_ray_info *)malloc(sizeof(struct ts_ray_info));
  memset(next->clicked_tsri, 0, sizeof(struct ts_ray_info));

  next->clicked_x_pxl = 0;
  next->clicked_y_pxl = 0;
  next->color_bar_min_val = 0.0;
  next->color_bar_max_val = 0.0;
  next->emphasis_min = 0.0;
  next->emphasis_max = 0.0;
  next->d_prev_time_stamp = 0.0;
  next->d_sweepfile_time_stamp = 0.0;
  next->d_sweepfile_time_inc = 0.0;
  next->swpfi_filter_set = 0;
  next->swpfi_time_sync_set = false;
  next->clicked_time = 0.0;
  next->magic_rng_annot = 1;
  next->filter_fxd_ang = 0.5;
  next->filter_tolerance = .25;
  next->filter_scan_modes = "PPI,SUR";
  next->color_bar_location = 0;
  next->color_bar_symbols = 0;
    
  // Now return the pointer to the window struct

  spi->solo_windows[ww_num] = next;
  return spi->solo_windows[ww_num];
}

/* c------------------------------------------------------------------------ */

void se_set_sfic_from_frame (int frme)
{
  // This routine nabs the sweep info from the frame info.
  // This routine is called when the editor widget is popped up.
  // and is called at the beginning of an editing pass just to
  // make sure the editor info is current.

  // Get pointers to the needed structures

  struct solo_edit_stuff *seds = return_sed_stuff();
  struct swp_file_input_control *sfic = return_swp_fi_in_cntrl();
  WW_PTR wwptr = solo_return_wwptr(frme);

  // Set the frame information in the different structures

  seds->focus_frame = wwptr->lead_sweep->window_num;
  sfic->frame = wwptr->lead_sweep->window_num;
  seds->popup_frame = frme;
  sfic->clicked_frame = frme + 1; /* a non-zero value here implies
				   * that the editor will operate on
				   * the sweep plotted in this frame
				   */

  sfic->radar_num = wwptr->sweep->radar_num;
  sfic->directory_text = wwptr->sweep->directory_name;
  sfic->radar_names_text = wwptr->sweep->radar_name;

  if (wwptr->view->type_of_plot & SOLO_TIME_SERIES)
  {
    sfic->start_time =
      DataManager::getInstance()->getTimeSeriesStartTime(sfic->frame,
							 wwptr->sweep->radar_num,
							 wwptr->sweep->start_time);
    sfic->stop_time = wwptr->sweep->stop_time;
  }
  else
  {
    sfic->start_time = 
      DataManager::getInstance()->getClosestDataTime(sfic->frame,
						     wwptr->sweep->radar_num,
						     wwptr->sweep->start_time,
						     wwptr->sweep->version_num);
    
    sfic->stop_time = sfic->start_time;
  }

  DateTime start_time;
  start_time.setTimeStamp(sfic->start_time);
  sfic->first_sweep_text = start_time.toString2();
  sfic->last_sweep_text = sfic->first_sweep_text;

  if (wwptr->sweep->version_num == DataManager::LATEST_VERSION)
  {
    sfic->version_text = "last";
  }
  else if (wwptr->sweep->version_num == DataManager::EARLIEST_VERSION)
  {
    sfic->version_text = "first";
  }
  else
  {
    char version_text[80];
    sprintf(version_text, "%d", wwptr->sweep->version_num);
    sfic->version_text = version_text;
  }
  
  sfic->version = wwptr->sweep->version_num;
}

/* c------------------------------------------------------------------------ */

void solo_trigger_dir_rescans(const std::string &dir)
{
  struct solo_perusal_info *spi = solo_return_winfo_ptr();

  // Get the directory path for this window

  std::string dirx = dir;
  if (dirx[dir.size()-1] != '/')
    dirx += "/";
    
  // Loop through all of the windows.  Any window that is displaying data
  // from the same directory as this window needs to rescan the directory.

  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    // Only process active windows

    if (!(spi->active_windows[ww]))
      continue;

    WW_PTR wwptr = solo_return_wwptr(ww);
    std::string diry = wwptr->sweep->directory_name;
    if (diry[diry.size()-1] != '/')
      diry += "/";

    if (dirx.compare(diry) == 0)
      DataManager::getInstance()->setRescanUrgent(ww);
  }
}

/* c------------------------------------------------------------------------ */

void solo_ww_top_line(int frame_index)
{
  // Get the information

  WW_PTR wwptr = solo_return_wwptr(frame_index);
  WW_PTR wwptrx = wwptr->lead_sweep;
  int ww = wwptr->lead_sweep->window_num;
  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(ww);
  DateTime data_time;
    
  // Get the data time

  if ((wwptrx->view->type_of_plot & SOLO_TIME_SERIES) == 0)
    data_time.setTimeStamp(data_info->getTime());
  else
    data_time.setTimeStamp(wwptrx->sweep->start_time);
    
  // Set the top line

  char top_line[1024];
  sprintf(top_line, "%d %s %s %.1f %s %s",
	  frame_index + 1,
	  data_time.toString().c_str(),
	  data_info->getRadarNameClean(8).c_str(),
	  data_info->getFixedAngle(),
	  data_info->scanMode2Str(data_info->getScanMode()).c_str(),
	  wwptr->parameter.parameter_name);
  wwptr->top_line = top_line;
}

/* c------------------------------------------------------------------------ */

int sp_max_frames (void)
{
    return(SOLO_MAX_WINDOWS);
}
