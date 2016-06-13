#include <EarthRadiusCalculator.hh>
#include <se_utils.hh>
#include <sii_utils.hh>

#include "SoloSubFrame.hh"

SoloSubFrame::SoloSubFrame(guint nframe, gboolean blowup, GdkRectangle *imageclippingregion, guint bup_factor, gboolean redraw_bnd_ind) :
  small_pro_fonta((gchar *)"Helvetica 10"),
  med_pro_fonta((gchar *)"Helvetica 12"),
  big_pro_fonta((gchar *)"Helvetica 14")
{
	m_sii_arcs = NULL;
	m_frame_num = nframe;
	m_blow_up = blowup;
	m_image_clipping_region = imageclippingregion;
	m_bup_factor = bup_factor;
	
	m_exposed = (m_image_clipping_region) ? 1 : 0;
	
  	// Get the time series information.
	time_series = _initTimeSeries(m_frame_num, ts_start, ts_span);

	// Get pointers to the frame information
	m_sfc = frame_configs[m_frame_num];  // image data = img = (guchar *)sfc->image->data + sfc->width * sfc->height;
	m_wwptr = solo_return_wwptr(m_frame_num);
	
	m_img = NULL;
    if (m_sfc->frame && m_sfc->image && m_sfc->width * m_sfc->height > 99)
      m_img = (guchar *)m_sfc->image->data + m_sfc->width * m_sfc->height;

    m_frame = m_sfc->frame;
    if (m_blow_up)
    {
      m_frame = frame_configs[0]->blow_up_frame;
    }
    m_cr = gdk_cairo_create(m_frame->window);
	
	m_redraw_bnd = redraw_bnd_ind;
	
    m_font = med_pro_fonta;
}

SoloSubFrame::~SoloSubFrame()
{
	cairo_destroy( m_cr );
}

void SoloSubFrame::Draw()
{
	_SetTitleFont(); // Not changed to Pango yet.
	
	// Draw the base image.  This includes the color bar, but not the color bar labels.
	_DrawBaseImage();
	
	// Get the center and landmark locations, needed for rendering the overlays.
	_CalcCenterLandmark();
	
	// Draw the plot title and the color bar labels
	_DrawPlotTitleColorbarLabels();
	
	// Set the color for drawing the overlays
	_SetColorOverlays();
	
	// Draw center crosshairs
	_DrawCenterCrosshair();
	
	// Set the clip rectangle
	_SetClipRectangle();
	
	// Find the maximum range
  	_FindMaxRange();

	// Draw the range rings
	_DrawRangeRings();
	  
	// Draw the azimuth lines
	_DrawAzimuthLines();
	
	// Draw the X/Y tic marks.
	_DrawXYTicks();
	
	// Draw a two pixel width border
	_DrawBorders();
}

void SoloSubFrame::_SetTitleFont()
{
  gint text_width;
  gint height;

//  gdk_text_extents(font, title_tmplt, strlen(title_tmplt),
//		   &lbearing, &rbearing, &text_width, &ascent, &descent);
  _GetLayoutPixelSize(title_tmplt, &text_width, &height);

  // Not sure how to change this ...
  if (text_width > (int)m_sfc->width)
  	  m_font = small_pro_fonta;

  m_sfc->font = m_font;
  m_sfc->font_height = height;//font->ascent + font->descent + 3;
}

void SoloSubFrame::_DrawBaseImage()
{
  // Draw the base image.  This includes the color bar, but not the color bar labels.
  // sii_draw_base_image(m_sfc, m_wwptr, m_img, m_image_clipping_region, m_blow_up);
  if (m_blow_up)
  {
    SiiFrameConfig *sfc0 = frame_configs[0];

    sfc0->big_font = big_pro_fonta;

    gint fwidth=0,fheight=0;
    m_font = big_pro_fonta;
    _GetLayoutPixelSize("", &fwidth, &fheight);

    sfc0->big_font_height = fheight;	// fheight-7;

    m_bup_factor = 2;
    m_frame = sfc0->blow_up_frame;

    guchar *big_img;
    
	// Update blow_up image - synchronize for color bar  
	sii_double_colorize_image (m_frame_num);

    if ((big_img = (guchar *)sfc0->big_image->data) != 0)
    {
      _DrawRGBImage(big_img, false, 8, sfc0->big_width, sfc0->big_height, 
                    sfc0->big_width * 3, 0, 0);
    }
  }
  else if (!m_image_clipping_region && m_img)
  {
    // We have an image, but it hasn't been rendered

    // Render the color bar into the data values region of the image data

    set_color_bar (m_sfc, m_wwptr);

    // Using the data values region of the image data, set the RGB values in
    // the color region of the image data.

    sii_colorize_image (m_sfc);

    // Draw the image in the frame
	_DrawRGBImage(m_img, false, 8, m_sfc->width, m_sfc->height, m_sfc->width * 3, 0, 0);
  }
  else if (m_img && m_sfc->colorize_count)
  {
    // We have an image that has been previously rendered.  We just need to
    // draw the image in the frame.

    gint stride = m_sfc->width * 3;
    if (m_image_clipping_region)
    {
      m_img += m_image_clipping_region->y * stride;
      m_img += m_image_clipping_region->x * 3;
      _DrawRGBImage(m_img, false, 8, 
                    m_image_clipping_region->width, m_image_clipping_region->height, 
                    stride,
                    m_image_clipping_region->x, m_image_clipping_region->y);
    }
    else
    {
      _DrawRGBImage(m_img, false, 8, m_sfc->width, m_sfc->height, stride, 0, 0);
    }
  }	
}

void SoloSubFrame::_DrawCenterCrosshair()
{
  if (main_window->isCrosshairCentering())
  {
    gint x1 = (gint)m_xctr * m_bup_factor;
	gint x2 = (gint)m_xctr * m_bup_factor;
	gint y1 = ((gint)m_yctr -7) * m_bup_factor;
	gint y2 = ((gint)m_yctr +7) * m_bup_factor;

	Glib::RefPtr<Gdk::Window> window =  get_window();
    if (window)
    {
		Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
		cr->set_line_width (1.0);
	    cr->move_to (x1, y1);
	    cr->line_to (x2, y2);
    	cr->stroke();	

		y1 = y2 = (gint)m_yctr * m_bup_factor;
		x1 = ((gint)m_xctr -7) * m_bup_factor;
		x2 = ((gint)m_xctr +7) * m_bup_factor;
	    cr->move_to (x1, y1);
	    cr->line_to (x2, y2);
    	cr->stroke();	
    }	
  }
}

void SoloSubFrame::_DrawPlotTitleColorbarLabels()
{
  // Draw the plot title and the color bar labels
  __sii_do_annotation();

}

void SoloSubFrame::_SetColorOverlays()
{
  // Set the color for drawing the overlays
  GdkColor *gcolor = sii_grid_color(m_frame_num, m_exposed);
  if (!gcolor)
    return;
  _SetForegroundColor(gcolor);
}

void SoloSubFrame::_DrawBorders()
{
  // Reset the clip rectangle
  cairo_reset_clip (m_cr);
 
  // Draw a two pixel width border
  GdkColor *gcolor = frame_border_colors[m_wwptr->lock_num];
  if (!gcolor)
    return;
  _SetForegroundColor(gcolor);

  _DrawRectangle(false, BORDER_PIXELS, 0, 0, m_frame->allocation.width, m_frame->allocation.height);
  _DrawRectangle(false, BORDER_PIXELS, 1, 1, m_frame->allocation.width - BORDER_PIXELS, m_frame->allocation.height - BORDER_PIXELS);

  // Redraw the boundaries
  if (m_redraw_bnd)
    __se_redraw_all_bnds_for_frame(m_frame_num);
 		     
}

void SoloSubFrame::_DrawXYTicks()
{
  // set coords of ulc relative to radar 
  gdouble lat = m_wwptr->radar_location.getLatitude();
  gdouble lon = m_wwptr->radar_location.getLongitude();
  gdouble alt = m_wwptr->radar_location.getAltitude();

  m_sfc->ulc_radar = m_sfc->corner[0];// get lat/lon/alt and absolute pixels
  SiiPoint *pt = &m_sfc->ulc_radar; // upper left 

  loop_xy2ll(&pt->lat, &pt->lon, &pt->alt,
	     &pt->xx, &pt->yy, &pt->zz,
	     lat, lon, alt,
	     EarthRadiusCalculator::getInstance()->getRadius(lat),
	     1);
  
  double xtici;
  double ytici;
  bool xyannot;
  
  m_sfc->view_window->getTicInfo(m_wwptr, xtici, ytici, xtics, ytics, xyannot);

  if (xtics || ytics)
  {
    SiiPoint *pt = m_sfc->corner; // upper left
    SiiPoint *ptlr = &m_sfc->corner[2];

    gint nxtics;
    double xtic1;
    
    if (time_series)
    {
      nxtics = 7;
      xtici = (m_sfc->corner[1].xx - m_sfc->corner[0].xx) / (nxtics + 1);
      xtic1 = m_sfc->corner[0].xx + xtici;
    }
    else
    {
      xtic1 = __sii_tic1(pt->xx, xtici);
      nxtics = (gint)((ptlr->xx - xtic1) / xtici + 1);
    }
    double ytic1 = __sii_tic1(ptlr->yy, ytici);
    gint nytics = (gint)((pt->yy - ytic1) / ytici + 1);
    double ytic = ytic1;
    gint ticlen = 13;

    for (gint kk = 0; kk < nytics; kk++, ytic += ytici)
    {
      double xtic = xtic1;
      for (gint jj = 0; jj < nxtics; jj++, xtic += xtici)
      {
		gint x;
		gint y;
	
		__sii_lmrk_coords_to_frame_coords(m_sfc, m_wwptr, xtic, ytic, &x, &y);
		x *= m_bup_factor;
		y *= m_bup_factor;

		if (xtics)
		{
		  // Draw x tic mark
		  gint x1 = x - (ticlen / 2);
		  gint x2 = x1 + ticlen - 1;
		  _DrawLine(false, x1, y, x2, y);
		}
		if (ytics)
		{
		  // Draw y tic mark
		  gint y1 = y - (ticlen / 2);
		  gint y2 = y1 + ticlen - 1;
		  _DrawLine(false, x, y1, x, y2);
		}
      }
    }

    double ytica = ytic - ytici;		// last tic drawn //

    if (xyannot)
    {
      double xtic = xtic1;
      double ytic = ytic1;
      gint x;
      gint y;
      gchar annot[16];
      
      for (xtic = xtic1; ; xtic += xtici)
      {
		__sii_lmrk_coords_to_frame_coords(m_sfc, m_wwptr, xtic, ytic, &x, &y);
		if (x >= clip.x)
		  break;
	  }

      for (; ; ytica -= ytici)
      {
		__sii_lmrk_coords_to_frame_coords(m_sfc, m_wwptr, xtic, ytica, &x, &y);
		if (y > clip.y - ticlen / 2)
		  break;
      }

      for (ytic = ytic1; ; ytic += ytici)
      {
		__sii_lmrk_coords_to_frame_coords(m_sfc, m_wwptr, xtic, ytic, &x, &y);
		if (y <= clip.y + clip.height)
		  break;
      }

      xtic1 = xtic;
      ytic1 = ytic;

      for (gint jj = 0; jj < nxtics; jj++, xtic += xtici)
      {
		__sii_lmrk_coords_to_frame_coords(m_sfc, m_wwptr, xtic, ytica, &x, &y);
	
		x *= m_bup_factor;
		y *= m_bup_factor;
	
		double tval;
		if (time_series)
		{
		  // Seconds offset from start time
		  tval = ts_span * xtic / (m_sfc->corner[1].xx - m_sfc->corner[0].xx) +0.5;
		}
		else
		{
		  tval = xtic;
		}
		
		std::string fmt = (xtici < 2.0) ? "%.1f" : "%.0f";
		sprintf(annot, fmt.c_str(), tval);

	    cairo_font_extents_t fe;
	    cairo_text_extents_t te;
	    cairo_font_extents (m_cr, &fe);
	    cairo_text_extents (m_cr, annot, &te);
	    x -= (gint) te.width / 2;
	    y += (gint) ((ticlen / 2) + fe.ascent + fe.descent);
	    _DrawText(annot, x, y);
      }

      xtic = xtic1;
      for (gint jj = 0; jj < nytics; jj++, ytic += ytici)
      {
		__sii_lmrk_coords_to_frame_coords(m_sfc, m_wwptr, xtic, ytic, &x, &y);
	
		x *= m_bup_factor;
		y *= m_bup_factor;
	
		std::string fmt = (ytici < 2.) ? "%.1f" : "%.0f";
		sprintf(annot, fmt.c_str(), ytic);

	    cairo_font_extents_t fe;
	    cairo_text_extents_t te;
	    cairo_font_extents (m_cr, &fe);
	    cairo_text_extents (m_cr, annot, &te);
	    x += (ticlen / 2) + 1;
	    y += (gint) (fe.ascent + fe.descent) / 2;
	    _DrawText(annot, x, y);
      }
    }
  }  
}

void SoloSubFrame::_DrawAzimuthLines()
{
  // Draw the azimuth lines.
  if (!time_series)
  {
    SiiFrameConfig *draw_sfc = m_sfc;
    if (m_blow_up)
      draw_sfc = frame_configs[0];
    
	gdouble rng = m_wwptr->view->az_annot_at_km;
	gdouble rri = m_wwptr->view->rng_ring_int_km;
	if (rri <= 0.0)
		rri = 5.0;

	gchar annot[16];

	for (gdouble angle = 0.0;
		m_wwptr->view->az_line_int_deg > 0 && angle < 360.0;
		angle += m_wwptr->view->az_line_int_deg)
	{
		gdouble theta = RADIANS (CART_ANGLE (angle));
    	gdouble xx = m_xlmrk + cos (theta) * rri * pixels_per_km;
    	gdouble yy = m_ylmrk - sin (theta) * rri * pixels_per_km;
	    gint x1 = (gint)((xx > 0) ? xx +.5 : xx -.5) * m_bup_factor;
    	gint y1 = (gint)((yy > 0) ? yy +.5 : yy -.5) * m_bup_factor;

	    xx = m_xlmrk + cos (theta) * max_range * pixels_per_km;
	    yy = m_ylmrk - sin (theta) * max_range * pixels_per_km;
	    gint x2 = (gint)((xx > 0) ? xx +.5 : xx -.5) * m_bup_factor;
	    gint y2 = (gint)((yy > 0) ? yy +.5 : yy -.5) * m_bup_factor;

		_DrawLine(true,x1, y1, x2, y2);
	
		// Draw label
	    if (rng <= 0)
	      continue;

	    sprintf (annot, "%.0f", angle);

		cairo_font_extents_t fe;
		cairo_text_extents_t te;
		cairo_font_extents (m_cr, &fe);
		cairo_text_extents (m_cr, annot, &te);

	    xx = m_xlmrk + (cos (theta) * (rng * pixels_per_km));
	    yy = m_ylmrk - (sin (theta) * (rng * pixels_per_km));
	    gint x = (gint)((xx > 0) ? xx + 0.5 : xx - 0.5) * m_bup_factor;
	    gint y = (gint)((yy > 0) ? yy + 0.5 : yy - 0.5) * m_bup_factor - (gint)(fe.ascent + fe.descent);
	
	    x = (gint) ((angle > 180) ? x : x - te.width - 4);
	    if (angle > 315 || angle < 45)
	      y -= (gint) (fe.ascent + fe.descent) / 2;
	    else if (angle > 135 && angle < 225)
	      y += (gint) (fe.ascent + fe.descent) / 2;

	    _DrawText(annot, x, y);
	}
  }
}

void SoloSubFrame::_DrawRangeRings()
{
  // Draw the range rings.
  if (!time_series)
  {

    // Determine the angle to use for the range ring labels
    gdouble label_angle;
  
    if (m_wwptr->magic_rng_annot)
    {
      gdouble theta = atan2 (pt_max->yy, pt_max->xx);
      label_angle = FMOD360 (CART_ANGLE (DEGREES (theta)));  }
    else
    {
      label_angle = m_wwptr->view->rng_annot_at_deg;
    }

    gchar annot[16];

    gdouble rng = m_wwptr->view->rng_ring_int_km;
    gboolean inside = __sii_lmrk_inside(m_sfc);

    // Set up the static arcs list

    if (!m_sii_arcs)
    {
      m_sii_arcs = (SiiArc *)g_malloc0(4 * sizeof (SiiArc));
      for (gint jj = 0; jj < 4; jj++)
      {
        m_sii_arcs[jj].id = jj;
        m_sii_arcs[jj].next = m_sii_arcs + jj + 1;
      }
      m_sii_arcs[3].next = m_sii_arcs; // make it circular
    }

    // Draw the rings

    for (; rng > 0 && rng < max_range; rng += m_wwptr->view->rng_ring_int_km)
    {
      gdouble xx = m_xlmrk - (rng * pixels_per_km);
      gdouble yy = m_ylmrk - (rng * pixels_per_km);
      gint x = (gint)((xx > 0) ? xx + 0.5 : xx - 0.5) * m_bup_factor;
      gint y = (gint)((yy > 0) ? yy + 0.5 : yy - 0.5) * m_bup_factor;
      gint width = (gint)(2 * rng * pixels_per_km + 0.5) * m_bup_factor;

      if (rng > m_wwptr->view->rng_ring_int_km)
      {
      // Look for partial circles. Drawing full circles gets very slow at
      // high zooms

        gint num_arcs = _RngRingArcs(m_sfc, rng);
        if (!inside && num_arcs == 0)
		  continue;
        SiiArc *arc = m_sii_arcs;
        gint loop_count = (num_arcs > 0) ? 4 : 1;
        gint iangle = 0;
        gint iarc_len = 360 * 64;

        for (; loop_count--; arc++)
        {
		  if (num_arcs && arc->angle1 >= 0)
		  {
		    iangle = (int)(64 * arc->angle1);
		    gdouble d = arc->angle2 - arc->angle1;
		    if (d < 0.0)
		      d += 360.0;
		    iarc_len = (int)(64 * d);
		  }
		  else if (num_arcs)
		  {
		    continue;
		  }
	
		  gint cx = x + width / 2;
		  gint cy = y + width / 2;
		  gint radius = width / 2;
		  
          _DrawCircle(true, cx, cy, radius);
        }
      }

	  // Label the range ring.  If the label angle is negative, we don't want
	  // the labels.
	  if (label_angle < 0)
	    continue;

      gdouble theta = RADIANS (CART_ANGLE (label_angle));

      xx = m_xlmrk + ((rng * pixels_per_km) * cos(theta));
      yy = m_ylmrk - ((rng * pixels_per_km) * sin(theta));
      x = (gint)((xx > 0) ? xx + 0.5 : xx - 0.5) * m_bup_factor;
      y = (gint)((yy > 0) ? yy + 0.5 : yy - 0.5) * m_bup_factor;

      std::string fmt = (m_wwptr->view->rng_ring_int_km < 3) ? "%.1f" : "%.0f";
      sprintf(annot, fmt.c_str(), rng);

	  // Draw label. This should be changed to PangoLayout.
	  cairo_font_extents_t fe;
	  cairo_text_extents_t te;
	  cairo_font_extents (m_cr, &fe);
	  cairo_text_extents (m_cr, annot, &te);
	  double xb = (label_angle > 180) ? x : x - te.width;
	  double yb = (label_angle > 90 && label_angle < 270) ? y : y -fe.ascent;//y + (fe.ascent + fe.descent);

	  _DrawText(annot, (gint)xb, (gint)yb);
	}
  }
}

gint SoloSubFrame::_RngRingArcs(SiiFrameConfig *sfc, gdouble range)
{
   SiiPoint *pt;
   gdouble xx, xx2, yy, yy2, theta, theta2, xmin, xmax, ymin, ymax, dg, dg2;
   gboolean inside, swap_xy;
   gint jj, kk, nn, num_intxns = 0, num_arcs = 0;
   gint mark, count = 0;
   SiiArc *side = m_sii_arcs, *side2;
   Dxyz xy, *xy1, *xy2;
   static double rng_trip = 21;
   
   pt = &sfc->corner[0];	// upper left km. relative to landmark
   xmin = pt->xx;
   ymax = pt->yy;
   pt = &sfc->corner[2];	// lower right
   xmax = pt->xx;
   ymin = pt->yy;
   
   inside = xmin < 0 && xmax > 0 && ymin < 0 && ymax > 0;

   // Go around the boundaries starting with the top

   if (range > rng_trip)
     { mark = 0; }

   for (jj=0; jj < 4; jj++, side++) {

      pt = &sfc->corner[jj];
      side->num_intxns = 0;
      side->nxt_ndx = 0;
      side->angle1 = side->angle2 = -360;

      if (jj & 1) {		// consider the x-axis
	 xx = pt->xx;
	 if (inside && range < fabs(xx))
	   { continue; }
	 else if (!inside && fabs (xx) > range)
	   { continue; }
	 theta = acos (xx/range);
	 yy = range * sin (theta);
	 yy2 = -yy;
	 kk = 0;
	 
	 if (yy > ymin && yy < ymax ) {
	    side->xy[kk].x = xx;
	    side->xy[kk].y = yy;
	    kk = ++side->num_intxns;
	    num_intxns++;
	 }
	 if (yy2 > ymin && yy2 < ymax ) {
	    side->xy[kk].x = xx;
	    side->xy[kk].y = yy2;
	    side->num_intxns++;
	    num_intxns++;
	 }
      }
      else {			// y-axis
	 yy = pt->yy;
	 if (inside && range < fabs(yy))
	   { continue; }
	 else if (!inside && fabs (yy) > range)
	   { continue; }
	 theta = asin (yy/range);
	 xx = range * cos (theta);
	 xx2 = -xx;
	 kk = side->num_intxns = 0;
	 if (xx > xmin && xx < xmax ) {
	    side->xy[kk].x = xx;
	    side->xy[kk].y = yy;
	    kk = ++side->num_intxns;
	    num_intxns++;
	 }
	 if (xx2 > xmin && xx2 < xmax ) {
	    side->xy[kk].x = xx2;
	    side->xy[kk].y = yy;
	    side->num_intxns++;
	    num_intxns++;
	 }
      }	// end if 

      if (side->num_intxns == 2) {

	 if (num_intxns == 2) {
	    // first side with intersections and
	    // there are two intersection; start with the second one
	    side->nxt_ndx = 1;
	 }
	 // order the intxns to move clockwise around the borders
	 swap_xy = FALSE;

	 switch (jj) {
	  case 0:		// top border
	    if (side->xy[0].x > side->xy[1].x)
	      { swap_xy = TRUE; }
	    break;
	  case 1:		// right border
	    if (side->xy[0].y < side->xy[1].y)
	      { swap_xy = TRUE; }
	    break;
	  case 2:		// bottom border
	    if (side->xy[0].x < side->xy[1].x)
	      { swap_xy = TRUE; }
	    break;
	  case 3:		// left border
	    if (side->xy[0].y > side->xy[1].y)
	      { swap_xy = TRUE; }
	    break;
	 };
	 if (swap_xy) {
	    xy = side->xy[0];
	    side->xy[0] = side->xy[1];
	    side->xy[1] = xy;
	 }
      }	// end if (side->num_intxns
   } // end for

   if (num_intxns & 1)
     { printf ("num_intxns:%d is odd!\n", num_intxns); return 0; }

   if (num_intxns < 2)
     { return 0; }

   // Go around clockwise starting at the top and define the arcs

   for (side = m_sii_arcs ;;) {			// c...mark
      count++;
      if (!side->num_intxns) {
	 side = side->next;
	 if (side->id == 0)
	   { return num_arcs; }
	 continue;
      }

      if (num_intxns == 2 && side->num_intxns == 2) {
	 num_arcs = 1;		// intersects only one boundary 
	 xy1 = &side->xy[0];
	 xy2 = &side->xy[1];
	 theta = atan2 (xy1->y, xy1->x);
	 side->angle1 = 
	   (theta < 0) ? DEGREES (theta) +360. : DEGREES (theta);
	 theta2 = atan2 (xy2->y, xy2->x);
	 side->angle2 = 
	   (theta2 < 0) ? DEGREES (theta2) +360. : DEGREES (theta2);
	 return num_arcs;
      }

      xy1 = &side->xy[side->nxt_ndx];
      side2 = side->next;
      nn = 3;
      
      if (side->num_intxns == 2)
	{ side->nxt_ndx = (side->nxt_ndx == 1) ? 0 : 1; }

      // find the next intersection
      for ( ; nn--; ) {
	 if (side2->num_intxns)
	   { break;}
	 side2 = side2->next;
      }
      xy2 = &side2->xy[side2->nxt_ndx];
      side2->nxt_ndx = (side2->nxt_ndx == 1) ? 0 : 1;
      num_arcs++;

      // set the angle limits of the arc
      theta = atan2 (xy1->y, xy1->x);
      dg = (theta < 0) ? DEGREES (theta) +360. : DEGREES (theta);
      theta2 = atan2 (xy2->y, xy2->x);
      dg2 = (theta2 < 0) ? DEGREES (theta2) +360. : DEGREES (theta2);

      // arcs are counter clockwise from the x-axis
      switch (side->id) {
       case 0:			// top border

	 switch (side2->id) {
	  case 1:		// right border
	    if (xy2->y < 0 ) {	// arc in lower-left quadrant
	       // arc from xy1 -> xy2
	       side->angle1 = dg;
	       side->angle2 = dg2;
	    }
	    else {		// upper-right quad 
	       side->angle1 = dg2;
	       side->angle2 = dg;
	    }
	    break;
	  case 2:		// bottom border
	    if (xy1->x <= 0) {	// left hemisphere 
	       side->angle1 = dg;
	       side->angle2 = dg2;
	    }
	    else {		// right 
	       side->angle1 = dg2;
	       side->angle2 = dg;
	    }
	    break;
	  case 3:		// left border 
	    if (xy2->y <= 0) {	// lower hemisphere
	       side->angle1 = dg2;
	       side->angle2 = dg;
	    }
	    else {		// upper hemisphere 
	       side->angle1 = dg;
	       side->angle2 = dg2;
	    }
	    break;
	 };
	 break;

       case 1:

	 // can only have an unprocessed
	 // intersections with the bottom or left border (2,3)
	 switch (side2->id) {
	  case 0:		// top line
	    if (xy1->y <= 0) {
	       side->angle1 = dg2;
	       side->angle2 = dg;
	    }
	    else {
	       side->angle1 = dg;
	       side->angle2 = dg2;
	    }
	    break;
	  case 2:		// bottom 
	    if (xy1->y <= 0) {	// lower right quad 
	       side->angle1 = dg2;
	       side->angle2 = dg;
	    }
	    else {		// upper left quad
	       side->angle1 = dg;
	       side->angle2 = dg2;
	    }
	    break;
	  case 3:
	    if (xy1->y <= 0) {	// lower hemisphere
	       side->angle1 = dg2;
	       side->angle2 = dg;
	    }
	    else {		// upper 
	       side->angle1 = dg;
	       side->angle2 = dg2;
	    }
	    break;
	 };
	 break;

       case 2:			// bottom 

	 switch (side2->id) {
	  case 0:		// top 
	    // if there are two intersections, this is the left one 
	    side->angle1 = dg2;
	    side->angle2 = dg;
	    break;
	  case 1:		// right border 
	    if (xy1->y < 0) {	
	       side->angle1 = dg2;
	       side->angle2 = dg;
	    }
	    else {		// upper 
	       side->angle1 = dg;
	       side->angle2 = dg2;
	    }
	    break;
	  case 3:		// left border 
	    if (xy1->x >= 0) {	// right hemisphere 
	       side->angle1 = dg;
	       side->angle2 = dg2;
	    }
	    else {		// upper 
	       side->angle1 = dg2;
	       side->angle2 = dg;
	    }
	    break;
	 };
	 break;

       case 3:			// left border 
	 switch (side2->id) {
	  case 0:
	    side->angle1 = dg2;
	    side->angle2 = dg;
	    break;
	  case 1:		// right 
	    // if there are two intersections, this is the top one 
	    side->angle1 = dg2;
	    side->angle2 = dg;
	    break;
	  case 2:		// bottom border 
	    if (xy1->y < 0) {
	       side->angle1 = dg;
	       side->angle2 = dg2;
	    }
	    else {		
	       side->angle1 = dg2;
	       side->angle2 = dg;
	    }
	    break;
	 };
	 break;
      };
      if (num_arcs *2 >= num_intxns)
	{ return num_arcs; }

      if (side2->num_intxns == 1) {
	 side = side2->next;
      }
      else {
	 side = side2;
      }

      if (count > 5)
	{ printf ("exit c:%d\n", count); return 0; }

   }

   return num_arcs;
}


void SoloSubFrame::_FindMaxRange()
{
  // Find the maximum range
  max_range = 0;
  SiiPoint *pt = m_sfc->corner;
  
  for (gint jj = 0; jj < 4; jj++, pt++)
  {
    pt->rng_km = sqrt (pt->xx*pt->xx + pt->yy*pt->yy);
    if (pt->rng_km > max_range)
    {
      max_range = pt->rng_km;
      pt_max = pt;
    }
  }

  gchar str[512];
  
  if (max_range > 500)
  {
    sprintf (str, "SoloSubFrame::_FindMaxRange(): Max: %.3f > 500", max_range);
    g_message (str);
    max_range = 500;
  }

  max_range *= .9999;
}

void SoloSubFrame::_CalcCenterLandmark()
{
  // Get the center and landmark locations, needed for rendering the overlays.

  // To draw range rings, find the location of the landmark
  // and turn it into screen coordinates
  //
  // Since the range rings are slant range, when the landmark is the
  // radar we have to factor in the fixed angle.
  //
  // When the landmark is not the radar, we project the range rings
  // onto the surface.
  //
  PointInSpace landmark;
  PointInSpace center;
  PointInSpace radar;
  
  if (m_wwptr->landmark_info->landmark_options == SOLO_LINKED_POSITIONING)
  {
    WW_PTR wwptrx =  solo_return_wwptr(m_wwptr->landmark_info->reference_frame-1);
    landmark = wwptrx->landmark;
  }
  else
  {
    landmark = m_wwptr->landmark;
  }
  center = m_wwptr->center_of_view;
  radar = m_wwptr->radar_location;

  // Calculate (x,y,z) of center relative to the landmark. The values are
  // placed in (x,y,z) of the landmark

  landmark.latLonRelative(center);

  gdouble xx2ctr;
  gdouble yy2ctr;
  
  if (landmark.getLatitude() != radar.getLatitude() ||
      landmark.getLongitude() != radar.getLongitude())
  {
    xx2ctr = landmark.getX();
    yy2ctr = landmark.getY();
  }
  else
  {
    gdouble theta = RADIANS(CART_ANGLE(center.getRotationAngle()));
    xx2ctr = center.getRange() * cos (theta);
    yy2ctr = center.getRange() * sin (theta);
  }
  
  m_xctr = m_sfc->width * 0.5;
  m_yctr = m_sfc->height * 0.5;
  pixels_per_km =
    m_wwptr->view->magnification / sp_meters_per_pixel() * 1000.0;

  if (time_series)
  {
    xx2ctr = m_xctr / pixels_per_km;
    yy2ctr = m_wwptr->center_of_view.getRange();
  }
  else if (m_wwptr->lead_sweep->scan_mode == DD_SCAN_MODE_RHI ||
	   m_wwptr->scan_mode == DD_SCAN_MODE_AIR)
  {
    gdouble theta =
      RADIANS(CART_ANGLE (m_wwptr->center_of_view.getRotationAngle()));
    xx2ctr = m_wwptr->center_of_view.getRange() * cos(theta);
    yy2ctr = m_wwptr->center_of_view.getRange() * sin(theta);
  }

  m_xlmrk = m_xctr - (xx2ctr * pixels_per_km);
  m_ylmrk = m_yctr + (yy2ctr * pixels_per_km);

  // set the corners relative to the landmark (ul, ur, lr, ll)

  SiiPoint *pt = m_sfc->corner;

  pt->dtime = ts_start;
  pt->x = 0;
  pt->y = 0;
  pt->xx = xx2ctr - (m_xctr / pixels_per_km);
  pt->yy = yy2ctr + (m_yctr / pixels_per_km);
  pt->zz = 0;

  // set the upper left corner lat/lon to facilitate overlays

  gdouble lat = center.getLatitude();
  gdouble lon = center.getLongitude();
  gdouble alt = center.getAltitude();

  pt++;				// next corner (upper right)
  pt->dtime = ts_start + ts_span;
  pt->x = m_sfc->width - 1;
  pt->y = 0;
  pt->xx = xx2ctr + (m_xctr / pixels_per_km);
  pt->yy = yy2ctr + (m_yctr / pixels_per_km);
  gdouble xx = m_xctr / pixels_per_km;
  gdouble yy = m_yctr / pixels_per_km;
  gdouble zz = 0;
  loop_xy2ll(&pt->lat, &pt->lon, &pt->alt, &xx, &yy, &zz,
	     lat, lon, alt ,
	     EarthRadiusCalculator::getInstance()->getRadius(lat),
	     1);

  pt++;				// next corner (lower right) 
  pt->dtime = ts_start + ts_span;
  pt->x = m_sfc->width - 1;
  pt->y = m_sfc->height - 1;
  pt->xx = xx2ctr + (m_xctr / pixels_per_km);
  pt->yy = yy2ctr - (m_yctr / pixels_per_km);
  xx = m_xctr / pixels_per_km;
  yy = -m_yctr / pixels_per_km;
  zz = 0;
  loop_xy2ll(&pt->lat, &pt->lon, &pt->alt, &xx, &yy, &zz,
	     lat, lon, alt ,
	     EarthRadiusCalculator::getInstance()->getRadius(lat),
	     1);

  pt++;				// next corner (lower left)
  pt->dtime = ts_start;
  pt->x = 0;
  pt->y = m_sfc->height - 1;
  pt->xx = xx2ctr - (m_xctr / pixels_per_km);
  pt->yy = yy2ctr - (m_yctr / pixels_per_km);
  xx = -m_xctr / pixels_per_km;
  yy = -m_yctr / pixels_per_km;
  zz = 0;
  loop_xy2ll(&pt->lat, &pt->lon, &pt->alt, &xx, &yy, &zz,
	     lat, lon, alt ,
	     EarthRadiusCalculator::getInstance()->getRadius(lat),
	     1);
}

void SoloSubFrame::_DrawRectangle(gboolean fill, gint lineWidth, gint x, gint y, gint width, gint height)
{
  CRectangle rectangle(m_cr, fill, x, y, width, height);
  rectangle.SetLineWidth(lineWidth);
  rectangle.Draw();
}

void SoloSubFrame::_DrawRectangle(gboolean fill, gint x, gint y, gint width, gint height)
{
  CRectangle rectangle(m_cr, fill, x, y, width, height);
  rectangle.Draw();
}

void SoloSubFrame::_DrawText(const std::string &text, gint x, gint y)
{
  Label label(m_cr, text.c_str(), x, y);
  label.SetPangoFont(m_font);
  label.Draw();
}

void SoloSubFrame::_GetLayoutPixelSize(const std::string &text,
				       gint *width, gint *height)
{		
  Label label(m_cr);
  label.SetPangoFont(m_font);
  label.GetLayoutPixelSize(text.c_str(), width, height);
}

void SoloSubFrame::_SetForegroundColor(GdkColor *gcolor)
{
	Graphic graphic(m_cr, 0, 0);
	graphic.SetForegroundColor(gcolor);
}

void SoloSubFrame::_DrawRGBImage(const guchar *data, gboolean has_alpha, 
    				   int bits_per_sample, int width, int height, int rowstride, 
                       gint pixbuf_x, gint pixbuf_y)
{
	RGBImage image(m_cr, pixbuf_x, pixbuf_y, data, has_alpha, bits_per_sample,
					 width, height, rowstride);
	image.Draw();         
}

void SoloSubFrame::_DrawCircle(gboolean dashflag, gint x, gint y, gint r)
{
	Circle circle(m_cr, dashflag, x, y, r);
	circle.Draw();
}

void SoloSubFrame::_DrawLine(gboolean dashflag, gint x1, gint y1, gint x2, gint y2)
{
  Line line(m_cr, dashflag, x1, y1, x2, y2);
  line.Draw();
}

void SoloSubFrame::_SetClipRectangle()
{
  // Set the clip rectangle.
  __sii_get_clip_rectangle(&clip);
  cairo_rectangle(m_cr, clip.x, clip.y, clip.width, clip.height);
  cairo_clip(m_cr);
}

gdouble SoloSubFrame::__sii_tic1 (gdouble tic, gdouble ticinc)
{
  gdouble d = fabs (tic), tic1, rem;

  rem = fmod (d, ticinc);
  tic1 = (tic < 0) ? tic + rem : tic + ticinc -rem;
  return tic1;
}

void SoloSubFrame::__sii_lmrk_coords_to_frame_coords (SiiFrameConfig *sfc, WW_PTR wwptr,
				     gdouble xx_km, gdouble yy_km,
				     gint *x, gint *y)
{
  gdouble ppk, xx, yy;

  ppk = wwptr->view->magnification/M_TO_KM (sp_meters_per_pixel());
  xx = (xx_km -sfc->corner[0].xx) * ppk;
  yy = -(yy_km -sfc->corner[0].yy) * ppk;
  *x = (gint) ((xx < 0) ? xx -.5 : xx +.5);
  *y = (gint) ((yy < 0) ? yy -.5 : yy +.5);
}

gboolean SoloSubFrame::__sii_lmrk_inside (SiiFrameConfig *sfc)
{
   SiiPoint *pt, *pt2;
   gboolean inside;
   pt = &sfc->corner[0];	// upper left km. relative to landmark
   pt2 = &sfc->corner[2];	// lower right

   inside = pt->xx <= 0 && pt2->xx >= 0 && pt2->yy <= 0 && pt->yy >= 0;
   return inside;
}

void SoloSubFrame::__sii_do_annotation()
{
  SiiFrameConfig *sfc0 = frame_configs[0];
  ParamData *pd = m_sfc->param_data;
  SiiPalette *pal;
  const gchar *cc;
  gint x, y, width, height, font_height;
  gint frame_width, frame_height, label_height;
  gint b_width, mx_width = 0, kk, widths[64], cb_height, yb;
  std::size_t jj;
  gint cb_thickness;
  gdouble cb_locs[64];

  if (m_blow_up) {
    m_font = sfc0->big_font;
    cb_thickness = m_sfc->font_height *2;
    font_height = sfc0->big_font_height;
    m_frame  = sfc0->blow_up_frame;
    frame_width = sfc0->big_width;
    frame_height = sfc0->big_height;
    m_cr  = gdk_cairo_create(m_frame->window);
  }
  else {
    m_font = m_sfc->font;
    cb_thickness = 
      font_height = m_sfc->font_height;
    m_frame  = m_sfc->frame;
    frame_width = m_sfc->width;
    frame_height = m_sfc->height;
    m_cr  = gdk_cairo_create(m_frame->window);
  }

  pal = (m_exposed) ? pd->orig_pal : pd->pal;
  _GetLayoutPixelSize(" ",&b_width,&height);

  // color bar labels 
  cc = set_cb_labeling_info (m_frame_num, cb_locs);
  std::vector< std::string > tokens;
  tokenize(cc, tokens);
  label_height = height - 2;
  
  for (mx_width=0,jj=0; jj < tokens.size(); jj++)
  {
    // set width of longest annotation
    _GetLayoutPixelSize(tokens[jj], widths+jj, &height);

    if (widths[jj] > mx_width)
      { mx_width = widths[jj]; }
  }

  if (pd->cb_loc == PARAM_CB_LEFT || pd->cb_loc == PARAM_CB_RIGHT) {
    if (m_blow_up) {
      sfc0->big_max_lbl_wdt = mx_width + 2*b_width;
    }
    else {
      m_sfc->max_lbl_wdt = mx_width + 2*b_width;
    }
    cb_height = frame_height;

    for (kk=tokens.size()-1,jj=0; jj < tokens.size(); jj++,kk--)
    {
      x = (pd->cb_loc == PARAM_CB_LEFT)
	      ? cb_thickness + b_width + mx_width -widths[jj]
	      : frame_width -cb_thickness -b_width -widths[jj];

      y = (gint) (frame_height -cb_height * cb_locs[jj]
	      - font_height/2 -2);
      
      const Gdk::Color &bkgnd_color =
	          pal->getFeatureColor(SiiPalette::FEATURE_BACKGROUND);
	  _SetForegroundColor((GdkColor *) bkgnd_color.gobj());

      if (pd->toggle[PARAM_HILIGHT]) {
	    _DrawRectangle(true, x-1, y+3, widths[jj]+1, font_height-3);		
      }
      
      y += label_height +1;
      
      const Gdk::Color &annot_color =
	          pal->getFeatureColor(SiiPalette::FEATURE_ANNOTATION);
	  _SetForegroundColor((GdkColor *) annot_color.gobj());
	 
      if (m_blow_up)
      	_DrawText(tokens[jj], x, y-font_height+4);
      else
      	_DrawText(tokens[jj], x, y-font_height+7);
    }
  }
  else {
    
    // clear space for color bar labels
    const Gdk::Color &bkgnd_color =
            pal->getFeatureColor(SiiPalette::FEATURE_BACKGROUND);
	_SetForegroundColor( (GdkColor *) bkgnd_color.gobj());

    y = frame_height -cb_thickness -4;
    yb = frame_height -cb_thickness -font_height +1;
    
	// Draw x-axis labels: g.g. -15 -5 5 15 25 35 45 for frame_0
    for (jj=0; jj < tokens.size(); jj++) 
    {
      x = (gint)(frame_width * cb_locs[jj]);
      x -= widths[jj]/2;

      if (pd->toggle[PARAM_HILIGHT])
      {
	    const Gdk::Color &bkgnd_color =
	            pal->getFeatureColor(SiiPalette::FEATURE_BACKGROUND);
	    _SetForegroundColor( (GdkColor *) bkgnd_color.gobj() );
        _DrawRectangle(true, x-1, yb, widths[jj]+2, font_height-2);		
      }
      
      const Gdk::Color &annot_color =
	          pal->getFeatureColor(SiiPalette::FEATURE_ANNOTATION);

	  _SetForegroundColor( (GdkColor *) annot_color.gobj() );

	  _DrawText(tokens[jj],x,yb-3);  //_DrawText(aa, x, y);
    }
  }

  // Draw title: e.g. 1 08/07/2006 00:06:02 KFTG_RVP 0.6 SUR DBZ for frame_0
  const Gdk::Color &bkgnd_color =
          pal->getFeatureColor(SiiPalette::FEATURE_BACKGROUND);
  _SetForegroundColor( (GdkColor *) bkgnd_color.gobj());
  
  _GetLayoutPixelSize(m_wwptr->top_line, &width, &height);
  x = (frame_width -width)/2;
 
  // clear space for plot title
  if (pd->toggle[PARAM_HILIGHT])
  {
	_DrawRectangle(true, x,0,width+1, font_height);
	//_DrawRectangle(true, x,0,frame_width, font_height);		
  }
  
  y = font_height -2;
  const Gdk::Color &annot_color =
          pal->getFeatureColor(SiiPalette::FEATURE_ANNOTATION);
  _SetForegroundColor((GdkColor *) annot_color.gobj());
  _DrawText(m_wwptr->top_line, x, 0);
}

void SoloSubFrame::__sii_get_clip_rectangle (Rectangle *clip)
{
  SiiFrameConfig *sfc0 = frame_configs[0];
  ParamData *pd = m_sfc->param_data;
  guint mlw;
  gint font_height, max_lbl_wdt, frame_width, frame_height, cb_thickness;

  if (m_blow_up) {
    max_lbl_wdt = sfc0->big_max_lbl_wdt;
    font_height = sfc0->big_font_height;  
    frame_width = sfc0->big_width;
    frame_height = sfc0->big_height;
    cb_thickness = 2 * m_sfc->font_height;
  }
  else {
    max_lbl_wdt = m_sfc->max_lbl_wdt;
    font_height = m_sfc->font_height;  
    frame_width = m_sfc->width;
    frame_height = m_sfc->height;
    cb_thickness = m_sfc->font_height;
  }

  if (pd->cb_loc == PARAM_CB_LEFT || pd->cb_loc == PARAM_CB_RIGHT) {
    mlw = max_lbl_wdt;

    clip->x = (pd->cb_loc == PARAM_CB_LEFT)
      ? cb_thickness + mlw +1
      : 0;
    clip->y = font_height +2;
    clip->width = frame_width -cb_thickness - mlw -2;
    clip->height = frame_height -font_height -2;
  }
  else {
    clip->x = 0;
    clip->width = frame_width;
    clip->y = font_height +2;
    clip->height = frame_height - 2*font_height -cb_thickness -4;
  }
}

void SoloSubFrame::__se_redraw_all_bnds_for_frame(int frame_num)
{
    struct one_boundary *ob;
    struct solo_perusal_info *spi;

    spi = solo_return_winfo_ptr();
    SeBoundaryList *sebs = SeBoundaryList::getInstance();
    ob = sebs->firstBoundary;

    for(; ob; ob = ob->next) {
    	
      if(ob->num_points > 1) {
		__se_draw_bnd_for_frame
	  		(frame_num, ob->top_bpm->last, ob->num_points, NO);
      }
	}
}

void SoloSubFrame::__se_draw_bnd_for_frame (int frame_num, struct bnd_point_mgmt *bpm, int num, int erase)
{
  /* draw or erase the requisite number of points in the active windows
   * see "se_shift_bnd()" for similar logic
   */
  int mm, mark, right_left, width, height;
  struct bnd_point_mgmt *bpmx;
  struct solo_perusal_info *spi;
  WW_PTR wwptr;
  double x, y, z, costilt, untilt, tiltfactor;
  double d, v_scale, h_scale, ta, time_span, top_edge;

  spi = solo_return_winfo_ptr();
  SeBoundaryList *sebs = SeBoundaryList::getInstance();
  if(sebs->viewBounds || frame_num >= SOLO_MAX_WINDOWS)
    return;

  PointInSpace boundary_radar = sebs->getOrigin();
  untilt = fabs(cos(RADIANS(boundary_radar.getTilt())));

  wwptr = solo_return_wwptr(frame_num);
  PointInSpace current_radar = wwptr->radar_location;

  /*
	 * we now have the frame radar (current_radar)
	 * and the location of the boundary radar
	 * see also the routine "sp_locate_this_point()" in file
	 * "sp_clkd.c"
	 */
  current_radar.latLonRelative(boundary_radar);

  /*
	 * x,y,z now contains the coordinates of the boundary radar
	 * relative to the current radar
	 */
  x = KM_TO_M(current_radar.getX());
  y = KM_TO_M(current_radar.getY());
  z = KM_TO_M(current_radar.getZ());

  if(fabs(costilt = cos(fabs(RADIANS(current_radar.getTilt())))) > 1.e-6) {
    tiltfactor = fabs(1./costilt);
  }
  else {
    tiltfactor = 0;
  }
  mm = num;
  bpmx = bpm;

  if(wwptr->view->type_of_plot & SOLO_TIME_SERIES) {
    d = sp_meters_per_pixel();
    d /= (double)wwptr->view->ts_magnification;
    v_scale = M_TO_KM(d);
    height = wwptr->view->height_in_pix-1;
    top_edge = wwptr->view->ts_ctr_km + .5 * height * v_scale;

    ta = wwptr->sweep->start_time;
    time_span = wwptr->sweep->stop_time - ta;
    width = wwptr->view->width_in_pix-1;
    h_scale = (double)width/time_span;
    right_left = wwptr->view->type_of_plot & TS_PLOT_RIGHT_TO_LEFT;
    /*
     * we will be passing pixel coodinates and assuming that (0,0) is
     * the upper left corner of the screen
     */
    for(; mm--; bpmx = bpmx->last) {
      bpmx->_x = (int32_t)((bpmx->pisp->getTime() - ta) * h_scale + 0.5);
      if(right_left) bpmx->_x = width - bpmx->_x;
      bpmx->_y = (int32_t)((top_edge - bpmx->pisp->getZ()) / v_scale + 0.5);
    }
  }	
  else if(!(wwptr->lead_sweep->sweep->radar_type == DD_RADAR_TYPE_GROUND ||
	    wwptr->lead_sweep->sweep->radar_type == DD_RADAR_TYPE_SHIP)) {

    for(; mm--; bpmx = bpmx->last) {
      bpmx->_x = bpmx->x;
      bpmx->_y = bpmx->y;
    }
  }
  else {			/* ground based */

    /* we need to project the original values
     * and then unproject the new values
     */
    for(; mm--; bpmx = bpmx->last) {
      switch (wwptr->lead_sweep->scan_mode) {
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

  if(!(wwptr->view->type_of_plot & SOLO_TIME_SERIES)) {
    mark = 0;
  }
  if(erase) {
    se_erase_segments(frame_num, num, bpm);
  }
  else {
    se_draw_segments(frame_num, num, bpm);
  }
}


/*********************************************************************
 * _initTimeSeries()
 */

bool SoloSubFrame::_initTimeSeries(guint frame_num,
				   gdouble &tstart, gdouble &tspan)
{
  int ii = SOLO_TIME_SERIES;
  WW_PTR wwptr = solo_return_wwptr(frame_num);
  tstart = wwptr->sweep->start_time;
  tspan = wwptr->sweep->stop_time - wwptr->sweep->start_time;
  ii = wwptr->view->type_of_plot & SOLO_TIME_SERIES;

  return ii != 0;
}
