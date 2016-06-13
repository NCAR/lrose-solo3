/* 	$Id$	 */

#include <dd_math.h>
#include <SeBoundaryList.hh>
#include <solo_window_structs.h>
#include "soloii.h"
#include "soloii.hh"
#include <solo_editor_structs.h>
#include "xyraster.h"
#include "sii_xyraster.hh"
#include "sp_basics.hh"
#include <se_utils.hh>
#include <se_bnd.hh>
#include "solo2.hh"

static int Num_xyras=0;
static struct xyras *xyraz[16];

/* c------------------------------------------------------------------------ */

int frme_intxns (struct xyras *rxy, struct line_segment *edge)
{
    struct bnd_point_mgmt *bpmx;

    bpmx = rxy->screen_boundary->first_intxn;

    if(rxy->radar_outside) {
	if((edge->num_intxns = rxy->screen_boundary->num_intxns) > 1) {
	    edge->p0.x = (int)((double)bpmx->rx * edge->cos);
	    edge->p0.y = (int)((double)bpmx->rx * edge->sin);
	    edge->p0.gate_num =
		  GATE_NUM(bpmx->rx, rxy->r0, rxy->rcp_gs);
	    
	    bpmx = bpmx->next_intxn;
	    edge->p1.x = (int)((double)bpmx->rx * edge->cos);
	    edge->p1.y = (int)((double)bpmx->rx * edge->sin);
	    edge->p1.gate_num =
		  GATE_NUM(bpmx->rx, rxy->r0, rxy->rcp_gs);
	}
	else if(edge->num_intxns == 1) {
	    /* ray ends before intersecting
	     * another boundary
	     */
	    edge->p0.x = (int)((double)bpmx->rx * edge->cos);
	    edge->p0.y = (int)((double)bpmx->rx * edge->sin);
	    edge->p0.gate_num =
		  GATE_NUM(bpmx->rx, rxy->r0, rxy->rcp_gs);

	    bpmx = bpmx->next_intxn;
	    edge->p1.x = (int)((double)rxy->r1 * edge->cos);
	    edge->p1.y = (int)((double)rxy->r1 * edge->sin);
	    edge->p1.gate_num =
		  GATE_NUM(rxy->r1, rxy->r0, rxy->rcp_gs);
	}
	else {
	    return(-1);
	}
    }
    else {			/* the radar is inside */
	edge->p0.x = 0;
	edge->p0.y = 0;
	edge->p0.gate_num = GATE_NUM(0, rxy->r0, rxy->rcp_gs);

	if((edge->num_intxns = rxy->screen_boundary->num_intxns) > 0) {
	    edge->p1.x = (int)((double)bpmx->rx * edge->cos);
	    edge->p1.y = (int)((double)bpmx->rx * edge->sin);
	    edge->p1.gate_num =
		  GATE_NUM(bpmx->rx, rxy->r0, rxy->rcp_gs);
	}
	else {			/* if the radar is inside and no intersections
				 * then the beam ends inside the frame
				 */
	    edge->p1.x = (int)((double)rxy->r1 * edge->cos);
	    edge->p1.y = (int)((double)rxy->r1 * edge->sin);
	    edge->p1.gate_num =
		  GATE_NUM(rxy->r1, rxy->r0, rxy->rcp_gs);
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

void 
init_xyraster (int frme)
{
    /* routine to initialize rasterization parameters at the beginning
     * of each sweep
     * for the sanity of the author all calculations are in cartesian
     * coordinates and the switch to screen coordinates is only at the
     * very last minute.
     * all coordinates are relative to the radar which is (0,0)
     */
    int ii;
    struct solo_click_info *sci;
    double d, theta;
    struct xyras *rxy;
    WW_PTR wwptr;
    struct bnd_point_mgmt *bpm;


    wwptr = solo_return_wwptr(frme);
    rxy = return_xyras(frme);

    /*
     * nab the plot parameters
     * coordinates to the center of the image and the corners are
     * in meters relative to the radar.
     */
    rxy->angle_ctr = wwptr->center_of_view.getRotationAngle();
    rxy->range_ctr = wwptr->center_of_view.getRange() * 1000.; /* meters! */

    if((d = wwptr->view->magnification) < .005) d = .005;
    rxy->meters_per_pixel = sp_meters_per_pixel()/d;
    rxy->pixels_per_meter = 1./rxy->meters_per_pixel;

    theta = RADIANS(CART_ANGLE(rxy->angle_ctr));
    rxy->center.x = rxy->range_ctr * cos(theta);
    rxy->center.y = rxy->range_ctr * sin(theta);

    rxy->image_width = wwptr->view->width_in_pix;
    rxy->image_height = wwptr->view->height_in_pix;
    rxy->image = (char *)wwptr->image->data;
    rxy->sin_corner = SIN45;
    rxy->image_l_left.x = rxy->center.x -(.5*(rxy->image_width-1.) *
	  rxy->meters_per_pixel);
    rxy->image_l_left.y = rxy->center.y -(.5*(rxy->image_height-1.) *
	  rxy->meters_per_pixel);

    rxy->view_width = rxy->image_width;
    rxy->view_height = rxy->image_height;
    /*
     * view width may be smaller than the image width
     * but the rasterization is defined by the view size
     */
    rxy->l_left.x = rxy->center.x -(.5*(rxy->view_width-1.) *
	  rxy->meters_per_pixel);
    rxy->l_left.y = rxy->center.y -(.5*(rxy->view_height-1.) *
	  rxy->meters_per_pixel);

    rxy->u_right.x = rxy->l_left.x +(rxy->view_width-1.) *
	  rxy->meters_per_pixel;
    rxy->u_right.y = rxy->l_left.y +(rxy->view_height-1.) *
	  rxy->meters_per_pixel;

    rxy->radar_outside = (rxy->l_left.x > 0 || rxy->u_right.x < 0 ||
			  rxy->l_left.y > 0 || rxy->u_right.y < 0) ? YES : NO;
    /*
     * generate screen boundary
     * this algorithm uses the editor boundary routines to
     * calculate intersections of the ray and the bounds
     * of the viewing area
     */
    if(!rxy->screen_boundary) {
	rxy->screen_boundary = se_malloc_one_bnd();
    }
    rxy->screen_boundary->radar_inside_boundary = !rxy->radar_outside;
    se_clr_bnd(rxy->screen_boundary);
    SeBoundaryList *sebs = SeBoundaryList::getInstance();
    sebs->viewBounds = YES;
    
    sci = clear_click_info();

    sci->x = IFIX(rxy->l_left.x);
    sci->y = IFIX(rxy->l_left.y);
    xse_add_bnd_pt(sci, rxy->screen_boundary, true);

    sci->y = IFIX(rxy->u_right.y);
    xse_add_bnd_pt(sci, rxy->screen_boundary, true);

    sci->x = IFIX(rxy->u_right.x);
    xse_add_bnd_pt(sci, rxy->screen_boundary, true);

    sci->y = IFIX(rxy->l_left.y);
    xse_add_bnd_pt(sci, rxy->screen_boundary, true);

    bpm = rxy->screen_boundary->top_bpm;
    /* set shifted x,y,z...no shift here!
     */
    for(ii=0; ii < 4; ii++, bpm = bpm->next) {
	bpm->_x = bpm->x;
	bpm->_y = bpm->y;
    }
    sebs->viewBounds = NO;
}

/* c------------------------------------------------------------------------ */
/* This routine does calculation of the intersections for both edges of the  */
/* ray to be rasterized and then keeps ajusting to loop constraints such     */
/* that no if tests are necessary in the innermost rasterization loops.      */

int ray_raster_setup(int frame_num, double angle0, double angle1,
		     struct xyras *rxy)
{
  // Get a pointer to the window information

  WW_PTR wwptr = solo_return_wwptr(frame_num);

  register double angle_rad = RADIANS(CART_ANGLE(angle0));
  rxy->edge0.sin = sin(angle_rad);
  rxy->edge0.cos = cos(angle_rad);

  angle_rad = RADIANS(CART_ANGLE(angle1));
  rxy->edge1.sin = sin(angle_rad);
  rxy->edge1.cos = cos(angle_rad);

  rxy->ignore_this_ray = YES;

  rxy->gs = wwptr->uniform_cell_spacing;
  rxy->rcp_gs = 1.0 / rxy->gs;
  rxy->r0 = wwptr->uniform_cell_one;
  rxy->max_gate = wwptr->number_cells - 1;
  rxy->r1 = rxy->r0 + (rxy->max_gate * rxy->gs);
  rxy->colors = wwptr->cell_colors;

  // Calculate the intersections of the two edges with the screen boundary

  xse_find_intxns(angle0, rxy->r1, rxy->screen_boundary);
  if (frme_intxns(rxy, &rxy->edge0) < 1)
    return -1;

  xse_find_intxns(angle1, rxy->r1, rxy->screen_boundary);
  if (frme_intxns(rxy, &rxy->edge1) < 1)
    return -1;

  if (fabs(rxy->edge0.sin) > rxy->sin_corner)
  {
    // Inner loop in x

    rxy->x_inner_loop = YES;
    
    if (rxy->edge0.p1.y < rxy->edge0.p0.y)
    {
      // Flip points so we can iterate in positive y

      swap_ends(&rxy->edge0);
      swap_ends(&rxy->edge1);
    }

    // intersections may not line up perfectly in double precision since the
    // boundary arithmetic and values start out as integer values so make them
    // line up

    register double dy0 = rxy->edge1.p0.y - rxy->edge0.p0.y;
    register double dy1 = rxy->edge1.p1.y - rxy->edge0.p1.y;

    if (fabs(dy0) < rxy->meters_per_pixel)
    {
      rxy->edge1.p0.y = rxy->edge0.p0.y + (0.5 * dy0);
      rxy->edge0.p0.y = rxy->edge1.p0.y;
    }
    if(fabs(dy1) < rxy->meters_per_pixel)
    {
      rxy->edge1.p1.y = rxy->edge0.p1.y = rxy->edge0.p1.y + .5*dy1;
    }

    // First calculate the change in x as you move up one pixel in y (the
    // change in y is rxy->meters_per_pixel) and then change in gate number
    // as you move up in y.

    rxy->edge0.x_inc = rxy->meters_per_pixel * rxy->edge0.cos / rxy->edge0.sin;

    rxy->edge0.major_gate_inc =
      rxy->meters_per_pixel / rxy->edge0.sin * rxy->rcp_gs;
    rxy->edge0.minor_gate_inc = 
      0.5 * (rxy->edge0.cos + rxy->edge1.cos) *
      rxy->meters_per_pixel * rxy->rcp_gs;
    rxy->edge1.minor_gate_inc = rxy->edge0.minor_gate_inc;
    rxy->edge0.num_iters =
      (int)((rxy->edge0.p1.y - rxy->edge0.p0.y) * rxy->pixels_per_meter);

    rxy->edge1.x_inc = rxy->meters_per_pixel * rxy->edge1.cos / rxy->edge1.sin;

    rxy->edge1.major_gate_inc =
      rxy->meters_per_pixel / rxy->edge1.sin * rxy->rcp_gs;
    rxy->edge1.num_iters =
      (int)((rxy->edge1.p1.y - rxy->edge1.p0.y) * rxy->pixels_per_meter);
    
    // First line up the y-values.

    register double d =
      (rxy->edge1.p0.y - rxy->edge0.p0.y) * rxy->pixels_per_meter;

    if (d >= 0.5)
    {
      int nn = (int)(d + 0.5);

      // Edge 1 starts higher up than edge 0. Make edge 0 line up in y with
      // edge 1

      rxy->edge0.p0.gate_num += nn * rxy->edge0.major_gate_inc;
      rxy->edge0.p0.x += nn * rxy->edge0.x_inc;
      rxy->edge0.p0.y += nn * rxy->meters_per_pixel;
      rxy->edge0.num_iters -= nn;
    }
    else if ( d <= -0.5)
    {
      // edge0 starts higher up than edge1

      int nn = (int)(-d - 0.5);
      rxy->edge1.p0.gate_num += nn * rxy->edge1.major_gate_inc;
      rxy->edge1.p0.x += nn * rxy->edge1.x_inc;
      rxy->edge1.p0.y += nn * rxy->meters_per_pixel;
      rxy->edge1.num_iters -= nn;
    }

    d = (rxy->edge1.p1.y - rxy->edge0.p1.y) * rxy->pixels_per_meter;

    if (d >= 0.5)
    {
      // edge1 ends higher than edge0 so bring it down

      int nn = (int)(d + 0.5);
      rxy->edge1.p1.gate_num -= nn * rxy->edge1.major_gate_inc;
      rxy->edge1.p1.x -= nn * rxy->edge1.x_inc;
      rxy->edge1.p1.y -= nn * rxy->meters_per_pixel;
      rxy->edge1.num_iters -= nn;
    }
    else if (d <= -0.5)
    {
      // Edge0 ends higher than edge1

      int nn = (int)(-d - 0.5);
      rxy->edge0.p1.gate_num -= nn * rxy->edge0.major_gate_inc;
      rxy->edge0.p1.x -= nn * rxy->edge0.x_inc;
      rxy->edge0.p1.y -= nn * rxy->meters_per_pixel;
      rxy->edge0.num_iters -= nn;
    }

    if (rxy->edge0.p0.x > rxy->edge1.p0.x ||
	rxy->edge0.p1.x > rxy->edge1.p1.x)
    {
      // Swap edges so we can iterate in positive x

      swap_edges(&rxy->edge0, &rxy->edge1);
    }

    // We will be referenceing on edge0.
    //
    // Now adjust things so the gate numbers are reasonable

    if (rxy->edge0.major_gate_inc < 0)
    {
      if (rxy->edge0.p0.gate_num > rxy->max_gate ||
	  rxy->edge1.p0.gate_num > rxy->max_gate)
      {
	register double d0 = rxy->edge0.p0.gate_num - rxy->max_gate;
	register double d1 = rxy->edge1.p0.gate_num - rxy->max_gate;
	register double d = d0 > d1 ? d0 : d1;
	int nn = (int)(-d / rxy->edge0.major_gate_inc + 1.0);

	// Adjusting the max gate number

	rxy->edge0.p0.gate_num += nn * rxy->edge0.major_gate_inc;
	rxy->edge0.p0.x += nn * rxy->edge0.x_inc;
	rxy->edge1.p0.x += nn * rxy->edge1.x_inc;
	rxy->edge0.p0.y += nn * rxy->meters_per_pixel;
	rxy->edge0.num_iters -= nn;
      }
      if (rxy->edge0.p1.gate_num < 0 || rxy->edge1.p1.gate_num < 0)
      {
	register double d0 = -rxy->edge0.p1.gate_num;
	register double d1 = -rxy->edge1.p1.gate_num;
	register double d = d0 > d1 ? d0 : d1;
	int nn = (int)(-d / rxy->edge0.major_gate_inc + 1.0);

	// Adjusting the min gate number

	rxy->edge0.num_iters -= nn;
      }
    }
    else
    {
      // Points up

      if (rxy->edge0.p0.gate_num < 0 || rxy->edge1.p0.gate_num < 0)
      {
	register double d0 = -rxy->edge0.p0.gate_num;
	register double d1 = -rxy->edge1.p0.gate_num;
	register double d = d0 > d1 ? d0 : d1;
	int nn = (int)(d / rxy->edge0.major_gate_inc + 1.0);

	// Adjusting the min gate number

	rxy->edge0.p0.gate_num += nn * rxy->edge0.major_gate_inc;
	rxy->edge0.p0.x += nn * rxy->edge0.x_inc;
	rxy->edge1.p0.x += nn * rxy->edge1.x_inc;
	rxy->edge0.p0.y += nn * rxy->meters_per_pixel;
	rxy->edge0.num_iters -= nn;
		
      }
      if (rxy->edge0.p1.gate_num > rxy->max_gate ||
	  rxy->edge1.p1.gate_num > rxy->max_gate)
      {
	register double d0 = rxy->edge0.p1.gate_num - rxy->max_gate;
	register double d1 = rxy->edge1.p1.gate_num - rxy->max_gate;
	register double d = d0 > d1 ? d0 : d1;
	int nn = (int)(d / rxy->edge0.major_gate_inc + 1.0);

	// Adjusting the max gate number

	rxy->edge0.num_iters -= nn;
      }
    }

    // Now convert location coordinates to screen coordinates

    rxy->edge0.p0.x =
      (rxy->edge0.p0.x - rxy->image_l_left.x) * rxy->pixels_per_meter;
    rxy->edge1.p0.x =
      (rxy->edge1.p0.x - rxy->image_l_left.x) * rxy->pixels_per_meter;
    rxy->edge0.p0.y =
      (rxy->edge0.p0.y - rxy->image_l_left.y) * rxy->pixels_per_meter;
    rxy->edge0.x_inc *= rxy->pixels_per_meter;
    rxy->edge1.x_inc *= rxy->pixels_per_meter;

    // Just fudge the indexing into the image to reflect that the screen
    // coordinates are upside down

  }
  else
  {
    // The inner loop is in y

    rxy->x_inner_loop = NO;

    if (rxy->edge0.p0.x > rxy->edge0.p1.x)
    {
      // Flip points to iterate in positive x

      swap_ends(&rxy->edge0);
      swap_ends(&rxy->edge1);
    }

    register double dx0 = rxy->edge1.p0.x -rxy->edge0.p0.x;
    register double dx1 = rxy->edge1.p1.x -rxy->edge0.p1.x;

    if (fabs(dx0) < rxy->meters_per_pixel)
    {
      rxy->edge1.p0.x = rxy->edge0.p0.x + (0.5 * dx0);
      rxy->edge0.p0.x = rxy->edge1.p0.x;
    }
    if (fabs(dx1) < rxy->meters_per_pixel)
    {
      rxy->edge1.p1.x = rxy->edge0.p1.x + (0.5 * dx1);
      rxy->edge0.p1.x = rxy->edge1.p1.x;
    }

    rxy->edge0.y_inc = rxy->meters_per_pixel * rxy->edge0.sin / rxy->edge0.cos;

    rxy->edge0.major_gate_inc =
      rxy->meters_per_pixel / rxy->edge0.cos * rxy->rcp_gs;
    rxy->edge1.minor_gate_inc =
      0.5 * (rxy->edge0.sin + rxy->edge1.sin) *
      rxy->meters_per_pixel * rxy->rcp_gs;
    rxy->edge0.minor_gate_inc = rxy->edge1.minor_gate_inc;
    
    rxy->edge0.num_iters =
      (int)((rxy->edge0.p1.x - rxy->edge0.p0.x) * rxy->pixels_per_meter);

    rxy->edge1.y_inc = rxy->meters_per_pixel * rxy->edge1.sin / rxy->edge1.cos;

    rxy->edge1.major_gate_inc =
      rxy->meters_per_pixel / rxy->edge1.cos * rxy->rcp_gs;
    rxy->edge1.num_iters =
      (int)((rxy->edge1.p1.x - rxy->edge1.p0.x) * rxy->pixels_per_meter);

    register double d =
      (rxy->edge1.p0.x - rxy->edge0.p0.x) * rxy->pixels_per_meter;

    if (d >= 0.5)
    {
      // Edge1 starts beyond edge0

      int nn = (int)(d + 0.5);
      rxy->edge0.p0.gate_num += nn * rxy->edge0.major_gate_inc;
      rxy->edge0.p0.y += nn * rxy->edge0.y_inc;
      rxy->edge0.p0.x += nn * rxy->meters_per_pixel;
      rxy->edge0.num_iters -= nn;
    }
    else if (d <= -0.5)
    {
      // Edge0 starts before edge1

      int nn = (int)(-d -.5);
      rxy->edge1.p0.gate_num += nn * rxy->edge1.major_gate_inc;
      rxy->edge1.p0.y += nn * rxy->edge1.y_inc;
      rxy->edge1.p0.x += nn * rxy->meters_per_pixel;
      rxy->edge1.num_iters -= nn;
    }
    d = (rxy->edge1.p1.x - rxy->edge0.p1.x) * rxy->pixels_per_meter;

    if (d >= 0.5)
    {
      // Edge1 ends beyond edge0

      int nn = (int)(d + 0.5);
      rxy->edge1.p1.gate_num -= nn * rxy->edge1.major_gate_inc;
      rxy->edge1.p1.y -= nn * rxy->edge1.y_inc;
      rxy->edge1.p1.x -= nn * rxy->meters_per_pixel;
      rxy->edge1.num_iters -= nn;
    }
    else if (d <= -0.5)
    {
      // Edge0 ends beyond edge1

      int nn = (int)(-d - 0.5);
      rxy->edge0.p1.gate_num -= nn * rxy->edge0.major_gate_inc;
      rxy->edge0.p1.y -= nn * rxy->edge0.y_inc;
      rxy->edge0.p1.x -= nn * rxy->meters_per_pixel;
      rxy->edge0.num_iters -= nn;
    }
    if (rxy->edge0.p0.y > rxy->edge1.p0.y || rxy->edge0.p1.y > rxy->edge1.p1.y)
    {
      // Swap edges so we can interate in positive y

      swap_edges(&rxy->edge0, &rxy->edge1);
    }
    
    // Now adjust things so the min and max gate numbers are reasonable

    if (rxy->edge0.major_gate_inc < 0)
    {
      if (rxy->edge0.p0.gate_num > rxy->max_gate ||
	  rxy->edge1.p0.gate_num > rxy->max_gate)
      {
	register double d0 = rxy->edge0.p0.gate_num - rxy->max_gate;
	register double d1 = rxy->edge1.p0.gate_num - rxy->max_gate;
	register double d = d0 > d1 ? d0 : d1;
	int nn = (int)(-d / rxy->edge0.major_gate_inc + 1.0);

	rxy->edge0.p0.gate_num += nn * rxy->edge0.major_gate_inc;
	rxy->edge0.p0.y += nn * rxy->edge0.y_inc;
	rxy->edge1.p0.y += nn * rxy->edge1.y_inc;
	rxy->edge0.p0.x += nn * rxy->meters_per_pixel;
	rxy->edge0.num_iters -= nn;
      }
      if (rxy->edge0.p1.gate_num < 0 || rxy->edge1.p1.gate_num < 0)
      {
	register double d0 = -rxy->edge0.p1.gate_num;
	register double d1 = -rxy->edge1.p1.gate_num;
	register double d = d0 > d1 ? d0 : d1;
	int nn = (int)(-d / rxy->edge0.major_gate_inc + 1.0);

	rxy->edge0.num_iters -= nn;
      }
    }
    else
    {			
      if (rxy->edge0.p0.gate_num < 0 || rxy->edge1.p0.gate_num < 0)
      {
	register double d0 = -rxy->edge0.p0.gate_num;
	register double d1 = -rxy->edge1.p0.gate_num;
	register double d = d0 > d1 ? d0 : d1;
	int nn = (int)(d / rxy->edge0.major_gate_inc + 1.0);

	rxy->edge0.p0.gate_num += nn * rxy->edge0.major_gate_inc;
	rxy->edge0.p0.y += nn * rxy->edge0.y_inc;
	rxy->edge1.p0.y += nn * rxy->edge1.y_inc;
	rxy->edge0.p0.x += nn * rxy->meters_per_pixel;
	rxy->edge0.num_iters -= nn;
      }
      if (rxy->edge0.p1.gate_num > rxy->max_gate ||
	  rxy->edge1.p1.gate_num > rxy->max_gate)
      {
	register double d0 = rxy->edge0.p1.gate_num - rxy->max_gate;
	register double d1 = rxy->edge1.p1.gate_num - rxy->max_gate;
	register double d = d0 > d1 ? d0 : d1;
	int nn = (int)(d / rxy->edge0.major_gate_inc + 1.0);

	rxy->edge0.num_iters -= nn;
      }
    }
    rxy->edge0.p0.y =
      (rxy->edge0.p0.y - rxy->image_l_left.y) * rxy->pixels_per_meter;
    rxy->edge1.p0.y =
      (rxy->edge1.p0.y - rxy->image_l_left.y) * rxy->pixels_per_meter;
    rxy->edge0.p0.x =
      (rxy->edge0.p0.x - rxy->image_l_left.x) * rxy->pixels_per_meter;
    rxy->edge0.y_inc *= rxy->pixels_per_meter;
    rxy->edge1.y_inc *= rxy->pixels_per_meter;
  }
    
  if (rxy->edge0.num_iters < 1)
  {
    return -1;
  }
  else
  {
    rxy->ignore_this_ray = NO;
    return 1;
  }
}

/* c------------------------------------------------------------------------ */

struct xyras *
return_xyras (int frme)
{
    int ii=0;

    if(!Num_xyras) {
	for(ii=0; ii < 16; ii++) {
	    xyraz[ii] = NULL;
	}
    }
    if(xyraz[frme])
	  return(xyraz[frme]);

    Num_xyras++;
    xyraz[frme] = (struct xyras *)malloc(sizeof(struct xyras));
    memset(xyraz[frme], 0, sizeof(struct xyras));
    return(xyraz[frme]);
}

/* c------------------------------------------------------------------------ */

double 
sp_meters_per_pixel (void)
{
    return((double)300.);
}

/* c------------------------------------------------------------------------ */

void 
swap_edges (struct line_segment *edge0, struct line_segment *edge1)
{
    int sl=sizeof(struct line_segment);
    struct line_segment edge_tmp;

    memcpy(&edge_tmp, edge0, sl);
    memcpy(edge0, edge1, sl);
    memcpy(edge1, &edge_tmp, sl);
}

/* c------------------------------------------------------------------------ */

void 
swap_ends (struct line_segment *edge)
{
    int sxy=sizeof(struct rxy_coords);
    struct rxy_coords xy_tmp;

    memcpy(&xy_tmp, &edge->p0, sxy);
    memcpy(&edge->p0, &edge->p1, sxy);
    memcpy(&edge->p1, &xy_tmp, sxy);
}

/* c------------------------------------------------------------------------ */
/* Render one of the beams in the east or west quadrant                      */

void xx_inner_loop(struct xyras *rxy)
{
  // Get the number of needed iterations.  

  int num_iters = rxy->edge0.num_iters;
  
  if (num_iters < 1)
    return;

  int width = rxy->image_width;

  // Remember...image y coords are reversed

  int y_row = (rxy->image_height - 1) - IFIX(rxy->edge0.p0.y);
  if (y_row < 0) y_row = 0;
  int ndx0 = y_row * width;

  char *image = (char*)rxy->image;
  u_int32_t *colors = rxy->colors;

  double gate_num;
  double start_gate_num = rxy->edge0.p0.gate_num;
  
  double x0_pxl = rxy->edge0.p0.x;
  if (x0_pxl < 0.0) x0_pxl = 0.0;
  double x1_pxl = rxy->edge1.p0.x;
  
  for (; num_iters > 0; num_iters--)
  {
    int ndx = ndx0 + (int)x0_pxl;
    gate_num = start_gate_num;
    
    for (int ii = (int)x1_pxl - (int)x0_pxl + 1; ii > 0; ndx++, ii--)
    {
      image[ndx] = colors[(int)gate_num];
      
      gate_num += rxy->edge0.minor_gate_inc;
    }

    start_gate_num += rxy->edge0.major_gate_inc;
    
    x0_pxl += rxy->edge0.x_inc;
    x1_pxl += rxy->edge1.x_inc;
    
    // Remember image y coords are reversed

    ndx0 -= width;
  }
}

/* c------------------------------------------------------------------------ */
/* Render one of the beams in the north or south quadrant                    */

void yy_inner_loop(struct xyras *rxy)
{
  // Get the number of needed iterations.  

  int num_iters = rxy->edge0.num_iters;

  if (num_iters < 1)
    return;
  
  int width = rxy->image_width;

  // Remember...image y coords are reversed!

  int ndx0 = IFIX(rxy->edge0.p0.x);
  if (ndx0 < 0) ndx0 = 0;

  int flipit = rxy->image_height - 1;
  
  char *image = (char *)rxy->image;
  u_int32_t *colors = rxy->colors;

  double start_gate_num = rxy->edge0.p0.gate_num;
  double gate_num;
  
  double y0_pxl = rxy->edge0.p0.y;
  if (y0_pxl < 0.0) y0_pxl = 0.0;
  double y1_pxl = rxy->edge1.p0.y;

  for (int iter = 0; iter < num_iters; ++ndx0, ++iter)
  {
    int ndx = ndx0 + (flipit - (int)y0_pxl) * width;
    int jj = (int)y1_pxl - (int)y0_pxl + 1;

    gate_num = start_gate_num;
    
    for (; jj-- > 0; ndx -= width)
    {
      image[ndx] = colors[(int)gate_num];
      gate_num += rxy->edge0.minor_gate_inc;
    }

    start_gate_num += rxy->edge0.major_gate_inc;
    
    y0_pxl += rxy->edge0.y_inc;
    y1_pxl += rxy->edge1.y_inc;
  }
}
