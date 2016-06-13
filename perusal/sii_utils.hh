#ifndef sii_utils_hh
#define sii_utils_hh

#include <string>

#include <gtk/gtk.h>
#include <glibmm/ustring.h>

#include <soloii.h>
#include <sii_overlays.h>

extern void sii_dump_debug_stuff(void);
extern void sii_append_debug_stuff(const std::string &new_debug_info);
extern void sii_glom_strings (const gchar **cptrs, int nn, GString *gs);
extern int sii_absorb_zeb_map (gchar *path_name, siiOverlay *overlay);
extern guint sii_inc_master_seq_num ();
extern guint sii_get_master_seq_num ();
extern void sii_zap_quick_message (GtkWidget *w, gpointer data );
extern void sii_message (const Glib::ustring &message);
extern void sii_set_widget_frame_num ( GtkWidget *w, gpointer data );
extern int sii_str_seek(char **sptrs, int count, const char *sought);
extern char *sii_glom_tokens(char *str, char **sptrs, int count, char *joint);
extern SiiLinkedList *sii_ll_queue(SiiLinkedList **list, SiiLinkedList *link);
extern SiiLinkedList *sii_ll_dequeue(SiiLinkedList **list);
extern SiiLinkedList *sii_ll_push(SiiLinkedList **list, SiiLinkedList *link);
extern SiiLinkedList *sii_ll_remove(SiiLinkedList **list, SiiLinkedList *link);
extern SiiLinkedList *sii_ll_pop(SiiLinkedList **list);
extern SiiLinkedList *sii_ll_malloc_item(void);
extern void sii_zap_quick_message (GtkWidget *w, gpointer data );
extern gchar *sii_nab_line_from_text (const gchar *txt, guint position );
extern gboolean sii_nab_region_from_text (const gchar *txt, guint position, gint *start, gint *end);
extern void sii_nullify_widget_cb (GtkWidget *widget, gpointer data);
gchar *sii_generic_gslist_insert ( gpointer name, gpointer data );
gboolean sii_str_values(const gchar *line, const guint nvals,
			gfloat &f1, gfloat &f2);

extern GtkWidget * sii_submenu_item (const gchar *name, GtkWidget *submenu,
				     guint widget_id, GtkSignalFunc sigfun,
				     guint frame_num );
extern GtkWidget *sii_submenu ( const gchar *name, GtkWidget *mbar );
extern GtkWidget *sii_toggle_submenu_item(const gchar *name, GtkWidget *submenu,
					  guint widget_id, GtkSignalFunc sigfun,
					  guint frame_num, guint radio_item,
					  GSList **radio_group_ptr );

extern gchar *sii_set_string_from_vals (GString *gs, guint nvals, gfloat f1, gfloat f2, guint precision);
extern SiiLinkedList *sii_init_circ_que (guint length);
extern const gchar *sii_item_delims ();
extern void sii_bad_entry_message ( const gchar *ee, guint items );
extern int loop_xy2ll(double *plat, double *plon, double *palt,
		      double *x, double *y, double *z,
		      double olat, double olon, double oalt,
		      double R_earth, int num_pts);

#endif
