/* 	$Id$	 */

#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <stdarg.h>
#include <string>

#include <gdkmm/screen.h>

#include <ClickQueue.hh>
#include <dd_math.h>
#include <DataInfo.hh>
#include <DataManager.hh>
#include <EditWindow.hh>
#include <se_bnd.hh>
#include <se_utils.hh>
#include <se_wgt_stf.hh>
#include "SeBoundaryList.hh"
#include <seds.h>
#include "sii_callbacks.hh"
#include "sii_config_stuff.hh"
#include "sii_frame_menu.hh"
#include "sii_param_widgets.hh"
#include "sii_perusal.hh"
#include "sii_png_image.hh"
#include "sii_utils.hh"
#include "sii_xyraster.hh"
#include <solo_editor_structs.h>
#include <solo_list_widget_ids.h>
#include "solo2.hh"
#include "soloii.hh"
#include "SoloSubFrame.hh"
#include "sp_basics.hh"
#include "sp_clkd.hh"
#include "sp_lists.hh"

using namespace std;

/* c------------------------------------------------------------------------ */
				/* displayq stuff */
/* c------------------------------------------------------------------------ */

static gint bup_factor = 1;

# define HIGH_PRF 0
# define LOW_PRF 1

/* c------------------------------------------------------------------------ */

static gboolean redraw_bnd = TRUE;
DD_TIME dts;

/* c------------------------------------------------------------------------ */

void sii_png_image_prep(const std::string &dir)
{
  // Get pointers to the objects we need

  WW_PTR wwptr = solo_return_wwptr(0);
  DataInfo *data_info = DataManager::getInstance()->getWindowInfo(0);

  // If a directory name wasn't specified, get it from the window info

  std::string local_dir = dir;
  if (local_dir == "")
    local_dir = wwptr->sweep->directory_name;

  // Get the full path name

  char fname[256];
  sprintf(fname, "%s/%s_%.1f_%s_%s",
	  local_dir.c_str(),
	  DataManager::getInstance()->getFileBaseName("png",
						      (int32_t)wwptr->sweep->start_time,
						      wwptr->sweep->radar_name,
						      0).c_str(),
	  data_info->getFixedAngle(),
	  DataInfo::scanMode2Str(data_info->getScanMode()).c_str(),
	  wwptr->parameter.parameter_name);
							   
  // Create the PNG image

  sii_png_image(fname);
}

/* c------------------------------------------------------------------------ */

void sii_reset_reconfig_flags(int frame_num)
{
  SiiFrameConfig *sfc = frame_configs[frame_num];
  /*
   * Record the frame size for reference later if
   * not a drag resize
   */
  if (!(sfc->reconfig_flag & FrameDragResize)) {
     sfc->x_drag_ref = sfc->width;
     sfc->y_drag_ref = sfc->height;
  }
  sfc->reconfig_count = 0;
  sfc->local_reconfig = 0;
  sfc->reconfig_flag = 0;
}

/* c------------------------------------------------------------------------ */

guint sii_frame_sync_num (guint frame_num)
{
  SiiFrameConfig *sfc = frame_configs[frame_num];
  return (sfc->sync_num);
}

/* c------------------------------------------------------------------------ */

guint sii_config_sync_num (guint frame_num)
{
  SiiFrameConfig *sfc = frame_configs[frame_num];
  return (sfc->config_sync_num);
}

/* c------------------------------------------------------------------------ */

int se_dump_sfic_widget(struct swp_file_input_control *sfic, int frame_num)
{
  WW_PTR wwptr = solo_return_wwptr(frame_num);

  sfic->directory_text = wwptr->sweep->directory_name;
  sfic->directory = wwptr->sweep->directory_name;
  sfic->radar_names_text = wwptr->sweep->radar_name;
  sfic->start_time = sfic->stop_time = wwptr->d_sweepfile_time_stamp;

  EditWindow *edit_window = frame_configs[frame_num]->edit_window;
  if (edit_window == 0)
  {
    std::cerr << "**** ERROR:  se_dump_sfic_widget called before EditWindow created" << std::endl;
    exit(1);
  }
  else
  {
    sfic->start_time = edit_window->getStartTime();
    sfic->stop_time = edit_window->getStopTime();
  }
  
  sfic->version_text = "last";
  sfic->version = 99999;
  sfic->new_version_flag = NO;

 if (!edit_window->editStartStop(sfic))
   return 0;

  return 1;
}

/* c------------------------------------------------------------------------ */

int 
sii_return_frame_count (void)
{ return sii_frame_count; }

/* c---------------------------------------------------------------------- */

double sii_return_swpfi_time_stamp (guint frame_num)
{
  WW_PTR wwptr = solo_return_wwptr(frame_num);
  return wwptr->lead_sweep->d_sweepfile_time_stamp;
}

/* c---------------------------------------------------------------------- */

std::string sii_return_swpfi_dir(guint frame_num)
{

  WW_PTR wwptr = solo_return_wwptr(frame_num);
  wwptr = wwptr->lead_sweep;
  return wwptr->sweep->directory_name;
}

/* c---------------------------------------------------------------------- */

std::vector< std::string > sii_return_swpfi_list(guint frame_num)
{
  struct solo_perusal_info *spi = solo_return_winfo_ptr();
  WW_PTR wwptr = solo_return_wwptr(frame_num);

  solo_gen_swp_list(wwptr->lead_sweep->window_num);
  return (spi->list_sweeps);
}

/* c------------------------------------------------------------------------ */

void sii_center_on_clicks(gint num_clicks)
{
  guint frame_num;
  
  ClickQueue *click_queue = ClickQueue::getInstance();
  
  // NOTE: Divide into two functions: sii_center_on_clicks(num_clicks) and
  // sii_center_on_clicks(num_clicks, frame_num).  The first part of this
  // condition is for the second function, the rest is for the first.  Then
  // move everything else into another function that is called by each of
  // these.

  if (num_clicks == 0 || num_clicks >= TASK_MODULO)
  {
    // zero => frame:0 & center on radar

    frame_num = num_clicks / TASK_MODULO;
    num_clicks %= TASK_MODULO;
  }
  else if (click_queue->empty())
  {
    sii_message ("Not enough clicks in the same frame");
    return;
  }
  else
  {
    SiiPoint point = click_queue->getLatestClick();

    if (num_clicks > (int)click_queue->getClickCount())
    {
      sii_message ("Not enough clicks in the same frame");
      return;
    }

    frame_num = point.frame_num;
  }

  // NOTE: Is this processing a single click or centering on the radar???

  // Process a single click in a frame

  if (num_clicks < 2 && frame_num >= 0)
  {
    LinksInfo *li = frame_configs[frame_num]->link_set[LI_CENTER];

    for (guint jj = 0; jj < SOLO_MAX_WINDOWS; jj++)
    {
      if (!li->link_set[jj])
	continue;

      WW_PTR linked_wwptr = solo_return_wwptr(jj);
      float f_angle = (num_clicks < 1) ? 0 : linked_wwptr->clicked_angle;
      float f_range = (num_clicks < 1) ? 0 : linked_wwptr->clicked_range;

      // The following angle and range are relative to the radar and are
      // used by "sp_rtheta_screen_to_xyz()"

      linked_wwptr->center_of_view.setRotationAngle(f_angle);
      linked_wwptr->center_of_view.setRange(f_range);
      linked_wwptr->view->ts_ctr_km = f_range;
    }

    sii_plot_data(frame_num, REPLOT_ALL);

    for (int jj = 0; jj < sii_return_frame_count(); jj++)
    {
      ViewWindow *view_window = frame_configs[jj]->view_window;
      if (view_window != 0)
	view_window->update();
    }

    return;
  }

  std::vector< SiiPoint > prev_clicks =
    click_queue->getClicksInFrame(frame_num, num_clicks);
  if ((int)prev_clicks.size() != num_clicks)
  {
    sii_message ("Not enough clicks in the same frame");
    return;
  }
  
  double minx = 0.0;
  double maxx = 0.0;
  double miny = 0.0;
  double maxy = 0.0;
  
  // Get the bounds of the clicks

  for (int i = 0; i < num_clicks; ++i)
  {
    if (prev_clicks[i].xx < minx)
      minx = prev_clicks[i].xx;
    if (prev_clicks[i].xx > maxx)
      maxx = prev_clicks[i].xx;
    if (prev_clicks[i].yy < miny)
      miny = prev_clicks[i].yy;
    if (prev_clicks[i].yy > maxy)
      maxy = prev_clicks[i].yy;
  }

  double dx = (maxx - minx);
  double dy = (maxy - miny);
  WW_PTR curr_wwptr = solo_return_wwptr(frame_num);
  double ppk = curr_wwptr->view->magnification/sp_meters_per_pixel() * 1000;

  double factor;
  
  SiiFrameConfig *sfc = frame_configs[frame_num];

  if (curr_wwptr->view->type_of_plot & SOLO_TIME_SERIES)
    factor = (sfc->height/ppk)/dy;
  else if( dx/dy > sfc->width/sfc->height)
    factor = (sfc->width/ppk)/dx;
  else
    factor = (sfc->height/ppk)/dy;

  LinksInfo *li = frame_configs[frame_num]->link_set[LI_VIEW];

  for (guint jj = 0; jj < SOLO_MAX_WINDOWS; jj++)
  {
    if (!li->link_set[jj])
      continue;

    WW_PTR linked_wwptr = solo_return_wwptr(jj);
    linked_wwptr->view->magnification *= factor;
    linked_wwptr->view->ts_magnification = linked_wwptr->view->magnification;
  }

  double xx = minx + .5 * dx;
  double yy = miny + .5 * dy;

  // Set range and rot. angle of center relative to radar

  double theta = atan2(yy, xx);
  curr_wwptr->view->ts_ctr_km = sqrt(xx*xx + yy*yy);
  curr_wwptr->center_of_view.setRange(curr_wwptr->view->ts_ctr_km);
  curr_wwptr->center_of_view.setRotationAngle(CART_ANGLE(DEGREES(theta)));

  PointInSpace radar = curr_wwptr->lead_sweep->radar_location;
  sp_rtheta_screen_to_xyz(frame_num, radar, curr_wwptr->center_of_view);
  curr_wwptr->center_of_view.latLonShift(radar);

  li = frame_configs[frame_num]->link_set[LI_CENTER];

  for (int jj = 0; jj < SOLO_MAX_WINDOWS; jj++)
  {
    if (!li->link_set[jj] || jj == curr_wwptr->window_num)
      continue;

    WW_PTR linked_wwptr = solo_return_wwptr(jj);

    if (curr_wwptr->sweep->linked_windows[jj])
    {
      linked_wwptr->center_of_view.setRotationAngle(curr_wwptr->center_of_view.getRotationAngle());
      linked_wwptr->view->ts_ctr_km = curr_wwptr->center_of_view.getRange();
      linked_wwptr->center_of_view.setRange(curr_wwptr->center_of_view.getRange());
    }
    else
    {
      PointInSpace radar = linked_wwptr->lead_sweep->radar_location;
      radar.latLonRelative(curr_wwptr->center_of_view); /* set xyz in "radar" */
      sp_xyz_to_rtheta_screen(jj, radar, linked_wwptr->center_of_view);
				/* set new (r,theta) in "center_of_view" */
    }
  }
  
  sii_plot_data(frame_num, REPLOT_ALL);
  for (int jj = 0; jj < sii_return_frame_count(); jj++)
  {
    ViewWindow *view_window = frame_configs[jj]->view_window;
    if (view_window != 0)
      view_window->update();
  }
}

/* c------------------------------------------------------------------------ */

void sii_set_geo_coords(int frame_num, gdouble dx, gdouble dy,
			SiiPoint &click)
{
  WW_PTR wwptrx = solo_return_wwptr(frame_num);

  double scale = M_TO_KM(sp_meters_per_pixel()) / wwptrx->view->magnification;

  double theta = RADIANS(CART_ANGLE(wwptrx->center_of_view.getRotationAngle()));
  double xxCtr = wwptrx->center_of_view.getRange() * cos(theta);
  double yyCtr = wwptrx->center_of_view.getRange() * sin(theta);
  double xx = xxCtr + scale * dx;
  double yy = yyCtr + scale * dy;

  click.xx = xx;
  click.yy = yy;
  sp_data_click(frame_num, xx, yy);

}

/* c------------------------------------------------------------------------ */

void print_fieldvalues(int frame_num,
		       const std::vector< std::string > &field_list)
{
  std::string field_list_string;
  
  for (size_t i = 0; i < field_list.size(); ++i)
  {
    if (i != 0)
      field_list_string += "\n";
    field_list_string += field_list[i];
  }
  
  sii_set_click_data_text(frame_num, field_list_string);
}

/* c------------------------------------------------------------------------ */

void 
yes_exit (void) {
  gtk_main_quit ();
}

/* c------------------------------------------------------------------------ */

void sii_set_boundary_pt_count (int frame_num, int num_points)
{
  frame_configs[0]->points_in_bnd = num_points;
  return;
}

/* c------------------------------------------------------------------------ */
void se_clear_segments(int frame_num)
{
  SiiFrameConfig *sfc = frame_configs[frame_num];
  GdkRectangle area;

  area.x = 0;
  area.y = 0;
  area.width = sfc->width;
  area.height = sfc->height;
  sii_xfer_images (frame_num, &area);
}

/* c------------------------------------------------------------------------ */

void sii_radar_coords_to_frame_coords(int frame_num,
				      gdouble xx_km, gdouble yy_km,
				      gint *x, gint *y)
{
  SiiFrameConfig *sfc = frame_configs[frame_num];
  WW_PTR wwptr = solo_return_wwptr(frame_num);
  gdouble ppk, xx, yy;

  ppk = wwptr->view->magnification/M_TO_KM (sp_meters_per_pixel());
  xx = (xx_km -sfc->ulc_radar.xx) * ppk;
  yy = -(yy_km -sfc->ulc_radar.yy) * ppk;
  *x = (gint) ((xx < 0) ? xx -.5 : xx +.5);
  *y = (gint) ((yy < 0) ? yy -.5 : yy +.5);
}

/* c------------------------------------------------------------------------ */

void se_erase_segments (int frame_num, int num_points
			, struct bnd_point_mgmt *bpm)
{
  GdkRectangle area;
  gint x1, y1, x2, y2, x, y;
  

  if (num_points < 2 || frame_num >= sii_return_frame_count()) {
    return;
  }
  if (num_points > 2) {
    se_clear_segments (frame_num);
  }
    sii_radar_coords_to_frame_coords (frame_num
				      , (gdouble)M_TO_KM (bpm->_x)
				      , (gdouble)M_TO_KM (bpm->_y)
				      , &x1, &y1);
    sii_radar_coords_to_frame_coords (frame_num
				      , (gdouble)M_TO_KM (bpm->last->_x)
				      , (gdouble)M_TO_KM (bpm->last->_y)
				      , &x2, &y2);
  area.x = (x1 < x2) ? x1 : x2;
  area.y = (y1 < y2) ? y1 : y2;
  x = (x1 < x2) ? x2 : x1;
  y = (y1 < y2) ? y2 : y1;
  area.width = x -area.x +1;
  area.height = y -area.y +1;
  redraw_bnd = FALSE;
  sii_xfer_images (frame_num, &area);
  redraw_bnd = TRUE;
}

/* c------------------------------------------------------------------------ */

void se_draw_segments (int frame_num, int num_points,
		       struct bnd_point_mgmt *bpm)
{
  /* Draw the segments of the boundary
   * what comes in are endpoints relative to the local radar
   * in meters.
   * the list is a stack and is drawn from bottom to top
   */
  SiiFrameConfig *sfc = frame_configs[frame_num];
  SiiFrameConfig *sfc0 = frame_configs[0];
  WW_PTR wwptr = solo_return_wwptr(frame_num);
  gint x1, y1, x2, y2, jj, xlate;
  GdkColor *gcolor;
  SiiPoint *pt;
  gdouble ppk;
  gboolean in1, in2;

  if (frame_num >= sii_return_frame_count() || num_points < 2)
    { return; }

  gcolor = sii_boundary_color (frame_num, 1);
  
  ppk = wwptr->view->magnification/M_TO_KM (sp_meters_per_pixel());
  pt = &sfc->ulc_radar;	  /* contains coordinates of the ulc per the radar */
  xlate = sfc->height -1;

  for(jj = 0; jj < num_points-1; jj++, bpm = bpm->last){
    /* convert to screen coordinates */
    
    if((wwptr->view->type_of_plot & SOLO_TIME_SERIES)) {
      /* already in screen coordinates */
      x1 = bpm->_x;
      x2 = bpm->last->_x;
      y1 = bpm->_y;
      y2 = bpm->last->_y;
    }
    else {
      sii_radar_coords_to_frame_coords (frame_num
					, (gdouble)M_TO_KM (bpm->_x)
					, (gdouble)M_TO_KM (bpm->_y)
					, &x1, &y1);
      sii_radar_coords_to_frame_coords (frame_num
					, (gdouble)M_TO_KM (bpm->last->_x)
					, (gdouble)M_TO_KM (bpm->last->_y)
					, &x2, &y2);
    }

    in1 = x1 >= 0 && x1 < (int)sfc->width && y1 >= 0 && y1 < (int)sfc->height;
    in2 = x2 >= 0 && x2 < (int)sfc->width && y2 >= 0 && y2 < (int)sfc->height;

    if (in1 || in2) {
      
      /* define the corners of a rectangle containing the boundary */
      if (jj == 0) {
	sfc->ulc_bnd.x = sfc->lrc_bnd.x = x1;
	sfc->ulc_bnd.y = sfc->lrc_bnd.y = y1;
      }
      if (x2 < sfc->ulc_bnd.x)
	{ sfc->ulc_bnd.x = x2; }
      if (x2 > sfc->lrc_bnd.x)
	{ sfc->lrc_bnd.x = x2; }
      if (y2 < sfc->ulc_bnd.y)
	{ sfc->ulc_bnd.y = y2; }
      if (y2 > sfc->lrc_bnd.y)
	{ sfc->lrc_bnd.y = y2; }
      
	  Line line(sfc->frame->window, false, x1, y1, x2, y2);
	  line.Graphic::SetForegroundColor(gcolor);
	  line.Draw();
      
      if (sfc->blow_up) {
	    Line line(sfc0->blow_up_frame->window, false, x1*2, y1*2, x2*2, y2*2);
	    line.Graphic::SetForegroundColor(gcolor);
	    line.Draw();
      }
    }
  }

  return;

}

/* c------------------------------------------------------------------------ */

gboolean sii_blow_up (guint frame_num)
{
  static GString *gs = NULL;

  if (frame_num >= sii_frame_count)
    return FALSE;

  // NOTE:  I'm not sure what the 32 is below.  Is this the height of the
  // menu bar?  I'm not sure how to get that from Gtk.

  SiiFrameConfig *sfc = frame_configs[frame_num];

  if (2 * (int)sfc->width >= Gdk::Screen::get_default()->get_width() ||
      2 * (int)sfc->height >= Gdk::Screen::get_default()->get_height() - 32)
    return FALSE;
    
  SiiFrameConfig *sfc0 = frame_configs[0];

  if (sfc->blow_up)
  {
    // Consecutive hits on this frame. Hide it!

    if (sfc0->blow_up_frame)
      gtk_widget_hide(sfc0->blow_up_window);

    sfc->blow_up = FALSE;
    sfc0->blow_up_frame_num = 0;
    return FALSE;
  }

//  if (sfc0->big_width != sfc->width * 2 ||
//      sfc0->big_height != sfc->height * 2)
  // When blow_up window is closed, need to set up sfc->blow_up to FALSE. 
  // Not sure if the change affects other logics.
  else
  {
    if (!gs)
      gs = g_string_new ("");

    if (sfc0->blow_up_window)
    {
      g_free (sfc0->big_image->data);
    
      if(sfc0!=NULL && GTK_IS_WIDGET(sfc0->blow_up_window))
	      gtk_widget_destroy(sfc0->blow_up_window);
    }
    else
    {
      sfc0->big_image = (SiiImage *)g_malloc0(sizeof (SiiImage));
    }

    sfc0->big_reconfig_count = 0;
    sfc0->big_width = sfc->width * 2;
    sfc0->big_height = sfc->height * 2;
    sfc0->big_image_size = sfc0->big_width * sfc0->big_height * 3;
    sfc0->big_image->data = (guchar *)g_malloc0(sfc0->big_image_size);

    // Create the window object

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    sfc0->blow_up_window = window;

    // Create the container object

    GtkWidget *vbox = gtk_vbox_new(FALSE,0);
    gtk_container_add(GTK_CONTAINER(window), vbox);

    // Create the frame

    GtkWidget *frame = gtk_drawing_area_new();
    
    sfc0->blow_up_frame = frame;

    gint event_flags = GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK |
      GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_FOCUS_CHANGE_MASK;
    gtk_widget_set_events(frame, event_flags);
    gtk_container_add(GTK_CONTAINER(vbox), frame);
    gtk_drawing_area_size(GTK_DRAWING_AREA(frame),
			  sfc0->big_width, sfc0->big_height);
      
    // Connect the appropriate signals to the blow-up frame

    gtk_signal_connect(GTK_OBJECT(frame), "expose_event",
		       (GtkSignalFunc)sii_blow_up_expose_event,
		       (gpointer)frame_num);
    gtk_signal_connect(GTK_OBJECT(frame),"configure_event",
		       (GtkSignalFunc)sii_blow_up_config_event,
		       (gpointer)frame_num);
    gtk_signal_connect(GTK_OBJECT(sfc0->blow_up_window),"key_press_event",
		       (GtkSignalFunc)sii_blow_up_keyboard_event,
		       (gpointer)frame_num);
    gtk_signal_connect(GTK_OBJECT(frame),"button_press_event",
		       (GtkSignalFunc)sii_blow_up_mouse_button_event,
		       (gpointer)frame_num);

    // When blow_up window is closed, set up sfc->blow_up to FALSE.
    gtk_signal_connect(GTK_OBJECT (sfc0->blow_up_window), "delete_event",
		       (GtkSignalFunc)sii_blow_up_mouse_delete_event,
		       (gpointer)frame_num);	       
  }

  // Put the current frame number into the window title

  g_string_sprintf(gs, "Frame %d", frame_num + 1);
  gtk_window_set_title(GTK_WINDOW(sfc0->blow_up_window), gs->str);

  GtkWidget *frame = sfc0->blow_up_frame;
  gtk_object_set_data(GTK_OBJECT(frame),
		      "frame_num",
		      (gpointer)frame_num);
  gtk_object_set_data(GTK_OBJECT(sfc0->blow_up_window),
		      "frame_num",
		      (gpointer)frame_num);

  gtk_widget_show_all(sfc0->blow_up_window);
  sii_double_colorize_image(frame_num);
  sfc->big_sync_num = sfc->sync_num;

  // Set the blow-up flag for this frame.  Make sure it is the only frame with
  // this flag set.

  for (gint jj = 0; jj < MAX_FRAMES; frame_configs[jj++]->blow_up = FALSE);
  sfc->blow_up = TRUE;

  sfc0->blow_up_frame_num = frame_num + 1;

  // Rerender the data for this frame

  sii_really_xfer_images(frame_num, NULL, TRUE);

  return TRUE;
}

/* c------------------------------------------------------------------------ */

void sii_xfer_images (int frame_num, GdkRectangle *area)
{
  gboolean blow_up = FALSE;
  SiiFrameConfig *sfc = frame_configs[frame_num];

  sii_really_xfer_images (frame_num, area, blow_up);

  if (sfc->blow_up) {
    if (sfc->big_sync_num != sfc->sync_num) {
      sfc->blow_up = FALSE;
      sii_blow_up (frame_num);
    }
    else
      { sii_really_xfer_images (frame_num, area, TRUE); }
  }
}

/* c------------------------------------------------------------------------ */

// Render the radar frame image.

void sii_really_xfer_images(int frame_num,
			    GdkRectangle *image_clipping_region,
			    gboolean blow_up)
{
  // NOTE: Need to look more closely at the blow-up rendering to see if we
  // can do this more cleanly.

  /*
   * transfer the image
   * label the plot 
   * label the color bar
   * draw boundaries
   * draw border
   */

  // Check the frame number.  If it is invalid, don't do anything.

  if (frame_num >= sii_return_frame_count())
    return;

  // Draw sub frames. Encapsulated all the functions into SoloSubFrame.
  SoloSubFrame solo_sub_frame(frame_num,blow_up,image_clipping_region,bup_factor,redraw_bnd);
  solo_sub_frame.Draw();
}

/* c------------------------------------------------------------------------ */

void sii_update_frame_info (guint frame_num)
{
  WW_PTR wwptr = solo_return_wwptr(frame_num);
  SiiFrameConfig *sfc = frame_configs[frame_num];

  wwptr->view->width_in_pix = sfc->width;
  wwptr->view->height_in_pix = sfc->height;

  wwptr->big_image = sfc->big_image;
  wwptr->image = sfc->image;

  sii_reset_image (frame_num);
}

/* c------------------------------------------------------------------------ */

void sii_check_image_size (guint frame_num)
{
  SiiFrameConfig *sfc = frame_configs[frame_num];
  guint size;
  gchar str[256];


  size = sfc->width * sfc->height;

  if (size > sfc->max_image_size) {
     sprintf (str, "Enlarge Image %d %d %d -> %d"
	      , frame_num, sii_frame_count, sfc->max_image_size, size);    
     if (sfc->image) {
	if (sfc->image->data)
	  { g_free (sfc->image->data); }
     }
     else {
	sfc->image = (SiiImage *)g_malloc0 (sizeof (SiiImage));
     }
     sfc->image->data = g_malloc0 (size * 8);
     sprintf (str+strlen(str), " data:%p", sfc->image->data);
     /*
     g_message (str);
      */
     sii_append_debug_stuff (str);
     sfc->max_image_size = size;
  }
  sfc->sync_num = sfc->config_sync_num;
  sfc->image_size = size;
}

/* c------------------------------------------------------------------------ */

void sii_plot_data2(guint frame_num, guint plot_function)
{
  if (plot_function < 0)
    return;

  switch (plot_function)
  {
  case BACKWARDS_FOREVER:
  case BACKWARDS_ONCE:
  case REPLOT_LOCK_STEP:
  case REPLOT_THIS_FRAME:
  case FORWARD_FOREVER:
  case FORWARD_ONCE:
    displayq(frame_num, plot_function);
    break;
    
  case REPLOT_ALL:
    sp_replot_all();
    break;
  }
}

/* c------------------------------------------------------------------------ */

void sii_plot_data(guint frame_num, guint plot_function)
{
  // Update and or modify image sizes
    
  SiiFrameConfig *sfc = frame_configs[frame_num];

  if (!sfc->local_reconfig &&
      sfc->drag_resize_count > 0 &&
      !(sfc->reconfig_flag & FrameDragResize))
  {
    // Next plot after a drag resize. Do a local reconfig
    // in case the frame sizes are not consistant

     sii_new_frames();
  }
  
  sii_plot_data2(frame_num, plot_function);
}

/* c---------------------------------------------------------------------- */

// NOTE: This method is called whenever ther are no events in the queue.
// When it returns false, it is removed from the event queue and not called
// again.

gboolean sii_batch (gpointer argu)
{
  gchar *png_dir;
				/*
  g_message ("sii_batch");
				 */
  for (guint jj = 0; jj < sii_frame_count; jj++)
  {
    if (frame_configs[jj]->expose_count == 0) {
      // NOTE: Flushes the X output buffer and waits until all requests have
      // been processed by the server. This is rarely needed by applications.
      // It's main use is for trapping X errors with gdk_error_trap_push()
      // and gdk_error_trap_pop(). 
      gdk_flush();
      return TRUE;
    }
  }
  if (batch_mode) {
				/* get png destination dir */
    png_dir = (gs_image_dir) ? gs_image_dir->str : NULL;
    
    for (;;) {
      if (png_dir == 0)
	sii_png_image_prep("");
      else
	sii_png_image_prep(png_dir);
      if(displayq(0, FORWARD_ONCE))
	{ break; }		/* non-zero reply => it's over */
    }
    gtk_main_quit();
  }
  return FALSE;
}
/* c------------------------------------------------------------------------ */

void solo_message(const char *message)
{
    printf("%s", message);
}
/* c---------------------------------------------------------------------- */

// NOTE: Need to update this so that we aren't returning a pointer to a
// constant string.  This is only used in soloii.cc in main(), but I need
// to think about memory management before fixing this, so for now we are
// just casting the "./" to get rid of the compiler warning

gchar *sii_get_swpfi_dir (gchar *dir)
{
   gint nn;

   if (!dir) {
     dir = getenv ("DORADE_DIR");
   }
   if (!dir)
   {
     dir = (gchar *)"./";
   }

   nn = DataManager::getInstance()->compileFileList(0, dir);
   return (nn < 1) ? NULL : dir;
}

/* c---------------------------------------------------------------------- */

// NOTE: This is called at startup if there are sweep files in the dorade
// directory

int sii_default_startup (const gchar *swpfi_dir)
{
  std::vector< std::string > param_names;
  
  struct solo_perusal_info *spi = solo_return_winfo_ptr();
   
  for (int frme = 0; frme < SOLO_MAX_WINDOWS; frme++)
  {
    // Just set it up so that the default plot is to plot variables from
    // the same radar for the same time

    spi->active_windows[frme] = YES;
    WW_PTR wwptr = solo_return_wwptr(frme); /* activate a window */
    wwptr->frame_ctr_info->reference_frame = frme + 1;
    wwptr->landmark_info->reference_frame = frme + 1;
    wwptr->parameter.linked_windows[frme] = YES;

    for (int jj = 0; jj < SOLO_MAX_WINDOWS; jj++)
    {
      wwptr->lock->linked_windows[jj] = YES;
      wwptr->sweep->linked_windows[jj] = YES;
      wwptr->view->linked_windows[jj] = YES;
      wwptr->landmark_info->linked_windows[jj] = YES;
      wwptr->frame_ctr_info->linked_windows[jj] = YES;
    }

    wwptr->active = YES;
    wwptr->changed = YES;
    wwptr->sweep->changed = YES;
    wwptr->lock->changed = YES;
    wwptr->parameter.changed = YES;
    wwptr->view->changed = YES;
  }
  solo_worksheet();

  WW_PTR wwptr0 = solo_return_wwptr(0);
  strncpy(wwptr0->sweep->directory_name, swpfi_dir, 128);
  if (wwptr0->sweep->directory_name[strlen(wwptr0->sweep->directory_name)-1] != '/')
    strncat(wwptr0->sweep->directory_name, "/", 128);
  solo_trigger_dir_rescans(wwptr0->sweep->directory_name);

  int nn =
    DataManager::getInstance()->compileFileList(0,
						wwptr0->sweep->directory_name);
  if (nn < 1)
    return 0;

  // Nab a radar name

  char *radar_name;
  
  if ((radar_name = DataManager::getInstance()->getRadarNameData(0, 0)) != 0)
    strncpy(wwptr0->sweep->radar_name, radar_name, 16);
  else
    return 0;

  // See if we can nab a sweep file for this directory

  if (! DataManager::getInstance()->nabNextFile(0,
					  DataManager::TIME_NEAREST,
					  DataManager::LATEST_VERSION, YES)) {
      return 0;
  }

  // Figure out what parameters you are going to plot.  Assign the parameters
  // in the order that they appear in the data to the frames.  If there are
  // more frames than parameters, start over at the beginning of the parameter
  // list.

  DataInfo *data_info = DataManager::getInstance()->getWindowInfo(0);
  std::vector< std::size_t > param_index_list = data_info->getParamIndices();
  int num_params = param_index_list.size();
  
  for (int frme = 0; frme < SOLO_MAX_WINDOWS; frme++)
  {
    WW_PTR wwptr = solo_return_wwptr(frme); 
    wwptr->parameter_num = param_index_list[frme % num_params];
    strncpy(wwptr->parameter.parameter_name,
	    data_info->getParamNameClean(wwptr->parameter_num, 8).c_str(), 16);

    // Find the appropriate color palette

    if (frme < num_params)
      param_names.push_back(wwptr->parameter.parameter_name);

    if (frme != 0)
      wwptr->radar_location = wwptr->lead_sweep->radar_location;

    sp_landmark_offsets(frme);
    sp_align_center(frme);

    sii_initialize_parameter(frme, wwptr->parameter.parameter_name);
    strncpy(wwptr->parameter.palette_name, sii_param_palette_name(frme), 16);
  }

  for (int frme = 0; frme < SOLO_MAX_WINDOWS; frme++)
    sii_set_param_names_list(frme, param_names);

  return nn;
}

/* c------------------------------------------------------------------------ */
/* Create the data coloration lookup table.  This is the master lookup table */
/* that is in effect until the parameter changes or color information        */
/* changes.  This is a 64K lookup table that works for raw data <= 16-bits   */

void solo_data_color_lut(int frame_num)
{
  // Get the information for calculating the color table

  SiiPalette *palette = frame_configs[frame_num]->param_data->pal;
  
  float ctr = palette->getCenterDataValue();
  float inc = palette->getColorWidth();
  int num_colors = palette->getNumColors();

  WW_PTR wwptr = solo_return_wwptr(frame_num);
  
  double f_min = (double)ctr - 0.5 * (double)num_colors * inc;
  double f_max = -0.000001 + (double)f_min + num_colors * inc;
  int min = (int)DD_SCALE(f_min, wwptr->parameter_scale,
			  wwptr->parameter_bias); 
  int max = (int)DD_SCALE(f_max, wwptr->parameter_scale,
			  wwptr->parameter_bias);
  min = min >= (-K32) ? min : -K32;
  max = max < K32 ? max : K32-1;

  wwptr->color_table.setColors(f_min, f_max,
			       wwptr->num_colors_in_table);
  
  // Fill in values outside of the data range with the exceeded color

  for (int i = (-K32); i < min; i++)
  {
    *(wwptr->data_color_lut+i) = wwptr->exceeded_color_num;
  }
  for (int i = max + 1; i < K32; i++)
  {
    *(wwptr->data_color_lut+i) = wwptr->exceeded_color_num;
  }

  wwptr->color_table.setExceededColorIndex(wwptr->exceeded_color_num);
  
  // Colors for the data

  double f_inc = (double)wwptr->num_colors_in_table/(double)(max-min+1);
  double f_color_num = 0.0;
  
  for (int i = min; i <= max; i++, f_color_num += f_inc)
  {
    wwptr->data_color_lut[i] = (int)f_color_num;
  }

  float emin = palette->getEmphasisZoneLower();
  float emax = palette->getEmphasisZoneUpper();
  
  if (emin < emax)
  {
    // Do the emphasis zone

    int min = (int)DD_SCALE(emin, wwptr->parameter_scale,
			    wwptr->parameter_bias);
    int max = (int)DD_SCALE(emax, wwptr->parameter_scale,
			    wwptr->parameter_bias);
    min = min >= (-K32) ? min : -K32;
    max = max < K32 ? max : K32-1;

    for (int i = min; i <= max; i++)
    {
      wwptr->data_color_lut[i] = wwptr->emphasis_color_num;
    }
  }

  wwptr->color_table.setEmphasisRange(emin, emax,
							 wwptr->emphasis_color_num);
  
  // Missing data color

  wwptr->data_color_lut[wwptr->parameter_bad_val] =
    wwptr->missing_data_color_num;

  double unscaled_missing = DD_UNSCALE(wwptr->parameter_bad_val,
				       1.0 / wwptr->parameter_scale,
				       wwptr->parameter_bias);
  
  wwptr->color_table.setMissingValueRange(unscaled_missing,
					  unscaled_missing,
					  wwptr->missing_data_color_num);
}
