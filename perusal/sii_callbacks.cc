/* 	$Id$	 */

#include <string>

#include <gdk/gdkkeysyms.h>

#include <ClickQueue.hh>
#include <SeBoundaryList.hh>
#include <sii_enums.h>
#include "soloii.hh"
#include "sii_callbacks.hh"
#include "sii_config_stuff.hh"
#include "solo2.hh"
#include "sii_utils.hh"
#include "sii_frame_menu.hh"
#include "sii_param_widgets.hh"
#include <SoloState.hh>

/* c---------------------------------------------------------------------- */

void sii_blow_up_expose_event(GtkWidget *frame, GdkEvent *event,
			      gpointer data )
{
  guint frame_num =
    GPOINTER_TO_UINT(gtk_object_get_data (GTK_OBJECT(frame), "frame_num" ));

  SiiFrameConfig *sfc0 = frame_configs[0];
  ++sfc0->big_expose_count;

  GdkEventExpose *expose = (GdkEventExpose *)event;
  
  GtkAllocation alloc = sfc0->blow_up_frame->allocation;
  GdkRectangle area = expose->area;

  gboolean reconfigured = sfc0->big_reconfig_count > 0;

  gboolean uncovered = alloc.width != expose->area.width ||
    alloc.height != expose->area.height;

  gboolean totally_exposed = alloc.width == expose->area.width &&
    alloc.height == expose->area.height;

  gchar str[256];
  
  *str = '\0';
  if (reconfigured)
    strcat (str, "reconfig ");

  if (uncovered)
    strcat (str, "uncovered ");

  if (totally_exposed)
    strcat (str, "totally ");

  if (!(reconfigured || uncovered || totally_exposed))
    strcat (str, "neither ");

  gchar mess[256];
  sprintf(mess,
	  "bup expose: %s frm:%d gtk:%dx%d sii:%dx%d exp:%dx:%d rc:%d xp:%d",
	  str,
	  frame_num,
	  alloc.width,
	  alloc.height,
	  sfc0->big_width, sfc0->big_height,
	  expose->area.width, expose->area.height,
	  sfc0->big_reconfig_count,
	  sfc0->big_expose_count);

  sii_append_debug_stuff(mess);

  sfc0->big_reconfig_count = 0;

  sii_really_xfer_images(frame_num, &area, TRUE);
}

/* c---------------------------------------------------------------------- */

void sii_frame_expose_event(GtkWidget *frame, GdkEvent *event
			      , gpointer data )
{
				/* c...expose */
   guint frame_num = GPOINTER_TO_UINT (data); /* frame number */
   SiiFrameConfig *sfc;
   guint mm, nn, ndx;
   gint mark;
   sii_table_parameters *stp;
   GdkEventExpose *expose = (GdkEventExpose *)event;
   GtkAllocation alloc;
   gboolean reconfigured, uncovered, totally_exposed, second_expose;
   GdkRectangle area;
   static GString *gs=NULL;


   if(!gs)
     { gs = g_string_new (""); }
   g_string_truncate (gs, 0);
   g_string_append (gs, "XPZ");

   sfc = frame_configs[frame_num];
   ++sfc->expose_count;
   alloc = sfc->frame->allocation;
   area = expose->area;

   reconfigured = sfc->colorize_count == 0;
   if (reconfigured)
     { g_string_append (gs, " REC"); }

   uncovered = alloc.width != expose->area.width ||
     alloc.height != expose->area.height;
   if (uncovered)
     { g_string_append (gs, " UNC"); }

   totally_exposed = alloc.width == expose->area.width &&
     alloc.height == expose->area.height;
   if (totally_exposed)
     { g_string_append (gs, " TOT"); }

   sfc->most_recent_expose = expose->area;

   g_string_sprintfa
     (gs, " frm:%d frm:%dx%d  gtk:%dx%d  ec:%d cc:%d nf:%d lc:%d"
      , frame_num
      , sfc->width, sfc->height
      , alloc.width
      , alloc.height
      , sfc->expose_count
      , sfc->reconfig_count
      , sfc->new_frame_count
      , sfc->local_reconfig
      );
   second_expose = sfc->config_sync_num == sfc->expose_sync_num;
  sfc->expose_sync_num = sfc->config_sync_num; 

   if (reconfigured) {
     mark = 0;
   }
   else if (uncovered) {
     sii_xfer_images (frame_num, &area);
   }
   else if (!sfc->reconfig_count && totally_exposed) {
      sii_xfer_images (frame_num, &area);
   }

   /* the reconfig_count is reset once it's been plotted by sii_displayq() */

   while (sfc->reconfig_count) {

      if (!sfc->local_reconfig && (sfc->reconfig_flag & FrameDragResize)) {
	 /*
	  * This requires no reconfiguration other than increasing
	  * the image size if the frame is bigger
	  * 
	  */
	 if (!second_expose) {
	    ndx = sfc->cfg_que_ndx;
	    stp = &sfc->tbl_parms;
	    mm = stp->right_attach - stp->left_attach;
	    nn = stp->bottom_attach - stp->top_attach;
	    sfc->width = sfc->cfg_width[ndx];
	    sfc->height = sfc->cfg_height[ndx];
	    sii_check_image_size(frame_num);
	    if (mm == 1) {
	       sii_table_widget_width = sfc->width;
	    }
	    if (nn == 1) {
	       sii_table_widget_height = sfc->height;
	    }
	    sii_check_def_widget_sizes();

	    sfc->sync_num = sfc->config_sync_num;
	    g_string_append (gs, " X0");
	    sii_plot_data2(frame_num, REPLOT_THIS_FRAME);
	 }
	 else
	   { g_string_append (gs, " X1"); }
	 break;
      }
     
     if (sfc->local_reconfig && sfc->expose_count == sfc->reconfig_count)
       {
	 /* reconfig_count == 1 for first time through
	  * other local reconfigs emit two exposes
	  * one for the new drawable and one for the config
	  */
	 sfc->width = alloc.width;
	 sfc->height = alloc.height;
	 sfc->data_width = alloc.width;
	 sfc->data_height = alloc.height;
	 sfc->local_reconfig = FALSE;
	 g_string_append (gs, " X2");
	 sfc->sync_num = sfc->config_sync_num;
	 sii_plot_data (frame_num, REPLOT_THIS_FRAME);
	 break;
       }

     if (sfc->expose_count == 1)
       {
	 /* We've rebuilt the tables and this is the last expose,
	  * now plot the data
	  */
	 g_string_append (gs, " X3");
	 sfc->sync_num = sfc->config_sync_num;
	 sii_plot_data(frame_num, REPLOT_THIS_FRAME);
	 break;
       }

     g_string_append (gs, " X4");

     break;
   }
   sii_append_debug_stuff (gs->str);

}

/* c---------------------------------------------------------------------- */

void sii_blow_up_config_event(GtkWidget *frame, GdkEvent *event,
			      gpointer data )
{
  guint frame_num =
    GPOINTER_TO_UINT(gtk_object_get_data (GTK_OBJECT(frame), "frame_num" ));

  ++frame_configs[0]->big_reconfig_count;
  frame_configs[0]->big_expose_count = 0;
  frame_configs[0]->big_colorize_count = 0;

  gchar mess[256];
  
  sprintf(mess, "bup configure frm %d %dx%d",
	  frame_num,
	  frame->allocation.width, frame->allocation.height);

  sii_append_debug_stuff(mess);
}

/* c---------------------------------------------------------------------- */

void sii_frame_config_event(GtkWidget *frame, GdkEvent *event
			      , gpointer data )
{
				/* c...config */
  guint frame_num = GPOINTER_TO_UINT (data), ndx;
  GdkEventConfigure *ce;
  gchar mess[256];
  SiiFrameConfig *sfc;
  GdkEvent *next_event;

  next_event = gdk_event_peek();
  if (next_event) {
     printf ("Next event type:%d\n", next_event->type);
  }
  strcpy (mess, " ");
  ce = (GdkEventConfigure *)event;
  sfc = frame_configs[frame_num];
  sfc->config_sync_num = sii_inc_master_seq_num ();
  ++sfc->reconfig_count;
  sfc->colorize_count = 0;

  ndx = (sfc->cfg_que_ndx +1) % CFG_QUE_SIZE;
  sfc->cfg_que_ndx = ndx;
  sfc->cfg_width[ndx] = frame->allocation.width;
  sfc->cfg_height[ndx] = frame->allocation.height;
  if (!sfc->local_reconfig) {
     sfc->reconfig_flag |= FrameDragResize;
     ++sfc->drag_resize_count;
  }

  sprintf (mess, "CFG frm:%d %dx%d gtk:%dx%d  ec:%d cc:%d nf:%d et:%d se:%d"
	     , frame_num
	     , sfc->width
	     , sfc->height
	     , frame->allocation.width
	     , frame->allocation.height
	     , sfc->expose_count
	     , sfc->reconfig_count
	     , sfc->new_frame_count
	     , event->type
	     , ce->send_event
	     );
  sii_append_debug_stuff (mess);
  sfc->expose_count = 0;
}

/* c---------------------------------------------------------------------- */

void sii_enter_event(GtkWidget *frame, GdkEvent *event
			      , gpointer data )
{
  gint xx, yy;
  GdkModifierType state;
  guint frame_num = GPOINTER_TO_UINT (data);
  gdk_window_get_pointer( frame->window, &xx, &yy, &state );
  cursor_frame = frame_num +1;
}

/* c---------------------------------------------------------------------- */

void sii_leave_event(GtkWidget *frame, GdkEvent *event
			      , gpointer data )
{
  cursor_frame = 0;
}

/* c---------------------------------------------------------------------- */

void sii_blow_up_mouse_delete_event(GtkWidget *frame, GdkEvent *event
			      , gpointer data )
{
  guint frame_num = GPOINTER_TO_UINT (data);
  SiiFrameConfig *sfc = frame_configs[frame_num];
  SiiFrameConfig *sfc0 = frame_configs[0];

  sfc->blow_up = false;

  if(sfc0!=NULL && GTK_IS_WIDGET(sfc0->blow_up_window))
	gtk_widget_destroy(sfc0->blow_up_window);
}

/* c---------------------------------------------------------------------- */

void sii_blow_up_mouse_button_event(GtkWidget *frame, GdkEvent *event,
				    gpointer data)
{
  guint frame_num =
    GPOINTER_TO_UINT(gtk_object_get_data(GTK_OBJECT(frame), "frame_num"));
 
  // Relative to the origin of this window

  gint xx;
  gint yy;
  GdkModifierType state;
  
  gchar mess[256];
  
  gdk_window_get_pointer(frame->window, &xx, &yy, &state);
  sprintf(mess, "xx:%d yy:%d state:%d", xx, yy, state);
  sii_append_debug_stuff(mess);

  xx /= 2;
  yy /= 2;
  SiiFrameConfig *sfc = frame_configs[frame_num];
  SiiFrameConfig *sfc0 = frame_configs[0];

  sfc->click_loc.x = xx;
  sfc->click_loc.y = yy;
  guint width = sfc0->data_width;
  guint height = sfc0->data_height;

  gboolean double_click = event->type == GDK_2BUTTON_PRESS;

  // Set offsets from the center of the frame

  gdouble dx = xx - 0.5 * width;
  gdouble dy = (height - 1 - yy) - 0.5 * height;

  gchar str[16];
  SiiPoint pt;
  
  *str = '\0';
  memset(&pt, 0, sizeof(pt));
  pt.button_mask = 0;

  if (double_click || state & GDK_BUTTON1_MASK || state & GDK_BUTTON2_MASK)
  {
    if (double_click || state & GDK_BUTTON2_MASK)
      pt.button_mask |= GDK_BUTTON2_MASK;
    else 
      pt.button_mask |= GDK_BUTTON1_MASK;

    // Send to editor which should ignore it if not shown or the suspend
    // boundary definition flag is set ("No BND" button)
    // void sii_editor_data_click (SiiPoint *point);

    strcat(str, (state & GDK_BUTTON1_MASK) ? "B1 ": "B2 ");

    pt.frame_num = frame_num;
    pt.x = xx;
    pt.y = yy;
    pt.dtime = 0;
    pt.lat = 0;
    pt.lon = 0;
    pt.alt = 0;

    // Set the geographical coordinates of the click

    sii_set_geo_coords((int)frame_num, dx, dy, pt);

    // Add the click to the queue

    ClickQueue::getInstance()->insert(pt);
    
    // See if the user clicked to recenter plot.  Need to do this after
    // adding the click to the queue since we pull clicks from the queue to
    // see where to recenter.

    if (main_window->isElectricCentering() &&
	pt.button_mask & GDK_BUTTON2_MASK)
      sii_center_on_clicks(1);
  }

  if (state & (GDK_CONTROL_MASK | GDK_BUTTON3_MASK))
  {
    strcat(str, "B3 ");
    pt.button_mask |= GDK_BUTTON3_MASK;

    gint x;
    gint y;
    
    gdk_window_get_pointer(NULL, &x, &y, NULL);
    GdkPoint *ptp = &frame_configs[frame_num]->widget_origin[FRAME_MENU];
    ptp->x = x;
    ptp->y = y;
    show_frame_menu(frame, data);
  }

}

/* c---------------------------------------------------------------------- */

void sii_mouse_button_event(GtkWidget *frame, GdkEvent *event,
			    gpointer data )
{
  guint frame_num = GPOINTER_TO_UINT(data);
  
  // Relative to the origin of this window

  gint xx;
  gint yy;
  GdkModifierType state;
  
  gdk_window_get_pointer(frame->window, &xx, &yy, &state);

  gdouble d_xx = frame_configs[frame_num]->ulc_radar.xx;
  gdouble d_yy = frame_configs[frame_num]->ulc_radar.yy;
  frame_configs[frame_num]->click_loc.x = xx;
  frame_configs[frame_num]->click_loc.y = yy;
  guint width = frame_configs[frame_num]->data_width;
  guint height = frame_configs[frame_num]->data_height;

  // Set offsets from the center of the frame

  gdouble dx = xx -.5 * width;
  gdouble dy = (height -1 -yy) -.5 * height;
  g_string_sprintf(gs_complaints, 
		   "click:%dx%d(%d,%d)(%.4f,%.4f)ulcR:(%.4f,%.4f)",
		   width, height, xx, yy, dx, dy, d_xx, d_yy);

  gboolean double_click = event->type == GDK_2BUTTON_PRESS;

  SiiPoint pt;
  memset(&pt, 0, sizeof(pt));

  if (double_click || state & GDK_BUTTON1_MASK || state & GDK_BUTTON2_MASK)
  {
    if (double_click || state & GDK_BUTTON2_MASK)
      pt.button_mask |= GDK_BUTTON2_MASK;
    else 
      pt.button_mask |= GDK_BUTTON1_MASK;

    pt.frame_num = frame_num;
    pt.x = xx;
    pt.y = yy;

    // Set the geographical coordinates of the click

    sii_set_geo_coords((int)frame_num, dx, dy, pt);

    // Add the click to the queue

    ClickQueue::getInstance()->insert(pt);

    // See if the user clicked to recenter plot.  Need to do this after
    // adding the click to the queue since we pull clicks from the queue to
    // see where to recenter.

    if (main_window->isElectricCentering() &&
	pt.button_mask & GDK_BUTTON2_MASK)
      sii_center_on_clicks(1);
  }

  if (state & (GDK_CONTROL_MASK | GDK_BUTTON3_MASK))
  {
    pt.button_mask |= GDK_BUTTON3_MASK;

    gint x;
    gint y;
    gdk_window_get_pointer(NULL, &x, &y, NULL);
    GdkPoint *ptp = &frame_configs[frame_num]->widget_origin[FRAME_MENU];
    ptp->x = x;
    ptp->y = y;
    show_frame_menu(frame, data);
  }
 
}

/* c---------------------------------------------------------------------- */

gint sii_blow_up_keyboard_event(GtkWidget *frame, GdkEvent *event
			      , gpointer data )
{
  GdkEventKey *kbev = (GdkEventKey *)event;
  guint frame_num = GPOINTER_TO_UINT (data);
  guint F_frame_id = 0;
  gboolean modifier_only = FALSE;
  gboolean modified = FALSE;

  frame_num = GPOINTER_TO_UINT
    (gtk_object_get_data (GTK_OBJECT(frame), "frame_num" ));

  modified = (kbev->state & GDK_CONTROL_MASK || kbev->state & GDK_SHIFT_MASK);

  switch( kbev->keyval ) {

  case GDK_T:
  case GDK_t:
    frame_configs[frame_num]->param_window->toggleField();
    break;

  case GDK_D:
  case GDK_d:
    SeBoundaryList::getInstance()->zapLastPoint();
    break;

  case GDK_Control_L:
  case GDK_Control_R:
  case GDK_Shift_L:
  case GDK_Shift_R:
    /* Lone modifier key. Ignore */
    modifier_only = TRUE;
    break;
    
  case GDK_F1:
  case GDK_F2:
  case GDK_F3:
  case GDK_F4:
  case GDK_F5:
  case GDK_F6:
  case GDK_F7:
  case GDK_F8:
  case GDK_F9:
  case GDK_F10:
  case GDK_F11:
  case GDK_F12:
    /* Function buttons
  case GDK_L1:
  case GDK_L2:
     */
    if( kbev->keyval == GDK_L1 )
      { F_frame_id = 11; }
    else if( kbev->keyval == GDK_L2 )
      { F_frame_id = 12; }
    else
      { F_frame_id = kbev->keyval - GDK_F1 +1; }
    break;

  case 268828432:		/* SunF36 when you punch F11 on Sun Solaris */
    g_message ("bup Caught funky SunF36 for F11");
    F_frame_id = 11; 
    break;

  case 268828433:		/* SunF37 when you punch F12 on Sun Solaris */
    g_message ("bup Caught funky SunF37 for F12");
    F_frame_id = 12; 
    break;
  };

  if (F_frame_id > 0)
    { sii_blow_up(F_frame_id-1); }

  return 0;
}

/* c---------------------------------------------------------------------- */

// NOTE: This method is called for the following events:
//        object            event
//        ------            -----
//        main_window       key_press_event
//        main_vbox         key_press_event

gint sii_frame_keyboard_event(GtkWidget *frame, GdkEvent *event
			      , gpointer data )
{
 /* Middle mouse button centers the lockstep on the clicked point
  * There will also be a option to set the limits of the plot
  * (center and magnification) by using the last N points clicked.
  *
  */
   gchar mess[256];

  std::string mod = "nomod";
  GdkEventKey *kbev = (GdkEventKey *)event;
  guint frame_num = GPOINTER_TO_UINT (data);
  guint F_frame_id = 0;
  gboolean modifier_only = FALSE;
  gboolean modified = FALSE;


  modified = (kbev->state & GDK_CONTROL_MASK || kbev->state & GDK_SHIFT_MASK);

  if( kbev->state & GDK_CONTROL_MASK )
    { mod = "Control"; }
  if( kbev->state & GDK_SHIFT_MASK )
    { mod = "Shift"; }

  switch( kbev->keyval ) {

  case GDK_T:
  case GDK_t:
    frame_configs[cursor_frame-1]->param_window->toggleField();
    break;

  case GDK_D:
  case GDK_d:
    SeBoundaryList::getInstance()->zapLastPoint();
    break;

  case GDK_J:
  case GDK_j:
    sii_dump_debug_stuff();
    break;

  case GDK_Control_L:
  case GDK_Control_R:
  case GDK_Shift_L:
  case GDK_Shift_R:
    /* Lone modifier key. Ignore */
    modifier_only = TRUE;
    break;
    
  case GDK_F1:
  case GDK_F2:
  case GDK_F3:
  case GDK_F4:
  case GDK_F5:
  case GDK_F6:
  case GDK_F7:
  case GDK_F8:
  case GDK_F9:
  case GDK_F10:
  case GDK_F11:
  case GDK_F12:
    if( kbev->keyval == GDK_L1 )
      { F_frame_id = 11; }
    else if( kbev->keyval == GDK_L2 )
      { F_frame_id = 12; }
    else
      { F_frame_id = kbev->keyval - GDK_F1 +1; }
    break;

  case 268828432:		/* SunF36 when you punch F11 on Sun Solaris */
    g_message ("Caught funky SunF36 for F11");
    F_frame_id = 11; 
    break;

  case 268828433:		/* SunF37 when you punch F12 on Sun Solaris */
    g_message ("Caught funky SunF37 for F12");
    F_frame_id = 12; 
    break;

  case GDK_Escape:		/* Stop display or editing  */
    g_message( "Caught Escape " );
    SoloState::getInstance()->setHaltFlag(true);
    break;

  case GDK_Delete:		/* delete the last boundary point */
      break;

  case GDK_Right:
    /* if the cursor is currently residing in a frame, then
     * plot the next sweep for the set of frames (lockstep)
     * of which this frame is a member
     */
    if( kbev->state & ( GDK_SHIFT_MASK | GDK_CONTROL_MASK )) {
      /* fast forward mode (escape interrupts this mode) */
      g_message( "Caught Control/Shift Right " );

      if (cursor_frame) {
	sii_plot_data(cursor_frame-1, FORWARD_FOREVER);
      }
    }
    else {
      if (cursor_frame) {
	sii_plot_data(cursor_frame-1, FORWARD_ONCE);
      }
    }
    break;
    
  case GDK_Left:
    /* Same as Right only the other direction */
    if( kbev->state & ( GDK_SHIFT_MASK | GDK_CONTROL_MASK )) {
      g_message( "Caught Control/Shift Left " );

      if (cursor_frame) {
	sii_plot_data(cursor_frame-1, BACKWARDS_FOREVER);
      }
    }
    else {
      if (cursor_frame) {
	sii_plot_data(cursor_frame-1, BACKWARDS_ONCE);
      }
    }
    break;

  case GDK_Up:
    /* Next sweep at same fixed angle */
      break;

  case GDK_Down:
    /* Previous sweep at same fixed angle */
    break;

    
  case GDK_Return:
      g_message( "Caught Return " );
      if (cursor_frame) {
	sii_plot_data (cursor_frame-1, REPLOT_LOCK_STEP);
      }
      else
	{ sii_plot_data (cursor_frame-1, REPLOT_ALL); }
    break;


  default:
    break;
  };


  if (F_frame_id > 0)		/* Blowup or pop down a big image */
    { sii_blow_up(F_frame_id-1); }


  std::string aa = "unk";

  if (gdk_keyval_name( kbev->keyval ) != 0)
    aa = gdk_keyval_name( kbev->keyval );

  sprintf (mess,"data %d frame: %d keyed k:%d s:%d n:%s aa:%s mod:%s"
	   , frame_num, cursor_frame
	   , kbev->keyval, kbev->state, kbev->string, aa.c_str(), mod.c_str());
  sii_append_debug_stuff (mess);

  return (frame_num > 2 * MAX_FRAMES) ? FALSE : TRUE;
}

/* c---------------------------------------------------------------------- */

void sii_focus_in_event(GtkWidget *frame, GdkEvent *event
			      , gpointer data )
{
  gint xx, yy;
  GdkModifierType state;
  gdk_window_get_pointer( frame->window, &xx, &yy, &state );
}

/* c---------------------------------------------------------------------- */

void sii_focus_out_event(GtkWidget *frame, GdkEvent *event
			      , gpointer data )
{
}
