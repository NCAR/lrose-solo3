/* 	$Id$	 */

#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#include <Radx/RadxSweep.hh>
#include <Radx/RadxVol.hh>
#endif

#include <dd_math.h>
#include <DataManager.hh>
#include <DataInfo.hh>
#include <DateTime.hh>
#include <EarthRadiusCalculator.hh>
#include <running_avg_que.hh>
#include <se_proc_data.hh>
#include <se_utils.hh>
#include <se_bnd.hh>
#include <SeBoundaryList.hh>
#include <sii_utils.hh>
#include <solo_editor_structs.h>
#include <solo_list_widget_ids.h>
#include <solo_window_structs.h>
#include <solo2.hh>
#include <SoloState.hh>
#include <sp_basics.hh>
#include <sp_clkd.hh>
#include <UiCommandFactory.hh>

extern GString *gs_complaints;

typedef enum {
   SED_ZERO,
   SED_FIELD,
   SED_INTEGER,
   SED_REAL,
   SED_WHERE
} cmd_data_types;


/* c------------------------------------------------------------------------ */

void se_cull_setup_input(std::vector< std::string > &input_list)
{  
  // Routine to cull setup-input commands from a list of commands

  for (std::size_t i = 0; i < input_list.size(); ++i)
  {
    std::string input = input_list[i];
    
    // Ignore leading blanks

    std::size_t nonblank_pos = input.find_first_not_of(" \t");
    if (nonblank_pos != std::string::npos)
      input = input.substr(nonblank_pos);
    
    if (input.compare(0, 7, "sweep-directory", 0, 7) == 0 ||
	input.compare(0, 7, "radar-names", 0, 7) == 0 ||
	input.compare(0, 7, "first-sweep", 0, 7) == 0 ||
	input.compare(0, 7, "last-sweep", 0, 7) == 0 ||
	input.compare(0, 4, "version", 0, 4) == 0 ||
	input.compare(0, 7, "new-version", 0, 7) == 0 ||
	input.compare(0, 9, "no-new-version", 0, 9) == 0)
    {
      input_list.erase(input_list.begin() + i);
    }
  }
}

/* c------------------------------------------------------------------------ */

void dd_edd(const int time_series, const int automatic, const int down,
	    const double d_ctr, const int frame_num)
{
  static const std::string method_name = "se_proc_data::dd_edd()";
  
  static PointInSpace radar;

  // Get the list of boundaries entered by the user

  SeBoundaryList *sebs = SeBoundaryList::getInstance();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Get a pointer to the data manager

  DataManager *data_mgr = DataManager::getInstance();
  
  data_mgr->setEditFlag(true);
  data_mgr->setOutputFlag(true);

  // Get the field information

#ifdef USE_RADX
  DataInfo *data_info = data_mgr->getWindowInfo(frame_num);
#else
  int radar_num = data_mgr->getRadarNum();
  DataInfo *data_info = data_mgr->getWindowInfo(radar_num);

  data_info->setCompressionScheme(HRD_COMPRESSION);
#endif
  
  data_info->setDiskOutput(true);
  
  // Get the directory name, adding the needed slash to the end

  data_info->setDir(data_mgr->getRadarDir());
  
  // Initialize the counts

  seds->process_ray_count = 0;
  seds->volume_count = 0;
  seds->sweep_count = 0;
  seds->setup_input_ndx = 0;
  seds->modified = NO;
#ifdef USE_RADX
#else
  seds->finish_up = NO;
#endif

  // Count the number of boundaries

  int num_boundaries = 0;
  
  for (struct one_boundary *ob = sebs->firstBoundary; ob != 0; ob = ob->next)
  {
    if(ob->num_points > 2)
      num_boundaries += ob->num_points;
  }

  seds->boundary_exists = num_boundaries ? YES : NO;
   
  // Process each ray

#ifdef USE_RADX

  RadxVol &radx_vol = data_info->getRadxVolume();
  RadxSweep *sweep = data_info->getRadxSweep();
  if (sweep == 0)
  {
    std::cerr << "ERROR: " << method_name << std::endl;
    std::cerr << "No current sweep -- cannot do editing" << std::endl;

    return;
  }
  
  std::vector< RadxRay* > rays = radx_vol.getRays();
  
  for (std::size_t ray_index = sweep->getStartRayIndex();
       ray_index <= sweep->getEndRayIndex(); ++ray_index)
  {
    if (SoloState::getInstance()->isHalt())
    {
      g_string_sprintfa (gs_complaints,"HALTING\n");
      break;
    } 

    RadxRay *ray = rays[ray_index];
    
    bool airborne = false;
    bool not_aligned = false;
    
    if (seds->process_ray_count == 0)
    {
      // First time through tack on the seds cmds

      data_info->addCommands(seds->edit_summary);
	
      if (seds->boundary_exists)
      {
	radar.setLatitude(data_info->getPlatformLatitude());
	radar.setLongitude(data_info->getPlatformLongitude());
	radar.setAltitude(data_info->getPlatformAltitudeMSL());
	radar.setEarthRadius(EarthRadiusCalculator::getInstance()->getRadius(radar.getLatitude()));
	radar.setTilt(data_info->getFixedAngle());
	    
	radar.latLonRelative(sebs->getOrigin());

	// radar->x is now the x coordinate of the boundary origin relative
	// to the radar.  Check to see if the boundary origin and the radar
	// origin are within 100m of each other.

	not_aligned = (SQ(radar.getX()) + SQ(radar.getY())) > 0.1;
	airborne = data_info->getScanMode() == DD_SCAN_MODE_RHI ||
	  !(data_info->getRadarType() == DD_RADAR_TYPE_GROUND ||
	    data_info->getRadarType() == DD_RADAR_TYPE_SHIP);
	    
	for (struct one_boundary *ob = sebs->firstBoundary; ob != 0;
	     ob = ob->next)
	{
	  if (ob->num_points > 2)
	  {
	    struct bnd_point_mgmt *bpm = ob->top_bpm;
	    
	    for (int mm = 0; mm < ob->num_points; bpm = bpm->next, ++mm)
	    {
	      bpm->_x = bpm->x;
	      bpm->_y = bpm->y;
	    }

	    // Check to see if the radar is inside of the boundary.
	    // ob->radar_inside_boundary is set to reflect this situation.

	    se_radar_inside_bnd(ob);
	  }
	}
      } /* boundary exists? */
    } /* first time through */

    // NOTE: Do editor commands edit more than one sweep???

//    if (data_info->isNewVolume())
//    {
//      data_info->startNewVolume();
//      
//      seds->volume_count++;
//      seds->volume_ray_count = 0;
//      data_info->setNewSweep(true);
//    }
//
//    if (data_info->isNewSweep())
//    {
//      seds->sweep_count++;
//      seds->sweep_ray_count = 0;
//    }

    seds->process_ray_count++;
    seds->sweep_ray_count++;
    seds->volume_ray_count++;
//    int nc = data_info->getClipGate() + 1;
    std::size_t num_gates = ray->getNGates();
    seds->num_segments = 0;
      
    if (seds->boundary_exists)
    {
      seds->use_boundary = YES;
      sebs->currentBoundary = sebs->firstBoundary;
      double az = ray->getAzimuthDeg();
//      double d = az;
      double max_range = ray->getMaxRangeKm() * 1000.0;
      seds->boundary_mask = seds->boundary_mask_array;
      unsigned short *bnd = seds->boundary_mask_array;
//      num_boundaries = nc + 1;

      int bflag;
	
      if (sebs->operateOutsideBnd)
      {
	bflag = 0;
	for (std::size_t ii = 0; ii < num_gates; ii++)
	  bnd[ii] = 1;
      }
      else
      {
	bflag = 1;
	memset(bnd, 0, num_gates * sizeof(unsigned short));
      }

      bool shift_bnd = false;
	
      // NOTE: Need to think about this!  What does it mean if this is
      // a new sweep???

//      if (data_info->isNewSweep())
//      {
//	radar.setLatitude(data_info->getPlatformLatitude());
//	radar.setLongitude(data_info->getPlatformLongitude());
//	radar.setAltitude(data_info->getPlatformAltitudeMSL());
//	radar.setEarthRadius(EarthRadiusCalculator::getInstance()->getRadius(radar.getLatitude()));
//	radar.setTilt(data_info->getFixedAngle());
//	radar.setElevation(data_info->getElevationAngleCalc());
//	radar.setAzimuth(data_info->getAzimuthAngleCalc());
//	radar.setHeading(data_info->getPlatformHeading());
//
//	if (airborne)
//	{
//	  shift_bnd = false;
//	}
//	else if (not_aligned)
//	{
//	  shift_bnd = true;
//	}
//	else if (FABS(radar.getTilt() - sebs->getOrigin().getTilt()) > 0.2)
//	{
//	  // Boundary and radar origin are the same but not the same tilt
//	  
//	  shift_bnd = true;
//	}
//	else
//	{
//	  shift_bnd = false;
//	}
//      }

      // For each boundary, set up the mask

      for (struct one_boundary *ob = sebs->firstBoundary; ob != 0;
	   ob = ob->next)
      {
	if (ob->num_points < 3)
	  continue;
	
	// NOTE:  Again, what does new sweep mean???

//	if (data_info->isNewSweep())
//	{
//	  if (shift_bnd && !time_series)
//	  {
//	    se_shift_bnd(ob, sebs->getOrigin(), radar,
//			 data_info->getScanMode(),
//			 data_info->getTiltAngle());
//	  }
//
//	  if (time_series || (not_aligned && !airborne))
//	  {
//	    // See if radar inside this boundary
//
//	    se_radar_inside_bnd(ob);
//	  }
//	}

	if (time_series)
	{
	  // NOTE:  Need to update getTime() call.

	  se_ts_find_intxns(radx_vol.getPlatform().getAltitudeKm(),
			    max_range, ob,
			    data_info->getTime(),
			    sweep->getFixedAngleDeg(),
			    automatic, down, d_ctr);
	}
	else
	{
	  xse_find_intxns(az, max_range, ob);
	}

	xse_num_segments(ob);
	seds->num_segments += ob->num_segments;

	for (int ii = 0; ii < ob->num_segments; ii++)
	{
	  double range1_m;
	  double range2_m;
	    
	  se_nab_segment(ii, range1_m, range2_m, ob);
	  if (range2_m <= 0.0)
	    continue;

	  int g1 = ray->getGateIndex(range1_m / 1000.0);
	  int g2 = ray->getGateIndex(range2_m / 1000.0) + 1;

	  for (; g1 < g2; g1++)
	  {
	    // Set boundary flags

	    bnd[g1] = bflag;
	  }
	} /* end segments loop */
      } /* end boundary for loop */
	 
    } /* boundary exists */
    else
    {
      // No boundary

      seds->use_boundary = NO;
      seds->boundary_mask = seds->all_ones_array;
    }

    // Loop through the for-each-ray operations

    se_perform_fer_cmds(seds->for_each_ray_cmds, frame_num, *ray);

    if (seds->punt)
      break;

    // NOTE: This code writes each updated ray.  I think we want to write
    // each updated sweep.

//    if (seds->modified)
//    {
//      data_info->writeRay();
//
//      // NOTE:  Is this just checking for successful writing?  If so, shouldn't
//      // we just return a boolean from writeRay() above instead?
//
//      if (data_info->getOutputFileId() < 0)
//      {
//	// Unable to open the new sweepfile
//
//	seds->punt = YES;
//	break;
//      }
//	    
//      if (data_mgr->isLastRayInSweep())
//      {
//	// End of current sweep
//	data_mgr->flushFile(radar_num);
//      }
//
//      if (data_mgr->getRayNum() != 0)
//      {
//	data_mgr->checkSweepFile();
//      }
//
//      if (data_info->isNewSweep())
//      {
//	g_string_sprintfa(gs_complaints,"%s",
//			  DataManager::getInstance()->getDDopenInfo().c_str());
//      }
//    }
//    
//    data_info->setNewSweep(false);
//    data_info->setNewVolume(false);
//    
//    if (data_mgr->getSweepNum() > data_mgr->getNumSweeps())
//      break;

  } /* end of loop through data */

  data_info->writeRadxVolume();

#else
      
  int ray_num = 0;
  
  for (;;)
  {
    ray_num++;
    
    if (SoloState::getInstance()->isHalt())
    {
      g_string_sprintfa (gs_complaints,"HALTING\n");
      break;
    } 

    if (!data_mgr->loadNextRay())
      break;
    
    bool airborne = false;
    bool not_aligned = false;
    
    if (seds->process_ray_count == 0)
    {
      // First time through tack on the seds cmds

      data_info->addCommands(seds->edit_summary);
	
      if (seds->boundary_exists)
      {
	radar.setLatitude(data_info->getPlatformLatitude());
	radar.setLongitude(data_info->getPlatformLongitude());
	radar.setAltitude(data_info->getPlatformAltitudeMSL());
	radar.setEarthRadius(EarthRadiusCalculator::getInstance()->getRadius(radar.getLatitude()));
	radar.setTilt(data_info->getFixedAngle());
	    
	radar.latLonRelative(sebs->getOrigin());

	// radar->x is now the x coordinate of the boundary origin relative
	// to the radar.  Check to see if the boundary origin and the radar
	// origin are within 100m of each other.

	not_aligned = (SQ(radar.getX()) + SQ(radar.getY())) > 0.1;
	airborne = data_info->getScanMode() == DD_SCAN_MODE_RHI ||
	  !(data_info->getRadarType() == DD_RADAR_TYPE_GROUND ||
	    data_info->getRadarType() == DD_RADAR_TYPE_SHIP);
	    
	for (struct one_boundary *ob = sebs->firstBoundary; ob != 0;
	     ob = ob->next)
	{
	  if (ob->num_points > 2)
	  {
	    struct bnd_point_mgmt *bpm = ob->top_bpm;
	    
	    for (int mm = 0; mm < ob->num_points; bpm = bpm->next, ++mm)
	    {
	      bpm->_x = bpm->x;
	      bpm->_y = bpm->y;
	    }

	    // Check to see if the radar is inside of the boundary.
	    // ob->radar_inside_boundary is set to reflect this situation.

	    se_radar_inside_bnd(ob);
	  }
	}
      } /* boundary exists? */
    } /* first time through */

    if (data_info->isNewVolume())
    {
      data_info->startNewVolume();
      
      seds->volume_count++;
      seds->volume_ray_count = 0;
      data_info->setNewSweep(true);
    }

    if (data_info->isNewSweep())
    {
      seds->sweep_count++;
      seds->sweep_ray_count = 0;
    }

    seds->process_ray_count++;
    seds->sweep_ray_count++;
    seds->volume_ray_count++;
    int nc = data_info->getClipGate() + 1;
    seds->num_segments = 0;
      
    if (seds->boundary_exists)
    {
      seds->use_boundary = YES;
      sebs->currentBoundary = sebs->firstBoundary;
      double az = data_info->getRotationAngleCalc();
      double d = az;
      double max_range = data_info->getGateRangeC(data_info->getNumCellsC() - 1);
      seds->boundary_mask = seds->boundary_mask_array;
      unsigned short *bnd = seds->boundary_mask_array;
      num_boundaries = nc + 1;

      int bflag;
	
      if (sebs->operateOutsideBnd)
      {
	bflag = 0;
	for (int ii = 0; ii < num_boundaries; ii++)
	  bnd[ii] = 1;
      }
      else
      {
	bflag = 1;
	memset(bnd, 0, num_boundaries * sizeof(*seds->boundary_mask));
      }

      bool shift_bnd = false;
	
      if (data_info->isNewSweep())
      {
	radar.setLatitude(data_info->getPlatformLatitude());
	radar.setLongitude(data_info->getPlatformLongitude());
	radar.setAltitude(data_info->getPlatformAltitudeMSL());
	radar.setEarthRadius(EarthRadiusCalculator::getInstance()->getRadius(radar.getLatitude()));
	radar.setTilt(data_info->getFixedAngle());
	radar.setElevation(data_info->getElevationAngleCalc());
	radar.setAzimuth(data_info->getAzimuthAngleCalc());
	radar.setHeading(data_info->getPlatformHeading());

	if (airborne)
	{
	  shift_bnd = false;
	}
	else if (not_aligned)
	{
	  shift_bnd = true;
	}
	else if (FABS(radar.getTilt() - sebs->getOrigin().getTilt()) > 0.2)
	{
	  // Boundary and radar origin are the same but not the same tilt
	  
	  shift_bnd = true;
	}
	else
	{
	  shift_bnd = false;
	}
      }

      // For each boundary, set up the mask

      for (struct one_boundary *ob = sebs->firstBoundary; ob != 0;
	   ob = ob->next)
      {
	if (ob->num_points < 3)
	  continue;
	
	if (data_info->isNewSweep())
	{
	  if (shift_bnd && !time_series)
	  {
	    se_shift_bnd(ob, sebs->getOrigin(), radar,
			 data_info->getScanMode(),
			 data_info->getTiltAngle());
	  }

	  if (time_series || (not_aligned && !airborne))
	  {
	    // See if radar inside this boundary

	    se_radar_inside_bnd(ob);
	  }
	}

	if (time_series)
	{
	  se_ts_find_intxns(data_info->getPlatformAltitudeMSL(), max_range, ob,
			    data_info->getTime(),
			    data_info->getTimeSeriesPointingAngle(),
			    automatic, down, d_ctr);
	}
	else
	{
	  xse_find_intxns(d, max_range, ob);
	}

	xse_num_segments(ob);
	seds->num_segments += ob->num_segments;

	for (int ii = 0; ii < ob->num_segments; ii++)
	{
	  double range1;
	  double range2;
	    
	  se_nab_segment(ii, range1, range2, ob);
	  if (range2 <= 0)
	    continue;

	  float r1 = range1;
	  float r2 = range2;
	  int g1 = data_info->getGateNum(r1);
	  int g2 = data_info->getGateNum(r2) + 1;

	  for (; g1 < g2; g1++)
	  {
	    // Set boundary flags

	    bnd[g1] = bflag;
	  }
	} /* end segments loop */
      } /* end boundary for loop */
	 
    } /* boundary exists */
    else
    {
      // No boundary

      seds->use_boundary = NO;
      seds->boundary_mask = seds->all_ones_array;
    }

    // Loop through the for-each-ray operations
    
    se_perform_fer_cmds(seds->for_each_ray_cmds);
    
    if (seds->punt)
      break;
    
    if (seds->modified)
    {
      data_info->writeRay();

      // NOTE:  Is this just checking for successful writing?  If so, shouldn't
      // we just return a boolean from writeRay() above instead?

      if (data_info->getOutputFileId() < 0)
      {
	// Unable to open the new sweepfile

	seds->punt = YES;
	break;
      }
	    
      if (data_mgr->isLastRayInSweep())
      {
	// End of current sweep
	data_mgr->flushFile(radar_num);
      }

      if (data_mgr->getRayNum())
      {
	data_mgr->checkSweepFile();
      }

      if (data_info->isNewSweep())
      {
	g_string_sprintfa(gs_complaints,"%s",
			  DataManager::getInstance()->getDDopenInfo().c_str());
      }
    }
    
    data_info->setNewSweep(false);
    data_info->setNewVolume(false);
    
    if (data_mgr->getSweepNum() > data_mgr->getNumSweeps())
      break;

  } /* end of loop through data */

#endif

  data_mgr->setEditFlag(false);
  
  if (seds->punt)
    return;
  
  // Make a final pass through all operations in case they need to finish up
  // NOTE: I don't think this does anything.

#ifdef USE_RADX
  se_perform_fer_finish_up(seds->for_each_ray_cmds);
#else
  seds->finish_up = YES;

  se_perform_fer_cmds(seds->for_each_ray_cmds);
#endif

  // Free running average queues if any where set up

  RAQ_free();

  if (seds->modified)
  {
    seds->time_modified = time(0);
    DataManager::getInstance()->setRescanUrgent(seds->se_frame);
  }

  if (seds->boundary_exists)
  {
    // Pack up the current boundary

    int size = se_sizeof_bnd_set();
    char *bbuf = (char *)malloc(size);
    memset(bbuf, 0, size);
    se_pack_bnd_set(bbuf);

    bool new_bnd;
    
    if (seds->last_pbs && size == seds->last_pbs->sizeof_set)
    {
      // See if this boundary is different from the last boundary

      new_bnd = se_compare_bnds(seds->last_pbs->at, bbuf, size);
    }
    else
    {
      new_bnd = YES;
    }

    if (new_bnd)
    {
      // Put this boundary in the queue

      if (seds->num_prev_bnd_sets < 7)
      {
	// Grow the circular queue till it reaches 7 boundaries

	seds->num_prev_bnd_sets++;
	seds->pbs =
	  (struct prev_bnd_sets *)malloc(sizeof(struct prev_bnd_sets));
	memset(seds->pbs, 0, sizeof(struct prev_bnd_sets));

	if (!seds->last_pbs)
	{
	  seds->pbs->last = seds->pbs->next = seds->pbs;
	}
	else
	{
	  seds->pbs->last = seds->last_pbs;
	  seds->pbs->next = seds->last_pbs->next;
	  seds->last_pbs->next->last = seds->pbs;
	  seds->last_pbs->next = seds->pbs;
	}
      }
      else
      {
	seds->pbs = seds->last_pbs->next;
      }

      seds->last_pbs = seds->pbs;

      if (seds->pbs->at)
	free(seds->pbs->at);

      seds->pbs->at = bbuf;
      seds->pbs->sizeof_set = size;
    }

    if (getenv("SOLO_DONT_CLEAR_BOUNDARIES") == 0)
      se_clr_all_bnds();

    // We should now be ready to draw the next boundary or to retrieve  the
    // last boundary we just put in the queue

  } /* end packing up current boundary */

  printf ("Finished!\n");
}

/* c------------------------------------------------------------------------ */

void se_edit_summary (void)
{
    /* saves the all the current editor commands 
     */
    int32_t tn;
    struct solo_edit_stuff *seds;

    seds = return_sed_stuff();
    tn = time(0);
    seds->edit_summary.clear();

    DateTime now;
    now.setTimeStamp(time(0));
    
    char print_buffer[1024];
    
    // Time stamp this set of cmds

    sprintf(print_buffer, "!   time-now %s GMT\n", now.toString().c_str());
    seds->edit_summary.push_back(print_buffer);
    
    // Dump out the setup-input cmds

    for (size_t i = 0; i < seds->setup_inputs[0].size(); ++i)
    {
      seds->edit_summary.push_back(seds->setup_inputs[0][i] + "\n");
    }

    /* dump out the current cmds
     */
    for (size_t i = 0; i < seds->once_cmds.size(); ++i)
    {
      seds->edit_summary.push_back(seds->once_cmds[i] + "\n");
    }
    seds->edit_summary.push_back("! for-each-ray\n");
    
    for (size_t i = 0; i < seds->fer_cmds.size(); ++i)
    {
      seds->edit_summary.push_back(seds->fer_cmds[i] + "\n");
    }
}

/* c------------------------------------------------------------------------ */

bool se_gen_sweep_list (void)
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // If we don't have any radars in the list then we can't do anything

  if (seds->radar_stack.size() == 0)
    return false;
  
  seds->punt = NO;

  // Pull the first radar name off the radar stack.  This removes the name
  // from the radar stack.  Then we pop the name on the top of the
  // se_spair_strings stack.

  seds->sfic->radar_name = seds->radar_stack.back();
  seds->radar_stack.pop_back();
  
  int radar_num;
  
  if (!DataManager::getInstance()->generateSweepList(seds->se_frame,
						     seds->sfic->directory,
						     seds->sfic->radar_name,
						     seds->sfic->version,
						     seds->sfic->new_version_flag,
						     seds->sfic->start_time,
						     seds->sfic->stop_time,
						     radar_num))
  {
    seds->punt = YES;
    return false;
  }
  
  seds->sfic->radar_num = radar_num;

  return true;
}

/* c------------------------------------------------------------------------ */

bool se_interpret_fer_commands(std::vector< std::string > &cmd_text_list,
			       std::vector< ForEachRayCmd* > &cmd_list,
			       int &cmds_count)
{
  // Initialize the returned command count

  cmds_count = 0;
  
  // Clear out the list and allocate space for all of the needed command objects

  for (std::size_t i = 0; i < cmd_list.size(); ++i)
    delete cmd_list[i];
  cmd_list.clear();
  cmd_list.reserve(cmd_text_list.size());
  
  // Process the command strings and put them into the command list

  bool ok = true;
    
  for (size_t ii = 0; ii < cmd_text_list.size(); ++ii)
  {
    ForEachRayCmd *command =
      UiCommandFactory::getInstance()->createFerCommand(cmd_text_list[ii]);
    
    if (command == 0)
    {
      ok = false;
    }
    else
    {
      cmds_count++;
      cmd_list.push_back(command);
    }
    
  }

  return ok;
}

bool se_interpret_oto_commands(std::vector< std::string > &cmd_text_list,
			       std::vector< OneTimeOnlyCmd* > &cmd_list,
			       int &cmds_count)
{
  // Initialize the returned command count

  cmds_count = 0;
  
  // Clear out the list and allocate space for all of the needed command objects

  for (std::size_t i = 0; i < cmd_list.size(); ++i)
    delete cmd_list[i];
  cmd_list.clear();
  cmd_list.reserve(cmd_text_list.size());
  
  // Process the command strings and put them into the command list

  bool ok = true;
    
  for (size_t ii = 0; ii < cmd_text_list.size(); ++ii)
  {
    OneTimeOnlyCmd *command =
      UiCommandFactory::getInstance()->createOtoCommand(cmd_text_list[ii]);
    
    if (command == 0)
    {
      ok = false;
    }
    else
    {
      cmds_count++;
      cmd_list.push_back(command);
    }
    
  }

  return ok;
}

/* c------------------------------------------------------------------------ */

std::vector< std::string > se_text_to_vector(const std::string &txt)
{
  std::vector< std::string > return_vector;
  
  // Separate the text into individual lines

  std::vector< std::string > lines;
  tokenize(txt, lines, "\n");
  
  for (size_t ii = 0; ii < lines.size(); ii++)
  {
    std::string line = lines[ii];
    
    // Remove any comments

    std::size_t comment_pos = line.find("!");
    if (comment_pos != std::string::npos)
      line = line.substr(0, comment_pos);
    
    // If there are no tokens in the remaining line, then the line is
    // empty and we should skip it

    std::vector < std::string > tokens;
    tokenize(line, tokens);
    if (tokens.size() == 0)
      continue;
    
    // Add the text to the list

    return_vector.push_back(line);
  }

  return return_vector;
}

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX

void se_perform_fer_cmds(std::vector< ForEachRayCmd* > &cmd_list,
			 const int frame_num, RadxRay &ray)
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  
  for (std::vector< ForEachRayCmd* >::iterator cmd = cmd_list.begin();
       cmd != cmd_list.end(); ++cmd)
  {
    (*cmd)->doIt(frame_num, ray);

    if (seds->punt)
      break;
  }
}

void se_perform_oto_cmds(std::vector< OneTimeOnlyCmd* > &cmd_list)
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  
  for (std::vector< OneTimeOnlyCmd* >::iterator cmd = cmd_list.begin();
       cmd != cmd_list.end(); ++cmd)
  {
    (*cmd)->doIt();

    if (seds->punt)
      break;
  }
}

void se_perform_fer_finish_up(std::vector< ForEachRayCmd* > &cmd_list)
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  
  for (std::vector< ForEachRayCmd* >::iterator cmd = cmd_list.begin();
       cmd != cmd_list.end(); ++cmd)
  {
    (*cmd)->finishUp();

    if (seds->punt)
      break;
  }
}

#else

void se_perform_fer_cmds (std::vector< ForEachRayCmd* > &cmd_list)
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  
  for (std::vector< ForEachRayCmd* >::iterator cmd = cmd_list.begin();
       cmd != cmd_list.end(); ++cmd)
  {
    (*cmd)->doIt();

    if (seds->punt)
      break;
  }
}

void se_perform_oto_cmds (std::vector< OneTimeOnlyCmd* > &cmd_list)
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  
  for (std::vector< OneTimeOnlyCmd* >::iterator cmd = cmd_list.begin();
       cmd != cmd_list.end(); ++cmd)
  {
    (*cmd)->doIt();

    if (seds->punt)
      break;
  }
}

#endif

/* c------------------------------------------------------------------------ */

bool se_process_data(const int time_series, const int automatic,
		     const int down,
		     const double d_ctr, const int frame_num)
{
  // Clear the error string

  g_string_truncate(gs_complaints, 0);

  // Set up the dd utilities to not print the information

  DataManager::getInstance()->setPrintDDopenInfo(false);
  
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  seds->punt = NO;

  seds->fer_cmds.clear();
  
  // Add the text to the For Each Ray command list

  int for_each_ray_count = 0;
  
  seds->fer_cmds = se_text_to_vector(seds->fer_lines);
  if (seds->fer_cmds.size() > 0)
  {
    // Convert the textual commands to UiCommand objects, stored in seds
    
    if (!se_interpret_fer_commands(seds->fer_cmds, seds->for_each_ray_cmds,
				   for_each_ray_count))
      for_each_ray_count = 0;
  }

  // If there aren't any "for each ray" commands, punt.

  if (!for_each_ray_count)
  {
    seds->punt = YES;
    return true;
  }

  // Convert the text to solo_str_mgmt objects, removing comments and blank
  // lines.

  seds->once_cmds.clear();
  
  int one_time_only_count = 0;
  seds->once_cmds = se_text_to_vector(seds->oto_lines);
  if (seds->once_cmds.size() > 0)
  {
    // Convert the textual commands to UiCommand objects, stored in seds

    if (!se_interpret_oto_commands(seds->once_cmds, seds->one_time_only_cmds,
				   one_time_only_count))
    {
      seds->punt = YES;
      return true;
    }
  }

  // Generate the setup-input cmds and loop through them.  These commands
  // come from the "Sweepfiles" widget.

  // Get the information from the "Sweepfiles" widget.

  se_dump_sfic_widget(seds->sfic, frame_num);

  // Take the contents of the "Sweepfiles" widget and construct input commands
  // from it.

  se_setup_input_cmds();

  seds->num_radars = 1;  // Just do one radar at a time now

  // Put the "Sweepfiles" commands at the front of the se_spair_strings list.

  seds->radar_stack.clear();
  
  // Put the radar names into the string and put them on the radar stack

  seds->radar_stack.push_back(seds->sfic->radar_names_text);
  
  // Put the "Sweepfiles" commands at the front of the se_spair_strings list.
  // I'm not sure why this is done again when it was just done above.

  se_setup_input_cmds();

  // I don't know what this is supposed to do since jj is undefined at this
  // point!  (I added the initialization to 0...)
  
  int jj = 0;
  seds->num_setup_input_cmds = jj;

  // NOTE: It looks to me like this puts the commands into seds->edit_summary
  // and doesn't write anything to a file.

  // Save all the edit commands in the edit summary list and dump them to
  // a temporary file

  se_edit_summary();

  // Loop through the stack of radars

  for (;;)
  {
    // Try to create a list of sweeps for the next radar

    if (!se_gen_sweep_list() || seds->punt)
    {
      if (!seds->batch_mode)
	se_set_sfic_from_frame(seds->popup_frame);

      return true;
    }

    // Bracket and execute the one-time commands procedure

    se_perform_oto_cmds(seds->one_time_only_cmds);

    // Now try to loop through it

    dd_edd(time_series, automatic, down, d_ctr, frame_num);
  }

  // NOTE:  Can never reach here!!!

  if (gs_complaints->len)
    sii_message(gs_complaints->str);

  return true;
}

/* c------------------------------------------------------------------------ */

void se_really_readin_cmds (void)
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  struct sed_command_files *scf = se_return_cmd_files_struct();

  std::string file_path = scf->directory_text;
  if (file_path[file_path.size()-1] != '/')
    file_path += "/";
  file_path += scf->file_name_text;
      
  std::vector< std::string > scratch = se_absorb_strings(file_path);
  se_cull_setup_input(scratch);

  // Move scratch1 to cmdz

  seds->cmdz = scratch;
}

/* c------------------------------------------------------------------------ */

void se_setup_input_cmds(void)
{
  // This routine takes the contents of the sfic widget and constructs
  // input control commands from it

  struct swp_file_input_control * sfic;
  struct solo_edit_stuff *seds;
  const char *dq="\"";

  seds = return_sed_stuff();
  sfic = seds->sfic;

  std::vector< std::string > scratch;
  std::string cmd;
  
  cmd = "    sweep-directory ";
  cmd += dq;
  cmd += sfic->directory_text;
  cmd += dq;
  scratch.push_back(cmd);

  seds->last_directory = sfic->directory_text;
  if (seds->last_directory[seds->last_directory.size()-1] != '/')
    seds->last_directory += "/";
    
  cmd = "    radar-names ";
  cmd += sfic->radar_names_text;
  scratch.push_back(cmd);
    
  seds->radar_stack.clear();
  seds->last_radar_stack.clear();
    
  // Also create the list of radar names for processing
    
  std::vector< std::string > radar_names;
  tokenize(sfic->radar_names_text, radar_names);

  for (size_t ii = 0; ii < radar_names.size(); ii++ )
  {
    seds->radar_stack.push_back(radar_names[ii]);
    seds->last_radar_stack.push_back(radar_names[ii]);
  }

  cmd = "    first-sweep ";
  if (sfic->first_sweep_text.find("first") != std::string::npos)
  {
    cmd += "first";
  }
  else if (sfic->first_sweep_text.find("last") != std::string::npos)
  {
    cmd += "last";
  }
  else
  {
    DateTime start_time;
    start_time.setTimeStamp(sfic->start_time);
    cmd += start_time.toString2();
  }
  scratch.push_back(cmd);
    
  cmd = "    last-sweep ";
  if (sfic->last_sweep_text.find("first") != std::string::npos)
  {
    cmd += "first";
  }
  else if (sfic->last_sweep_text.find("last") != std::string::npos)
  {
    cmd += "last";
  }
  else
  {
    DateTime stop_time;
    stop_time.setTimeStamp(sfic->stop_time);
    cmd += stop_time.toString2();
  }
  scratch.push_back(cmd);
    
  cmd = "    version ";
  if (sfic->version_text.find("first") != std::string::npos)
  {
    cmd += "first";
  }
  else if (sfic->version_text.find("last") != std::string::npos)
  {
    cmd += "last";
  }
  else
  {
    char buffer[80];
    sprintf(buffer, "%d", sfic->version);
    cmd += buffer;
  }
  scratch.push_back(cmd);
    
  if (sfic->new_version_flag)
    cmd = "new-version";
  else
    cmd = "no-new-version";
  scratch.push_back(cmd);
    
  seds->last_new_version_flag = sfic->new_version_flag;

  // Now pop this on top of the setup_inputs stack

  se_shift_setup_inputs(scratch);
}

/* c------------------------------------------------------------------------ */

void se_shift_setup_inputs(const std::vector< std::string > &input_list)
{
  // Routine to pop the current set of setup-input cmds on top
  // of the stack of previous inputs

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Shift the current setup inputs back in the array

  for (size_t i = MAX_SETUP_INPUTS - 1; i > 0; --i)
    seds->setup_inputs[i] = seds->setup_inputs[i-1];
  
  // Copy the new list to the first setup inputs

  seds->setup_inputs[0] = input_list;
}

/* c------------------------------------------------------------------------ */
/* Saves the editor commands before beginning a pass through the data.       */

int se_write_sed_cmds(const std::vector< UiCommandToken > &cmd_tokens /* not used */ )
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  struct swp_file_input_control *sfic = return_swp_fi_in_cntrl();
  struct sed_command_files *scf = se_return_cmd_files_struct();

  // If we don't have a valid directory name, get the dir name from the sfic

  if (scf->directory_text == "")
    scf->directory_text = sfic->directory_text;

  // Get the directory name

  std::string dir = scf->directory_text;
  
  if (dir[dir.size() - 1] != '/')
    dir += std::string("/");

  // Get the radar name

  std::string radar_name;
  if (seds->last_radar_stack.size() > 0 &&
      seds->last_radar_stack[0] != "")
  {
    radar_name = seds->last_radar_stack[0];
  }
  else if (sfic->radar_names_text != "")
  {
    radar_name = sfic->radar_names_text;
    std::size_t first_nonwhite =
      radar_name.find_first_not_of(" \t\n");
    if (first_nonwhite != std::string::npos)
      radar_name = radar_name.substr(first_nonwhite);
    
    std::size_t filename_end =
      radar_name.find_first_of(" \t\n<>;");
    if (filename_end != std::string::npos)
      radar_name = radar_name.substr(0, filename_end-1);
  }
  else
  {
    radar_name = "UNK";
  }

  // Get the file name based on the radar name

  std::string file_name =
    DataManager::getInstance()->getFileBaseName("sed", time(0),
						radar_name, getuid());
  scf->file_name_text = file_name;

  // If a comment was specified, add it to the file name

  if (scf->comment_text != "")
  {
    se_fix_comment(scf->comment_text);
    file_name += std::string(".") + scf->comment_text;
  }

  std::string file_path = dir + file_name;
  
  FILE *stream;
  
  if ((stream = fopen(file_path.c_str(), "w")) == 0)
  {
    g_string_sprintfa (gs_complaints,"Unable to save editor commands to %s\n",
		       file_path.c_str());
    return 1;
  }

  for (size_t i = 0; i < seds->edit_summary.size(); ++i)
  {
    int nn = fputs(seds->edit_summary[i].c_str(), stream);
    if (nn <= 0 || nn == EOF)
    {
      // We have an error
      break;
    }
  }

  fclose(stream);

  return 1;
}

