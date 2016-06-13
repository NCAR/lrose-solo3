/* 	$Id$	 */

#include <glib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <dd_math.h>
#include <dd_crackers.hh>
#include <DataManager.hh>
#include <DateTime.hh>
#include <PointInSpace.hh>
#include "se_bnd.hh"
#include "se_histog.hh"
#include "se_utils.hh"
#include <SeBoundaryList.hh>
#include <sii_utils.hh>
#include <sii_xyraster.hh>
#include <solo_editor_structs.h>
#include <solo_list_widget_ids.h>
#include <solo_window_structs.h>
#include <solo2.hh>
#include <sp_basics.hh>
#include <sp_clkd.hh>

// NOTE:  I copied this definition from dd_general_info.h.  This seems to be
// the only place it is used in the non-translate code.  Not sure if this
// should go somewhere else.

# ifndef GNERIC_DESC
# define GNERIC_DESC

struct generic_descriptor {
    char name_struct[4];        /* "????" */
    int32_t sizeof_struct;
};

# endif  /* GNERIC_DESC */

static struct bnd_point_mgmt *bpm_spairs = 0;

/* c------------------------------------------------------------------------ */

void xse_absorb_bnd (void)
{
  SeBoundaryList *sebs = SeBoundaryList::getInstance();

  // Construct the file path

  std::string file_path = sebs->directoryText;
  if (file_path[file_path.size()-1] != '/')
    file_path += "/";
  file_path += sebs->fileNameText;

  // Open the file

  FILE *stream;
  
  if ((stream = fopen(file_path.c_str(), "r")) == 0)
  {
    char message[256];
    sprintf(message, "Unable to open boundary file %s\n", file_path.c_str());
    sii_message(message);
    return;
  }

  // Determine the size of the file

  fseek(stream, 0L, SEEK_END); /* at end of file */
  int len = ftell(stream);	/* how big is the file */
  rewind(stream);
      
  // Read the file

  char *buf = (char *)malloc(len);
  memset(buf, 0, len);

  int bytes_read;
  
  if ((bytes_read = fread(buf, sizeof(char), len, stream)) < len)
  {
    char message[1024];
    
    sprintf(message, "Unable to read file %s\n", file_path.c_str());
    sii_message(message);
    free(buf);
    return;
  }

  // Close the file

  fclose(stream);

  // Unpack the buffer

  se_unpack_bnd_buf(buf, len);

  // Reclaim space

  free(buf);
}

/* c------------------------------------------------------------------------ */

void xse_add_bnd_pt(struct solo_click_info *sci, struct one_boundary *ob,
		    const bool allow_duplicates)
{
  // Nab a fresh boundary point.  This seems to pull an unused structure from
  // bpm_spairs, just to avoid allocating memory.

  struct bnd_point_mgmt *bpm = se_pop_bpms();

  // The append happens first so the last pointers are fresh.

  se_append_bpm(&ob->top_bpm, bpm);
  bpm->x = sci->x;
  bpm->y = sci->y;

  if (++ob->num_points > 1)
  {
    // Avoid duplicates.

    if (!allow_duplicates && bpm->x == bpm->last->x && bpm->y == bpm->last->y)
    {
      ob->num_points--;
      se_delete_bnd_pt(bpm, ob);
      return;
    }
  }
  bpm->which_frame = sci->frame;

  SeBoundaryList *sebs = SeBoundaryList::getInstance();

  if (!(sebs->viewBounds || sebs->absorbingBoundary))
  {
    // This routine is in ../perusal/sp_clkd.c

    sp_locate_this_point(sci, bpm);
  }
  else if (sebs->absorbingBoundary)
  {
    delete bpm->pisp;
    bpm->pisp = new PointInSpace(sebs->pisp);
    bpm->r = (int32_t)KM_TO_M(bpm->pisp->getRange());
  }

  double x = sci->x;
  double y = sci->y;
  if (bpm->pisp->isTimeSeries())
  {
    x = bpm->pisp->getTime();
    y = bpm->pisp->getRange();
  }

  // The rasterization code sets sebs->view_bounds to YES and also uses
  // this routine and others to bound the rasterization.

  if (ob->num_points > 1)
  {
    if (!sebs->viewBounds)
    {
      se_draw_bnd(bpm, 2, NO);
      return_sed_stuff()->boundary_exists = YES;
      sii_set_boundary_pt_count (sci->frame, ob->num_points);
    }
    if(x > ob->max_x)
      ob->max_x = x;
    if(x < ob->min_x)
      ob->min_x = x;
    if(y > ob->max_y)
      ob->max_y = y;
    if(y < ob->min_y)
      ob->min_y = y;

    // Calculate boundary point attributes

    se_bnd_pt_atts(bpm);
    xse_x_insert(bpm, ob);
    xse_y_insert(bpm, ob);

    // Now do it for the line between this point and the first point

    bpm = ob->top_bpm;
    se_bnd_pt_atts(bpm);
  }
  else
  {
    // First point

    if (!(sebs->absorbingBoundary || sebs->viewBounds))
    {
      // Get the radar origin and radar name from the first point

      strncpy(ob->bh->radar_name,
	      solo_return_radar_name(sci->frame).c_str(), 12);
      sebs->setOrigin(*bpm->pisp);
      sebs->setOriginId("BND_ORIGIN");
    }
    ob->min_x = ob->max_x = x;
    ob->min_y = ob->max_y = y;
  }
}

/* c------------------------------------------------------------------------ */

void se_append_bpm(struct bnd_point_mgmt **top_bpm, struct bnd_point_mgmt *bpm)
{
  // Append this point to the list of boundary points.
  // (*top_bpm)->last always points to the last point appended
  // to the boundary,  thus all points are linked by the last pointer
  // except for the last point appended the next pointer points
  // to the next point and the NULL pointer in the last point
  // serves to terminate loops.

  if (*top_bpm == 0)
  {
    // No list yet

    *top_bpm = bpm;
    (*top_bpm)->last = bpm;
  }
  else
  {
    // Last bpm on list should point to this one

    (*top_bpm)->last->next = bpm;
    bpm->last = (*top_bpm)->last;
    (*top_bpm)->last = bpm;
  }
  bpm->next = NULL;

  return;
}

/* c------------------------------------------------------------------------ */

void 
se_bnd_pt_atts (struct bnd_point_mgmt *bpm)
{

  if (bpm->pisp->isTimeSeries()) {
    bpm->dt = bpm->last->pisp->getTime() - bpm->pisp->getTime();
    bpm->dr = bpm->last->pisp->getRange() - bpm->pisp->getRange();
    if(bpm->dt) bpm->slope = bpm->dr/bpm->dt;
    if(bpm->dr) bpm->slope_90 = -1./bpm->slope; 
    bpm->len = SQRT(SQ(bpm->dt) + SQ(bpm->dr));
    bpm->t_mid = bpm->pisp->getTime() + 0.5 * bpm->dt;
    bpm->r_mid = bpm->pisp->getRange() + 0.5 * bpm->dr;
    }
    else {
	bpm->dy = bpm->last->y - bpm->y;
	bpm->dx = bpm->last->x - bpm->x;
	
	if(bpm->dx)
	      bpm->slope = (double)bpm->dy/bpm->dx;
	
	if(bpm->dy)
	      bpm->slope_90 = -1./bpm->slope; /* slope of the line
					       * perpendicular to this line */
	
	bpm->len = sqrt((SQ((double)bpm->dx) + SQ((double)bpm->dy)));
	bpm->x_mid = (int32_t)(bpm->x + 0.5 * bpm->dx);
	bpm->y_mid = (int32_t)(bpm->y + 0.5 * bpm->dy);
    }
}

/* c------------------------------------------------------------------------ */

int 
xse_ccw (double x0, double y0, double x1, double y1)
{
    /* is point 0 left or right of a line between the origin and point 1
     * form a line between the origin and point 0 called line 0
     */
    if(y0 * x1 > y1 * x0)
	  return(1);
				/*
				 * this says the slope of line 0
				 * is greater than the slope of line 1
				 * and is counter-clockwise or left
				 * of the line.
				 */
    if(y0 * x1 < y1 * x0)
	  return(-1);
				/* cw or right */
    return(0);			/* on the line */
}

/* c------------------------------------------------------------------------ */

void 
se_clr_all_bnds (void)
{
    int ww;
    struct one_boundary *ob;
    struct solo_edit_stuff *seds;
    struct solo_perusal_info *spi;

    spi = solo_return_winfo_ptr();
    seds = return_sed_stuff();
    seds->boundary_exists = NO;
    SeBoundaryList *sebs = SeBoundaryList::getInstance();
    ob = sebs->currentBoundary = sebs->firstBoundary;

    for(; ob; ob = ob->next) {
	se_clr_bnd(ob);
    }
    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!spi->active_windows[ww])
	      continue;
	se_clear_segments(ww);
    }
}

/* c------------------------------------------------------------------------ */

void se_clr_bnd(struct one_boundary *ob)
{
  se_push_all_bpms(&ob->top_bpm);

  ob->bh->force_inside_outside = 0;
  ob->num_points = 0;

  ob->last_line = 0;
  ob->last_point = 0;
  ob->x_mids = 0;
  ob->y_mids = 0;
}

/* c------------------------------------------------------------------------ */

void 
se_clr_current_bnd (void)
{
    struct one_boundary *ob;
    int erase, ww;

    SeBoundaryList *sebs = SeBoundaryList::getInstance();

    if(!sebs->firstBoundary)
	  return;
    ob = sebs->currentBoundary;
    erase = ob->num_points > 1;

    se_clr_bnd(sebs->currentBoundary);

    if(erase) {
      for (ww=0; ww < SOLO_MAX_WINDOWS; ww++)
	{ se_clear_segments (ww); }
    }
}

/* c------------------------------------------------------------------------ */

void 
se_cycle_bnd (void)
{
    /* the purpose of this routine is to move between boundaries
     * the first move from a boundary with two or more points in it
     * might be to an empty boundary in order to start a new boundary
     * so a move always clears the screen and may redraw a boundary if
     * the move is to an existing boundary otherwise the screen will
     * be blank and can be refilled with a redraw boundaries command
     * moves will always be in on direction towards an empty boundary
     * a move from an empty (the last) boundary will be to the first
     * boundary.
     */
    int ww;
    struct one_boundary *ob;
    struct solo_perusal_info *spi;

    spi = solo_return_winfo_ptr();
    SeBoundaryList *sebs = SeBoundaryList::getInstance();
    ob = sebs->currentBoundary;

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!spi->active_windows[ww])
	      continue;
	se_clear_segments(ww);
    }

    /* check to see if we are moving from a cleared boundary to a
     * boundary with points in it
     */
    if(!ob->num_points && ob->next && ob->next->num_points) {
	sebs->currentBoundary = ob->next;
	/* if so put the cleared boundary at the end
	 */
	if(ob == sebs->firstBoundary) {
	    sebs->firstBoundary->next->last = sebs->firstBoundary->last;
	    sebs->firstBoundary = sebs->firstBoundary->next;
	}
	else {
	    ob->last->next = ob->next;
	    ob->next->last = ob->last;
	}
	sebs->firstBoundary->last->next = ob;
	ob->last = sebs->firstBoundary->last;
	sebs->firstBoundary->last = ob;
	ob->next = NULL;
    }
    else if(!ob->num_points) {
	/* shift back to first boundary
	 * does the right thing when the first boundary is empty
	 */
	sebs->currentBoundary = sebs->firstBoundary;
    }
    else {
	se_return_next_bnd();
    }
    ob = sebs->currentBoundary;

    if(ob->num_points > 1) {
	se_draw_bnd(ob->top_bpm->last, ob->num_points, NO);
    }
}

/* c------------------------------------------------------------------------ */

void se_delete_bnd_pt(struct bnd_point_mgmt *bpm, struct one_boundary *ob)
{
  // Remove this point from the boundary.
  // Remember to recalculate mins, maxs, slopes and deltas.

  if (bpm == ob->top_bpm)
  {
    if (!bpm->next)
    {
      // This is the only point

      ob->top_bpm = NULL;
    }
    ob->top_bpm->next->last = ob->top_bpm->last;
    ob->top_bpm = ob->top_bpm->next;
  }
  else
  {
    bpm->last->next = bpm->next;
    if (bpm->next)
      bpm->next->last = bpm->last;
    if (bpm == ob->top_bpm->last)
      ob->top_bpm->last = bpm->last;
  }
  se_push_bpm(bpm);

  // Now do it for the line between this point and the first point.

  bpm = ob->top_bpm;
  se_bnd_pt_atts(bpm);

  ob->max_x = ob->min_x = bpm->x;
  ob->max_y = ob->min_y = bpm->y;
  for (bpm = bpm->next; bpm; bpm = bpm->next)
  {
    if (bpm->x > ob->max_x)
      ob->max_x = bpm->x;
    if (bpm->x < ob->min_x)
      ob->min_x = bpm->x;
    if (bpm->y > ob->max_y)
      ob->max_y = bpm->y;
    if (bpm->y < ob->min_y)
      ob->min_y = bpm->y;
  }
}

/* c------------------------------------------------------------------------ */

void se_draw_bnd(struct bnd_point_mgmt *bpm, const int num, const int erase)
{
  // Draw or erase the requisite number of points in the active windows.
  // see "se_shift_bnd()" for similar logic.

  struct solo_perusal_info *spi = solo_return_winfo_ptr();
  SeBoundaryList *sebs = SeBoundaryList::getInstance();
  if (sebs->viewBounds)
    return;

  PointInSpace boundary_radar = sebs->getOrigin();
  double untilt = fabs(cos(RADIANS(boundary_radar.getTilt())));

  for (int ww = 0; ww < SOLO_MAX_WINDOWS; ww++)
  {
    if (!spi->active_windows[ww])
      continue;

    WW_PTR wwptr = solo_return_wwptr(ww);
    PointInSpace current_radar = wwptr->radar_location;

    // We now have the frame radar (current_radar) and the location of the
    // boundary radar.
    // See also the routine "sp_locate_this_point()" in file "sp_clkd.c"

    current_radar.latLonRelative(boundary_radar);

    // x,y,z now contains the coordinates of the boundary radar
    // relative to the current radar

    double x = KM_TO_M(current_radar.getX());
    double y = KM_TO_M(current_radar.getY());
    double z = KM_TO_M(current_radar.getZ());

    double costilt = cos(fabs(RADIANS(current_radar.getTilt())));
    double tiltfactor;
    
    if (fabs(costilt) > 1.e-6)
      tiltfactor = fabs(1./costilt);
    else
      tiltfactor = 0;

    int mm = num;
    struct bnd_point_mgmt *bpmx = bpm;

    if (wwptr->view->type_of_plot & SOLO_TIME_SERIES)
    {
      double d = sp_meters_per_pixel();
      d /= (double)wwptr->view->ts_magnification;
      double v_scale = M_TO_KM(d);
      int height = wwptr->view->height_in_pix - 1;
      double top_edge = wwptr->view->ts_ctr_km + 0.5 * height * v_scale;

      double ta = wwptr->sweep->start_time;
      double time_span = wwptr->sweep->stop_time - ta;
      int width = wwptr->view->width_in_pix-1;
      double h_scale = (double)width/time_span;
      int right_left = wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT;

      // We will be passing pixel coodinates and assuming that (0,0) is
      // the upper left corner of the screen

      for (; mm--; bpmx = bpmx->last)
      {
	bpmx->_x = (int32_t)((bpmx->pisp->getTime() - ta) * h_scale + 0.5);
	if (right_left)
	  bpmx->_x = width - bpmx->_x;
	bpmx->_y = (int32_t)((top_edge - bpmx->pisp->getZ()) / v_scale + 0.5);
      }
    }	
    else if (wwptr->lead_sweep->sweep->radar_type == DD_RADAR_TYPE_GROUND ||
	     wwptr->lead_sweep->sweep->radar_type == DD_RADAR_TYPE_SHIP)
    {
      // Ground based

      // We need to project the original values and then unproject the
      // new values

      for (; mm--; bpmx = bpmx->last)
      {
	switch (wwptr->lead_sweep->scan_mode)
	{
	case DD_SCAN_MODE_RHI:
	  bpmx->_x = bpmx->x;
	  bpmx->_y = bpmx->y;
	  break;
	default:
	  bpmx->_x = (int32_t)((bpmx->x * untilt + x) * tiltfactor);
	  bpmx->_y = (int32_t)((bpmx->y * untilt + y) * tiltfactor);
	  break;
	}
      }
    }
    else
    {
      for (; mm--; bpmx = bpmx->last)
      {
	bpmx->_x = bpmx->x;
	bpmx->_y = bpmx->y;
      }
    }

    if (erase)
      se_erase_segments(ww, num, bpm);
    else
      se_draw_segments(ww, num, bpm);
  }
}

/* c------------------------------------------------------------------------ */

void se_erase_all_bnds()
{
    int ww;
    struct one_boundary *ob;
    struct solo_perusal_info *spi;

    spi = solo_return_winfo_ptr();
    SeBoundaryList *sebs = SeBoundaryList::getInstance();
    ob = sebs->firstBoundary;

    for(ww=0; ww < SOLO_MAX_WINDOWS; ww++) {
	if(!spi->active_windows[ww])
	      continue;
	se_clear_segments(ww);
    }
}

/* c------------------------------------------------------------------------ */

void se_ts_find_intxns(double radar_alt, double d_max_range, struct one_boundary *ob, 
					   double d_time, double d_pointing, int automatic, int down, double d_ctr)
{
   int mm;
   struct bnd_point_mgmt *bpm, *bpma, *bpmb;
   double d, ta, tb, zz;

   ob->num_intxns = 0;
   ob->first_intxn = NULL;
   mm = ob->num_points;
   bpm = ob->top_bpm;
   d_time += .005;
   d = RADIANS(CART_ANGLE(d_pointing));
   /*
    * we bump the time by 5 milliseconds so a ray and a vertex
    * are not coincident in time
    */
   if(automatic) {
      down = sin(d) < 0;
   }
   else {
      radar_alt = down ? 2. * d_ctr : 0;
   }
    
   for(; mm--; bpm = bpm->last) {
     if (bpm->last->pisp->getTime() < bpm->pisp->getTime()) {
	 bpma = bpm->last;
	 bpmb = bpm;
      }
      else {
	 bpma = bpm;
	 bpmb = bpm->last;
      }
     ta = bpma->pisp->getTime();
     tb = bpmb->pisp->getTime();

      if(d_time >= ta && d_time < tb) {
	 /* possible intxn */
	zz = ((d_time - ta)/(tb - ta)) * (bpmb->pisp->getZ() - bpma->pisp->getZ())
	  + bpma->pisp->getZ();
	 if((down && zz < radar_alt) || (!down && zz > radar_alt)) {
	    /* intxn! */
	    bpm->rx = down
	      ? (int32_t)KM_TO_M(radar_alt -zz) : (int32_t)KM_TO_M(zz - radar_alt);
	    ob->num_intxns++;
	    bpm->next_intxn = NULL;
	    se_merge_intxn(bpm, ob);
	 }
      }
   }
   ob->radar_inside_boundary = ob->num_intxns & 1;
   /* i.e. an odd number of intxns implies radar inside */

   if(ob->bh->force_inside_outside) {
      if(ob->bh->force_inside_outside == BND_FIX_RADAR_INSIDE) {
	 ob->radar_inside_boundary = YES;
      }
      if(ob->bh->force_inside_outside == BND_FIX_RADAR_OUTSIDE) {
	 ob->radar_inside_boundary = NO;
      }
   }
}

/* c------------------------------------------------------------------------ */

int se_merge_intxn(struct bnd_point_mgmt *bpm,  struct one_boundary *ob)
{
    struct bnd_point_mgmt *bpmx;

    if(!(bpmx = ob->first_intxn)) { /* first intersection */
	ob->first_intxn = bpm->last_intxn = bpm;
	return(YES);
    }

    for(; bpmx; bpmx = bpmx->next_intxn) {
	if(bpm->rx < bpmx->rx) { 
				/* insert intxn here */
	    bpm->next_intxn = bpmx;
	    bpm->last_intxn = bpmx->last_intxn;
	    if(bpmx == ob->first_intxn) { /* new first intxn */
		ob->first_intxn = bpm;
	    }
	    else {
		bpmx->last_intxn->next_intxn = bpm;
	    }
	    bpmx->last_intxn = bpm;
	    break;
	}
    }
    if(!bpmx) {			/* furthest out...tack it onto the end */
	ob->first_intxn->last_intxn->next_intxn = bpm;
	bpm->last_intxn = ob->first_intxn->last_intxn;
	ob->first_intxn->last_intxn = bpm;
    }
    return(YES);
}

/* c------------------------------------------------------------------------ */

int xse_find_intxns(double angle, double range, struct one_boundary *ob)
{
  // This routine creates a list of endpoints of lines that intersect
  // the current ray

  double theta = RADIANS(CART_ANGLE(angle));

  ob->num_intxns = 0;
  ob->first_intxn = NULL;

  // Compute the endpoints of the ray

  double xx = range * cos(theta);
  double yy = range * sin(theta);
  int32_t x = (int32_t)(xx < 0 ? xx -.5 : xx +.5);
  int32_t y = (int32_t)(yy < 0 ? yy -.5 : yy +.5);

  double slope = 0.0;
  if (xx != 0.0) slope = yy / xx;

  // Find the first point that is not on the line

  // When we are doing intersections, we want to use the shifted
  // x,y values because the boundary may be for another radar

  struct bnd_point_mgmt *bpmx = 0;
  struct bnd_point_mgmt *bpm = ob->top_bpm;

  for (bpmx = NULL; bpm != 0; bpm = bpm->next)
  {
    bpm->which_side = xse_ccw((double)bpm->_x, (double)bpm->_y, xx, yy);
    if (bpm->which_side)
    {
      bpmx = bpm;
      break;
    }
  }

  // Check for no intersections

  if (bpmx == 0)
    return 0;
  
  int mm = ob->num_points;

  // Since "->last" links all points we loop through all the
  // points using the last pointer

  for (; mm--; bpm = bpm->last)
  {
    // a return of 0 from se_ccw says this point is
    // colinear with the ray. So we do nothing.

    bpm->last->which_side =
      xse_ccw((double)bpm->last->_x, (double)bpm->last->_y, xx, yy);

    if (bpm->last->which_side)
    {
      if (bpm->last->which_side != bpmx->which_side)
      {
	// We may have an intersection between "bpm"
	// and "bpm->last". See if it's real.

	xse_set_intxn(xx, yy, slope, bpm, ob);
      }
      bpmx = bpm->last;
    }
  }

  return ob->num_intxns;
}

/* c------------------------------------------------------------------------ */

struct one_boundary *se_malloc_one_bnd()
{
  struct one_boundary *ob;
  ob = new struct one_boundary;

  ob->last = 0;
  ob->next = 0;
  
  ob->top_bpm = 0;
  ob->x_mids = 0;
  ob->y_mids = 0;
  ob->first_intxn = 0;
  ob->next_segment = 0;
  ob->last_line = 0;
  ob->last_point = 0;
  
  ob->r0 = 0.0;
  ob->r1 = 0.0;
  ob->num_segments = 0;
  ob->num_intxns = 0;
  ob->num_points = 0;
  ob->min_x = 0.0;
  ob->max_x = 0.0;
  ob->min_y = 0.0;
  ob->max_y = 0.0;
  ob->min_z = 0.0;
  ob->max_z = 0.0;
  ob->radar_inside_boundary = 0;
  ob->open_boundary = 0;

  ob->bh = new struct boundary_header;
  strncpy(ob->bh->name_struct, "BDHD", 4);
  ob->bh->sizeof_struct = sizeof(struct boundary_header);
  ob->bh->time_stamp = 0;
  ob->bh->offset_to_origin_struct = sizeof(struct boundary_header);
  ob->bh->type_of_info = 0;
  ob->bh->num_points = 0;
  ob->bh->offset_to_first_point =
    ob->bh->offset_to_origin_struct + PointInSpace::NUM_BYTES;
  ob->bh->sizeof_boundary_point = PointInSpace::NUM_BYTES;
  ob->bh->xy_plane_horizontal = 0;
  ob->bh->open_boundary = 0;
  ob->bh->version = 0;
  for (int i = 0; i < 12; ++i)
    ob->bh->radar_name[i] = '\0';
  for (int i = 0; i < 16; ++i)
    ob->bh->user_id[i] = '\0';
  ob->bh->force_inside_outside = 0;

  return ob;
}

/* c------------------------------------------------------------------------ */

void se_nab_segment(const int num, double &r0, double &r1,
		    struct one_boundary *ob)
{
  // This routine returns the start and stop range of the requested segment

  if (num < 0 || num >= ob->num_segments)
  {
    r0 = -999999.0;
    r1 = -999998.0;
    return;
  }

  if (num != 0)
  {
    r0 = ob->next_segment->rx;

    if (ob->next_segment->next_intxn)
    {
      r1 = ob->next_segment->next_intxn->rx;
      ob->next_segment = ob->next_segment->next_intxn->next_intxn;
    }
    else
    {
      r1 = 1.0e9;
    }
  }
  else
  {
    // First segment

    r0 = ob->r0;
    r1 = ob->r1;
  }
}

/* c------------------------------------------------------------------------ */

int xse_num_segments(struct one_boundary *ob)
{
    /* calculate the number of segments and set up
     * the first segment
     */
    int nx;

    nx = ob->num_intxns;

    if(ob->radar_inside_boundary) {
	if(!nx) {
	    /* the end of the ray is inside the boundary */
	    ob->num_segments = 1;
	    ob->r0 = 0;
	    ob->r1 = 1.e9;
	    return(1);
	}
	ob->r0 = 0;
	ob->r1 = ob->first_intxn->rx;
	ob->next_segment = ob->first_intxn->next_intxn;

	if(nx & 1) {		/* no funny stuff */
	    ob->num_segments = (nx+1)/2;
	}
	else {
	    /*
	     * even number of intersections
	     * assume the boundary is past the end of the ray
	     */
	    ob->num_segments = nx/2 +1;
	}
	return(ob->num_segments);
    }
    /* radar is outside the boundary
     */
    if(!nx) {
	ob->num_segments = 0;
	return(ob->num_segments);
    }
    ob->r0 = ob->first_intxn->rx;

    if(nx & 1) {		/* the boundary is past the end of the ray */
	if(nx == 1) {
	    ob->num_segments = 1;
	    ob->r1 = 1.e9;
	    return(1);
	}
	ob->num_segments = (nx+1)/2;
    }
    else {
	ob->num_segments = nx/2;
    }
    ob->r1 = ob->first_intxn->next_intxn->rx;
    ob->next_segment = ob->first_intxn->next_intxn->next_intxn;

    return(ob->num_segments);
}

/* c------------------------------------------------------------------------ */

bool se_compare_bnds(char *aa, char *bb, int size)
{
    struct boundary_header bh, *bha, *bhb;
    struct point_in_space *pispa, *pispb;
    struct generic_descriptor *gda, *gdb;
    char *ee = aa + size;
    
    for(; aa < ee; ) {
	gda = (struct generic_descriptor *)aa;
	gdb = (struct generic_descriptor *)bb;
	if(gda->sizeof_struct != gdb->sizeof_struct)
	      return true;

	if(!strncmp(aa, "BDHD", 4)) {
	    /* copy aa to local and make the times the same before comparing
	     */
	    memcpy(&bh, aa, gda->sizeof_struct);
	    bha = (struct boundary_header *)aa;
	    bhb = (struct boundary_header *)bb;
	    bh.time_stamp = bhb->time_stamp;
	    if (memcmp(&bh, bb, gda->sizeof_struct) != 0)
		  return false;	/* they don't compare */
	}
	else if(!strncmp(aa, "PISP", 4)) {
	    pispa = (struct point_in_space *)aa;
	    pispb = (struct point_in_space *)bb;

	    if (memcmp(aa, bb, gda->sizeof_struct) != 0)
		  return false;	/* they don't compare */
	}
	aa += gda->sizeof_struct;
	bb += gda->sizeof_struct;
    }

    return true;
}

/* c------------------------------------------------------------------------ */

void se_pack_bnd_set(char *buf)
{
    struct one_boundary *ob;
    struct boundary_header *bh;
    struct bnd_point_mgmt *bpm;
    char *bb = buf;

    if(!(buf))
	  return;
    SeBoundaryList *sebs = SeBoundaryList::getInstance();
    ob = sebs->firstBoundary;


    /*
     * for each boundary
     */
    for(; ob; ob = ob->next) {
	bh = ob->bh;
	bh->time_stamp = time(0);
	/* reset all this stuff in case we sucked in an old boundary
	 */
	ob->bh->sizeof_struct = sizeof(struct boundary_header);
	ob->bh->offset_to_origin_struct = sizeof(struct boundary_header);
	ob->bh->offset_to_first_point =
	  ob->bh->offset_to_origin_struct + PointInSpace::NUM_BYTES;
	ob->bh->sizeof_boundary_point = PointInSpace::NUM_BYTES;

	if((bh->num_points = ob->num_points) > 1) {
	    /*
	     * the boundary header
	     */
	    memcpy(bb, bh, sizeof(struct boundary_header));
	    bb += sizeof(struct boundary_header);
	    /*
	     * the radar location
	     */
	    PointInSpace sebs_origin = sebs->getOrigin();
	    memcpy(bb, sebs_origin.getBufPtr(), sebs_origin.getBufLen()); 
	    bb += sebs_origin.getBufLen();
	    /*
	     * now the points in the boundary
	     */
	    bpm = ob->top_bpm;
	    
	    for(; bpm; bpm = bpm->next) {
	      memcpy(bb, bpm->pisp->getBufPtr(), bpm->pisp->getBufLen());
	      bb += bpm->pisp->getBufLen();
	    }
	}
    }
}

/* c------------------------------------------------------------------------ */

struct bnd_point_mgmt *se_pop_bpms()
{
    struct bnd_point_mgmt *bpm=bpm_spairs;

    if(!bpm_spairs) {
	bpm = (struct bnd_point_mgmt *)malloc(sizeof(struct bnd_point_mgmt));
	memset(bpm, 0, sizeof(struct bnd_point_mgmt)); /* clear it out */
	if(!bpm) {
	    printf("Unable to pop bpms\n");
	    exit(1);
	}
	bpm->pisp = new PointInSpace();
    }
    else {
	bpm_spairs = bpm->next;
    }
    PointInSpace *pisp = bpm->pisp;
    memset(bpm, 0, sizeof(struct bnd_point_mgmt)); /* clear it out */
    bpm->pisp = pisp;		/* except for the pisp */
    return(bpm);
}

/* c------------------------------------------------------------------------ */

void se_prev_bnd_set()
{
    struct solo_edit_stuff *seds;

    seds = return_sed_stuff();
    if(!seds->last_pbs)
	  return;
    SeBoundaryList *sebs = SeBoundaryList::getInstance();

    if(sebs->lastOperation != PREV_BND_SET) {
	seds->pbs = seds->last_pbs;
    }
    else {
	seds->pbs = seds->pbs->last;
    }
    se_clr_all_bnds();
    if(seds->pbs)
	  se_unpack_bnd_buf(seds->pbs->at, seds->pbs->sizeof_set);
}

/* c------------------------------------------------------------------------ */

void se_push_all_bpms(struct bnd_point_mgmt **bpmptr)
{
    struct bnd_point_mgmt *bpm=(*bpmptr);

    if(!bpm)
	  return;
    bpm->last->next = bpm_spairs;
    bpm_spairs = bpm;
    *bpmptr = NULL;
    return;
}

/* c------------------------------------------------------------------------ */

void se_push_bpm(struct bnd_point_mgmt *bpm)
{
    bpm->next = bpm_spairs;
    bpm_spairs = bpm;
    return;
}

/* c------------------------------------------------------------------------ */

int se_radar_inside_bnd(struct one_boundary *ob)
{
    /* determine if the radar is inside or outside the boundary
     */
    struct bnd_point_mgmt *bpm;
    double r, x, y, theta;
    int ii, mm = ob->num_points-1, nn, inside_count=0;


    if(ob->bh->force_inside_outside) {
	if(ob->bh->force_inside_outside == BND_FIX_RADAR_INSIDE) {
	    return(ob->radar_inside_boundary = YES);
	}
	if(ob->bh->force_inside_outside == BND_FIX_RADAR_OUTSIDE) {
	    return(ob->radar_inside_boundary = NO);
	}
    }

    bpm = ob->top_bpm;
    x = ABS(bpm->_x);
    y = ABS(bpm->_y);
    /*
     * we are using the shifted values of x and y in case this boundary
     * is for a different radar
     */
    for(bpm = bpm->next; mm--; bpm = bpm->next) {
	if(ABS(bpm->_x) > x) x = ABS(bpm->_x);
	if(ABS(bpm->_y) > y) y = ABS(bpm->_y);
    }
    x += 11;
    y += 11;

    for(ii=0; ii < 4; ii++) {
	switch(ii) {
	case 1:
	    x = -x;		/* x negative, y positive */
	    break;
	case 2:
	    y = -y;		/* both negative */
	    break;
	case 3:
	    x = -x;		/* x postive, y negative */
	    break;
	default:		/* case 0: both positive */
	    break;
	}
	r = sqrt(SQ(x)+SQ(y));
	theta = atan2(y, x);
	theta = DEGREES(theta);
	theta = CART_ANGLE(theta);
	theta = FMOD360(theta);
	nn = xse_find_intxns(theta, r, ob);
	inside_count += (int)(nn & 1); /* i.e. and odd number of intxns */
    }
    ob->radar_inside_boundary = inside_count > 2;
    return(ob->radar_inside_boundary);
}

/* c------------------------------------------------------------------------ */

void se_redraw_all_bnds()
{
    struct one_boundary *ob;
    struct solo_perusal_info *spi;

    spi = solo_return_winfo_ptr();
    SeBoundaryList *sebs = SeBoundaryList::getInstance();
    ob = sebs->firstBoundary;

    for(; ob; ob = ob->next) {
	if(ob->num_points > 1) {
	    se_draw_bnd(ob->top_bpm->last, ob->num_points, NO);
	}
    }
}

/* c------------------------------------------------------------------------ */

struct one_boundary *se_return_next_bnd()
{
    struct one_boundary *ob;

    SeBoundaryList *sebs = SeBoundaryList::getInstance();

    if(sebs->currentBoundary->next) {
	ob = sebs->currentBoundary->next;
    }
    else {
	ob = se_malloc_one_bnd();
	sebs->currentBoundary->next = ob;
	ob->last = sebs->currentBoundary;
	sebs->firstBoundary->last = ob;
    }
    sebs->currentBoundary = ob;
    return(ob);
}

/* c------------------------------------------------------------------------ */

void xse_save_bnd ()
{
    int bn, nn, size, bnd_loop;
    FILE *stream, *ASCIIstream;
    struct boundary_header *bh;
    struct bnd_point_mgmt *bpm;
    WW_PTR wwptr;
    struct one_boundary *ob;
    char *a, str[256], mess[256], *bbuf;

    SeBoundaryList *sebs = SeBoundaryList::getInstance();

    /* loop through the boundaries and make sure there is some data
     */
    for(bnd_loop=0; bnd_loop < 2; bnd_loop++) {
	ob = sebs->firstBoundary;
	for(nn=0; ob; ob = ob->next) {
	    if(ob->num_points > 1)
		  nn += ob->num_points;
	}
	if(nn) break;
	/* they may be referring to the previous set of boundaries
	 */
	se_prev_bnd_set();
    }
    if(!nn) {
	sprintf(mess,
		"There are not enough points in the current boundary!\n");
	sii_message(mess);
	return;
    }
    bh = sebs->firstBoundary->bh;

    /* buffer up the boundary info
     */
    size = se_sizeof_bnd_set();
    bbuf = (char *)malloc(size);
    memset(bbuf, 0, size);
    se_pack_bnd_set(bbuf);

    if (sebs->directoryText == "")
    {
      wwptr = solo_return_wwptr(sebs->firstBoundary->top_bpm->which_frame);
      sebs->directoryText = wwptr->sweep->directory_name;
      if (sebs->directoryText[sebs->directoryText.size()-1] != '/')
	sebs->directoryText += "/";
    }
    sprintf(str, "%s", sebs->directoryText.c_str());
    a = str +strlen(str);
    std::string file_name =
      DataManager::getInstance()->getFileBaseName("bnd", (int32_t)bh->time_stamp,
						  bh->radar_name, getuid());
    strcpy(a, file_name.c_str());
    sebs->fileNameText = a;
    a = a +strlen(a);
    if (sebs->commentText.size() > 0)
    {
      se_fix_comment(sebs->commentText);
      strcat(a, ".");
      strcat(a, sebs->commentText.c_str());
    }

    if(!(stream = fopen(str, "w"))) {
	sprintf(mess, "Problem opening %s for writing\n", str);
	solo_message(mess);
	return;
    }
    /* we are also going to write an ascii file of boundary info
     */
    a = strstr(str, "bnd.");
    strncpy(a, "asb", 3);	/* change the prefix */
    if(!(ASCIIstream = fopen(str, "w"))) {
	sprintf(mess, "Problem opening %s for writing\n", str);
	solo_message(mess);
	return;
    }
    /* print a header line in the ASCII file
     */
    fprintf(ASCIIstream, "# solo boundary for: %s  file: %s\n"
	    , bh->radar_name, str);

    /* dump out the ascii file
     */
    ob = sebs->firstBoundary;
    for(bn=0; ob; bn++, ob = ob->next) {
	if(ob->num_points > 1) {
	    bpm = ob->top_bpm;
	    for(; bpm; bpm = bpm->next) { /* each boundary point */
		bpm->pisp->writeAscii(ASCIIstream, bn);
	    }
	}
    }
    /* write out bbuf
     */
    if((nn = fwrite(bbuf, sizeof(char), (size_t)size, stream)) < size) {
	sprintf(mess, "Problem writing boundary info: %d\n", nn);
	solo_message(mess);
    }
    if(bnd_loop > 0) {		/* nabbed and saved the prev bnd set
				 * so clear it out again */
	se_clr_all_bnds();
    }
    if(bbuf) free(bbuf);
    fclose(stream);
    fclose(ASCIIstream);
}

/* c------------------------------------------------------------------------ */

int xse_set_intxn(const double x, const double y, const double slope,
		  struct bnd_point_mgmt *bpm, struct one_boundary *ob)
{
  // Compute the intersection of a ray and a boundary segment.
  // x & y are the endpoints of the ray.
  // slope is the slope of the ray.
  // bpm represents the boundary segment.
  //
  // When we are doing intersections, we want to use the shifted
  // x,y values because the boundary may be for another radar

  struct bnd_point_mgmt *bpmx;
  double xx, yy;

  // First compute the x coordinate of the intersection

  if (x == 0.0)
  {
    // The ray is vertical

    xx = 0.0;
  }

  if (bpm->dx == 0.0)
  {
    // The segment is vertical

    xx = bpm->_x;
  }
  else
  {
    // Remember the origin of the ray is (0,0)

    xx = (-bpm->slope * bpm->_x + bpm->_y) / (slope - bpm->slope);
  }

  if (x < 0.0)
  {
    // Check for an imaginary intersection

    if (xx < x || xx > 0.0)
      return NO;
  }
  else if (xx < 0.0 || xx > x)
    return NO;

  if (y == 0.0)
  {
    // The ray is horizontal

    yy = 0.0;
  }
  else if (bpm->dy = 0.0)
  {
    // The segment is horizontal

    yy = bpm->_y;
  }    
  else
  {
    yy = slope * xx;
  }

  if (y < 0.0)
  {
    // Check for an imaginary intersection

    if (yy < y || yy > 0.0)
      return NO;
  }
  else if (yy < 0.0 || yy > y)
    return NO;

  ob->num_intxns++;
  bpm->rx = (int32_t)sqrt(((double)SQ(xx) + (double)SQ(yy)));

  bpm->next_intxn = NULL;

  if ((bpmx = ob->first_intxn) == 0)
  {
    // First intersection

    ob->first_intxn = bpm;
    bpm->last_intxn = bpm;

    // First intxn always points to the last intxn tacked on

    return YES;
  }

  // Insert this intxn and preserve the order

  for (; bpmx; bpmx = bpmx->next_intxn)
  {
    if (bpm->rx < bpmx->rx)
    {
      // Insert intxn here

      bpm->next_intxn = bpmx;
      bpm->last_intxn = bpmx->last_intxn;
      if (bpmx == ob->first_intxn)
      {
	// New first intxn

	ob->first_intxn = bpm;
      }
      else
      {
	bpmx->last_intxn->next_intxn = bpm;
      }
  
      bpmx->last_intxn = bpm;
      break;
    }
  }

  if (!bpmx)
  {
    // Furthest out...tack it onto the end

    ob->first_intxn->last_intxn->next_intxn = bpm;
    bpm->last_intxn = ob->first_intxn->last_intxn;
    ob->first_intxn->last_intxn = bpm;
  }

  return YES;
}

/* c------------------------------------------------------------------------ */

void se_shift_bnd(struct one_boundary *ob, const PointInSpace &boundary_radar, 
		  PointInSpace &current_radar, int scan_mode,
		  double current_tilt)
{
    /* shift this boundary's points so as to be relative to
     * the current radar represented by "usi"
     */
    int mm;
    struct bnd_point_mgmt *bpm;
    double x, y, z, costilt, untilt, tiltfactor;

    /*
     * calculate the offset between the radar that is the origin
     * of the boundary and the current radar
     */
    current_radar.latLonRelative(boundary_radar);
    x = KM_TO_M(current_radar.getX());
    y = KM_TO_M(current_radar.getY());
    z = KM_TO_M(current_radar.getZ());

    untilt = fabs(cos(RADIANS(boundary_radar.getTilt())));

    if(fabs(costilt = cos(fabs(RADIANS(current_tilt)))) > 1.e-6) {
	tiltfactor = 1./costilt;
    }
    else {
	tiltfactor = 0;
    }
    bpm = ob->top_bpm;
    mm = ob->num_points;

    if(scan_mode == DD_SCAN_MODE_AIR) {
	for(; mm--; bpm = bpm->last) {
	}
    }

    else {			/* ground based */
	/* you need to project the original values
	 * and then unproject the new values
	 */
	for(; mm--; bpm = bpm->last) {
	    bpm->_x = (int32_t)((bpm->x * untilt + x) * tiltfactor);
	    bpm->_y = (int32_t)((bpm->y * untilt + y) * tiltfactor);
	}
    }
}

/* c------------------------------------------------------------------------ */

int se_sizeof_bnd_set()
{
    /* determine you much space this boundary will occupy
     */
    int size=0;
    struct one_boundary *ob;
    struct bnd_point_mgmt *bpm;

    SeBoundaryList *sebs = SeBoundaryList::getInstance();
    ob = sebs->firstBoundary;

    for(; ob; ob = ob->next) {
	if(ob->num_points > 1) {
	    size += sizeof(struct boundary_header);
	    size += PointInSpace::NUM_BYTES; /* origin */
	    bpm = ob->top_bpm;
	    for(; bpm; bpm = bpm->next) { /* each boundary point */
	      size += PointInSpace::NUM_BYTES;
	    }
	}
    }
    return(size);
}

/* c------------------------------------------------------------------------ */

void se_unpack_bnd_buf(char *buf, int size)
{
  int gottaSwap=NO;
  unsigned int gdsos;
  struct solo_click_info *sci;
  struct one_boundary *ob, *obnx;
  struct generic_descriptor *gd;
  char *cc=buf, *ee=buf+size;

  SeBoundaryList *sebs = SeBoundaryList::getInstance();
  obnx = sebs->firstBoundary;
  sebs->numBoundaries = 0;
  sebs->absorbingBoundary = YES;

  for (;;)
  {
    gd = (struct generic_descriptor *)cc;
    gdsos = gd->sizeof_struct;
    // NOTE:  This is screwy.  Probably want to check for non-equality
    // instead.  Better yet, always write things big endian and swap if
    // we are on a little endian machine.
    if (gdsos > MAX_REC_SIZE)
    {
      gottaSwap = YES;
      swack4((char *)&gd->sizeof_struct, (char *)&gdsos);
    }

    if (!strncmp(cc, "BDHD", 4))
    {
      if (!sebs->numBoundaries)
      {
	ob = sebs->currentBoundary = sebs->firstBoundary;
      }
      else
      {
	ob = se_return_next_bnd();
      }
      se_clr_current_bnd();

      if (gottaSwap)
      {
	se_crack_bdhd (cc, (char *)ob->bh, (int)0);
      }
      else
      {
	memcpy(ob->bh, cc, gdsos);
      }
    }
    else if (!strncmp(cc, "PISP", 4))
    {
      sebs->pisp.readFromBuffer(cc, gottaSwap);
      if (strstr(sebs->pisp.getId().c_str(), "BND_ORIGIN"))
      {
	PointInSpace origin;
	origin.readFromBuffer(cc, false);
	sebs->setOrigin(origin);
      }
      else
      {
	sci = clear_click_info();
	if (sebs->pisp.isTimeSeries())
	{
	  sci->x = (int32_t) sebs->pisp.getX();
	  sci->y = (int32_t) sebs->pisp.getY();
	}
	else
	{
	  sci->x = (int32_t)KM_TO_M(sebs->pisp.getX());
	  sci->y = (int32_t)KM_TO_M(sebs->pisp.getY());
	}
	xse_add_bnd_pt(sci, sebs->currentBoundary);
      }
    }

    cc += gdsos;
    if (cc >= ee)
      break;
  }

  sebs->absorbingBoundary = NO;
}

/* c------------------------------------------------------------------------ */

#ifdef USE_RADX
#else

int se_use_bnd(const std::vector< UiCommandToken > &cmd_tokens)
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  if (seds->finish_up)
    return 1;

  if (cmd_tokens[0].getCommandText().compare(0, 4, "dont", 0, 4) == 0)
  {
    seds->use_boundary = NO;
    seds->boundary_mask = seds->all_ones_array;
  }
  else
  {
    seds->boundary_mask = seds->boundary_mask_array;
    seds->use_boundary = YES;
  }

  return 1;
}  

#endif

/* c------------------------------------------------------------------------ */

void xse_x_insert(struct bnd_point_mgmt *bpm, struct one_boundary *ob)
{
    /* insert sort of x coorinates of the midpoints of the line
     */
    struct bnd_point_mgmt *bpmx;

    if(ob->num_points < 1)
	  return;
    bpm->x_left = bpm->x_right = NULL;

    if(!(bpmx = ob->x_mids)) {
	bpm->x_parent = NULL;
	ob->x_mids = bpm;
	return;
    }
    /*
     * the top node is an x value
     */
    if(bpm->pisp->isTimeSeries()) {
	for(;;) {
	    if(bpm->t_mid < bpmx->t_mid) {
		if(!bpmx->x_left) {
		    bpm->x_parent = bpmx;
		    bpmx->x_left = bpm;
		    break;
		}
		bpmx = bpmx->x_left;
	    }
	    else {
		if(!bpmx->x_right) {
		    bpm->x_parent = bpmx;
		    bpmx->x_right = bpm;
		    break;
		}
		bpmx = bpmx->x_right;
	    }
	}
    }
    else {
	for(;;) {
	    if(bpm->x_mid < bpmx->x_mid) {
		if(!bpmx->x_left) {
		    bpm->x_parent = bpmx;
		    bpmx->x_left = bpm;
		    break;
		}
		bpmx = bpmx->x_left;
	    }
	    else {
		if(!bpmx->x_right) {
		    bpm->x_parent = bpmx;
		    bpmx->x_right = bpm;
		    break;
		}
		bpmx = bpmx->x_right;
	    }
	}
    }
}

/* c------------------------------------------------------------------------ */

void xse_y_insert(struct bnd_point_mgmt *bpm, struct one_boundary *ob)
{
    /* insert sort of x coorinates of the midpoints of the line
     */
    struct bnd_point_mgmt *bpmx;

    if(ob->num_points < 1)
	  return;
    bpm->y_left = bpm->y_right = NULL;

    if(!(bpmx = ob->y_mids)) {
	bpm->y_parent = NULL;
	ob->y_mids = bpm;
	return;
    }
    /*
     * the top node is an x value
     */
    if(bpm->pisp->isTimeSeries()) {
	for(;;) {
	    if(bpm->r_mid < bpmx->r_mid) {
		if(!bpmx->y_left) {
		    bpm->y_parent = bpmx;
		    bpmx->y_left = bpm;
		    break;
		}
		bpmx = bpmx->y_left;
	    }
	    else {
		if(!bpmx->y_right) {
		    bpm->y_parent = bpmx;
		    bpmx->y_right = bpm;
		    break;
		}
		bpmx = bpmx->y_right;
	    }
	}
    }
    else {
	for(;;) {
	    if(bpm->y_mid < bpmx->y_mid) {
		if(!bpmx->y_left) {
		    bpm->y_parent = bpmx;
		    bpmx->y_left = bpm;
		    break;
		}
		bpmx = bpmx->y_left;
	    }
	    else {
		if(!bpmx->y_right) {
		    bpm->y_parent = bpmx;
		    bpmx->y_right = bpm;
		    break;
		}
		bpmx = bpmx->y_right;
	    }
	}
    }
}

/* c------------------------------------------------------------------------ */

struct bnd_point_mgmt *se_zap_last_bpm(struct bnd_point_mgmt **top_bpm)
{
    /* remove the last points from the list of boundary points
     */
    struct bnd_point_mgmt *bpm = (*top_bpm);

    if(!bpm) {			/* no list yet */
	return(NULL);
    }
    if(bpm->last == bpm) {	/* list of one */
	*top_bpm = NULL;
    }
    else {
	/* point to next to last point
	 */
	bpm = bpm->last;
	(*top_bpm)->last = bpm->last;
	bpm->last->next = NULL;	/* last point always points to NULL */
	se_bnd_pt_atts(*top_bpm);
    }
    return(bpm);
}

/* c------------------------------------------------------------------------ */

char *absorb_zmap_bnd(const std::string &ifile, const int skip, int &nbytes )
{
    /* routine to read in a zmap file and convert it to a boundary
     */
    FILE *fs;
    char str[256], line[256], mess[256], *bbuf;
    int ii, trip=0;
    int max_pts = 0;
    int point_count = 1;
    int offs = 0;
    std::size_t size;
    struct boundary_header *bh;
    WW_PTR wwptr;

    wwptr = solo_return_wwptr( 0 );
    PointInSpace origin = wwptr->radar_location;
    PointInSpace p1 = wwptr->radar_location;
    PointInSpace pisp = wwptr->radar_location;
    strcpy( str, ifile.c_str());

    if(!(fs = fopen(str, "r"))) {
	sprintf(mess, "Unable to open zmap boundary file %s\n", str);
	solo_message(mess);
	return(NULL);
    }

    max_pts = 100;
    offs = size = sizeof( struct boundary_header );
    size += ( max_pts +1 ) * PointInSpace::NUM_BYTES;
    bbuf = (char *)malloc( size );
    memset( bbuf, 0, size );
    /*
     * boundary header
     */
    bh = (struct boundary_header *)bbuf;
    strncpy( bh->name_struct, "BDHD", 4);
    bh->sizeof_struct = sizeof(struct boundary_header);
    bh->offset_to_origin_struct = sizeof(struct boundary_header);
    bh->offset_to_first_point =
      bh->offset_to_origin_struct + PointInSpace::NUM_BYTES;
    bh->sizeof_boundary_point = PointInSpace::NUM_BYTES;
    strncpy(bh->radar_name, solo_return_radar_name(0).c_str(), 12);
    /*
     * boundary origin
     */
    origin.setId("BND_ORIGIN");
    memcpy( bbuf + offs, origin.getBufPtr(), origin.getBufLen());
    offs += PointInSpace::NUM_BYTES;

    while( fgets( line, sizeof(line), fs ) != NULL ) {

      std::vector < std::string > tokens;
      
      tokenize(line, tokens);
      int nt = tokens.size();
	
	if (nt & 1)
	  { nt--; }
	/*
	 * make sure we aren't going to overrun our space
	 */
	if( offs + nt/2 * PointInSpace::NUM_BYTES > size )
	{
	  /* need to realloc space */
	    size = sizeof( struct boundary_header );
	    max_pts += 100;
	    size += ( max_pts +1 ) * PointInSpace::NUM_BYTES;
	    if( !( bbuf = (char *)realloc( bbuf, size ) )) {
		sprintf(mess, "Unable to malloc space for zmap boundary: %d\n"
		    , point_count );
		solo_message(mess);
		return(NULL);
	    }
	}

	for(ii = 0; ii < nt; ) {
	  p1.setLatitude(atof( tokens[ii++].c_str() ));
	  p1.setLongitude(atof( tokens[ii++].c_str() ));

	    /* the (x,y,z) for p1 relative to the origin
	     * are going to come back in pisp
	     */
	    pisp.latLonRelative(p1);
	    pisp.setRange(sqrt((SQ(pisp.getX())+SQ(pisp.getY()))));
	    if( skip && ( trip++ % skip ) != 0 )
		{ continue; }
	    memcpy( bbuf + offs, pisp.getBufPtr(), pisp.getBufLen());
	    offs += PointInSpace::NUM_BYTES;
	    nbytes = offs;
	    point_count++;
	}
    }
    return( bbuf );
}

/* c------------------------------------------------------------------------ */

int absorb_zmap_pts(struct zmap_points **top, const std::string &ifile)
{
  /* This version nabs the Florida Precip98 gauges */
  /* This routine is called in se_histog_setup() in se_histog.c */

    FILE *fs;
    char str[256], *aa, line[256], mess[256];
    int nn, point_count = 0;
    double d;
    struct zmap_points * zmp, *zmpl = NULL;
    int lon, lon_mm, lat, lat_mm;
    float lon_ss, lat_ss;


    strcpy( str, ifile.c_str() );

    if(!(fs = fopen(str, "r"))) {
	sprintf(mess, "Unable to open zmap points file %s\n", str);
	solo_message(mess);
	return(-1);
    }

    while( fgets( line, sizeof(line), fs ) != NULL ) {
	if( *line == '!' )	/* comment */
	    { continue; }

	if ((aa = strchr( line, '!' )) != 0) {
	  *aa = '\0';
	}

	std::vector < std::string > tokens;
	tokenize(line, tokens);

	if (tokens[0].compare("type") == 0)
	  continue;

	if( !point_count++ ) {	/* start list */
	    zmp = *top;
	    if( !zmp ) {
		zmp = ( struct zmap_points *)
		    malloc( sizeof( struct zmap_points ));
		memset( zmp, 0, sizeof( struct zmap_points ));
		*top = zmp;
	    }
	}	
	else if( !zmpl->next ) { /* extend list */
	    zmpl->next = zmp = ( struct zmap_points *)
		malloc( sizeof( struct zmap_points ));
	    memset( zmp, 0, sizeof( struct zmap_points ));
	}
	else {
	    zmp = zmpl->next;
	}
	zmpl = zmp;

	nn = sscanf( line, "%s%s%d%d%f%d%d%f", zmp->list_id, zmp->id
		     , &lon, &lon_mm, &lon_ss, &lat, &lat_mm, &lat_ss );

	if( nn == 8 ) {		/* two styles of input */
	  d = lon_mm/60. + lon_ss/3600.;
	  zmp->lon = lon < 0 ? (double)lon -d : (double)lon +d;
	  d = lat_mm/60. + lat_ss/3600.;
	  zmp->lat = lat < 0 ? (double)lat -d : (double)lat +d;
	}
	else {
	  zmp->lat = atof(tokens[2].c_str());
	  zmp->lon = atof(tokens[3].c_str());
	}
	strcpy(zmp->list_id, tokens[0].c_str()); /* nasa id */
	strcpy(zmp->id, tokens[1].c_str()); /* network id */
    }

    return( point_count );
}

/* c------------------------------------------------------------------------ */

int se_rain_gauge_info(double dlat, double dlon)
{
  /* This routine lists all the rain gauges within a given radius
   * of the input lat/lon
   */
  char mess[256];
  int ii, jj, mark, num_pts = 0, num_hits = 0;
  PointInSpace p0;
  PointInSpace p1;
  struct zmap_points * zmpc, *zzmpc[200], *ztmp;
  struct solo_edit_stuff *seds;
  double min_radius = 15;	/* km. */

  seds = return_sed_stuff();

  if( !seds->top_zmap_list )
    { return(0); }

  zmpc = seds->top_zmap_list;
  p0.setAltitude(0.0);
  p1.setAltitude(0.0);

  for(; zmpc ; zmpc = zmpc->next ) {
    num_pts++;
    p0.setLatitude(dlat);
    p0.setLongitude(dlon);
    p1.setLatitude(zmpc->lat);
    p1.setLongitude(zmpc->lon);
    /*
     * the (x,y,z) for p1 relative to p0
     * are going to come back in p0
     */
    p0.latLonRelative(p1);
    zmpc->x = p0.getX();
    zmpc->y = p0.getY();
    zmpc->rng = sqrt(SQ(p0.getX()) + SQ(p0.getY()));
    zmpc->number = zmpc->rng < min_radius;
    if( zmpc->rng < min_radius ) {
      zzmpc[num_hits++] = zmpc;
    }
  }
  if( !num_hits ) {
    sprintf( mess, "No gauges within %.3f of lat: %.4f lon: %.4f\n"
	     , min_radius, dlat, dlon );
    solo_message(mess);
    return( 0 );
  }
  
  for(ii = 0; ii < num_hits -1; ii++ ) {
    for( jj = ii+1; jj < num_hits; jj++ ) {
      if( zzmpc[jj]->rng < zzmpc[ii]->rng ) {
	ztmp = zzmpc[ii];
	zzmpc[ii] = zzmpc[jj];
	zzmpc[jj] = ztmp;
      }
    }
  }
  mark = 0;
  solo_message( "\n" );

  for(ii = 0; ii < num_hits; ii++ ) {
    zmpc = zzmpc[ii];
    sprintf( mess, "%10s rng:%7.2f x:%7.2f y:%7.2f\n"
	     , zmpc->list_id, zmpc->rng, zmpc->x, zmpc->y );
    solo_message(mess);
  }
  return(num_hits);
}

