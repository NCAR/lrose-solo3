/* created using cproto */
/* Thu Jul  7 17:55:07 UTC 2011*/

#ifndef sii_links_widget_hh
#define sii_links_widget_hh

#include <soloii.h>

extern void sii_set_links_from_solo_struct(int frame_num, int links_id, int32_t *linked_windows);
extern void show_links_widget(LinksInfo *linkfo);
extern void sii_links_set_foreach (GtkWidget *button, gpointer data );
extern void  sii_links_set_from_struct (GtkWidget *button, gpointer data );
extern void  sii_links_get_foreach (GtkWidget *button, gpointer data );
extern LinksInfo *sii_new_links_info(const gchar *name, const guint frame_num,
				     const guint widget_id,
				     const gboolean this_frame_only);
extern void sii_links_widget_cb ( GtkWidget *w, gpointer   data );
extern void sii_links_cb ( GtkWidget *w, gpointer   data );
extern void sii_view_update_links (guint frame_num, int li_type);
extern void sii_links_widget( guint frame_num, guint widget_id, LinksInfo *linkfo );
extern gboolean sii_set_swpfi_info (guint frame_num, gint sweep_num);		       

#endif
