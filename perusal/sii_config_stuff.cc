/* 	$Id$	 */

#include <string>

#include "soloii.h"
#include "soloii.hh"
#include "sii_config_stuff.hh"
#include "sii_callbacks.hh"
#include "solo2.hh"
#include "sii_param_widgets.hh"
#include "sii_utils.hh"

static const std::string def_param_names[] =
{ "DZ", "VE", "NCP", "SW", "DM", "ZDR", "LDR", "PHIDP", "RHOHV", "DX",
    "KDP", "PD", "DCZ", "DZ", "VE", "NCP", "SW", "", "", "", "", };

/* c---------------------------------------------------------------------- */

void 
sii_check_for_illegal_shrink (void)
{
  SiiFrameConfig *sfc = frame_configs[0];
  sii_table_parameters *stp;
  gint jj, mm;
  gdouble d;

  for (jj=0; jj < sii_return_frame_count(); jj++) {
    if (frame_configs[jj]->reconfig_count)
      { return; }
    /* don't do anything in the middle of a reconfig */
  }
  stp = &sfc->tbl_parms;
  mm = stp->right_attach - stp->left_attach;
  d = (gdouble)sfc->width/mm;
  if (d < sii_table_widget_width)
    { sii_new_frames (); }
}

/* c---------------------------------------------------------------------- */

void 
sii_check_def_widget_sizes (void)
{
  int nn;

  if (sii_table_widget_width < DEFAULT_WIDTH)
    { sii_table_widget_width = DEFAULT_WIDTH; }

  if (sii_table_widget_height < DEFAULT_HEIGHT)
    { sii_table_widget_height = DEFAULT_HEIGHT; }

  nn = (sii_table_widget_width -1)/4 +1;
  sii_table_widget_width = nn * 4;

  nn = (sii_table_widget_height -1)/4 +1;
  sii_table_widget_height = nn * 4;
}

/* c---------------------------------------------------------------------- */

void sii_init_frame_configs()
{
  std::vector< std::string > param_names;
  
  for (int fn = 0; fn < MAX_FRAMES; fn++)
  {
    frame_configs[fn] = new SiiFrameConfig;

    frame_configs[fn]->frame_num = fn;
    frame_configs[fn]->blow_up_frame_num = 0;

    frame_configs[fn]->click_loc.frame_num = 0;
    frame_configs[fn]->click_loc.button_mask = 0;
    frame_configs[fn]->click_loc.x = 0;
    frame_configs[fn]->click_loc.y = 0;
    frame_configs[fn]->click_loc.xx = 0.0;
    frame_configs[fn]->click_loc.yy = 0.0;
    frame_configs[fn]->click_loc.zz = 0.0;
    frame_configs[fn]->click_loc.dtime = 0.0;
    frame_configs[fn]->click_loc.lat = 0.0;
    frame_configs[fn]->click_loc.lon = 0.0;
    frame_configs[fn]->click_loc.alt = 0.0;
    frame_configs[fn]->click_loc.rng_km = 0.0;

    frame_configs[fn]->ncols = 0;
    frame_configs[fn]->nrows = 0;
    frame_configs[fn]->blow_up = false;
    frame_configs[fn]->changed = false;
    frame_configs[fn]->local_reconfig = false;
    frame_configs[fn]->width = 0;
    frame_configs[fn]->height = 0;
    frame_configs[fn]->data_width = 0;
    frame_configs[fn]->data_height = 0;
    frame_configs[fn]->lock_step = 0;
    frame_configs[fn]->sync_num = 0;
    frame_configs[fn]->reconfig_count = 0;
    frame_configs[fn]->expose_count = 0;
    frame_configs[fn]->prev_expose_num = 0;
    frame_configs[fn]->new_frame_count = 0;
    frame_configs[fn]->colorize_count = 0;
    frame_configs[fn]->max_lbl_wdt = 0;
    frame_configs[fn]->big_max_lbl_wdt = 0;
    frame_configs[fn]->points_in_bnd = 0;
    frame_configs[fn]->big_reconfig_count = 0;
    frame_configs[fn]->big_expose_count = 0;
    frame_configs[fn]->big_colorize_count = 0;

    frame_configs[fn]->tbl_parms.left_attach = 0;
    frame_configs[fn]->tbl_parms.right_attach = 0;
    frame_configs[fn]->tbl_parms.top_attach = 0;
    frame_configs[fn]->tbl_parms.bottom_attach = 0;

    frame_configs[fn]->font = 0;
    frame_configs[fn]->big_font = 0;
    frame_configs[fn]->font_height = 0;
    frame_configs[fn]->big_font_height = 0;
    frame_configs[fn]->gc = 0;
    frame_configs[fn]->big_gc = 0;
    frame_configs[fn]->border_gc = 0;
    frame_configs[fn]->color_map = 0;
    frame_configs[fn]->accel_group = gtk_accel_group_new();
    for (int i = 0; i < 128; ++i)
      frame_configs[fn]->frame_title[i] = 0;
    frame_configs[fn]->frame = 0;
    frame_configs[fn]->blow_up_window = 0;
    frame_configs[fn]->blow_up_frame = 0;
    for (int i = 0; i < MAX_TOP_WINDOWS; ++i)
      frame_configs[fn]->toplevel_windows[i] = 0;

    frame_configs[fn]->param_data = 0;
    frame_configs[fn]->view_data = 0;
    for (int i = 0; i < MAX_LINK_SETS; ++i)
      frame_configs[fn]->link_set[i] = 0;
    for (int i = 0; i < 4; ++i)
    {
      frame_configs[fn]->corner[i].frame_num = 0;
      frame_configs[fn]->corner[i].button_mask = 0;
      frame_configs[fn]->corner[i].x = 0;
      frame_configs[fn]->corner[i].y = 0;
      frame_configs[fn]->corner[i].xx = 0.0;
      frame_configs[fn]->corner[i].yy = 0.0;
      frame_configs[fn]->corner[i].zz = 0.0;
      frame_configs[fn]->corner[i].dtime = 0.0;
      frame_configs[fn]->corner[i].lat = 0.0;
      frame_configs[fn]->corner[i].lon = 0.0;
      frame_configs[fn]->corner[i].alt = 0.0;
      frame_configs[fn]->corner[i].rng_km = 0.0;
    }

    frame_configs[fn]->center.frame_num = 0;
    frame_configs[fn]->center.button_mask = 0;
    frame_configs[fn]->center.x = 0;
    frame_configs[fn]->center.y = 0;
    frame_configs[fn]->center.xx = 0.0;
    frame_configs[fn]->center.yy = 0.0;
    frame_configs[fn]->center.zz = 0.0;
    frame_configs[fn]->center.dtime = 0.0;
    frame_configs[fn]->center.lat = 0.0;
    frame_configs[fn]->center.lon = 0.0;
    frame_configs[fn]->center.alt = 0.0;
    frame_configs[fn]->center.rng_km = 0.0;

    frame_configs[fn]->radar.frame_num = 0;
    frame_configs[fn]->radar.button_mask = 0;
    frame_configs[fn]->radar.x = 0;
    frame_configs[fn]->radar.y = 0;
    frame_configs[fn]->radar.xx = 0.0;
    frame_configs[fn]->radar.yy = 0.0;
    frame_configs[fn]->radar.zz = 0.0;
    frame_configs[fn]->radar.dtime = 0.0;
    frame_configs[fn]->radar.lat = 0.0;
    frame_configs[fn]->radar.lon = 0.0;
    frame_configs[fn]->radar.alt = 0.0;
    frame_configs[fn]->radar.rng_km = 0.0;

    frame_configs[fn]->ulc_radar.frame_num = 0;
    frame_configs[fn]->ulc_radar.button_mask = 0;
    frame_configs[fn]->ulc_radar.x = 0;
    frame_configs[fn]->ulc_radar.y = 0;
    frame_configs[fn]->ulc_radar.xx = 0.0;
    frame_configs[fn]->ulc_radar.yy = 0.0;
    frame_configs[fn]->ulc_radar.zz = 0.0;
    frame_configs[fn]->ulc_radar.dtime = 0.0;
    frame_configs[fn]->ulc_radar.lat = 0.0;
    frame_configs[fn]->ulc_radar.lon = 0.0;
    frame_configs[fn]->ulc_radar.alt = 0.0;
    frame_configs[fn]->ulc_radar.rng_km = 0.0;

    frame_configs[fn]->landmark.frame_num = 0;
    frame_configs[fn]->landmark.button_mask = 0;
    frame_configs[fn]->landmark.x = 0;
    frame_configs[fn]->landmark.y = 0;
    frame_configs[fn]->landmark.xx = 0.0;
    frame_configs[fn]->landmark.yy = 0.0;
    frame_configs[fn]->landmark.zz = 0.0;
    frame_configs[fn]->landmark.dtime = 0.0;
    frame_configs[fn]->landmark.lat = 0.0;
    frame_configs[fn]->landmark.lon = 0.0;
    frame_configs[fn]->landmark.alt = 0.0;
    frame_configs[fn]->landmark.rng_km = 0.0;

    frame_configs[fn]->ulc_bnd.frame_num = 0;
    frame_configs[fn]->ulc_bnd.button_mask = 0;
    frame_configs[fn]->ulc_bnd.x = 0;
    frame_configs[fn]->ulc_bnd.y = 0;
    frame_configs[fn]->ulc_bnd.xx = 0.0;
    frame_configs[fn]->ulc_bnd.yy = 0.0;
    frame_configs[fn]->ulc_bnd.zz = 0.0;
    frame_configs[fn]->ulc_bnd.dtime = 0.0;
    frame_configs[fn]->ulc_bnd.lat = 0.0;
    frame_configs[fn]->ulc_bnd.lon = 0.0;
    frame_configs[fn]->ulc_bnd.alt = 0.0;
    frame_configs[fn]->ulc_bnd.rng_km = 0.0;

    frame_configs[fn]->lrc_bnd.frame_num = 0;
    frame_configs[fn]->lrc_bnd.button_mask = 0;
    frame_configs[fn]->lrc_bnd.x = 0;
    frame_configs[fn]->lrc_bnd.y = 0;
    frame_configs[fn]->lrc_bnd.xx = 0.0;
    frame_configs[fn]->lrc_bnd.yy = 0.0;
    frame_configs[fn]->lrc_bnd.zz = 0.0;
    frame_configs[fn]->lrc_bnd.dtime = 0.0;
    frame_configs[fn]->lrc_bnd.lat = 0.0;
    frame_configs[fn]->lrc_bnd.lon = 0.0;
    frame_configs[fn]->lrc_bnd.alt = 0.0;
    frame_configs[fn]->lrc_bnd.rng_km = 0.0;

    frame_configs[fn]->cb_pattern_len = 0;
    frame_configs[fn]->cb_pattern = 0;
    frame_configs[fn]->image = 0;
    frame_configs[fn]->image_size = 0;
    frame_configs[fn]->image_stride = 0;
    frame_configs[fn]->max_image_size = 0;
    frame_configs[fn]->big_sync_num = 0;
    frame_configs[fn]->big_width = 0;
    frame_configs[fn]->big_height = 0;
    frame_configs[fn]->big_mag_multiple = 0;
    frame_configs[fn]->big_image = 0;
    frame_configs[fn]->big_image_size = 0;
    frame_configs[fn]->big_image_stride = 0;
    frame_configs[fn]->max_big_image_size = 0;
    frame_configs[fn]->big_image_show_count = 0;
    frame_configs[fn]->cfg_que_ndx = 0;
    for (int i = 0; i < CFG_QUE_SIZE; ++i)
    {
      frame_configs[fn]->cfg_width[i] = 0;
      frame_configs[fn]->cfg_height[i] = 0;
    }
    frame_configs[fn]->config_sync_num = 0;
    frame_configs[fn]->drag_resize_count = 0;
    frame_configs[fn]->expose_sync_num = 0;
    frame_configs[fn]->reconfig_flag = 0;
    frame_configs[fn]->x_drag_ref = 0;
    frame_configs[fn]->y_drag_ref = 0;

    // NOTE: I would like to allocate all of the windows here, but there are
    // built in dependencies that I have to figure out and clean up before that
    // is possible.

    // These need to go at the end because they depend on some of the above
    // initializations.

    frame_configs[fn]->sweepfile_window = 0;
    frame_configs[fn]->view_window = new ViewWindow(default_font, fn);
    frame_configs[fn]->param_window = 0;
    frame_configs[fn]->edit_window = 0;
    frame_configs[fn]->examine_window = 0;
    frame_configs[fn]->data_window = new DataWindow(default_font, fn);

    sii_initialize_parameter(fn, def_param_names[fn]);
    
    param_names.push_back(def_param_names[fn]);
  }

  for (int fn = 0; fn < MAX_FRAMES; fn++)
    sii_set_param_names_list(fn, param_names);
}

/* c---------------------------------------------------------------------- */

void 
sii_reset_config_cells (void)
{
  int cn;

  if( !config_cells ) {		/* allocate them */

    config_cells = (sii_config_cell **)
      g_malloc0( MAX_CONFIG_CELLS * sizeof( sii_config_cell *));

    for( cn = 0; cn < MAX_CONFIG_CELLS; cn++ ) {
      config_cells[cn] = (sii_config_cell *)
	g_malloc0( sizeof( sii_config_cell ));
      config_cells[cn]->cell_num = cn;
      config_cells[cn]->row = cn/MAX_CONFIG_ROWS;
      config_cells[cn]->col = cn % MAX_CONFIG_COLS;
    }
  }

  for( cn = 0; cn < MAX_CONFIG_CELLS; cn++ ) {
    config_cells[cn]->frame_num = 0;
    config_cells[cn]->processed = no;
  }
}

/* c---------------------------------------------------------------------- */

void 
sii_new_frames (void)
{
   int mm, nn, width, height;
   SiiFrameConfig *sfc, *sfc0 = frame_configs[0];
   sii_table_parameters *stp;
   GtkWidget *frame;
   gchar str[256];
   GdkColormap *cmap = gdk_rgb_get_cmap ();

   gint event_flags = GDK_BUTTON_PRESS_MASK | GDK_KEY_PRESS_MASK
     | GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK
     | GDK_FOCUS_CHANGE_MASK;

   main_window->resetTable(sfc0->nrows, sfc0->ncols);

  for (guint fn = 0; fn < sii_frame_count; fn++ ) {

    sfc = frame_configs[fn];
    sfc->color_map = cmap;
    sfc->local_reconfig = TRUE;
    sfc->drag_resize_count = 0;
    ++sfc->new_frame_count;

    sfc->frame = gtk_drawing_area_new ();
    frame = sfc->frame;
    gtk_widget_set_events (frame, event_flags );
    stp = &sfc->tbl_parms;
    mm = stp->right_attach - stp->left_attach;
    nn = stp->bottom_attach - stp->top_attach;

    width = mm * sii_table_widget_width;
    height = nn * sii_table_widget_height;
    sprintf (str, "new_frame %d  %dx%d", fn, width, height );
    sii_append_debug_stuff (str);

    sfc->width = sfc->data_width = width;
    sfc->height = sfc->data_height = height;
    sii_check_image_size (fn);

    gtk_drawing_area_size (GTK_DRAWING_AREA (frame)
			   , width
			   , height );

    /* --- Signals! --- */
    gtk_signal_connect (GTK_OBJECT (frame), "expose_event",
			(GtkSignalFunc) sii_frame_expose_event, GINT_TO_POINTER(fn));
    gtk_signal_connect (GTK_OBJECT(frame),"configure_event",
			(GtkSignalFunc) sii_frame_config_event, GINT_TO_POINTER(fn));

    gtk_signal_connect (GTK_OBJECT(frame),"button_press_event",
			(GtkSignalFunc) sii_mouse_button_event, GINT_TO_POINTER(fn));
    gtk_signal_connect (GTK_OBJECT(frame),"key_press_event",
			(GtkSignalFunc) sii_frame_keyboard_event, GINT_TO_POINTER(fn));
    
    gtk_signal_connect (GTK_OBJECT(frame),"enter_notify_event",
			(GtkSignalFunc) sii_enter_event, GINT_TO_POINTER(fn));
    gtk_signal_connect (GTK_OBJECT(frame),"leave_notify_event",
			(GtkSignalFunc) sii_leave_event, GINT_TO_POINTER(fn));

    gtk_signal_connect (GTK_OBJECT(frame),"focus_in_event",
			(GtkSignalFunc) sii_focus_in_event, GINT_TO_POINTER(fn));
    gtk_signal_connect (GTK_OBJECT(frame),"focus_out_event",
			(GtkSignalFunc) sii_focus_out_event, GINT_TO_POINTER(fn));
    
    main_window->addFrame((Gtk::DrawingArea *)Glib::wrap(frame),
			  stp->left_attach, stp->right_attach,
			  stp->top_attach, stp->bottom_attach);
    
  }

  // Resize the window to fit the frames.  Unfortunately, I can't seem to
  // figure out how to get the actual new window size, including the menu
  // bar.  So I'm adding 30 pixels for the menu bar, which seems to work on
  // my version.  This should be changed in the future to do the right thing.

  main_window->resize(sfc0->ncols * sii_table_widget_width,
		      (sfc0->nrows * sii_table_widget_height) + 30);
  
  main_window->show_all_children();
}

/* c---------------------------------------------------------------------- */

gboolean sii_set_config()
{
  int cll, cll2, jj, mm, nn, mark, frame, row, col;
  int row2, col2, ncols = 1, nrows = 1;
  int frame_count = 0;
  sii_config_cell * scc;
  sii_table_parameters tbl_parms[MAX_FRAMES], *stp;

				/* start in the upper left hand corner */
				/* moving accross the columns in each row */

  for( cll = 0; cll < MAX_CONFIG_CELLS; cll++ ) {
    scc = config_cells[cll];

    if( !scc->frame_num )
      { continue; }
    if(  scc->processed )
      { continue; }
				/* at next cell to process */
    stp = &tbl_parms[frame_count++];
    scc->processed = yes;
    frame = scc->frame_num;	/* 1-12 */
    row = row2 = cll/MAX_CONFIG_ROWS;
    col = col2 = cll % MAX_CONFIG_COLS;

    stp->left_attach = col;
    stp->right_attach = col +1;
    stp->top_attach = row;
    stp->bottom_attach = row +1;
    if( (int)stp->bottom_attach > nrows )
      { nrows = stp->bottom_attach; }
    if( (int)stp->right_attach > ncols )
      { ncols = stp->right_attach; }
    
				/* see if this number appears again */
    for( jj = cll+1; jj < MAX_CONFIG_CELLS; jj++ ) {
      if( (int)config_cells[jj]->frame_num != frame ||
	  config_cells[jj]->processed )
	{ continue; }

      if( jj/MAX_CONFIG_ROWS > row2 )
	{ row2 = jj/MAX_CONFIG_ROWS; }

      if( jj % MAX_CONFIG_COLS > col2 )
	{ col2 = jj % MAX_CONFIG_COLS; }
    }

    if( row2 > row || col2 > col ) {

      for( ; row <= row2; row++ ) {
	if( row +1 > (int)stp->bottom_attach ) {
	   stp->bottom_attach = row+1;
	   if( (int)stp->bottom_attach > nrows )
	     { nrows = stp->bottom_attach; }
	}

	for( jj = col; jj <= col2; jj++ ) {
	  cll2 = row * MAX_CONFIG_ROWS + jj;
	  config_cells[cll2]->processed = yes;

	  if( jj +1 > (int)stp->right_attach ) {
	     stp->right_attach = jj+1;
	     if( (int)stp->right_attach > ncols )
	       { ncols = stp->right_attach; }
	  }
	}
      }
      mark = 0;
    } /* row2 > row || col2 > col */
  } /* for( cll = 0; cll < MAX_CONFIG_CELLS; */

  if( frame_count > 0 ) {
     for( jj = 0; jj < frame_count; jj++ ) {

	mm = tbl_parms[jj].right_attach - tbl_parms[jj].left_attach;
	nn = tbl_parms[jj].bottom_attach - tbl_parms[jj].top_attach;

	frame_configs[jj]->tbl_parms = tbl_parms[jj];
	frame_configs[jj]->ncols = ncols;
	frame_configs[jj]->nrows = nrows;
	frame_configs[jj]->local_reconfig = TRUE;
	frame_configs[jj]->reconfig_count = 0;

	/* Fix this! */
	frame_configs[jj]->lock_step = jj;
     }
     sii_frame_count = frame_count;
     return( yes );
  }

  return( no );
}
