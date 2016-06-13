/* 	$Id$	 */

#include <string>

#include <gdk/gdktypes.h>
#include <gtk/gtktext.h>
#include <cairo.h>
#include <gtk/gtk.h>

#include <ColorManager.hh>
#include <ExamineWindow.hh>
#include <Label.hh>
#include <se_utils.hh>
#include <sii_enums.h>
#include <sii_exam_widgets.hh>
#include <sii_frame_menu.hh>
#include <sii_param_widgets.hh>
#include <sii_utils.hh>
#include <solo2.hh>
#include <soloii.hh>
#include <sxm_examine.hh>

enum {
   FRAME_MENU_ZERO,
   FRAME_MENU_CANCEL,
   FRAME_MENU_REPLOT,
   FRAME_MENU_SWEEPFILES,
   FRAME_MENU_SWPFI_LIST,
   FRAME_MENU_PARAMETERS,
   FRAME_MENU_VIEW,
   FRAME_MENU_CENTERING,
   FRAME_MENU_LOCKSTEPS,
   FRAME_MENU_EDITOR,
   FRAME_MENU_EXAMINE,
   FRAME_MENU_CLICK_DATA,
   FRAME_MENU_LAST_ENUM,
};

/* c---------------------------------------------------------------------- */

void sii_set_click_data_text(guint frame_num, const std::string &text)
{
  DataWindow *data_window = frame_configs[frame_num]->data_window;
  
  if (data_window != 0)
    data_window->setText(text);
}

/* c---------------------------------------------------------------------- */

void frame_menu_cb(GtkWidget *frame, gpointer data )
{
  GtkWidget *widget; 
  guint num = GPOINTER_TO_UINT (data);
  guint frame_num, task, window_id;

  frame_num = num/TASK_MODULO;
  window_id = task = num % TASK_MODULO;

  switch (task) {

  case FRAME_MENU_REPLOT:
  {
    ParameterWindow *param_window = frame_configs[frame_num]->param_window;
    if (param_window != 0 &&
	param_window->setParamInfo())
    {
      sii_plot_data (frame_num, REPLOT_THIS_FRAME);
    }
    break;
  }
  
  case FRAME_MENU_CANCEL:
    widget = frame_configs[frame_num]->toplevel_windows[FRAME_MENU];
    if( widget )
    { gtk_widget_hide (widget); }
    break;

  case FRAME_MENU_SWEEPFILES:
    widget = frame_configs[frame_num]->toplevel_windows[FRAME_MENU];
    gtk_widget_hide (widget);
    show_swpfi_widget(frame_num);
    break;

  case FRAME_MENU_PARAMETERS:
    widget = frame_configs[frame_num]->toplevel_windows[FRAME_MENU];
    gtk_widget_hide(widget);
    show_param_widget(frame_num);
    break;

  case FRAME_MENU_VIEW:
    widget = frame_configs[frame_num]->toplevel_windows[FRAME_MENU];
    gtk_widget_hide (widget);
    show_view_widget(frame_num);
    break;

  case FRAME_MENU_LOCKSTEPS:
    break;

  case FRAME_MENU_EDITOR:
    widget = frame_configs[frame_num]->toplevel_windows[FRAME_MENU];
    gtk_widget_hide (widget);
    show_edit_widget(frame_num);
    break;

  case FRAME_MENU_EXAMINE:
  { 
    // Close the frame menu

    widget = frame_configs[frame_num]->toplevel_windows[FRAME_MENU];
    gtk_widget_hide(widget);

    // Open the examine window

    show_exam_widget(frame_num);
    break;
  }
  case FRAME_MENU_CLICK_DATA:
  {
    // Toggle the data window.

    DataWindow *data_window = frame_configs[frame_num]->data_window;
    
    if (data_window == 0)
    {
      data_window = new DataWindow(default_font, frame_num);
      frame_configs[frame_num]->data_window = data_window;
    }
    
    data_window->toggleWindow();

    // Close the frame menu

    widget = frame_configs[frame_num]->toplevel_windows[FRAME_MENU];
    gtk_widget_hide (widget);
    break;
  }
  };
}

/* c---------------------------------------------------------------------- */

void show_exam_widget(int frame_num)
{
  ExamineWindow *examine_window = frame_configs[frame_num]->examine_window;
  
  if (examine_window == 0)
  {
    examine_window = new ExamineWindow(default_font, frame_num);
    
    frame_configs[frame_num]->examine_window = examine_window;
  }

  examine_window->setFrameActive(true);
  sxm_refresh_list(frame_num);
  examine_window->updateWidgets();
  examine_window->show_all();
}

/* c---------------------------------------------------------------------- */

void show_view_widget(int frame_num)
{
  ViewWindow *view_window = frame_configs[frame_num]->view_window;
  
  if (view_window == 0)
  {
    view_window = new ViewWindow(default_font, frame_num);
    frame_configs[frame_num]->view_window = view_window;
  }
  
  view_window->setActive(true);
  view_window->update();
  view_window->show_all();
}

/* c---------------------------------------------------------------------- */

void show_edit_widget(int frame_num)
{
  EditWindow *edit_window = frame_configs[frame_num]->edit_window;
  if (edit_window == 0)
  {
    edit_window = new EditWindow(default_font, frame_num);
    frame_configs[frame_num]->edit_window = edit_window;
  }
  
  edit_window->setFrameActive(true);
  edit_window->show_all();
}

/* c---------------------------------------------------------------------- */

void show_param_widget(const int frame_num)
{
  // Open the window

  ParameterWindow *param_window = frame_configs[frame_num]->param_window;
  
  if (param_window == 0)
  {
    param_window = new ParameterWindow(default_font, frame_num);
    frame_configs[frame_num]->param_window = param_window;
  }
  
  param_window->update();
  param_window->show_all();
}

/* c---------------------------------------------------------------------- */

void show_swpfi_widget(int frame_num)
{
  SweepfileWindow *sweepfile_window =
    frame_configs[frame_num]->sweepfile_window;
  
  if (sweepfile_window == 0)
  {
    sweepfile_window = new SweepfileWindow(default_font, frame_num);
    frame_configs[frame_num]->sweepfile_window = sweepfile_window;
  }
  else
  {
    sweepfile_window->update();
  }
  
  sweepfile_window->show_all();
}

/* c---------------------------------------------------------------------- */

void show_frame_menu(GtkWidget *text, gpointer data )
{
  guint frame_num = GPOINTER_TO_UINT(data);

  GtkWidget *widget = frame_configs[frame_num]->toplevel_windows[FRAME_MENU];

  if (widget == 0)
    frame_menu_widget(frame_num);
  else
    gtk_widget_show(widget);
}

/* c---------------------------------------------------------------------- */

void frame_menu_widget( guint frame_num )
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *button;

  guint nn;
  gchar *bb;
  GdkPoint *ptp;
  gint x, y;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  frame_configs[frame_num]->toplevel_windows[FRAME_MENU] = window;
  ptp = &frame_configs[frame_num]->widget_origin[FRAME_MENU];
  x = ptp->x; y = ptp->y;
  gtk_widget_set_uposition (window, ptp->x, ptp->y);

  gtk_signal_connect (GTK_OBJECT (window), "destroy",
		      GTK_SIGNAL_FUNC(gtk_widget_destroyed),
		      &window);
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
		      GTK_SIGNAL_FUNC(sii_nullify_widget_cb),
		      GINT_TO_POINTER(frame_num * TASK_MODULO + FRAME_MENU) );

  /* --- Title and border --- */
  bb = g_strdup_printf ("Frame %d", frame_num+1 );
  gtk_window_set_title (GTK_WINDOW (window), bb);
  g_free( bb );
  gtk_container_border_width (GTK_CONTAINER (window), 0);

  /* --- Create a new vertical box for storing widgets --- */
  vbox = gtk_vbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER(window), vbox);

  button = gtk_button_new_with_label ("Cancel");
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0 );

  nn = frame_num*TASK_MODULO + FRAME_MENU_CANCEL;
  gtk_signal_connect (GTK_OBJECT(button)
		      ,"clicked"
		      , (GtkSignalFunc) frame_menu_cb
		      , (gpointer)nn
		      );
  button = gtk_button_new_with_label ("Sweepfiles");
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0 );

  nn = frame_num*TASK_MODULO + FRAME_MENU_SWEEPFILES;
  gtk_signal_connect (GTK_OBJECT(button)
		      ,"clicked"
		      , (GtkSignalFunc) frame_menu_cb
		      , (gpointer)nn
		      );

  button = gtk_button_new_with_label (" Parameters + Colors ");
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0 );

  nn = frame_num*TASK_MODULO + FRAME_MENU_PARAMETERS;
  gtk_signal_connect (GTK_OBJECT(button)
		      ,"clicked"
		      , (GtkSignalFunc) frame_menu_cb
		      , (gpointer)nn
		      );

  button = gtk_button_new_with_label ("View");
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0 );

  nn = frame_num*TASK_MODULO + FRAME_MENU_VIEW;
  gtk_signal_connect (GTK_OBJECT(button)
		      ,"clicked"
		      , (GtkSignalFunc) frame_menu_cb
		      , (gpointer)nn
		      );
  button = gtk_button_new_with_label ("Editor");
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0 );

  nn = frame_num*TASK_MODULO + FRAME_MENU_EDITOR;
  gtk_signal_connect (GTK_OBJECT(button)
		      ,"clicked"
		      , (GtkSignalFunc) frame_menu_cb
		      , (gpointer)nn
		      );

  button = gtk_button_new_with_label ("Examine");
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0 );

  nn = frame_num*TASK_MODULO + FRAME_MENU_EXAMINE;
  gtk_signal_connect (GTK_OBJECT(button)
		      ,"clicked"
		      , (GtkSignalFunc) frame_menu_cb
		      , (gpointer)nn
		      );

  button = gtk_button_new_with_label ("Data Widget");
  gtk_box_pack_start (GTK_BOX (vbox), button, TRUE, TRUE, 0 );

  nn = frame_num*TASK_MODULO + FRAME_MENU_CLICK_DATA;
  gtk_signal_connect (GTK_OBJECT(button)
		      ,"clicked"
		      , (GtkSignalFunc) frame_menu_cb
		      , (gpointer)nn
		      );

  /* --- Make everything visible --- */
  gtk_widget_show_all (window);

}

/* c---------------------------------------------------------------------- */

