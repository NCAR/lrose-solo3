#include <iostream>
#include <stdarg.h>
#include <errno.h>
#include <dirent.h>

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxVol.hh>
#endif

#include <dd_math.h>
#include <DataInfo.hh>
#include <DataManager.hh>
#include <dorade_share.hh>
#include <EarthRadiusCalculator.hh>
#include <EditWindow.hh>
#include <ParameterWindow.hh>
#include <se_utils.hh>
#include <sed_shared_structs.h>
#include <seds.h>
#include "sii_bscan.hh"
#include "sii_param_widgets.hh"
#include "sii_perusal.hh"
#include "sii_utils.hh"
#include "sii_xyraster.hh"
#include <solo_editor_structs.h>
#include <solo_list_widget_ids.h>
#include <solo_window_structs.h>
#include "solo2.hh"
#include "soloii.h"
#include "soloii.hh"
#include <SoloState.hh>
#include "sp_accepts.hh"
#include "sp_basics.hh"
#include "sp_clkd.hh"
#include "sp_dorade.hh"
#include "xyraster.h"

using namespace std;

// Local types

struct linked_frames {
  struct linked_frames *next;
  WW_PTR wwptr;
  WW_PTR lead_sweep;
};

// Forward declarations

int sp_data_loop(struct linked_frames *flink0);

static int diag_flag=0;

static struct linked_frames *original_sweep_set[SOLO_MAX_WINDOWS];
static struct linked_frames *linked_sweep_set[SOLO_MAX_WINDOWS];
static struct linked_frames *linked_frames[2*SOLO_MAX_WINDOWS];
static int original_sweep_set_count;
static int original_sweep_count;
static int real_sweep_set_count, nex_sweep_count;

# define HIGH_PRF 0
# define LOW_PRF 1

/* c------------------------------------------------------------------------ */

int displayq(int click_frme, int command)
{
  static bool first_plot = true;
  static bool print_message = true;

  if (SoloState::getInstance()->isHalt())
  {
    SoloState::getInstance()->setHaltFlag(false);
    return 1;
  }

  if (solo_busy())
    return 0;

  WW_PTR wwptr = solo_return_wwptr(click_frme);

  if (wwptr->view->type_of_plot & SOLO_TIME_SERIES)
    return tsDisplay(click_frme, command);

  // Add some information to the debug message.

  {
    char message[256];
    sprintf(message, "Enter displayq(%d,%d)", click_frme, command);
    sii_append_debug_stuff(message);
  }
  
  solo_set_busy_signal();
  main_window->changeCursor(false);
  struct solo_perusal_info *spi = solo_return_winfo_ptr();

  // A sweep set is the set of frames that are plotting the same sweep

  if (first_plot)
  {
    for (int ii = 0; ii < SOLO_MAX_WINDOWS; ii++)
    {
      // Produce an array of linked frame structs to be used in sweep sets

      linked_frames[ii] = new struct linked_frames;
      linked_frames[ii]->next = 0;
      linked_frames[ii]->wwptr = 0;
      linked_frames[ii]->lead_sweep = 0;
    }

    // Get the batch threshold field and value.  batch_threshold_field and
    // batch_threshold_value are globals defined in sii_externals.h

    char *batch_thresh_string = getenv("BATCH_THRESHOLD");
    
    if (batch_thresh_string != 0)
    {
      std::vector< std::string > tokens;
      tokenize(batch_thresh_string, tokens);

      if (tokens.size() >= 2)
      {
	float batch_thresh;
	
	if (sscanf(tokens[1].c_str(), "%f", &batch_thresh) == 1)
	{
	  batch_threshold_value = batch_thresh;
	  batch_threshold_field = g_string_new(tokens[0].c_str());
	}
      }
      else
      {
	batch_threshold_field = g_string_new ("DM");
	batch_threshold_value = -111;
      }
    }

    first_plot = false;
  }

  int frame_count = 0;
  int frame_list[SOLO_MAX_WINDOWS];
  int loop_count = 0;
  
  if (command == REPLOT_THIS_FRAME)
  {
    frame_list[frame_count++] = click_frme;
  }
  else
  {
    loop_count = sii_return_frame_count();
    for (int frme = 0; frme < loop_count; frme++)
    {
      if (spi->active_windows[frme] && wwptr->lock->linked_windows[frme])
	frame_list[frame_count++] = frme;
    }
  }

  bool one_pass = true;
  DataManager::time_type_t original_file_action = DataManager::TIME_AFTER;
  int original_sweep_skip = 0;
  bool original_replot = false;
  DataManager::version_t original_version = DataManager::LATEST_VERSION;
  
  switch(command)
  {
  case BACKWARDS_FOREVER:
    one_pass = false;
    original_file_action = DataManager::TIME_BEFORE;
    break;

  case BACKWARDS_ONCE:
    original_file_action = DataManager::TIME_BEFORE;
    break;

  case REPLOT_LOCK_STEP:
  case REPLOT_THIS_FRAME:
    original_sweep_skip = 1;
    original_replot = true;
    original_file_action = DataManager::TIME_NEAREST;
    original_version = (DataManager::version_t)wwptr->sweep->version_num;
    break;

  case FORWARD_FOREVER:
    one_pass = false;
    break;

  case FORWARD_ONCE:
    break;

  default:
    printf("Illegal command to displayq: %d\n", command);
    return 1;
  }

//  int backwards = DataManager::TIME_BEFORE;
//  original_file_action == DataManager::TIME_BEFORE;
//  int forward = DataManager::TIME_AFTER;
//  original_file_action == DataManager::TIME_AFTER;
  wwptr->sweep->version_num = original_version;
  solo_worksheet();

  bool checked_off[SOLO_MAX_WINDOWS];
  
  for (int ii = 0; ii < frame_count; ii++)
  {
    checked_off[frame_list[ii]] = false;
  }

  // original_sweep_count and original_sweep_set_count are global statics
  // in this file

  original_sweep_count = 0;
  original_sweep_set_count = 0;

  struct linked_frames *flink0;
  struct linked_frames *flink;
  
  for (int ii = 0; ii < frame_count; ii++)
  {
    int ww = frame_list[ii];

    if (checked_off[ww])
      continue;
    checked_off[ww] = true;

    // We are at the lead sweep for the next sweep set.

    int set_num = original_sweep_set_count++;
    original_sweep_set[set_num] = linked_frames[original_sweep_count++];
    flink0 = original_sweep_set[set_num];
    flink = original_sweep_set[set_num];
    WW_PTR wwptr = solo_return_wwptr(ww);
    flink->wwptr = wwptr;
    flink->lead_sweep = wwptr->lead_sweep;
    flink->next = NULL;
    if (frame_count == 1)
      break;
    for (wwptr = wwptr->next_sweep;  wwptr;  wwptr = wwptr->next_sweep)
    {
      if (wwptr->window_num >= loop_count)
	break;
      flink->next = linked_frames[original_sweep_count++];
      flink = flink->next;
      flink->wwptr = wwptr;
      flink->lead_sweep = wwptr->lead_sweep;
      flink->next = NULL;
      checked_off[wwptr->window_num] = true;
    }
  }

  bool new_worksheet = true;
  int return_flag = 0;
  bool broke = false;
  
  for(;;)
  {
    // Be ready to plot this lockstep forever (real_sweep_set_count and
    // nex_sweep_count are static globals in this file)

    real_sweep_set_count = 0;
    nex_sweep_count = 0;

    if (new_worksheet)
    {
      new_worksheet = false;

      // Put all the sweep links back in case they've been twiddled by nexrad

      solo_worksheet();
    }
    
    for (int sset = 0; sset < original_sweep_set_count; sset++)
    {
      flink0 = original_sweep_set[sset];
      int lead_frame = flink0->lead_sweep->window_num;
//      original_lead_frame = lead_frame;
//      sweep_skip = original_replot ? 1 : flink0->lead_sweep->sweep->sweep_skip;
      DataManager::time_type_t file_action = original_file_action;
      DataManager::version_t version = original_version;
      bool replot = original_replot;
//      int num_dz = 0;
//      int num_ve = 0;
//      ve_next = NO;
//      dz_next = NO;

      if (!DataManager::getInstance()->nabNextFile(lead_frame,
						   file_action,
						   version, replot))
      {
	// Could not access data for this plot

	return_flag = 1;

	if (print_message)
	{
	  if (file_action == 1)
	    g_message ("Cannot access data for next plot - skipping");
	  else
	    g_message ("Cannot access data for prev plot - skipping");

	  print_message = false;
	}
	
	broke = true;
      }
      else
      {
	print_message = true;
      }
      
      linked_sweep_set[real_sweep_set_count++] = original_sweep_set[sset];
	    
    } /* end for(sset */
	
    if (broke)
      break;
	
    // Do some initialization for each sweep set

    for (int sset = 0; sset < original_sweep_set_count; sset++)
    {
      flink = original_sweep_set[sset];
      int lead_frame = flink->lead_sweep->window_num;

      for (; flink; flink = flink->next)
      {
	// For each linked sweep

	WW_PTR wwptr = flink->wwptr;
	int ww = wwptr->window_num;
	if (!solo_parameter_setup(ww))
	{
	  broke = true;
	  g_message("Broke at solo_parameter_setup");
	  solo_clear_busy_signal();
	  main_window->changeCursor(true);
	  return 0;
	  break;
	}
	sii_param_set_plot_field(ww, wwptr->parameter.parameter_name);
	if (ww != lead_frame)
	{
	  // Copy radar location

	  wwptr->radar_location = wwptr->lead_sweep->radar_location;
	}
	sp_landmark_offsets(ww);
	sp_align_center(ww);
	sii_update_frame_info(ww);
	init_xyraster(ww);
	solo_ww_top_line(ww);
	wwptr->uniform_cell_spacing = 0;
	solo_cell_lut(ww);
	checked_off[ww] = true;
      }
      if (broke)
	break;
    }
    if (broke)
      yes_exit();
        
    // Loop through the data for each sweep set

    for (int sset = 0; sset < real_sweep_set_count; sset++)
    {
      if (sp_data_loop(linked_sweep_set[sset]) < 0)
      {
	one_pass = true;
	break;
      }
      if (SoloState::getInstance()->isHalt())
      {
	SoloState::getInstance()->setHaltFlag(false);
	one_pass = true;
	break;
      }
    }
    if (broke || one_pass)
      break;

  } /* end for(;; */

  // Update sweep info in all frames plotted

  for (int ii = 0; ii < real_sweep_set_count; ii++)
  {
    flink = linked_sweep_set[ii];
    int lead_frame = flink->wwptr->lead_sweep->window_num;
    for (; flink; flink = flink->next)
    {
      solo_cpy_sweep_info(lead_frame, flink->wwptr->window_num);
      SweepfileWindow *sweepfile_window =
	frame_configs[flink->wwptr->window_num]->sweepfile_window;
      if (sweepfile_window != 0)
	sweepfile_window->update();
      ParameterWindow *param_window =
	frame_configs[flink->wwptr->window_num]->param_window;
      if (param_window != 0)
	param_window->update();
      ViewWindow *view_window =
	frame_configs[flink->wwptr->window_num]->view_window;
      if (view_window != 0)
	view_window->update();
      EditWindow *edit_window =
	frame_configs[flink->wwptr->window_num]->edit_window;
      if (edit_window != 0)
	edit_window->resetTimes();
    }
  }

  solo_clear_busy_signal();
  main_window->changeCursor(true);

  return return_flag;
}
/* c------------------------------------------------------------------------ */

//void 
//sp_nex_sweep_set (int ns, struct linked_frames *sweep_list[])
//{
//    int ii, jj, kk;
//    struct linked_frames *flink, *flinkx = 0, *last_flink = 0;
//
//    jj = real_sweep_set_count++;
//
//    for(ii=0; ii < ns; ii++) {
//	kk = original_sweep_count + nex_sweep_count++;
//	flink = linked_frames[kk];
//	memcpy(flink, sweep_list[ii], sizeof(struct linked_frames));
//	if(!ii) {
//	    linked_sweep_set[jj] = flinkx = flink;
//	}
//	else {
//	    last_flink->next = flink;
//	    last_flink->wwptr->next_sweep = flink->wwptr;
//	}
//	flink->wwptr->lead_sweep = flink->lead_sweep = 
//	      flinkx->wwptr;
//	flink->wwptr->next_sweep = NULL;
//	flink->next = NULL;
//	last_flink = flink;
//    }
//    return;
//}

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX

void _calcSector(const std::vector< RadxRay* > &rays,
		 const std::size_t ray_index, const std::size_t num_rays,
		 double &sector, double &angle0, double angle1)
{
  // Get pointers to the current and surrounding rays

  RadxRay *ray = rays[ray_index];
  
  RadxRay *prev_ray = 0;
  RadxRay *next_ray = 0;
  
  if (ray_index > 0)
    prev_ray = rays[ray_index-1];
  if (ray_index < num_rays - 1)
    next_ray = rays[ray_index+1];
      
  // Calculate the angles

  if (prev_ray == 0)
  {
    sector = angdiff(ray->getAzimuthDeg(), next_ray->getAzimuthDeg());
    angle0 = FMOD360(ray->getAzimuthDeg() - (0.5 * sector) + 360.0);
    angle1 = FMOD360(ray->getAzimuthDeg() + (0.5 * sector) + 360.0);
  }
  else if (next_ray == 0)
  {
    sector = angdiff(prev_ray->getAzimuthDeg(), ray->getAzimuthDeg());
    angle0 = FMOD360(ray->getAzimuthDeg() - (0.5 * sector) + 360.0);
    angle1 = FMOD360(ray->getAzimuthDeg() + (0.5 * sector) + 360.0);
  }
  else
  {
    angle0 = prev_ray->getAzimuthDeg() +
      (0.5 * angdiff(prev_ray->getAzimuthDeg(), ray->getAzimuthDeg()));
    angle1 = ray->getAzimuthDeg() +
      (0.5 * angdiff(ray->getAzimuthDeg(), next_ray->getAzimuthDeg()));
	
    sector = angdiff(angle0, angle1);
  }
}

#endif

/* c------------------------------------------------------------------------ */

int sp_data_loop (struct linked_frames *flink0)
{
  static const float MAX_SECTOR = 4.0;
  static int count = 0;

  // Get the needed information

  int lead_frame = flink0->lead_sweep->window_num;
  WW_PTR wwptr = solo_return_wwptr(lead_frame);
  double ang_fill = wwptr->view->angular_fill_pct * 0.01;
    DataInfo *data_info =
      DataManager::getInstance()->getWindowInfo(lead_frame);

  guint frame_sync_nums[SOLO_MAX_WINDOWS];
    
  for (struct linked_frames *flink = flink0; flink != 0; flink = flink->next)
  {
    int ww = flink->wwptr->window_num;
    frame_sync_nums[ww] = sii_frame_sync_num(ww);
  }
   
#ifdef USE_RADX

  // Get access to the data.  Note that the volume in the DataInfo object
  // should always just contain the information for the current sweep so we
  // will always use the first sweep.

  const RadxVol &radx_vol = data_info->getRadxVolume();
  const RadxSweep *sweep = data_info->getRadxSweep();
  const std::vector< RadxRay* > rays = radx_vol.getRays();
  
  if (sweep == 0)
    return -1;
  
  double prev_angle1 = EMPTY_FLAG;

  for (std::size_t ray_index = sweep->getStartRayIndex();
       ray_index <= sweep->getEndRayIndex(); ++ray_index)
  {
    // Get a pointer to the current ray

    const RadxRay *ray = rays[ray_index];
    
    // Get the sector to plot based on the rotation angle
    
    double ray_sector = 0.0;
    double ray_angle0 = 0.0;
    double ray_angle1 = 0.0;
    
    _calcSector(rays, ray_index, radx_vol.getNRays(),
		ray_sector, ray_angle0, ray_angle1);
    
    double sector;
    double angle0;
    double angle1;
    
    if (ang_fill != 0.0 && ang_fill < 1.0)
    {
      sector = ray_sector < 0 ? -ang_fill : ang_fill;
      angle0 = FMOD360(ray->getAzimuthDeg() - (0.5 * sector));
      angle1 = FMOD360(ray->getAzimuthDeg() + (0.5 * sector));
    }
    else if (FABS(ray_sector) > MAX_SECTOR)
    {
      if (ray_sector < 0)
	sector = -MAX_SECTOR * 0.5;
      else
	sector = MAX_SECTOR * 0.5;
      if (prev_angle1 == EMPTY_FLAG)
	angle0 = FMOD360(ray->getAzimuthDeg() - (0.5 * sector));
      else
	angle0 = prev_angle1;
      angle1 = FMOD360(ray->getAzimuthDeg() + (0.5 * sector));
      prev_angle1 = EMPTY_FLAG;
    }
    else
    {
      sector = ray_sector;
      angle0 = ray_angle0;
      if (ang_fill != 0.0)
	sector *= ang_fill;
      else
	sector *= 1.2;

      angle1 = FMOD360(angle0 + sector);
      prev_angle1 = FMOD360(ray->getAzimuthDeg() +(0.5 * ray_sector));
	   
      // All this thrashing is to keep the plot from rotating when the
      // angular fill is increased
    }
    
    for (struct linked_frames *flink = flink0; flink != 0; flink = flink->next)
    {
      count++;

      int ww = flink->wwptr->window_num;
      if (frame_sync_nums[ww] != sii_config_sync_num((guint)ww))
	return -1;
      
      // NOTE: These calls update the beam-related information in the xyras
      // structure.  These values are used locally and then overwritten when
      // the next beam is processed.  These should probably be pulled out into
      // a local structure to avoid confusion, but I need to get more familiar
      // with this code before I do something like that since I might be
      // missing something.

      struct xyras *rxy = return_xyras(ww);
      ray_raster_setup(ww, angle0, angle1, rxy);
      if (rxy->ignore_this_ray)
	continue;
      
      // Assign colors to the data and rasterize this variable for this ray

      // NOTE: After the solo_color_cells call, wwptr->cell_colors is set
      // based on the data in the current parameter.  I haven't yet figured
      // out how this gets the data for the current beam.  It looks to me like
      // it would get the same data each time rather than marching around the
      // scan.
      //
      // In ray_raster_setup(), rxy->colors is set to wwptr->cell_colors.
      // So the changes to wwptr->cell_colors in solo_color_cells() will cause
      // rxy->colors to change also.  rxy->colors is used in xx_inner_loop()
      // and yy_inner_loop() to provide the color values.

      solo_color_cells(ww, ray);
      if(rxy->x_inner_loop)
	xx_inner_loop(rxy);
      else
	yy_inner_loop(rxy);
    }

    // Check for stop event.

    if (SoloState::getInstance()->isHalt())
    {
      printf("HALTING\n");
      break;
    } 
  } /* endfor - ray_index */
  
#else

  // Now really loop through the data
    
  double prev_angle1 = EMPTY_FLAG;

  while (true)
  {
    // For each ray 

    // Get the sector to plot based on the rotation angle

    struct dd_ray_sector *ddrc =
      data_info->rayCoverage(data_info->getSourceRayNum());
    
    double sector;
    double angle0;
    double angle1;
    
    if (ang_fill != 0.0 && ang_fill < 1.0)
    {
      sector = ddrc->sector < 0 ? -ang_fill : ang_fill;
      angle0 = FMOD360(ddrc->rotation_angle - (0.5 * sector));
      angle1 = FMOD360(ddrc->rotation_angle + (0.5 * sector));
    }
    else if (FABS(ddrc->sector) > MAX_SECTOR)
    {
      if (ddrc->sector < 0)
	sector = -MAX_SECTOR * 0.5;
      else
	sector = MAX_SECTOR * 0.5;
      if (prev_angle1 == EMPTY_FLAG)
	angle0 = FMOD360(ddrc->rotation_angle - (0.5 * sector));
      else
	angle0 = prev_angle1;
      angle1 = FMOD360(ddrc->rotation_angle + (0.5 * sector));
      prev_angle1 = EMPTY_FLAG;
    }
    else
    {
      sector = ddrc->sector;
      angle0 = ddrc->angle0;
      if (ang_fill != 0.0)
	sector *= ang_fill;
      else
	sector *= 1.2;

      angle1 = FMOD360(angle0 + sector);
      prev_angle1 = FMOD360(ddrc->rotation_angle +(0.5 * ddrc->sector));
	   
      // All this thrashing is to keep the plot from rotating when the
      // angular fill is increased
    }
    
    for (struct linked_frames *flink = flink0; flink != 0; flink = flink->next)
    {
      count++;

      int ww = flink->wwptr->window_num;
      if (frame_sync_nums[ww] != sii_config_sync_num((guint)ww))
	return -1;
      
      // NOTE: These calls update the beam-related information in the xyras
      // structure.  These values are used locally and then overwritten when
      // the next beam is processed.  These should probably be pulled out into
      // a local structure to avoid confusion, but I need to get more familiar
      // with this code before I do something like that since I might be
      // missing something.

      struct xyras *rxy = return_xyras(ww);
      ray_raster_setup(ww, angle0, angle1, rxy);
      if (rxy->ignore_this_ray)
	continue;

      // Assign colors to the data and rasterize this variable for this ray

      // NOTE: After the solo_color_cells call, wwptr->cell_colors is set
      // based on the data in the current parameter.  I haven't yet figured
      // out how this gets the data for the current beam.  It looks to me like
      // it would get the same data each time rather than marching around the
      // scan.
      //
      // In ray_raster_setup(), rxy->colors is set to wwptr->cell_colors.
      // So the changes to wwptr->cell_colors in solo_color_cells() will cause
      // rxy->colors to change also.  rxy->colors is used in xx_inner_loop()
      // and yy_inner_loop() to provide the color values.

      solo_color_cells(ww);
      if(rxy->x_inner_loop)
	xx_inner_loop(rxy);
      else
	yy_inner_loop(rxy);
    }

    if (data_info->getSourceRayNum() >= data_info->getNumRays())
	    break;

    // Check for stop event.

    if (SoloState::getInstance()->isHalt())
    {
      printf("HALTING\n");
      break;
    } 

    // Nab the next ray

    if (!data_info->loadNextRay())
      break;
  }

#endif

  for (struct linked_frames *flink = flink0; flink != 0; flink = flink->next)
  {
    int ww = flink->wwptr->window_num;
    sii_xfer_images(ww, NULL);
    sii_reset_reconfig_flags(ww);
  }
  
  return 0;
}

/* c------------------------------------------------------------------------ */

int sp_diag_flag (void)
{
    return(diag_flag);
}
/* c------------------------------------------------------------------------ */

void 
sp_set_diag_flag (int flag)
{
    diag_flag = flag;
}
/* c------------------------------------------------------------------------ */

void sp_replot_all(void)
{
  solo_worksheet();
  struct solo_perusal_info *spi = solo_return_winfo_ptr();

  for (int ii = 0; ii < spi->num_locksteps; ii++)
  {
    int ww = spi->first_ww_for_lockstep[ii];
    displayq(ww, REPLOT_LOCK_STEP);
  }
}
/* c------------------------------------------------------------------------ */

// I think this goes through the windows and clean up the lock information.

int solo_worksheet(void)
{
  struct solo_perusal_info *spi = solo_return_winfo_ptr();

  bool checked_off[SOLO_MAX_WINDOWS];
  bool locks_changed = false;
  
  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    checked_off[ww] = false;
    WW_PTR wwptr = solo_return_wwptr(ww);
    wwptr->next_lock = NULL;
    wwptr->next_sweep = NULL;
    if (wwptr->lock->changed)
      locks_changed = true;

    wwptr->lock->changed = NO;
  } /* endfor - ww */

  // Determine the frame links organization

  spi->num_locksteps = 0;
  
  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    if (!spi->active_windows[ww])
      continue;

    if (checked_off[ww]) /* already examined */
      continue;

    WW_PTR wwptr = solo_return_wwptr(ww);
    for (int ii = ww; ii < SOLO_MAX_WINDOWS; ii++)
    {
      if (!spi->active_windows[ii])
	continue;

      if (wwptr->lock->linked_windows[ii])
      {
	// This is the first window of a new lockstep. Record it and check in
	// the rest of the frames in the lockstep.

	WW_PTR last_wwptr = wwptr;
	int lock_num = spi->num_locksteps++;
	spi->first_ww_for_lockstep[lock_num] = ww;

	for (int jj = ii; jj < SOLO_MAX_WINDOWS; jj++)
	{
	  if (!spi->active_windows[jj])
	    continue;

	  if (wwptr->lock->linked_windows[jj])
	  {
	    WW_PTR wwptrc = solo_return_wwptr(jj);
	    wwptrc->lock_num = lock_num;
	    last_wwptr->next_lock = wwptrc;
	    last_wwptr = wwptrc;
	    checked_off[jj] = true;
	  }
	} /* endfor - jj */

	break;

      } /* endif - wwptr->lock->linked_windows[ii] */
      
    } /* endfor - ii */
  } /* endfor - ww */
  
  if (locks_changed)
  {
    for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
    {
      WW_PTR wwptr = solo_return_wwptr(ww);
      wwptr->lock_color_num =
	frame_border_colors[wwptr->lock_num]->pixel;
    }
  }

  for (int ii = 0; ii < SOLO_MAX_WINDOWS; checked_off[ii++] = false);

  // We want to compile a list of unique sweeps for each lock step and break
  // any sweep links to frames outside the lock step.

  int nus = 0;
  
  for (int mm = 0; mm < spi->num_locksteps; mm++)
  {
    // For each lockstep

    WW_PTR wwptr = solo_return_wwptr(spi->first_ww_for_lockstep[mm]);
    int32_t *lock_list = wwptr->lock->linked_windows;
    spi->sweeps_in_lockstep[mm] = 0;

    for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
    {
      if (checked_off[ww] || !(*(lock_list+ww)) ||
	  !spi->active_windows[ww])
	continue;	/* ignore! */

      checked_off[ww] = true;

      // We now have a window from the current lock list that has not been
      // checked.  Record unique sweep info and break the sweep links to
      // windows not in the current lockstep.

      bool unlink[SOLO_MAX_WINDOWS];
      for (int jj = 0; jj < SOLO_MAX_WINDOWS; unlink[jj++] = false);

      nus = spi->sweeps_in_lockstep[mm]++;
      WW_PTR wwptr = solo_return_wwptr(ww);
      WW_PTR last_wwptr = wwptr;
      WW_PTR lead_sweep = wwptr;
      wwptr->lead_sweep = wwptr;
      int32_t *sweep_links = wwptr->sweep->linked_windows;

      int nun = 0;
      
      for (int jj = ww + 1; jj < SOLO_MAX_WINDOWS; jj++)
      {
	if (!spi->active_windows[jj])
	  continue;

	if (*(sweep_links+jj))
	{
	  if (*(lock_list+jj))
	  {
	    // Uses same source sweep

	    checked_off[jj] = true;
	    last_wwptr->next_sweep = solo_return_wwptr(jj);
	    last_wwptr = last_wwptr->next_sweep;
	    last_wwptr->lead_sweep = lead_sweep;
	    spi->sweeps_in_lockstep[mm]++;
	  }
	  else
	  {
	    // Need to break sweep links to this frame because it is no
	    // longer part of the lock step

	    *(sweep_links+jj) = NO;
	    unlink[jj] = true;
	    nun++;
	  }
	}
      }

      if (nun != 0)
      {
	// Unlink should have flags for windows that are not part of the
	// current lock step but were linked to windows that are part of the
	// current lock step for this sweep. So we break the links.

	for (int jj = 0; jj < SOLO_MAX_WINDOWS; jj++)
	{
	  if (!spi->active_windows[jj])
	    continue;

	  if (unlink[jj])
	  {
	    // Links that were not part of the lock step should be preserved so
	    // unlink becomes the new set of links for these windows.

	    WW_PTR wwptr = solo_return_wwptr(jj);
	    for (int kk = 0; kk < SOLO_MAX_WINDOWS; kk++)
	      wwptr->sweep->linked_windows[kk] = unlink[kk];
	  } /* endif - unlink[jj] */
	} /* endfor - jj */
      } /* endif - nun != 0 */
    } /* endfor - ww */
  } /* endfor - mm */

  return nus;
}

/* c------------------------------------------------------------------------ */

int 
sp_nab_fixed_angle_from_fn (char *fn, double *fxd)
{
    float f;
    char *a=fn;
    const char *delim=".";

    if(!a || !strlen(a))
	  return(NO);

    for(; *a && *a != *delim; a++); /* file type */
    if(*a++ != *delim)
	  return(NO);
    for(; *a && *a != *delim; a++); /* time stamp */
    if(*a++ != *delim)
	  return(NO);
    for(; *a && *a != *delim; a++); /* radar_name */
    if(*a++ != *delim)
	  return(NO);
    for(; *a && *a != *delim; a++); /* version num */
    if(*a++ != *delim)
	  return(NO);
    if(!strstr(a, "_"))
	  return(NO);
    if(sscanf(a, "%f_", &f) != 1)
	  return(NO);
    *fxd = f;
    return(YES);
}
/* c------------------------------------------------------------------------ */

bool solo_parameter_setup (int frame_num)
{
  // Get the needed window information

  WW_PTR wwptr = solo_return_wwptr(frame_num);
  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);
  
  // Get the parameter number

  int pnum = data_info->getParamIndex(wwptr->parameter.parameter_name);

  // Check to see if the parameter has changed

  // NOTE:  Why is a different index used for the bad data value check than for
  // the scale/bias checks???

  if (pnum != wwptr->parameter_num ||
      wwptr->parameter_scale != data_info->getParamScale(pnum) ||
      wwptr->parameter_bias != data_info->getParamBias(pnum) ||
      wwptr->parameter_bad_val != data_info->getParamBadDataValue(wwptr->parameter_num))
  {
    wwptr->parameter.changed = YES;
  }

  // If this isn't a valid parameter, substitute a valid one.

  if (pnum < 0)
  {
    Glib::ustring message;
    
    wwptr->parameter.changed = YES;

    message = (Glib::ustring)"Parameter " + wwptr->parameter.parameter_name +
      " does not exist in this sweep";
    g_message(message.c_str());

    // Get the new parameter name

    pnum = (frame_num < (int)data_info->getNumParams()) ? frame_num :
      frame_num % data_info->getNumParams();
    strncpy(wwptr->parameter.parameter_name,
	    data_info->getParamNameClean(pnum, 8).c_str(), 16);

    message = (Glib::ustring)"Substituting " + wwptr->parameter.parameter_name;
    g_message (message.c_str());
  }
    
  // If we changed the parameter, we need to set up the window properly

  bool ok = true;
  
  if (wwptr->parameter.changed)
  {
    wwptr->parameter.changed = NO;
    wwptr->parameter_num = pnum;
    wwptr->parameter_scale =
      data_info->getParamScale(wwptr->parameter_num);
    wwptr->parameter_bias =
      data_info->getParamBias(wwptr->parameter_num);
    wwptr->parameter_bad_val = 
      (int32_t)data_info->getParamBadDataValue(wwptr->parameter_num);

    // Set up the color table for this frame

    if ((!solo_hardware_color_table(frame_num)))
      ok = false;

    // Create the data color look-up table

    solo_data_color_lut(frame_num);
  }

  return ok;
}
/* c------------------------------------------------------------------------ */

void 
sp_landmark_offsets (int frme)
{
    int ww;
    struct landmark_info *lmi;
    WW_PTR wwptr, wwptrx;
    double pxm;

    wwptr = solo_return_wwptr(frme);
    pxm = wwptr->view->magnification/sp_meters_per_pixel();
    lmi = wwptr->landmark_info;
    PointInSpace radar = wwptr->radar_location;

    if(!lmi->landmark_options) { /* local radar is the landmark */
	wwptr->landmark_x_offset = 0;
	wwptr->landmark_y_offset = 0;
	wwptr->landmark = wwptr->radar_location;
	wwptr->landmark.setId("SOL_LANDMARK");
 	return;
    }
    else if(SOLO_LINKED_POSITIONING SET_IN lmi->landmark_options) {
	ww = lmi->reference_frame -1;
	wwptrx = solo_return_wwptr(ww);
	wwptr->landmark = wwptrx->radar_location;
	wwptr->landmark.setId("SOL_LANDMARK");
    }

    radar.latLonRelative(wwptr->landmark);

    if (strncmp("AIR", wwptr->lead_sweep->sweep->scan_type, 3) == 0)
    {
      wwptr->landmark_x_offset = 1000. * radar.getX();
      wwptr->landmark_y_offset = 1000. * radar.getZ();
    }
    else
    {
      wwptr->landmark_x_offset = 1000. * radar.getX();
      wwptr->landmark_y_offset = 1000. * radar.getY();
    }
}
/* c------------------------------------------------------------------------ */

void 
sp_align_center (int frme)
{
    struct frame_ctr_info *fci;
    WW_PTR wwptr, wwptrf;

    wwptr = solo_return_wwptr(frme);

    fci = wwptr->frame_ctr_info;

    /*
     * make sure we've got the radar location
     */
    if(frme != wwptr->lead_sweep->window_num)
      wwptr->radar_location = wwptr->lead_sweep->radar_location;


    if(SOLO_FIXED_POSITIONING SET_IN fci->centering_options) {
      /*
       * "center" contains the lat/lon/alt of the fixed point
       * first calcualte (x,y,z) relative the to radar
       * which will come back in the radar pisp
       */
      wwptr->radar_location.latLonRelative(wwptr->center_of_view);
      sp_xyz_to_rtheta_screen(frme, wwptr->radar_location,
			      wwptr->center_of_view);
      /*
       * for possible future reference calculate the actual (x,y,z)
       * of the center of the screen based on r, theta, and tilt
       */
      sp_rtheta_screen_to_xyz(frme, wwptr->radar_location,
			      wwptr->center_of_view);
    }
    else if(SOLO_LINKED_POSITIONING SET_IN fci->centering_options) {
      /* reference frame */
      wwptrf = solo_return_wwptr(fci->reference_frame-1);
      /*
       * convert the r,theta of the screen center to and (x,y,z)
       * value relative the lat/lon/alt of the radar
       */
      sp_rtheta_screen_to_xyz(wwptrf->window_num, wwptrf->radar_location,
			      wwptrf->center_of_view);
      /* calculate true lat/lon/alt
       * of the center
       */
      wwptr->center_of_view.latLonShift(wwptrf->radar_location);

      if(frme != wwptrf->window_num) { /* not the reference frame */
	/* get center (x,y,z) relative to radar in frame
	 */
	wwptr->radar_location.latLonRelative(wwptr->center_of_view);
      }
      /* calculate r,theta for this frame
       */
      sp_xyz_to_rtheta_screen(frme, wwptr->radar_location,
			      wwptr->center_of_view);
      sp_rtheta_screen_to_xyz(frme, wwptr->radar_location,
			      wwptr->center_of_view);
    }
    else {			/* center is relative to the local radar
				 */
      sp_rtheta_screen_to_xyz(frme, wwptr->radar_location,
			      wwptr->center_of_view);
      wwptr->center_of_view.latLonShift(wwptr->radar_location);
    }
    wwptr->clicked_ctr_of_frame = NO;
    return;
}
