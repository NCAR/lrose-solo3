/* 	$Id$	 */

# define new_stuff

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef USE_RADX
#include <Radx/RadxSweep.hh>
#include <Radx/RadxRay.hh>
#endif

#include <dd_math.h>
#include <DataInfo.hh>
#include <DataManager.hh>
#include <DateTime.hh>
#include "sii_bscan.hh"
#include "sii_param_widgets.hh"
#include "sii_perusal.hh"
#include "sii_utils.hh"
#include "sii_xyraster.hh"
#include <solo_window_structs.h>
#include "solo2.hh"
#include "soloii.hh"
#include <SoloState.hh>
#include "sp_accepts.hh"
#include "sp_basics.hh"
#include "sp_dorade.hh"


// Local types

struct times_que
{
  struct times_que *last;
  struct times_que *next;
  int x_pxl;
  double time;
//    double delta;
  double delta2;
  double av_delta;
  double t_pxl;
};

struct ts_linked_frames
{
  struct ts_linked_frames *next;
  WW_PTR wwptr;
  WW_PTR lead_sweep;

  double time_span;
  double cell_num;
  double cells_per_pixel;
//  double sweep_start;

//  int sweep_count;
  int width;
  int height;
  int py0;
  int py_count;
  int type_of_plot;
    
  unsigned char *frame_image_ptr;
  unsigned char *at;

  struct times_que *tq;
};

// Forward declarations

void sii_bscan_allocate_globals();
int sp_ts_data_loop(struct ts_linked_frames *tsLink0);
void sp_ts_line(struct ts_linked_frames *tsl, int top_down);
void ts_ray_clone(char *aa, char *cc, int y_count, int frame_width, int nlines,
		  int top_down, int right_left);
void sp_tsYpixel_info(struct ts_linked_frames *tsLink, int top_down);

// Globals

static struct ts_linked_frames *original_sweep_set[SOLO_MAX_WINDOWS];
static struct ts_linked_frames *linked_frames[2*SOLO_MAX_WINDOWS];

# define TS_NUM_DELTAS 9

/* c------------------------------------------------------------------------ */

void sii_bscan_allocate_globals()
{
  for (int ii = 0; ii < 2 * SOLO_MAX_WINDOWS; ii++)
  {
    // Produce an array of linked frame structs to be used in sweep sets

    linked_frames[ii] = new struct ts_linked_frames;
    linked_frames[ii]->next = 0;
    linked_frames[ii]->wwptr = 0;
    linked_frames[ii]->lead_sweep = 0;
    linked_frames[ii]->time_span = 0.0;
    linked_frames[ii]->cell_num = 0.0;
    linked_frames[ii]->cells_per_pixel = 0.0;
//    linked_frames[ii]->sweep_start = 0.0;
//    linked_frames[ii]->sweep_count = 0;
    linked_frames[ii]->width = 0;
    linked_frames[ii]->height = 0;
    linked_frames[ii]->py0 = 0;
    linked_frames[ii]->py_count = 0;
    linked_frames[ii]->type_of_plot = 0;
    linked_frames[ii]->frame_image_ptr = 0;
    linked_frames[ii]->at = 0;
    linked_frames[ii]->tq = 0;
      
    struct ts_linked_frames *tsLink = linked_frames[ii];

    for (int jj = 0; jj < TS_NUM_DELTAS; jj++)
    {
      struct times_que *tq = new struct times_que;
      tq->last = 0;
      tq->next = 0;
      tq->x_pxl = 0;
      tq->time = 0.0;
//      tq->delta = 0.0;
      tq->delta2 = 0.0;
      tq->av_delta = 0.0;
      tq->t_pxl = 0.0;
      
      if (jj == 0)
      {
	tsLink->tq = tq;
	tsLink->tq->last = tq;
      }
      else
      {
	tq->last = tsLink->tq->last;
	tsLink->tq->last->next = tq;
      }		    
      tq->next = tsLink->tq;
      tsLink->tq->last = tq;
    }
  }
}


/* c------------------------------------------------------------------------ */

int sp_ts_data_loop (struct ts_linked_frames *tsLink0)
{
  static int dia_count = 0;
  static int line_count = 0;
  static int count = 0;
  static int ray_count = 0;

  // Get pointers to the needed information

  int lead_frame = tsLink0->lead_sweep->window_num;

  DataManager *data_mgr = DataManager::getInstance();
  WW_PTR wwptr = solo_return_wwptr(lead_frame);

  sii_update_frame_info(lead_frame);

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(lead_frame);

#ifdef USE_RADX
//  RadxVol &radx_vol = data_info->getRadxVolume();
//  RadxSweep *sweep = data_info->getRadxSweep();
//  std::vector< RadxRay* > rays = radx_vol.getRays();

  std::vector< FileInfo > file_list = data_info->getTimeSeriesFileList();
  
#endif

  int top_down = tsLink0->type_of_plot & TS_PLOT_DOWN;
  int right_left = tsLink0->type_of_plot & TS_PLOT_RIGHT_TO_LEFT;
  int automatic = tsLink0->type_of_plot & TS_AUTOMATIC;
  int msl_relative = tsLink0->type_of_plot & TS_MSL_RELATIVE;

  guint frame_sync_nums[SOLO_MAX_WINDOWS];
  
  for (struct ts_linked_frames *tsl = tsLink0; tsl; tsl = tsl->next)
  {
    int ww = tsl->wwptr->window_num;
    if (ww >= sii_return_frame_count())
      break;

    frame_sync_nums[ww] = sii_frame_sync_num(ww);
    sii_update_frame_info(ww);
    tsl->width = tsl->wwptr->view->width_in_pix;
    tsl->height = tsl->wwptr->view->height_in_pix;
    tsl->frame_image_ptr = (unsigned char *)tsl->wwptr->image->data;
    tsl->type_of_plot = tsl->wwptr->view->type_of_plot;

    // Calculate the starting pixel number, the gate num of this pixel
    // and the gate increment for each subsequent pixel in the Y direction

    sp_tsYpixel_info(tsl, top_down);
  }

  // Start avg. delta at one sec.

#ifndef USE_RADX
  tsLink0->tq->time = data_info->getTime() - 1.0;
#endif

  tsLink0->tq->av_delta = 0.2;

  // Check time between pixels

  double pxl_span = (wwptr->sweep->stop_time - wwptr->sweep->start_time) /
    ((double)tsLink0->width - 1.0);
  
  if (pxl_span <= 0)
    return 0;
  
  double rcp_pxl_span = 1.0 / pxl_span;

  // Start time is assummed to be in the center of the first pixel
  // and stop time is assumed to be in the center of the last pixel.
  // "t_pxl" is the trip time for associating the current ray with
  // the next pixel

  double sstime = wwptr->sweep->start_time - 0.5 * pxl_span;
  double t_pxl = sstime;
  int xmax = tsLink0->width - 1;
  int xinc = right_left ? -1 : 1;
  int x_pxl = right_left ? xmax + 1 : -1;

  double delta;
  int advance = 0;

#ifndef USE_RADX  
  if ((delta = data_info->getTime() - t_pxl) > 0)
  {
    // Skip to the first pixel that should contain data

    advance = (int)(delta * rcp_pxl_span);
  }
#endif

#ifndef USE_RADX
  t_pxl += (double)advance * pxl_span;
  int px_count = tsLink0->width - advance;
#endif
  struct ts_ray_table *tsrt = tsLink0->wwptr->tsrt;
  tsrt->sweep_count = -1;

  if (tsrt->max_ray_entries < tsLink0->width)
  {
    if (tsrt->tsri_top)
      free(tsrt->tsri_top);
    tsrt->max_ray_entries = tsLink0->width;
    int mm = tsrt->max_ray_entries * sizeof(struct ts_ray_info);
    tsrt->tsri_top = (struct ts_ray_info *)malloc(mm);
    memset(tsrt->tsri_top, 0, mm);
  }
  struct ts_sweep_info *tssi = tsrt->tssi_top;
  struct ts_ray_info *tsri = tsrt->tsri_top;
  struct ts_ray_info *tsri_right = tsrt->tsri_top + tsrt->max_ray_entries;

  // Initialize pixel/ray info

  for (; tsri < tsri_right; tsri++)
    tsri->ray_num = -1;

#ifndef USE_RADX
  x_pxl += (right_left ?  -advance : advance);
#endif
  dia_count = 0;

  // The pointers and pixel num are set up this way so when it is time 
  // to replicate they are refering to the last ray.
  //
  // Now really loop through the data

  double avg_delta = 0.0;

#ifdef USE_RADX

  bool done = false;
  bool first_ray = true;
  int px_count;
  
  for (std::vector< FileInfo >::const_iterator file_info = file_list.begin();
       file_info != file_list.end(); ++file_info)
  {
    // Read in the sweep file

    std::string file_path = data_info->getDir() + file_info->getFileName();
    
    RadxFile radx_file;
    RadxVol radx_vol;
    
    radx_file.clearRead();
    radx_file.setReadSweepNumLimits(0, 0);
    if (radx_file.readFromPath(file_path, radx_vol) != 0)
      break;
    
    radx_vol.convertToFl32();
    radx_vol.loadFieldsFromRays();
    const std::vector< RadxSweep* > sweeps = radx_vol.getSweeps();
    if (sweeps.size() == 0)
      break;
    
    RadxSweep *sweep = sweeps[0];
    std::vector< RadxRay* > rays = radx_vol.getRays();
    
    // Process each of the rays in the sweep

    for (std::size_t ray_index = sweep->getStartRayIndex();
	 ray_index <= sweep->getEndRayIndex(); ++ray_index)
    {
      const RadxRay *ray = rays[ray_index];

      // Do some initialization when processing the first ray

      if (first_ray)
      {
	tsLink0->tq->time = ray->getTimeDouble() - 1.0;
	if ((delta = ray->getTimeDouble() - t_pxl) > 0)
	{
	  // Skip to the first pixel that should contain data

	  advance = (int)(delta * rcp_pxl_span);
	}
	t_pxl += (double)advance * pxl_span;
	px_count = tsLink0->width - advance;
	x_pxl += (right_left ?  -advance : advance);
	first_ray = false;
      }
    
      delta = ray->getTimeDouble() - t_pxl;
      
      if (delta >= 0)
      {
	line_count++;
	int mm = (int)(delta * rcp_pxl_span);
	int nn = mm;
      
	// Make sure there isn't a gap in the data

	if (ray_count > TS_NUM_DELTAS && mm > 0 && delta > 1.1 * avg_delta)
	{
	  // Fill in the prev ray to the avg_delta time

	  mm = (int)(avg_delta * rcp_pxl_span);
	}

	// Do any replication that's needed

	if (nn > 0 && line_count == 1)
	{
	  // First time through but the data starts beyond the first
	  // pixel becaue the time resolution is higher than the
	  // data resolution

	  mm = 0;
	}

	if (mm > 0)
	{
	  // mm is the number of times we repeat the ray info

	  int xstop = x_pxl + (mm * xinc);
	
	  if (xstop < 0)
	    mm = x_pxl;
	  else if (xstop > xmax)
	    mm = xmax - x_pxl;

	  int jj = mm;
	  for (int kk = x_pxl + xinc;  jj-- ; kk += xinc)
	    memcpy(tsrt->tsri_top + kk, tsri, sizeof(struct ts_ray_info));

	  for (struct ts_linked_frames *tsl = tsLink0; tsl; tsl = tsl->next)
	  {
	    ts_ray_clone((char *)tsl->at, (char *)tsl->at + xinc, tsl->py_count,
			 tsl->width, mm, top_down, right_left);
	  }
	}

	if (nn > 0)
	{
	  x_pxl += nn * xinc;
	  px_count -= nn;
	  t_pxl += nn * pxl_span;
	}

	x_pxl += xinc;
	if (x_pxl < 0 || x_pxl > xmax)
	{
	  done = true;
	  break;
	}
	
	tsri = tsrt->tsri_top + x_pxl;

	for (struct ts_linked_frames *tsl = tsLink0; tsl; tsl = tsl->next)
	{
	  // For each frame using this sweep

	  count++;

	  if (automatic)
	    top_down = data_info->isInSector(ray->getAzimuthDeg(),
					     135.0, 225.0);
	  
	  if (msl_relative || automatic)
	    sp_tsYpixel_info(tsl, top_down);

	  int ww = tsl->wwptr->window_num;
	  if (frame_sync_nums[ww] != sii_config_sync_num ((guint)ww))
	    return -1;
	
	  // Assign colors

	  solo_color_cells(ww, ray);
	  tsl->at = tsl->frame_image_ptr + x_pxl +
	    tsl->py0 * tsl->width;
	  sp_ts_line(tsl, top_down);
	}
	 
	if (tsrt->sweep_count != data_mgr->getSweepNum())
	{
	  tsrt->sweep_count = data_mgr->getSweepNum();
	  if (tsrt->sweep_count + 1 > tsrt->max_sweep_entries)
	  {
	    tsrt->max_sweep_entries += 11;
	    tsrt->tssi_top = (struct ts_sweep_info *)
	      realloc(tsrt->tssi_top, tsrt->max_sweep_entries *
		      sizeof(struct ts_sweep_info));
	  }
	  tssi = tsrt->tssi_top + tsrt->sweep_count;
	  tssi->dir_num = data_mgr->getDirNum();
	  tssi->ray_count = data_mgr->getNumRays();
	  strcpy(tssi->directory, data_mgr->getRadarDir().c_str());
	  strcpy(tssi->filename, data_mgr->getRadarFilename().c_str());
	}
	t_pxl += pxl_span;
	tsri->ray_num = data_mgr->getRayNum() - 1;
	tsri->sweep_num = tsrt->sweep_count;
      }	/* if (1) { */

      // Check for stop event.

      if (SoloState::getInstance()->isHalt())
      {
	printf("HALTING\n");
	break;
      } 

      // NOTE: Here is where I took out the new file stuff

      // Compute the average time diff between rays so we can detect
      // time gaps

      struct ts_linked_frames *tsl = tsLink0;
      double delta2 = ray->getTimeDouble() - tsLink0->tq->time;
      struct times_que *tq = tsl->tq = tsl->tq->next;
      tq->time = ray->getTimeDouble();
      tq->x_pxl = x_pxl;
      tq->t_pxl = t_pxl;
      tq->delta2 = delta2;
      tq->av_delta = .2;

      int seed_deltas = TS_NUM_DELTAS;
    
      if (delta2 > 0)
      {
	if (TS_NUM_DELTAS - seed_deltas > 3 &&
	    tq->last->delta2 < 2.2 && tq->last->last->delta2 > 2.2)
	{
	  // We have a forward time glitch
	  t_pxl = tq->last->last->t_pxl;
	  tq->t_pxl = t_pxl;
	  x_pxl = tq->last->last->x_pxl;
	  tq->x_pxl = x_pxl;
	}

	double d = delta2 > .2 ? .2 : delta2;

	double sum_deltas = 0.0;
	double rcp_num_deltas = 1.0 / (double)TS_NUM_DELTAS;
      
	if (seed_deltas)
	{
	  seed_deltas--;
	  sum_deltas += d;
	  avg_delta = sum_deltas * rcp_num_deltas;
	}
	else
	{
	  sum_deltas -= tq->last->av_delta;
	  sum_deltas += d;
	  avg_delta = sum_deltas * rcp_num_deltas;
	}
	tq->av_delta = d;
	tq->delta2 = delta2;
      }
    }

    if (done)
      break;
    
    for (struct ts_linked_frames *tsl = tsLink0; tsl; tsl=tsl->next)
    {
      tsl->wwptr->uniform_cell_spacing = 0;
      solo_cell_lut(tsl->wwptr->window_num);
      sp_tsYpixel_info(tsl, top_down);
    }
  }
  
#else

  for (;;)
  {
    // For each ray

    delta = data_info->getTime() - t_pxl;

    if (delta >= 0)
    {
      line_count++;
      int mm = (int)(delta * rcp_pxl_span);
      int nn = mm;
      
      // Make sure there isn't a gap in the data

      if (ray_count > TS_NUM_DELTAS && mm > 0 && delta > 1.1 * avg_delta)
      {
	// Fill in the prev ray to the avg_delta time

	mm = (int)(avg_delta * rcp_pxl_span);
      }

      // Do any replication that's needed

      if (nn > 0 && line_count == 1)
      {
	// First time through but the data starts beyond the first
	// pixel becaue the time resolution is higher than the
	// data resolution

	mm = 0;
      }

      if (mm > 0)
      {
	// mm is the number of times we repeat the ray info

	int xstop = x_pxl + (mm * xinc);
	
	if (xstop < 0)
	  mm = x_pxl;
	else if (xstop > xmax)
	  mm = xmax - x_pxl;

	int jj = mm;
	for (int kk = x_pxl + xinc;  jj-- ; kk += xinc)
	  memcpy(tsrt->tsri_top + kk, tsri, sizeof(struct ts_ray_info));

	for (struct ts_linked_frames *tsl = tsLink0; tsl; tsl = tsl->next)
	{
	  ts_ray_clone((char *)tsl->at, (char *)tsl->at + xinc, tsl->py_count,
		       tsl->width, mm, top_down, right_left);
	}
      }

      if (nn > 0)
      {
	x_pxl += nn * xinc;
	px_count -= nn;
	t_pxl += nn * pxl_span;
      }

      x_pxl += xinc;
      if (x_pxl < 0 || x_pxl > xmax)
	break;

      tsri = tsrt->tsri_top + x_pxl;

      for (struct ts_linked_frames *tsl = tsLink0; tsl; tsl=tsl->next)
      {
	// For each frame using this sweep

	count++;

	if (automatic)
	  top_down = data_info->isInSector(135.0, 225.0);

	if (msl_relative || automatic)
	  sp_tsYpixel_info(tsl, top_down);

	int ww = tsl->wwptr->window_num;
	if (frame_sync_nums[ww] != sii_config_sync_num ((guint)ww))
	  return -1;
	
	// Assign colors

	solo_color_cells(ww);
	tsl->at = tsl->frame_image_ptr + x_pxl +
	  tsl->py0 * tsl->width;
	sp_ts_line(tsl, top_down);
      }
	 
      if (tsrt->sweep_count != data_mgr->getSweepNum())
      {
	tsrt->sweep_count = data_mgr->getSweepNum();
	if (tsrt->sweep_count + 1 > tsrt->max_sweep_entries)
	{
	  tsrt->max_sweep_entries += 11;
	  tsrt->tssi_top = (struct ts_sweep_info *)
	    realloc(tsrt->tssi_top, tsrt->max_sweep_entries *
		    sizeof(struct ts_sweep_info));
	}
	tssi = tsrt->tssi_top + tsrt->sweep_count;
	tssi->dir_num = data_mgr->getDirNum();
	tssi->ray_count = data_mgr->getNumRays();
	strcpy(tssi->directory, data_mgr->getRadarDir().c_str());
	strcpy(tssi->filename, data_mgr->getRadarFilename().c_str());
      }
      t_pxl += pxl_span;
      tsri->ray_num = data_mgr->getRayNum() - 1;
      tsri->sweep_num = tsrt->sweep_count;
    }	/* if (1) { */

    // Check for stop event.

    if (SoloState::getInstance()->isHalt())
    {
      printf("HALTING\n");
      break;
    } 

    // Nab the next ray

    if (!data_mgr->loadNextRay())
      break;

    // NOTE: Do I need to update this for Radx?

    if (data_info->isNewSweep())
    {
      for (struct ts_linked_frames *tsl = tsLink0; tsl; tsl=tsl->next)
      {
	tsl->wwptr->uniform_cell_spacing = 0;
	solo_cell_lut(tsl->wwptr->window_num);
	sp_tsYpixel_info(tsl, top_down);
      }
    }

    // NOTE: Do I need to update these for Radx?

    data_info->setNewSweep(false);
    data_info->setNewVolume(false);
      
    // Compute the average time diff between rays so we can detect
    // time gaps

    struct ts_linked_frames *tsl = tsLink0;
    double delta2 = data_info->getTime() - tsLink0->tq->time;
    struct times_que *tq = tsl->tq = tsl->tq->next;
    tq->time = data_info->getTime();
    tq->x_pxl = x_pxl;
    tq->t_pxl = t_pxl;
    tq->delta2 = delta2;
    tq->av_delta = .2;

    int seed_deltas = TS_NUM_DELTAS;
    
    if (delta2 > 0)
    {
      if (TS_NUM_DELTAS - seed_deltas > 3 &&
	  tq->last->delta2 < 2.2 && tq->last->last->delta2 > 2.2)
      {
	// We have a forward time glitch
	t_pxl = tq->last->last->t_pxl;
	tq->t_pxl = t_pxl;
	x_pxl = tq->last->last->x_pxl;
	tq->x_pxl = x_pxl;
      }

      double d = delta2 > .2 ? .2 : delta2;

      double sum_deltas = 0.0;
      double rcp_num_deltas = 1.0 / (double)TS_NUM_DELTAS;
      
      if (seed_deltas)
      {
	seed_deltas--;
	sum_deltas += d;
	avg_delta = sum_deltas * rcp_num_deltas;
      }
      else
      {
	sum_deltas -= tq->last->av_delta;
	sum_deltas += d;
	avg_delta = sum_deltas * rcp_num_deltas;
      }
      tq->av_delta = d;
      tq->delta2 = delta2;
    }
  }

#endif

  for (struct ts_linked_frames *tsl = tsLink0; tsl; tsl=tsl->next)
  {
    int ww = tsl->wwptr->window_num;
    sii_xfer_images(ww, 0);	
    sii_reset_reconfig_flags(ww);
  }

  return 0;
}

/* c------------------------------------------------------------------------ */

void sp_ts_line(struct ts_linked_frames *tsl, int top_down)
{
  // color in the line. Remember y is really upside down

  int fw = top_down ? tsl->width : -tsl->width;

  for(int nn = 0; nn < tsl->py_count; ++nn)
  {
    int ii = (int)(tsl->cell_num + (nn * tsl->cells_per_pixel));
    tsl->at[fw * nn] = (unsigned char)tsl->wwptr->cell_colors[ii];
  }
}

/* c------------------------------------------------------------------------ */

int tsDisplay(int click_frame, int command)
{
  static bool first_time = true;

  // If solo is busy, don't do anything

  if (SoloState::getInstance()->isHalt())
  {
    SoloState::getInstance()->setHaltFlag(false);
    return 1;
  }

  if(solo_busy())
    return 0;

  struct solo_perusal_info *spi = solo_return_winfo_ptr();

  // The first time through, allocate space for the globals

  if (first_time)
  {
    sii_bscan_allocate_globals();
    first_time = false;
  }    

  solo_set_busy_signal();
  main_window->changeCursor(false);

  // Get pointers to the window information

  WW_PTR wwptr = solo_return_wwptr(click_frame);
  WW_PTR wwptrx = wwptr->lead_sweep;
  int lead_frame = wwptrx->window_num;

  std::vector< int > frame_list;
//  int frame_list[SOLO_MAX_WINDOWS];
//  int frame_count = 0;
  
  if (command == REPLOT_THIS_FRAME)
  {
//    frame_list[frame_count++] = click_frame;
    frame_list.push_back(click_frame);
  }
  else
  {
    int total_frames = sii_return_frame_count();
    for (int frame = 0; frame < total_frames; frame++)
    {
      if (spi->active_windows[frame] && wwptr->lock->linked_windows[frame])
//	frame_list[frame_count++] = frame;
	frame_list.push_back(frame);
    }
  }

  // Loop through the window structures and clean up all of the link info
  // (I think)

  solo_worksheet();

  // Set up links between frames using the same sweep plus
  // an array of unique sweeps in case we are plotting more than
  // one time series

  bool checked_off[SOLO_MAX_WINDOWS];
  
//  for (int ii = 0; ii < frame_count; ii++)
//    checked_off[frame_list[ii]] = false;
  for (int ii = 0; ii < SOLO_MAX_WINDOWS; ii++)
    checked_off[ii] = false;

  int original_sweep_count = 0;
  int original_sweep_set_count = 0;

//  for (int ii = 0; ii < frame_count; ii++)
  for (std::size_t ii = 0; ii < frame_list.size(); ++ii)
  {
    int ww = frame_list[ii];
    
    if (checked_off[ww])
      continue;
    checked_off[ww] = true;

    // We are at the lead sweep for the next sweep set

    struct ts_linked_frames *tsLink = linked_frames[original_sweep_count++];
    original_sweep_set[original_sweep_set_count++] = tsLink;

    tsLink->wwptr = solo_return_wwptr(ww);
    
    tsLink->type_of_plot = tsLink->wwptr->view->type_of_plot;
//    tsLink->sweep_count = -1;
    tsLink->lead_sweep = wwptr->lead_sweep;
    tsLink->next = NULL;

//    if (frame_count == 1)
    if (frame_list.size() == 1)
      break;

    for (WW_PTR wwptr = tsLink->wwptr->next_sweep; wwptr != 0;
	 wwptr = wwptr->next_sweep)
    {
      if (wwptr->window_num < sii_return_frame_count())
      {
	tsLink->next = linked_frames[original_sweep_count++];
	tsLink = tsLink->next;
	tsLink->wwptr = wwptr;
	tsLink->lead_sweep = wwptr->lead_sweep;
	tsLink->next = NULL;
      }
      checked_off[wwptr->window_num] = true;
    }
  }
  
  bool broke = false;
  bool one_pass = true;
  if (command == FORWARD_FOREVER || command == BACKWARDS_FOREVER)
    one_pass = false;

  int return_flag = 0;
  
  for(;;)
  {
    for (int sset = 0; sset < original_sweep_set_count; sset++)
    {
      struct ts_linked_frames *tsLink0 = original_sweep_set[sset];
      struct ts_linked_frames *tsLink = tsLink0;
      
      lead_frame = tsLink->lead_sweep->window_num;
      wwptrx = tsLink->lead_sweep;

      switch(command)
      {
      case BACKWARDS_FOREVER:
      case BACKWARDS_ONCE:
	if ((tsLink->time_span = wwptrx->sweep->stop_time -
	     wwptrx->sweep->start_time) > 0)
	{
	  wwptrx->sweep->start_time -= tsLink->time_span;
	  wwptrx->sweep->stop_time -= tsLink->time_span;
	}
	break;
	    
      case FORWARD_FOREVER:
      case FORWARD_ONCE:
	if ((tsLink->time_span = wwptrx->sweep->stop_time -
	     wwptrx->sweep->start_time) > 0)
	{
	  wwptrx->sweep->start_time += tsLink->time_span;
	  wwptrx->sweep->stop_time += tsLink->time_span;
	}
	break;
	    
      default:
	break;
      }

      if (DataManager::getInstance()->compileFileList(lead_frame,
						      wwptrx->sweep->directory_name) < 1 )
      {
	char message[256];
	sprintf(message, "Directory %s contains no sweep files\n",
		wwptrx->sweep->directory_name);
	sii_message(message);
	broke = true;
	return_flag = 1;
	break;
      }

      if ((wwptrx->sweep->radar_num =
	   DataManager::getInstance()->getRadarNumData(lead_frame,
						       wwptrx->sweep->radar_name)) < 0)
      {
	char message[256];
	sprintf(message, "No sweep files for radar %s\n",
		wwptrx->sweep->radar_name);
	sii_message(message);
	broke = true;
	return_flag = 1;
	break;
      }

      if ((tsLink->time_span = wwptrx->sweep->stop_time -
	   wwptrx->sweep->start_time) <= 0)
	tsLink->time_span = 20;

      if (!wwptrx->sweep->start_time)
	wwptrx->sweep->start_time = wwptrx->sweep->time_stamp;

      wwptrx->sweep->stop_time = wwptrx->sweep->start_time + tsLink->time_span;
	
      DateTime start_time;
      start_time.setTimeStamp(wwptrx->sweep->start_time);
      wwptrx->lead_sweep->start_time_text = start_time.toString();
      DateTime stop_time;
      stop_time.setTimeStamp(wwptrx->sweep->stop_time);
      wwptrx->lead_sweep->stop_time_text = stop_time.toString();
	    
      DataManager *data_mgr = DataManager::getInstance();

#ifdef USE_RADX

      DataInfo *data_info =
	data_mgr->getWindowInfo(wwptrx->window_num);
      
      data_info->setTimeSeriesFileList(DateTime(wwptrx->sweep->start_time),
				       DateTime(wwptrx->sweep->stop_time));
      
#else

      // First see if there are any sweeps in the current time span

      data_mgr->setDirNum(lead_frame);
      data_mgr->setRadarNum(lead_frame);
      data_mgr->setRadarNumData(wwptrx->sweep->radar_num);
      data_mgr->setVersion(DataManager::LATEST_VERSION);
      data_mgr->setNewVersionFlag(false);
      data_mgr->setRadarDir(wwptrx->sweep->directory_name);

      // NOTE: These start/stop times match the start/stop times showing in
      // the sweepfile window (although the times in the sweepfile window don't
      // update while the window is open.  they update if you close and reopen
      // the window)

      if (data_mgr->generateSweepFileList(wwptrx->sweep->start_time,
					  wwptrx->sweep->stop_time) <= 0)
      {
	char message[1024];
	sprintf(message,
		"Unable to produce sweep file list for radar: %s\nin %s\nFrom %s to %s",
		wwptrx->sweep->radar_name,
		wwptrx->sweep->directory_name,
		wwptrx->lead_sweep->start_time_text.c_str(),
		wwptrx->lead_sweep->stop_time_text.c_str());
	sii_message(message);
	broke = true;
	return_flag = 1;
	break;
      }

      std::string file_name = wwptrx->sweep->file_name;
	    
//      tsLink->sweep_start = 
      double sweep_start =
	data_mgr->getTimeSeriesStartTimeInfo(lead_frame,
					     wwptrx->sweep->radar_num,
					     wwptrx->sweep->start_time,
					     wwptrx->show_file_info,
					     file_name);
      strncpy(wwptrx->sweep->file_name, file_name.c_str(), 128);
//      wwptrx->sweep->time_stamp = (int32_t) tsLink->sweep_start;	      
      wwptrx->sweep->time_stamp = (int32_t)sweep_start;
      
      // NOTE: This time matches the time of the sweep as specified in the
      // file name.  As I arrow over, this changes to the next file in the
      // directory that matches the specified radar name.

      // NOTE: This loads the list of sweep file names into the global usi
      // structure (dis->usi_top)

//      if (data_mgr->generateSweepFileList(tsLink->sweep_start,
      if (data_mgr->generateSweepFileList(sweep_start,
					  wwptrx->sweep->stop_time) <= 0)
      {
	char message[1024];
	sprintf(message,
		"Unable to produce sweep file list for radar: %s in %s From %s to %s",
		wwptrx->sweep->radar_name,
		wwptrx->sweep->directory_name,
		wwptrx->lead_sweep->start_time_text.c_str(),
		wwptrx->lead_sweep->stop_time_text.c_str());
	sii_message(message);
	broke = true;
	return_flag = 1;
	break;
      }

      // NOTE: One of the above calls must have the side-effect of rewinding
      // the current file so that the first ray is reread.  I'm not sure where
      // that is happening.

      // Get the first ray

      if (!data_mgr->loadNextRay())
      {
	char message[1024];
	sprintf(message, "Problems reading first ray of %s/%s\n",
		data_mgr->getRadarDir().c_str(),
		data_mgr->getRadarFilename().c_str());
	sii_message(message);
	broke = true;
	return_flag = 1;
	break;
      }

#endif
      
      {
	
	printf("plot: %s to %s  %.3f - %.3f\n",
		wwptrx->lead_sweep->start_time_text.c_str(),
		wwptrx->lead_sweep->stop_time_text.c_str(),
		wwptrx->sweep->start_time,
		wwptrx->sweep->stop_time);
      }

      for (; tsLink != 0; tsLink = tsLink->next)
      {
	// For each linked sweep

	WW_PTR wwptr = tsLink->wwptr;
	int ww = wwptr->window_num;
	solo_parameter_setup(ww);
	solo_ww_top_line(ww);
	wwptr->uniform_cell_spacing = 0;
	solo_cell_lut(ww);
      }

      // Loop through the data for this sweep set

      sp_ts_data_loop(tsLink0);

      // Update sweep info in all frames just plotted

      tsLink = tsLink0->next;
      for (; tsLink; tsLink = tsLink->next)
      {
	solo_cpy_sweep_info(lead_frame, tsLink->wwptr->window_num);
	SweepfileWindow *sweepfile_window =
	  frame_configs[tsLink->wwptr->window_num]->sweepfile_window;
	if (sweepfile_window != 0)
	  sweepfile_window->update();
	ParameterWindow *param_window =
	  frame_configs[tsLink->wwptr->window_num]->param_window;
	if (param_window != 0)
	  param_window->update();
	ViewWindow *view_window =
	  frame_configs[tsLink->wwptr->window_num]->view_window;
	if (view_window != 0)
	  view_window->update();
	if (tsLink->wwptr->window_num != lead_frame)
	{
	  tsLink->wwptr->start_time_text =
	    tsLink->wwptr->lead_sweep->start_time_text;
	  tsLink->wwptr->stop_time_text =
	    tsLink->wwptr->lead_sweep->stop_time_text;
	}
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
  }

  solo_clear_busy_signal();
  main_window->changeCursor(true);

  return return_flag;
}

/* c------------------------------------------------------------------------ */

void 
ts_ray_clone (char *aa, char *cc, int y_count, int frame_width, int nlines, int top_down, int right_left)
{
    /* copies a vertical line of data from left to right and bottom up
     * "aa" points to the bottom pixel of the vertical line to be cloned
     * "cc" points to the bottom pixel along the same horizontal line where
     *     the cloning is to begin
     */
    int nn, fw, rl;
    char *bb, *dd;
    /*
     * 
     */
    fw = top_down ? frame_width : -frame_width;
    rl = right_left ? -1 : 1;

    for(; nlines--; cc += rl) {
	for(bb=aa,dd=cc,nn=y_count;  nn--;  bb += fw, dd += fw) {
	    *dd = *bb;
	}
    }
}
/* c------------------------------------------------------------------------ */

void 
sp_tsYpixel_info (struct ts_linked_frames *tsLink, int top_down)
{
    int ii, nc, ymax, mark;
    struct ts_linked_frames *tsl=tsLink;
    double d, rmax, rmin, gs, rcp_gs;
    double rloc, rctr, rbottom, ppm, mpp, rtop;
    char mess[256];

    /* first pretend that (0,0) is in the lower left corner and
     * we are plotting increasing range from bottom to top
     */
    DataInfo *data_info =
      DataManager::getInstance()->getWindowInfo(tsl->lead_sweep->window_num);
    nc = tsl->wwptr->number_cells -1;
    gs = tsl->wwptr->uniform_cell_spacing;
    rcp_gs = 1./gs;
    rmin = tsl->wwptr->uniform_cell_one;
    rmax = rmin + nc * gs;
    mpp = sp_meters_per_pixel()/tsl->wwptr->view->ts_magnification;
    ppm = 1./mpp;
    rctr = KM_TO_M(tsl->wwptr->view->ts_ctr_km);
    tsl->cells_per_pixel = mpp * rcp_gs;
    tsl->py_count = tsl->height;
    rbottom = rctr - .5 * (double)(tsl->py_count-1) * mpp;
    rtop = rctr + .5 * (double)(tsl->py_count-1) * mpp;
    ymax = tsl->height -1;


    if(tsl->type_of_plot & TS_MSL_RELATIVE) {
      d = KM_TO_M(data_info->getPlatformAltitudeMSL());
	rloc = top_down ? d - rmin : d + rmin;
    }
    else {
	rloc = top_down ? 2. * rctr : 0;
    }

    if(top_down) {
	if((d = rtop - rloc) > 0) {
	    /* data starts below the top pixel
	     */
	    tsl->py0 = (int)(d * ppm);
	    tsl->py_count -= tsl->py0 +1;
	    tsl->cell_num = 0.5;
	}
	else {
	    tsl->py0 = 0;
	    tsl->cell_num = (rloc - rtop) * rcp_gs +.5;
	}
	if((d = (rloc - rmax) - rbottom) > 0) {	/* data stops before bottom */
	    tsl->py_count -= (int)(d * ppm +.5);
	}
    }
    else {			/* bottom up...reverse y coord */
	if((d = rloc - rbottom) > 0) {
	    /* data starts above the bottom pixel
	     */
	    ii = (int)(d * ppm);
	    tsl->py0 = ymax -ii;
	    tsl->py_count -= ii +1;
	    tsl->cell_num = 0.5;
	}
	else {
	    tsl->py0 = ymax;
	    tsl->cell_num = (rbottom -rloc) * rcp_gs +.5;
	}
	if((d = rtop - (rloc + rmax)) > 0) {
	    tsl->py_count -= (int)(d * ppm +.5);
	}
    }

    if(tsl->py_count < 0 || tsl->py0 < 0 || tsl->py0 > ymax) {
	sprintf (mess, "tsl->py_count < 0 : %d %.3f %.3f %.3f %.3f\n",
		 tsl->py_count, data_info->getPlatformAltitudeMSL(),
		 rmin, rmax, rloc);
	printf (mess);
	tsl->py_count = 0;
	mark = 0;
    }
}
