/* 	$Id$	 */

#include <iostream>
#include <string>

#include <gdkmm.h>
#include <gtkmm/main.h>

#include "soloii.hh"
#include "solo2.hh"
#include "sii_utils.hh"
#include "sii_callbacks.hh"
#include "sp_basics.hh"
#include "sp_save_frms.hh"
#include "sii_frame_menu.hh"
#include "sii_colors_stuff.hh"
#include "sii_config_stuff.hh"
#include "SoloMainWindow.hh"
#include "MainConfigWindow.hh"
#include "CRectangle.hh"

Pango::FontDescription default_font("Sans 8");
SoloMainWindow *main_window;

GtkAllocation *sii_frame_alloc[MAX_FRAMES];

GSList *gen_gslist = NULL;	/* generic GSList */

GSList *color_names = NULL;
GSList *Xcolor_names = NULL;

GTree *color_tables = NULL;
GTree *color_palettes_tree = NULL;


guint sii_config_count = 0;
gfloat sii_config_w_zoom = 1.0;
gfloat sii_config_h_zoom = 1.0;
sii_config_cell **config_cells = NULL;

guint sii_frame_count = 0;
SiiFrameConfig *frame_configs[2*MAX_FRAMES +1];
GdkColor *frame_border_colors[MAX_FRAMES];

guint sii_table_widget_width = DEFAULT_WIDTH;
guint sii_table_widget_height = DEFAULT_WIDTH;

guint previous_F_keyed_frame = 0;

gint cursor_frame = 0;
/* 0 => cursor not in any frame
 * otherwise frame_num = cursor_frame-1;
 */

guint sii_seq_num = 0;

gchar *small_pro_fonta = NULL;
gchar *med_pro_fonta = NULL;
gchar *big_pro_fonta = NULL;

const gchar *title_tmplt = "00 00/00/2000 00:00:00 HHHHHHHH 000.0 HHH HHHHHHHH";

gboolean time_series_mode = FALSE;

GString *gs_complaints = NULL;

gboolean nexrad_mode_set = FALSE;

gboolean batch_mode = FALSE;

GString *gs_image_dir = NULL;

gfloat batch_threshold_value = -32768;

GString *batch_threshold_field = NULL;

GString *gen_gs = NULL;

/* c---------------------------------------------------------------------- */

static GString *gs_initial_dir_name = NULL;

GtkWidget * nexrad_check_item;
GtkWidget * anglr_fill_item;

/* c---------------------------------------------------------------------- */

int main( int argc, char *argv[])
{
  // Set some global variables.
  // NOTE: These will probably be replaced with objects.  I need to look
  // into them further.  Also need to decide how they fit into the rest of
  // the system.

  color_tables = g_tree_new ((GCompareFunc)strcmp);
  gs_complaints = g_string_new ("\n");
  gen_gs = g_string_new ("");

  // Initialize the GTK toolkit and process and GTK-specific command line
  // arguments.  This must be done before calling any other GTK methods.

  Gtk::Main gtk_main(argc, argv);
  
  // Do the solo3-specific initializations

  sii_set_default_colors ();
  sii_init_frame_configs();
  sii_reset_config_cells();

  // Create the main window.

  main_window = new SoloMainWindow(default_font);
  
  // Get the relevant environment variables and initialize the display

  gs_initial_dir_name = g_string_new ("./");
  main_window->setRootDir(gs_initial_dir_name->str);
  batch_mode = (getenv ("BATCH_MODE")) ? TRUE : FALSE;

  gchar *aa;
  gchar *bb;
  gchar *cc;
  gchar swi_dir[256];
  
  if ((aa = getenv ("WINDOW_DUMP_DIR")) != 0)
  {
    if (strlen (aa))
      { gs_image_dir = g_string_new (aa); }
  }

  // NOTE: I haven't figured out how the SOLO_WINDOW_INFO stuff works...

  if ((aa = getenv ("swi")) || (aa = getenv ("SOLO_WINDOW_INFO"))) {
    if (strlen (aa)) {
      bb = strrchr (aa, '/');
      if (!bb)
	{ strcpy (swi_dir, "./"); }
      else
	{ for (cc = swi_dir; aa <= bb; *cc++ = *aa++); *cc = '\0'; }
      
      gint jj = solo_absorb_window_info (swi_dir, aa, 0);

      if (batch_mode && jj < 0 ) {
	g_message
	  ("Unable to start BATCH_MODE solo3 from config file");
	exit (1);
      }
    }
    else
      { aa = NULL; }
  }

  // At this point aa is still set from SOLO_WINDOW_INFO above.
  //
  // sii_get_swpfi_dir() gets the name of the directory containing the DORADE
  // files.  As a side-effect, I believe it stores a list of the file names
  // in a static variable within the translate module.  It looks like if there
  // are no sweep files in the dorade directory, sii_get_swpfi_dir() returns
  // NULL.

  if (!aa && (aa = sii_get_swpfi_dir (NULL))) {
    sii_default_startup (aa);
    main_window->configFrames(2, 2);
    g_string_assign (gs_initial_dir_name, aa);
    main_window->setRootDir(gs_initial_dir_name->str);
  }
  // NOTE: This is a really confusing condition.  At this point, aa either
  // contains the SOLO_WINDOW_INFO value or, if that wasn't set, the sweep
  // file directory found above.  So, !aa means that neither of these values
  // is set.
  else if (!aa) {
     g_string_truncate (gen_gs, 0);
     g_string_append
       (gen_gs, "The environment variable DORADE_DIR has not been\n");
     g_string_append
       (gen_gs, "set to point to a directory containing sweepfiles or the\n");
     g_string_append
       (gen_gs, "local directory does not contain any sweepfiles.\n");

    // NOTE: This brings up solo3 with the Config Files window open and 
    // no sweep frames

     main_window->showConfigWindow();
  }

  // Recursively shows main_window and any child widgets

  main_window->show_all_children();
  
  // Call sii_batch() whenever the display is idle

  g_idle_add (sii_batch, 0);	/* routine to control batch mode */

  // This begins the event loop which handles events. No events propagate
  // until this has been called.

  Gtk::Main::run(*main_window);
  
  // Reclaim memory

  delete main_window;
  
  return(0);  /* c...end */

}
