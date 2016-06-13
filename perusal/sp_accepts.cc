/* 	$Id$	 */

#include <gtk/gtk.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include <dd_math.h>
#include <solo_window_structs.h>
#include <seds.h>
#include <solo_list_widget_ids.h>
#include <solo_editor_structs.h>
#include <sed_shared_structs.h>
#include "sp_accepts.hh"
#include "sp_basics.hh"
#include "solo2.hh"
#include "sii_utils.hh"
#include "sii_perusal.hh"
#include <DataManager.hh>
#include <DateTime.hh>

/* c------------------------------------------------------------------------ */

void 
sp_cpy_lmrk_info (int src_frme, int dst_frme)
{
    WW_PTR wwptrs, wwptrd;

    if (src_frme == dst_frme)
      { return; }
    wwptrs = solo_return_wwptr(src_frme);
    wwptrd = solo_return_wwptr(dst_frme);

    memcpy(wwptrd->landmark_info, wwptrs->landmark_info
	   , sizeof(struct landmark_info));
    wwptrd->landmark_info->window_num = dst_frme;
    if(wwptrd->landmark_info->landmark_options != SOLO_LINKED_POSITIONING) {
	wwptrd->landmark_info->reference_frame = dst_frme+1;
    }
    wwptrd->landmark_info->changed = YES;
    wwptrd->landmark = wwptrs->landmark;
}
/* c------------------------------------------------------------------------ */

void 
sp_cpy_ctr_info (int src_frme, int dst_frme)
{
    WW_PTR wwptrs, wwptrd;

    if (src_frme == dst_frme)
      { return; }
    wwptrs = solo_return_wwptr(src_frme);
    wwptrd = solo_return_wwptr(dst_frme);

    memcpy(wwptrd->frame_ctr_info, wwptrs->frame_ctr_info
	   , sizeof(struct frame_ctr_info));
    wwptrd->frame_ctr_info->window_num = dst_frme;
    if(wwptrd->frame_ctr_info->centering_options != SOLO_LINKED_POSITIONING) {
	wwptrd->frame_ctr_info->reference_frame = dst_frme+1;
    }
    wwptrd->frame_ctr_info->changed = YES;
    wwptrd->center_of_view = wwptrs->center_of_view;
}
/* c------------------------------------------------------------------------ */

void 
solo_cpy_lock_info (int src_frme, int dst_frme)
{
    WW_PTR wwptrs, wwptrd;

    if (src_frme == dst_frme)
      { return; }
    wwptrs = solo_return_wwptr(src_frme);
    wwptrd = solo_return_wwptr(dst_frme);

    memcpy(wwptrd->lock, wwptrs->lock, sizeof(struct solo_plot_lock));
    wwptrd->lock->window_num = dst_frme;
    wwptrd->lock->changed = YES;
}
/* c------------------------------------------------------------------------ */

void 
solo_cpy_parameter_info (int src_frme, int dst_frme)
{
  if (src_frme == dst_frme)
    return;

  WW_PTR wwptrs = solo_return_wwptr(src_frme);
  WW_PTR wwptrd = solo_return_wwptr(dst_frme);

  wwptrd->parameter = wwptrs->parameter;

  wwptrd->parameter.window_num = dst_frme;
  wwptrd->palette = wwptrs->palette;
  wwptrd->parameter.changed = YES;
  wwptrd->color_bar_location = wwptrs->color_bar_location;
  wwptrd->color_bar_symbols = wwptrs->color_bar_symbols;
}
/* c------------------------------------------------------------------------ */

void 
solo_cpy_sweep_info (int src_frme, int dst_frme)
{
    WW_PTR wwptrs, wwptrd;

    if (src_frme == dst_frme)
      { return; }

    wwptrs = solo_return_wwptr(src_frme);
    wwptrd = solo_return_wwptr(dst_frme);

    memcpy(wwptrd->sweep, wwptrs->sweep, sizeof(struct solo_sweep_file));
    wwptrd->sweep->window_num = dst_frme;
    wwptrd->ddir_radar_num = wwptrs->ddir_radar_num;
    strncpy(wwptrd->radar->radar_name, wwptrs->sweep->radar_name, 16);
    wwptrd->show_file_info = wwptrs->show_file_info;
    wwptrd->sweep->changed = YES;
    wwptrd->radar_location = wwptrs->radar_location;
    wwptrd->start_time_text = wwptrs->start_time_text;
    wwptrd->stop_time_text = wwptrs->stop_time_text;

    wwptrd->d_prev_time_stamp = wwptrs->d_prev_time_stamp;
    wwptrd->d_sweepfile_time_stamp = wwptrs->d_sweepfile_time_stamp;
    wwptrd->swpfi_time_sync_set = wwptrs->swpfi_time_sync_set;
    wwptrd->swpfi_filter_set = wwptrs->swpfi_filter_set;
    wwptrd->filter_scan_modes = wwptrs->filter_scan_modes;
    wwptrd->filter_fxd_ang = wwptrs->filter_fxd_ang;
    wwptrd->filter_tolerance = wwptrs->filter_tolerance;
}
/* c------------------------------------------------------------------------ */

void 
solo_cpy_view_info (int src_frme, int dst_frme)
{
    WW_PTR wwptrs, wwptrd;

    if (src_frme == dst_frme)
      { return; }
    wwptrs = solo_return_wwptr(src_frme);
    wwptrd = solo_return_wwptr(dst_frme);

    memcpy(wwptrd->view, wwptrs->view
	   , sizeof(struct solo_view_info));
    wwptrd->view->window_num = dst_frme;
    wwptrd->view->changed = YES;
    wwptrd->magic_rng_annot = wwptrs->magic_rng_annot;
    
}
/* c------------------------------------------------------------------------ */

std::string solo_gen_file_info(int frme)
{
    WW_PTR wwptr;

    wwptr = solo_return_wwptr(frme);
    return wwptr->show_file_info;
}

/* c------------------------------------------------------------------------ */

void sp_set_landmark_info(const struct landmark_widget_info &lwi)
{
    /* this routine examines the info from the view widget
     *
     */
    int frme=lwi.frame_num;
    int ww, jj;
    int dangling_links[SOLO_MAX_WINDOWS];
    WW_PTR wwptr, wwptrc;
    struct solo_perusal_info *spi;

    wwptr = solo_return_wwptr(frme);
    spi = solo_return_winfo_ptr();

    wwptr->landmark_info->landmark_options = lwi.options;
    wwptr->landmark_info->linked_windows[frme] = YES;
    wwptr->landmark_info->reference_frame =
	  wwptr->landmark_info->landmark_options != SOLO_LINKED_POSITIONING
		? frme + 1 : lwi.reference_frame;

    if(SOLO_FIXED_POSITIONING SET_IN lwi.options) {
      wwptr->landmark.setLatitude(lwi.latitude);
      wwptr->landmark.setLongitude(lwi.longitude);
      wwptr->landmark.setAltitude(lwi.altitude);
    }

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!(spi->active_windows[ww])) {
	    wwptr->landmark_info->linked_windows[ww] = NO;
	    continue;
	}
	dangling_links[ww] =
	      wwptr->landmark_info->linked_windows[ww]
		    && !lwi.linked_windows[ww]
			  ? YES : NO;
	wwptr->landmark_info->linked_windows[ww]  =
	      lwi.linked_windows[ww];
    }

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!(spi->active_windows[ww]))
	      continue;
	if(ww == frme || !wwptr->landmark_info->linked_windows[ww])
	      continue;
	sp_cpy_lmrk_info(frme, ww);
    }

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!(spi->active_windows[ww]))
	      continue;
	wwptrc = solo_return_wwptr(ww);
	if(!dangling_links[ww])
	      continue;
	/* the dangling list becomes the links for the dangling frames
	 */
	for(jj=0; jj < SOLO_MAX_WINDOWS; jj++) 
	      wwptrc->landmark_info->linked_windows[jj] = dangling_links[jj];
    }
}
/* c------------------------------------------------------------------------ */

void sp_set_center_of_view_info(struct view_widget_info &pvi,
				struct centering_widget_info &cwi)
{
    /* this routine examines the info from the view widget
     *
     */
    int frme=pvi.frame_num, time_series;
    int ww, jj;
    int dangling_links[SOLO_MAX_WINDOWS], linked_windows[SOLO_MAX_WINDOWS];
    int ref_frme, option;
    WW_PTR wwptr, wwptrc;
    struct solo_perusal_info *spi;
    char mess[256];

    wwptr = solo_return_wwptr(frme);
    time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES;
    spi = solo_return_winfo_ptr();

    ref_frme = cwi.reference_frame;

    if(ref_frme < 1 || ref_frme > SOLO_MAX_WINDOWS ||
       !spi->active_windows[ref_frme-1]) {
	sprintf(mess, "Illegal reference frame number: %d\n", ref_frme);
	sii_message (mess);
	return;
    }
    option = cwi.options;

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!(spi->active_windows[ww]))
	      continue;
	linked_windows[ww] = cwi.linked_windows[ww];  
    }

    if(pvi.ctr_from_last_click && time_series) {
	for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	    if(!spi->active_windows[ww] || !linked_windows[ww])
		  continue;
	    wwptrc = solo_return_wwptr(ww);
	    wwptrc->clicked_ctr_of_frame = YES;
	    wwptrc->clicked_ctr.setLatitude(wwptrc->field_vals->next->lat);
	    wwptrc->clicked_ctr.setLongitude(wwptrc->field_vals->next->lon);
	    wwptrc->clicked_ctr.setAltitude(wwptrc->field_vals->next->alt);
	    wwptrc->view->ts_ctr_km = wwptrc->clicked_range;
	}
    }
    else if(pvi.ctr_from_last_click) {
	for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	    if(!spi->active_windows[ww] || !linked_windows[ww])
		  continue;
	    wwptrc = solo_return_wwptr(ww);
	    wwptrc->clicked_ctr_of_frame = YES;
	    wwptrc->clicked_ctr.setLatitude(wwptrc->field_vals->next->lat);
	    wwptrc->clicked_ctr.setLongitude(wwptrc->field_vals->next->lon);
	    wwptrc->clicked_ctr.setAltitude(wwptrc->field_vals->next->alt);
	    cwi.latitude = wwptrc->clicked_ctr.getLatitude();
	    cwi.longitude = wwptrc->clicked_ctr.getLongitude();
	    cwi.altitude = wwptrc->clicked_ctr.getAltitude();
	    wwptrc->center_of_view.setRotationAngle(wwptrc->clicked_angle);
	    wwptrc->center_of_view.setRange(wwptrc->clicked_range);
	    wwptrc->clicked_ctr.setRange(wwptrc->clicked_range);
	    wwptrc->clicked_ctr.setAzimuth(wwptrc->field_vals->next->az);
	    wwptrc->clicked_ctr.setElevation(wwptrc->field_vals->next->el);
	    wwptrc->clicked_ctr.rangeAzEl2xyz();
	}
    }
    else {
      if(fabs((double)(wwptr->center_of_view.getRotationAngle()
			 - cwi.az_of_ctr)) > .008) {
	wwptr->center_of_view.setRotationAngle(cwi.az_of_ctr);
	}
      if(fabs((double)(wwptr->center_of_view.getRange()
			 - cwi.rng_of_ctr)) > .0008) {
	wwptr->center_of_view.setRange(cwi.rng_of_ctr);
	}
    }

    /*
     * code to process centering widget info
     */
    wwptr->frame_ctr_info->centering_options = cwi.options;
    wwptr->frame_ctr_info->reference_frame =
	  wwptr->frame_ctr_info->centering_options != SOLO_LINKED_POSITIONING
		? frme + 1 : cwi.reference_frame;

    cwi.linked_windows[frme] = YES;
    if(cwi.options == SOLO_LINKED_POSITIONING)
	  cwi.linked_windows[cwi.reference_frame-1] = YES;

    if(SOLO_FIXED_POSITIONING SET_IN cwi.options) {
      wwptr->center_of_view.setLatitude(cwi.latitude);
      wwptr->center_of_view.setLongitude(cwi.longitude);
      wwptr->center_of_view.setAltitude(cwi.altitude);
    }
    pvi.ctr_from_last_click = 0;

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!(spi->active_windows[ww])) {
	    wwptr->frame_ctr_info->linked_windows[ww] = NO;
	    continue;
	  }
	dangling_links[ww] =
	      wwptr->frame_ctr_info->linked_windows[ww]
		    && !cwi.linked_windows[ww]
			  ? YES : NO;
	wwptr->frame_ctr_info->linked_windows[ww]  =
	      cwi.linked_windows[ww];
    }

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!(spi->active_windows[ww]))
	      continue;
	if(ww == frme || !wwptr->frame_ctr_info->linked_windows[ww])
	      continue;
	sp_cpy_ctr_info(frme, ww);
    }

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!(spi->active_windows[ww]))
	      continue;
	wwptrc = solo_return_wwptr(ww);
	if(!dangling_links[ww])
	      continue;
	/* the dangling list becomes the links for the dangling frames
	 */
	for(jj=0; jj < SOLO_MAX_WINDOWS; jj++) 
	      wwptrc->frame_ctr_info->linked_windows[jj] = dangling_links[jj];
    }
}
/* c------------------------------------------------------------------------ */

int solo_set_view_info(struct view_widget_info &pvi,
		       const struct landmark_widget_info &lwi,
		       struct centering_widget_info &cwi)
{
    /* this routine examines the info from the view widget
     *
     */
    int frme=pvi.frame_num, time_series;
    int ww, ii, j, changed=NO;
    int dangling_links[SOLO_MAX_WINDOWS];
    WW_PTR wwptr, wwptrc;
    struct solo_perusal_info *spi;
    char mess[256];

    if(frme < 0 || frme >= SOLO_MAX_WINDOWS) {
	sprintf(mess, "Bad frame number: %d\n", frme);
	solo_message(mess);
	return(SOLO_BAD_DIR_NAME);
    }
    wwptr = solo_return_wwptr(frme);
    wwptr->magic_rng_annot = pvi.magic_rng_annot;
    wwptr->view->type_of_plot = pvi.type_of_plot;
    time_series = wwptr->view->type_of_plot & SOLO_TIME_SERIES;

    if (time_series) {		/* set it in all windows */
      for (ii=0; ii < SOLO_MAX_WINDOWS; ii++)
	{ solo_return_wwptr(ii)->view->type_of_plot |= SOLO_TIME_SERIES; }
    }
    else {			/* clear it in all windows */
      for (ii=0; ii < SOLO_MAX_WINDOWS; ii++)
	{ solo_return_wwptr(ii)->view->type_of_plot &= ~SOLO_TIME_SERIES; }
    }
    spi = solo_return_winfo_ptr();

    sp_set_landmark_info(lwi);
    
    sp_set_center_of_view_info(pvi, cwi);
    /*
     * see what numbers have changed
     */
    if(FABS((wwptr->view->magnification -pvi.magnification)) >= .001) {
	wwptr->view->magnification = pvi.magnification > .001 ?
	      pvi.magnification : 1.0;
	changed = YES;
    }
    /* time series stuff
     */
    if(FABS((wwptr->view->ts_magnification -pvi.ts_magnification)) >= .001) {
	wwptr->view->ts_magnification = pvi.ts_magnification > .001 ?
	      pvi.ts_magnification : 1.0;
	changed = YES;
    }
    if (pvi.angular_fill_pct >= 0 && pvi.angular_fill_pct <= 500.) {
       wwptr->view->angular_fill_pct = pvi.angular_fill_pct;
    }
    if(FABS(wwptr->view->ts_ctr_km - pvi.ts_ctr_km) > .001) {
	wwptr->view->ts_ctr_km = pvi.ts_ctr_km;
    }

    if(pvi.az_annot_at_km <= 0) {
	wwptr->view->az_annot_at_km = -1;
    }
    else if(FABS((wwptr->view->az_annot_at_km
		     -pvi.az_annot_at_km)) > .001) {
	wwptr->view->az_annot_at_km = pvi.az_annot_at_km;
	changed = YES;
    }
    if(pvi.az_line_int_deg <= 0) {
      wwptr->view->az_line_int_deg = -1;
    }
    else if(FABS((wwptr->view->az_line_int_deg -pvi.az_line_int_deg))
       > .001) {
	wwptr->view->az_line_int_deg = pvi.az_line_int_deg;
	changed = YES;
    }

    if(pvi.rng_annot_at_deg <= 0) {
	wwptr->view->rng_annot_at_deg = -1;
    }
    else if(FABS((wwptr->view->rng_annot_at_deg
		     -pvi.rng_annot_at_deg)) > .001) {
	wwptr->view->rng_annot_at_deg = pvi.rng_annot_at_deg;
	changed = YES;
    }
    if(pvi.rng_ring_int_km <= 0) {
      wwptr->view->rng_ring_int_km = -1;
    }
    else if(fabs((double)(wwptr->view->rng_ring_int_km -pvi.rng_ring_int_km))
       > .001) {
	wwptr->view->rng_ring_int_km = pvi.rng_ring_int_km;
	changed = YES;
    }

    if(pvi.vert_tic_mark_km < .001) {
	wwptr->view->vert_tic_mark_km = 0;
    }
    else {
	wwptr->view->vert_tic_mark_km = pvi.vert_tic_mark_km;
    }
    if(pvi.horiz_tic_mark_km < .001) {
      wwptr->view->horiz_tic_mark_km = 0;
    }
    else {
      wwptr->view->horiz_tic_mark_km = pvi.horiz_tic_mark_km;
    }
    wwptr->view->changed = YES;

    /*
     * deal with the links
     */

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!(spi->active_windows[ww])) {
	    wwptr->view->linked_windows[ww] = NO;
	    continue;
	}
	if(wwptr->view->linked_windows[ww] && !pvi.linked_windows[ww])
	      dangling_links[ww] = YES;
	else
	      dangling_links[ww] = NO;
	wwptr->view->linked_windows[ww]  = pvi.linked_windows[ww];
    }
    /* now copy this view struct to the other linked windows
     */
    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!(spi->active_windows[ww]))
	      continue;
	if(ww == frme || !wwptr->view->linked_windows[ww])
	      continue;
	solo_cpy_view_info(frme, ww);
    }
    /* now deal with the dangling links
     */
    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!(spi->active_windows[ww]))
	      continue;
	if(!dangling_links[ww])
	      continue;
	/* the dangling list becomes the links for the dangling frames
	 */
	wwptrc = solo_return_wwptr(ww);
	for(j=0; j < SOLO_MAX_WINDOWS; j++) 
	      wwptrc->view->linked_windows[j] = dangling_links[j];
    }

    return(0);

}
