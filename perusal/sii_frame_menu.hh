/* created using cproto */
/* Thu Jul  7 17:55:07 UTC 2011*/

#ifndef sii_frame_menu_hh
#define sii_frame_menu_hh

#include <glib.h>
#include <gdk/gdk.h>

extern void frame_menu_cb(GtkWidget *frame, gpointer data );
extern void show_edit_widget(int frame_num);
extern void show_swpfi_widget(int frame_num);
extern void show_view_widget(int frame_num);
extern void show_param_widget(const int frame_num);
extern void show_exam_widget (int frame_num);
extern void frame_menu_widget( guint frame_num );
extern void sii_set_click_data_text(guint frame_num, const std::string &text);
extern void show_frame_menu(GtkWidget *text, gpointer data );

#endif
