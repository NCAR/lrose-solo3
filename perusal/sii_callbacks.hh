/* created using cproto */
/* Thu Jul  7 17:55:00 UTC 2011*/

#ifndef sii_callbacks_hh
#define sii_callbacks_hh

#include <glib.h>
#include <gtk/gtk.h>

extern void sii_blow_up_expose_event (GtkWidget *frame, GdkEvent *event, gpointer data );

extern void sii_blow_up_config_event(GtkWidget *frame, GdkEvent *event, gpointer data );
extern gint sii_blow_up_keyboard_event(GtkWidget *frame, GdkEvent *event, gpointer data );
extern gint sii_frame_keyboard_event(GtkWidget *frame, GdkEvent *event, gpointer data );
extern void sii_blow_up_mouse_button_event (GtkWidget *frame, GdkEvent *event, gpointer data );
extern void sii_frame_expose_event(GtkWidget *frame, GdkEvent *event, gpointer data );
extern void sii_frame_config_event(GtkWidget *frame, GdkEvent *event, gpointer data );
extern void sii_mouse_button_event (GtkWidget *frame, GdkEvent *event, gpointer data );
extern void sii_enter_event(GtkWidget *frame, GdkEvent *event, gpointer data );
extern void sii_leave_event(GtkWidget *frame, GdkEvent *event, gpointer data );
extern void sii_focus_in_event(GtkWidget *frame, GdkEvent *event, gpointer data );
extern void sii_focus_out_event(GtkWidget *frame, GdkEvent *event, gpointer data );
extern void sii_focus_in_event(GtkWidget *frame, GdkEvent *event, gpointer data );
extern void sii_focus_out_event(GtkWidget *frame, GdkEvent *event, gpointer data );
extern void sii_blow_up_mouse_delete_event(GtkWidget *frame, GdkEvent *event, gpointer data );
#endif
