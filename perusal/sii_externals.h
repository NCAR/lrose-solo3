/* 	$Id$	 */

# ifndef SII_EXTERNALS_H
# define SII_EXTERNALS_H

#include <gtk/gtk.h>

#include <SoloMainWindow.hh>
#include <soloii.h>

extern Pango::FontDescription default_font;
extern SoloMainWindow *main_window;

extern GtkWidget *main_event_box;

extern GtkWidget *main_table;

extern GtkAllocation *sii_frame_alloc[MAX_FRAMES];

// NOTE: I don't know what this is and think we can just get rid of it.
// It is initialized to NULL in soloii.cc, and then is only accessed in
// sii_generic_gslist_insert() [sii_utils.cc], which doesn't seem to be
// called anywhere.

extern GSList *gen_gslist;	/* generic GSList */

// NOTE: This tree is set to NULL at the top of soloii.cc, and then is
// initialized in main() [soloii.cc].  It doesn't seem to be being used
// anywhere.

extern GTree *color_tables;

// NOTE: This tree is set to NULL at the top of soloii.cc, but is never
// referenced anywhere else in the code.

extern GTree *color_palettes_tree;

extern guint sii_config_count;
extern gfloat sii_config_w_zoom;
extern gfloat sii_config_h_zoom;
extern sii_config_cell **config_cells;

extern guint sii_frame_count;
extern SiiFrameConfig *frame_configs[2*MAX_FRAMES +1];
extern GdkColor *frame_border_colors[MAX_FRAMES];

extern guint sii_table_widget_width;
extern guint sii_table_widget_height;

extern guint previous_F_keyed_frame;
extern gint cursor_frame;

//extern guint click_count;
//extern guint clicks_in_que;

//extern const guint click_que_size;
//extern SiiLinkedList *sii_click_que;

extern guint sii_seq_num;

extern const gchar *title_tmplt;

extern gboolean time_series_mode;

extern GString *gs_complaints;

extern gboolean nexrad_mode_set;

extern gdouble angular_fill_value;

extern gboolean batch_mode;

extern GString *gs_image_dir;

extern gfloat batch_threshold_value;

extern GString *batch_threshold_field;

extern GString *gen_gs;

# endif



