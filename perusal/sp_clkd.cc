/* 	$Id$	 */

#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>

#include <ClickQueue.hh>
#include <dd_math.h>
#include <DataInfo.hh>
#include <DataManager.hh>
#include <DateTime.hh>
#include <EarthRadiusCalculator.hh>
#include <se_bnd.hh>
#include <se_utils.hh>
#include <SeBoundaryList.hh>
#include "sii_exam_widgets.hh"
#include "sii_perusal.hh"
#include "sii_xyraster.hh"
#include "solo2.hh"
#include "soloii.h"
#include "soloii.hh"
#include "sp_basics.hh"
#include "sp_clkd.hh"
#include "sxm_examine.hh"

/* c------------------------------------------------------------------------ */

void solo_new_list_field_vals(const int frame_index)
{
  static int count=0;
  WW_PTR wwptr;
  struct solo_field_vals *sfv;
  char str[256];
  double dcost;
  double d;
  int ii, hilite=YES, ctr_val, ts;

  count++;
  wwptr = solo_return_wwptr(frame_index);
  ts = wwptr->view->type_of_plot & SOLO_TIME_SERIES;
  wwptr->list_field_vals.clear();
  sfv = wwptr->field_vals;
  ctr_val = sfv->num_vals/2;
  PointInSpace radar = wwptr->radar_location;

  radar.setAzimuth(sfv->next->az);
  radar.setElevation(sfv->next->el);
  radar.setRange(sfv->rng);
  radar.rangeAzEl2xyz();

  PointInSpace clicked;
    
  if(ts) {
    clicked = wwptr->radar_location;
    clicked.setAltitude(wwptr->clicked_range);
  }
  else {
    // NOTE: Is this okay?  clicked hasn't been set up to this point
    clicked.latLonShift(radar);
  }

  if (!ts) {
    DateTime next_time;
    next_time.setTimeStamp(sfv->next->time);
    sprintf(str, "%s:  %s\n", wwptr->parameter.parameter_name,
	    next_time.toString().c_str());
  } else {
    sprintf(str, "   %s\n", wwptr->parameter.parameter_name);
  }
        
    
  wwptr->list_field_vals.push_back(str);
  
  switch(wwptr->lead_sweep->scan_mode) {
  case DD_SCAN_MODE_RHI:
    dcost = cos(fabs(RADIANS(sfv->next->el)));
    break;
  default:
    dcost = cos(fabs(RADIANS(sfv->next->tilt)));
    break;
  }
  d = ts ? sfv->next->alt :
    sfv->next->agl + (clicked.getAltitude() - radar.getAltitude());

  sprintf(str, " Hdg. %9.1f   Agl: %7.3f\n"
	  , sfv->next->heading, d);
  wwptr->list_field_vals.push_back(str);
  
  d = ts ? sfv->next->lon : clicked.getLongitude();
  sprintf(str, " Lon. %9.4f     x: %7.3f\n"
	  , d, radar.getX()); 
  wwptr->list_field_vals.push_back(str);
  
  d = ts ? sfv->next->lat : clicked.getLatitude();
  sprintf(str, " Lat. %9.4f     y: %7.3f\n"
	  , d, radar.getY());
  wwptr->list_field_vals.push_back(str);
  
  sprintf(str, " Alt. %9.4f     z: %7.3f\n"
	  , clicked.getAltitude(), radar.getZ()); 
  wwptr->list_field_vals.push_back(str);
  
  sprintf(str, " Angle    %7.2f %7.2f %7.2f\n", sfv->rot_ang
	  , sfv->next->rot_ang, sfv->next->next->rot_ang); 

  if(ts) {
    DateTime next_time;
    next_time.setTimeStamp(sfv->next->time);
    std::string next_time_string = next_time.toString();
      
    // Skip the date in the string and just show the time value

    std::size_t blank_pos = next_time_string.find(' ');
    if (blank_pos != std::string::npos)
      next_time_string = next_time_string.substr(blank_pos + 1);
    
    sprintf(str, "Time          %s", next_time_string.c_str());
  }
  wwptr->list_field_vals.push_back(str);
  
  for(ii=0; ii < sfv->num_vals; ii++) {
    d = M_TO_KM(sfv->ranges[ii]);
    if(!ts)
      d *= dcost;

    sprintf(str, "%7.2fkm.%7.2f %7.2f %7.2f\n"
	    , d
	    , sfv->field_vals[ii]
	    , sfv->next->field_vals[ii]
	    , sfv->next->next->field_vals[ii]); 
    hilite = ii == ctr_val ? YES : NO;
    wwptr->list_field_vals.push_back(str);
  }
  print_fieldvalues(frame_index, wwptr->list_field_vals);
}
/* c------------------------------------------------------------------------ */

void 
sp_ts_seek_field_vals (int frme)
{
    /* 
     * routine to printout relevent field values corresponding to
     * the clicked location for time series data
     */
    int state, ray_num, top_down;
    WW_PTR wwptr, wwptrx;
    struct solo_field_vals *sfv;
    struct ts_ray_table *tsrt;
    struct ts_ray_info *tsri, **tlist;
    struct ts_sweep_info *tssi;
    float range, theta=0;
    char str[256], mess[256];


    wwptrx = solo_return_wwptr(frme);
    wwptr = wwptrx->lead_sweep;
    tsrt = wwptr->tsrt;
    if(!tsrt || !tsrt->tsri_top) {
	sprintf (mess, "NULL tsrt in sp_ts_seek_field_vals() for frame %d\n"
		 , frme);
	g_message (mess);
	return;
    }
    tlist = tsrt->tsri_list;

    DataInfo *data_info =
      DataManager::getInstance()->getWindowInfo(wwptr->window_num);
    
    state = SFV_CENTER;


    for(ray_num=0; ray_num < 3; ray_num++, sfv = sfv->next) {
	if(wwptr->file_id > 0)
	      close(wwptr->file_id);
	tsri = *(tlist +ray_num);
	tssi = tsrt->tssi_top + tsri->sweep_num;
	if (tssi->directory[strlen(tssi->directory)-1] == '/')
	  sprintf(str, "%s%s", tssi->directory, tssi->filename);
	else
	  sprintf(str, "%s/%s", tssi->directory, tssi->filename);
	if((wwptr->file_id = open(str, O_RDONLY, 0)) < 0) {
	    sprintf(mess, "Unable to open sweep %s\n", str);
	    solo_message(mess);
	    return;
	}
	data_info->setDir(tssi->directory);
	data_info->setInputFileId(wwptr->file_id);
	data_info->loadHeaders();
	data_info->loadRay(tsri->ray_num);

	for(wwptrx = wwptr ; wwptrx; wwptrx = wwptrx->next_sweep) {
	    if(!(wwptrx->view->type_of_plot & SOLO_TIME_SERIES))
		  continue;
	    if (wwptrx->window_num >= sii_return_frame_count())
	      { continue; }
	    
	    if(wwptr->view->type_of_plot & TS_MSL_RELATIVE) {
		if(wwptr->view->type_of_plot & TS_AUTOMATIC) {
		  top_down = data_info->isInSector(135.0, 225.0);
		}
		else {
		    top_down = wwptr->view->type_of_plot & TS_PLOT_DOWN;
		}
		if(top_down) {
		  range = data_info->getPlatformAltitudeMSL() - wwptr->clicked_range;
		}
		else {
		  range = wwptr->clicked_range - data_info->getPlatformAltitudeMSL();
		}
	    }
	    else if(wwptr->view->type_of_plot & TS_PLOT_DOWN) {
		range = 2. * wwptr->view->ts_ctr_km - wwptr->clicked_range;
	    }
	    else {
		range = wwptr->clicked_range;
	    }
	    /* range is now radar relative */
	    
	    switch(ray_num) {
	    case 0:
		sfv = wwptrx->field_vals;
		break;
	    case 1:
		sfv = wwptrx->field_vals->next;
		break;
	    case 2:
		sfv = wwptrx->field_vals->next->next;
		break;
	    }
	    data_info->putFieldValues(wwptrx->parameter_num,
				     (float)theta, (float)range,
				     *sfv, state);
	}
    }
    for(wwptrx = wwptr ; wwptrx; wwptrx = wwptrx->next_sweep) {
      if (wwptrx->window_num >= sii_return_frame_count())
	{ continue; }
	solo_new_list_field_vals(wwptrx->window_num);
    }
}
/* c------------------------------------------------------------------------ */

int 
sp_ts_ray_list (int frme, int xpos, int nr)
{
    /* construct a list of "nr" rays of time series data
     * centered on xpos.
     */
    int jj, mm, nn, nr2 = 2*nr;
    int nurl=0, nurr=0;
    WW_PTR wwptrx;
    struct solo_perusal_info *spi;
    struct ts_ray_table *tsrt;
    struct ts_ray_info *tsri, *tsri_right, *tsrix, **tlist;

    spi = solo_return_winfo_ptr();
    wwptrx = solo_return_wwptr(frme)->lead_sweep;
    tsrt = wwptrx->tsrt;
    if(!tsrt || !tsrt->tsri_top) {
      printf("NULL tsri in sp_ts_ray_list() for frame: %d\n", frme);
      return((int)0);
    }
    

    if(tsrt->max_list_entries < nr2) { /* check the list to be sure
					* it's large enough */
	tsrt->max_list_entries = nr2;
	if(tsrt->tsri_list) free(tsrt->tsri_list);
	tsrt->tsri_list = (struct ts_ray_info **)malloc
	      (tsrt->max_list_entries*sizeof(struct ts_ray_info *));
	memset(tsrt->tsri_list, 0, tsrt->max_list_entries*
	       sizeof(struct ts_ray_info *));
    }
    tsri_right = tsrt->tsri_top + wwptrx->view->width_in_pix;
    tsri = tsrix = tsrt->tsri_top + xpos;
    tlist = tsrt->tsri_list + nr -1; /* the list has 2 * nr entries */

    if(tsrix->ray_num >=0) {	/* really clicked on some data */
	*tlist-- = tsrix = tsri--;
	nurl++;
    }

    /* look to the left for nr unique rays
     */
    for(; nurl < nr && tsri >= tsrt->tsri_top;) {
	for(; tsri >= tsrt->tsri_top; tsri--) {
	    if(tsri->ray_num < 0)
		  continue;
	    if(tsri->ray_num != tsrix->ray_num ||
	       tsri->sweep_num != tsrix->sweep_num) { /* new entry */
		*tlist-- = tsrix =  tsri--;
		nurl++;
		break;
	    }
	}
    }
    tlist = tsrt->tsri_list + nr;
    tsri = tsrix = tsrt->tsri_top + xpos;
    /*
     * look to the right for nr rays
     */
    for(; nurr < nr && tsri < tsri_right;) {
	for(; tsri < tsri_right; tsri++) {
	    if(tsri->ray_num < 0)
		  continue;
	    if(tsri->ray_num != tsrix->ray_num ||
	       tsri->sweep_num != tsrix->sweep_num) {
		*tlist++ = tsrix = tsri++;
		nurr++;
		break;
	    }
	}
    }
    nn = (nr +1)/2;		/* nr odd => nn = nr/2 +1 */
    tlist = tsrt->tsri_list;
    if((mm = nurl + nurr) <= nr) { /* found only nr or less rays */
	jj = nr - nurl;
	for(; mm-- ; tlist++) {
	    *tlist = *(tlist +jj);
	}
	return(nurl + nurr);
    }
    jj = nr - nn;

    if(nurl < nn) {		/* not symmetric (less on the left) */
	jj += nn - nurl;
    }
    else if(nurr < nr/2) {	/* not symmetric (less on the right) */
	jj -= nr/2 - nurr;
    }
    if(jj) {
	for(mm=nr; mm-- ; tlist++) {
	    *tlist = *(tlist +jj);
	}
    }    
    return(nr);
}

/* c------------------------------------------------------------------------ */

void sii_edit_click_in_data(gint frame_num)
{
  EditWindow *edit_window = frame_configs[frame_num]->edit_window;
  
  if (edit_window == 0 ||
      !edit_window->isFrameActive() || !edit_window->isDrawBoundaries())
    return;

  WW_PTR wwptr = solo_return_wwptr(frame_num);

  // Set up the click information

  struct solo_click_info sci;

  sci.frame = frame_num;

  if (wwptr->view->type_of_plot & SOLO_TIME_SERIES)
  {
    sci.x = wwptr->clicked_x_pxl;
    sci.y = wwptr->clicked_y_pxl;
  }
  else
  {
    double theta = RADIANS(CART_ANGLE(wwptr->clicked_angle));
    double rr = KM_TO_M(wwptr->clicked_range);

    double xx = rr * cos(theta);
    xx += (xx < 0) ? -0.5 : 0.5;
    double yy = rr * sin(theta);
    yy += (yy < 0) ? -0.5 : 0.5;

    // (x,y) should be meters relative to the local radar

    sci.x = (gint)xx;
    sci.y = (gint)yy;
  }
  
  // Add the clicked point to the current boundary

  SeBoundaryList *sebs = SeBoundaryList::getInstance();
  xse_add_bnd_pt(&sci, sebs->currentBoundary);
  sebs->lastOperation = SeBoundaryList::BND_POINT_APPENDED;
}

/* c------------------------------------------------------------------------ */

void 
sp_ts_data_click (int frme, int xpos, int ypos)
{
    int ii, ww, width, checked_off[SOLO_MAX_WINDOWS];
    int nr;
    double d, scale, range, height;
    WW_PTR wwptrx, wwptr;
    struct solo_perusal_info *spi;
    struct ts_ray_table *tsrt;
    struct ts_ray_info *tsri = 0;
    char mess[256];

    spi = solo_return_winfo_ptr();
    wwptrx = solo_return_wwptr(frme)->lead_sweep;
    tsrt = wwptrx->tsrt;
    if(!tsrt || !tsrt->tsri_top) {
	sprintf (mess, "NULL tsri in sp_ts_ray_list() for frame: %d\n"
		 , frme);
	g_message (mess);
	return;
    }
    SiiPoint point = ClickQueue::getInstance()->getLatestClick();
    xpos = point.x;
    ypos = point.y;
    scale = M_TO_KM(sp_meters_per_pixel()/wwptrx->view->ts_magnification);
    height = wwptrx->view->height_in_pix;
    width = wwptrx->view->width_in_pix;
    /* range in this case will be an altitude
     */
    range = wwptrx->view->ts_ctr_km + (.5*height -ypos) * scale;
    wwptrx->clicked_x_pxl = xpos;
    wwptrx->clicked_y_pxl = ypos;
    if((wwptrx->view->type_of_plot & SOLO_TIME_SERIES)) {
       d = wwptrx->sweep->stop_time - wwptrx->sweep->start_time;
       wwptrx->clicked_time = wwptrx->sweep->start_time +
	 d * ((double)xpos/wwptrx->view->width_in_pix);
    }
    sxm_set_click_frame(frme, (float)0, (float)range);
    for(ii=0; ii < SOLO_MAX_WINDOWS; checked_off[ii++] = NO);

    /* you need to do the above based on xpos for now in all the
     * active time series windows
     */
    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!(spi->active_windows[ww]) || checked_off[ww])
	      continue;
	if (ww >= sii_return_frame_count())
	  { continue; }

	wwptr = solo_return_wwptr(ww);
	if(!(wwptr->view->type_of_plot & SOLO_TIME_SERIES))
	      continue;
	/* get a list of 3 rays around xpos */
	if((nr = sp_ts_ray_list(ww, xpos, 3)) > 0) {
	    /* center ray info struct */
	    tsri = *(wwptr->tsrt->tsri_list +1);
	}
	for(; wwptr ; wwptr = wwptr->next_sweep) {
	    checked_off[wwptr->window_num] = YES;
	    if (wwptr->window_num >= sii_return_frame_count())
	      { continue; }
	    wwptr->clicked_x_pxl = xpos;
	    wwptr->clicked_y_pxl = ypos;
	    wwptr->clicked_range = range;
	    if(nr > 0)
		  sp_copy_tsri(tsri, wwptr->clicked_tsri);
	}
	sp_ts_seek_field_vals(ww);
	sii_exam_click_in_data (ww);
	sii_edit_click_in_data (ww);
    }
}
/* c------------------------------------------------------------------------ */

void sp_data_click(int frme, double x, double y)
{
    /* (x,y) are in km. relative to the radar
     * (fieldvalues)
     * routine to printout relevent field values corresponding to
     * the clicked location
     */
    int ww;
    WW_PTR wwptrx, wwptr;
    struct solo_perusal_info *spi;
    double range, theta;

    if(solo_busy())
      return;
    
    wwptrx = solo_return_wwptr(frme);
    solo_set_busy_signal();

    if(wwptrx->view->type_of_plot & SOLO_TIME_SERIES) {
	sp_ts_data_click(frme, 0,0);
	solo_clear_busy_signal();
	return;
    }

    PointInSpace radarx = wwptrx->radar_location;
    PointInSpace pisp;

    theta = atan2(y, x);
    theta = fmod((double)(450. -DEGREES(theta)), (double)360.);
    pisp.setRotationAngle(theta);
    range = sqrt(SQ(x) + SQ(y));
    pisp.setRange(range);

    sxm_set_click_frame(frme, (float)theta, (float)range);
    /*
     * calculate (x,y,z) relative to this radar
     */
    sp_rtheta_screen_to_xyz(frme, radarx, pisp);
    /* calculate the absolute lat/lon/alt of this point
     */
    pisp.latLonShift(radarx);
    se_rain_gauge_info(pisp.getLatitude(), pisp.getLongitude());

    spi = solo_return_winfo_ptr();
    /*
     * convert this into a range and angle for all the other windows
     */
    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!(spi->active_windows[ww]))
	      continue;
	wwptr = solo_return_wwptr(ww);
	if(wwptrx->sweep->linked_windows[ww]) {
	    wwptr->clicked_angle = theta;
	    wwptr->clicked_range = range;
	}
	else {
	  PointInSpace radar = wwptr->radar_location;
	  /* calculate (x,y,z) relative to this radar
	   */
	  radar.latLonRelative(pisp);
	  /* calculate range and rotation angle relatvie to this radar
	   */
	  sp_xyz_to_rtheta_screen(ww, radar, pisp);
	  wwptr->clicked_angle = pisp.getRotationAngle();
	  wwptr->clicked_range = pisp.getRange();
	}
	delete wwptr->pisp_click;
	wwptr->pisp_click = new PointInSpace(pisp);
    }

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(spi->active_windows[ww]) {
	  sp_seek_field_vals(ww);
	  sii_exam_click_in_data (ww);
	  sii_edit_click_in_data (ww);
	}
    }    
    solo_clear_busy_signal();
}
/* c------------------------------------------------------------------------ */

void 
sp_locate_this_point (struct solo_click_info *sci, struct bnd_point_mgmt *bpm)
{
    /* this routine loads up a pisp struct with all the positioning
     * info for this boundary point
     */
    int frme, ww, kk, nr, ypos;
    int replot=YES;
    WW_PTR wwptrld, wwptr;
    struct ts_ray_info *tsri;
    struct ts_sweep_info *tssi;
    struct ts_ray_table *tsrt;
    double theta, range = 0.0, x, y, scale;
    char str[256];


    frme = sci->frame;
    wwptr = solo_return_wwptr(frme);
    wwptrld = wwptr->lead_sweep;
    ww = wwptrld->window_num;
    DataInfo *data_info = DataManager::getInstance()->getWindowInfo(ww);
    
    bpm->pisp->setId("BND_PT_V1");
    bpm->pisp->clearState();
    bpm->pisp->setStateAzElRg(true);
    bpm->pisp->setStateEarth(true);
    
    if(wwptr->view->type_of_plot & SOLO_TIME_SERIES) {
	/*
	 * for time series the center of the screen is assumed to be
	 * the specified number in km. above msl
	 */
	scale = M_TO_KM(sp_meters_per_pixel())/wwptr->view->ts_magnification;
	ypos = wwptr->view->height_in_pix -1 - sci->y;

	/* set msl altitude of clicked location */
	double range = wwptr->view->ts_ctr_km -
	  ((.5*wwptr->view->height_in_pix -.5) -ypos) * scale;
	bpm->pisp->setRange(range);
	bpm->pisp->setZ(range);
	bpm->pisp->setX(sci->x);	/* pixel values */
	bpm->pisp->setY(sci->y);
	bpm->pisp->setStateTimeSeries(true);
	nr = sp_ts_ray_list(ww, (int)sci->x, 3);
	tsrt = wwptrld->tsrt;
	if(!tsrt || !tsrt->tsri_top) {
	  printf("NULL tsrt pointer in sp_locate_this_point()  frme: %d\n", frme);
	  return;
	}
	tsri = *(tsrt->tsri_list + nr/2); /* center ray info struct */
	kk = tsri->ray_num;
	tssi = tsrt->tssi_top + tsri->sweep_num;
	if (tssi->directory[strlen(tssi->directory)-1] == '/')
	  sprintf(str, "%s%s", tssi->directory, tssi->filename);
	else
	  sprintf(str, "%s/%s", tssi->directory, tssi->filename);
	if(wwptrld->file_id)
	      close(wwptrld->file_id);
	if((wwptrld->file_id = open(str, O_RDONLY, 0)) < 0) {
	  printf("Unable to open sweep %s\n", str);
	  return;
	}
	data_info->setDir(tssi->directory);
	data_info->setInputFileId(wwptrld->file_id);
	data_info->loadHeaders();
	wwptrld->sweep_file_modified = NO;
    }
    else {
	x = sci->x;
	y = sci->y;
	theta = atan2(y, x);
	theta = FMOD360(CART_ANGLE(DEGREES(theta)));
	range = M_TO_KM(sqrt(SQ(x) + SQ(y))); /* km. */
	
	if(wwptr->lead_sweep->sweep_file_modified) {
	  DataManager::getInstance()->nabNextFile(ww,
						  DataManager::TIME_NEAREST,
						  DataManager::LATEST_VERSION,
						  replot);
	    wwptr->lead_sweep->sweep_file_modified = NO;
	}
	kk = data_info->getAngleIndex(theta);
	bpm->pisp->setX(M_TO_KM(bpm->x));
	bpm->pisp->setY(M_TO_KM(bpm->y));
	bpm->pisp->setRange(range);
	bpm->r = (int32_t)KM_TO_M(range);

	if(wwptr->lead_sweep->scan_mode == DD_SCAN_MODE_AIR)
	  bpm->pisp->setStatePlotRelative(true);
    }

    data_info->loadRay(kk);
    
    bpm->pisp->setTime(data_info->getTime());
    bpm->pisp->setRotationAngle(data_info->getRotationAngleCalc());
    // NOTE: Why do we set tilt twice in a row???
    bpm->pisp->setTilt(data_info->getTiltAngle());
    bpm->pisp->setTilt(data_info->getFixedAngle());
    
    bpm->pisp->setAzimuth(data_info->getAzimuthAngleCalc());
    bpm->pisp->setElevation(data_info->getElevationAngleCalc());

    bpm->pisp->setLatitude(data_info->getPlatformLatitude());
    bpm->pisp->setLongitude(data_info->getPlatformLongitude());
    bpm->pisp->setAltitude(data_info->getPlatformAltitudeMSL());

    if(wwptr->view->type_of_plot & SOLO_TIME_SERIES) {
	/* true range of clicked location from radar
	 */
	if(wwptr->view->type_of_plot & TS_MSL_RELATIVE) {
	  bpm->pisp->setRange((bpm->pisp->getZ() - bpm->pisp->getAltitude()) *
			      sin(bpm->pisp->getElevation()));
	}
	else if(wwptr->view->type_of_plot & TS_PLOT_DOWN) {
	  bpm->pisp->setRange(2.0 * wwptr->view->ts_ctr_km - bpm->pisp->getZ());
	}
	else {			/* plot up */
	  bpm->pisp->setRange(bpm->pisp->getZ());
	}
	bpm->r = (int32_t)KM_TO_M(bpm->pisp->getRange());
    }
    else {
	/* msl altitude of clicked location */
      bpm->pisp->setZ((bpm->pisp->getAltitude() + range *
		       sin((double)RADIANS(bpm->pisp->getElevation()))));
    }
    bpm->pisp->setEarthRadius(EarthRadiusCalculator::getInstance()->getRadius(bpm->pisp->getLatitude()));

    bpm->pisp->setHeading(data_info->getPlatformHeading());
    bpm->pisp->setRoll(data_info->getPlatformRoll());
    bpm->pisp->setPitch(data_info->getPlatformPitch());
    bpm->pisp->setDrift(data_info->getPlatformDrift());
}

/* c------------------------------------------------------------------------ */
/* Calculate absolute (x,y,z) relative to the radar for the point on the     */
/* screen.  pisp->range (slant range) and pisp->rotation_angle are relative  */
/* to the radar.                                                             */

void sp_rtheta_screen_to_xyz(const int frme, PointInSpace &radar,
			     const PointInSpace &pisp)
{
  WW_PTR wwptr = solo_return_wwptr(frme);

  double range = pisp.getRange();
  
  if (range < .0005)	/* half a meter */
  {
    radar.setX(0.0);
    radar.setY(0.0);
    radar.setZ(0.0);
    return;
  }

  double tilt = RADIANS(radar.getTilt());
  double rotation = RADIANS(CART_ANGLE(pisp.getRotationAngle()));

  switch(wwptr->lead_sweep->scan_mode)
  {
    case DD_SCAN_MODE_AIR:
    {
      double x = range * cos(tilt) * cos(rotation);	/* aircraft relative */
      double y = range * sin(tilt);	/* aircraft relative */
      double heading = RADIANS(radar.getHeading());
      double sin_h = sin(heading);
      double cos_h = cos(heading);
      radar.setX((x * cos_h) - (y * sin_h));
      radar.setY((x * sin_h) + (y * cos_h));
      radar.setZ(range * cos(tilt) * sin(rotation));
      break;
    }
    
  case DD_SCAN_MODE_RHI:
  {
    radar.setZ(range * sin(rotation)); /* rotation = elevation in this case */
    double rxy = range * cos(rotation);
    radar.setX(rxy * cos(tilt));
    radar.setY(rxy * sin(tilt));
    break;
  }
  
  default:
  {
    
    radar.setZ(range * sin(tilt));
    double rxy = range * cos(tilt);
    radar.setX(rxy * cos(rotation));
    radar.setY(rxy * sin(rotation));
    break;
  }
  }
}
/* c------------------------------------------------------------------------ */

void 
sp_seek_field_vals (int frme)
{
    /* 
     * routine to printout relevent field values corresponding to
     * the clicked location
     */
    int ii, jj, kk, ll, ww, state;
    int replot=YES;
    WW_PTR wwptr;
    struct solo_field_vals *sfv;
    int indices[3];
    float angles[3], angle_k, range, theta;


    wwptr = solo_return_wwptr(frme);
    range = wwptr->clicked_range;
    theta = wwptr->clicked_angle;

    DataInfo *data_info =
      DataManager::getInstance()->getWindowInfo(wwptr->lead_sweep->window_num);

    if(wwptr->lead_sweep->sweep_file_modified) {
	ww = wwptr->lead_sweep->window_num;
	DataManager::getInstance()->nabNextFile(ww,
						DataManager::TIME_NEAREST,
						DataManager::LATEST_VERSION,
						replot);
	wwptr->lead_sweep->sweep_file_modified = NO;
    }
    
    /* get the index of entry closest to theta
     */
    kk = data_info->getAngleIndex(theta);
    indices[1] = kk;
    angles[1] = angle_k = data_info->getRATangle(kk);
    
    jj = DEC_NDX(kk, data_info->getNumRaysRAT()); /* index of ray before */
    indices[0] = jj;
    angles[0] = data_info->getRATangle(jj);
    wwptr->clicked_tsri->ray_num = jj +1;
    wwptr->clicked_tsri->sweep_num = 0;
    
    indices[2] = ll = INC_NDX(kk, data_info->getNumRaysRAT()); /* index of ray after */
    angles[2] = data_info->getRATangle(ll);

    for(ii=0; ii < 3; ii++) {
      if(fabs(angle_diff((float)theta, angles[ii])) > 5.) {
	    state = SFV_GARBAGE;
	}
	else {
	    state = SFV_CENTER;
	}
	data_info->loadRay(indices[ii]);
	
	sfv = wwptr->field_vals;
	if(ii == 1) {
	    sfv = sfv->next;
	}
	else if(ii == 2) {
	    sfv = sfv->next->next;
	}
	data_info->putFieldValues(wwptr->parameter_num, angles[ii],
				 (float)range, *sfv, state);
    }

    solo_new_list_field_vals(frme);

}

/* c------------------------------------------------------------------------ */
/* Calculate r & theta for the point of the screen relative to the radar.    */

void sp_xyz_to_rtheta_screen(const int frme, const PointInSpace &radar,
			     PointInSpace &pisp)
{
  WW_PTR wwptr = solo_return_wwptr(frme);
  double tilt = RADIANS(radar.getTilt());

  double range = sqrt(SQ(radar.getX()) + SQ(radar.getY()) + SQ(radar.getZ()));
  if (range < .0005)	/* half a meter */
  {
    pisp.setRange(0.0);
    pisp.setRotationAngle(0.0);
    return;
  }

  double tolerance = 1.e-4;
  double rotation;
  
  switch (wwptr->lead_sweep->scan_mode)
  {
  case DD_SCAN_MODE_AIR:
  {
    // Take into account the heading of the aircraft and the fact
    // that the screen is vertical rather than horizontal

    double heading = -RADIANS(radar.getHeading());
    double x = (radar.getX() * cos(heading)) - (radar.getY() * sin(heading));
    double z = radar.getZ();

    if (SQ(x) > tolerance)
    {
      rotation = atan2(z, x);
      range = fabs(sqrt(SQ(x) + SQ(z)) / cos(tilt));
    }
    else if (SQ(z) > tolerance)
    {
      rotation = z > 0.0 ? PIOVR2 : 3*PIOVR2;
      range = fabs(z / cos(tilt));
    }
    else
    {
      pisp.setRotationAngle(0.0);
      pisp.setRange(0.0);
      return;
    }
    break;
  }
  
  case DD_SCAN_MODE_RHI:
  {
    range = sqrt(SQ(radar.getX()) + SQ(radar.getY()) + SQ(radar.getZ()));
    
    if (range > tolerance)
    {
      rotation = asin(radar.getZ() / range);
    }
    else
    {
      rotation =  radar.getZ() >= 0 ? PIOVR2 : 3*PIOVR2;
    }
    break;
  }
  
  default:
  {
    double x = radar.getX();
    double y = radar.getY();
	
    if (SQ(x) > tolerance)
    {
      rotation = atan2(y, x);
      range = fabs(sqrt(SQ(x) + SQ(y)) / cos(tilt));
    }
    else if (SQ(y) > tolerance)
    {
      rotation =  y > 0.0 ? PIOVR2 : 3*PIOVR2;
      range = fabs(y / cos(tilt));
    }
    else
    {
      pisp.setRotationAngle(0.0);
      pisp.setRange(0.0);
      return;
    }
    break;
  }
  }

  pisp.setRange(range);
  pisp.setRotationAngle(fmod((double)450.0 - DEGREES(rotation),
			     (double)360.0));
}

/* c------------------------------------------------------------------------ */

