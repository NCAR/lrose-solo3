/* created using cproto */
/* Thu Jul  7 17:55:12 UTC 2011*/

#ifndef solo2_hh
#define solo2_hh

#include <string>
#include <vector>

#include <glib.h>

#include <solo_window_structs.h>
#include <sed_shared_structs.h>
#include "soloii.h"

struct sii_arc {
   int id;
   int num_intxns;
   double angle1;
   double angle2;
   int nxt_ndx;
   Dxyz xy[2];
   struct sii_arc *next;
};

typedef struct sii_arc SiiArc;

extern gboolean sii_nab_state_set (int frame_num, const char *dir, const char *filename);
extern void sii_png_image_prep(const std::string &dir);
extern void sii_reset_reconfig_flags(int frame_num);
extern guint sii_frame_sync_num (guint frame_num);
extern guint sii_config_sync_num (guint frame_num);
extern int se_dump_sfic_widget(struct swp_file_input_control *sfic, int frame_num);
extern int sii_return_frame_count(void);
const GString *sii_return_config_files (const char *dir);
extern double sii_return_swpfi_time_stamp (guint frame_num);
extern std::string sii_return_swpfi_dir(guint frame_num);
extern std::vector< std::string > sii_return_swpfi_list (guint frame_num);
extern void print_fieldvalues(int frame_num,
			      const std::vector< std::string > &field_list);
extern void yes_exit (void);
extern void sii_radar_coords_to_frame_coords (int frame_num,
					      gdouble xx_km, gdouble yy_km,
					      gint *x, gint *y);
extern gdouble sii_tic1 (gdouble tic, gdouble ticinc);
extern int sii_lmrk_coords_to_frame_coords(SiiFrameConfig *sfc, WW_PTR wwptr,
					   gdouble xx_km, gdouble yy_km,
					   gint *x, gint *y);
extern void sii_really_xfer_images(int frame_num,
				   GdkRectangle *image_clipping_region,
				   gboolean blow_up);
extern gboolean sii_blow_up(guint frame_num);
extern void sii_xfer_images(int frame_num, GdkRectangle *area);
extern void sii_really_xfer_images (int frame_num, GdkRectangle *area
			     , gboolean blow_up);
extern gboolean sii_lmrk_inside (SiiFrameConfig *sfc);
extern gint sii_rng_ring_arcs (SiiFrameConfig *sfc, gdouble range);
extern void sp_do_lock_color(int frame);
extern void sii_update_frame_info (guint frame_num);
extern void sii_check_image_size (guint frame_num);
extern void sii_plot_data2 (guint frame_num, guint plot_function);
extern void sii_plot_data (guint frame_num, guint plot_function);
extern gboolean sii_batch (gpointer argu);
extern gchar *sii_get_swpfi_dir (gchar *dir);
extern int sii_default_startup (const gchar *swpfi_dir);
extern void solo_data_color_lut (int frme);
extern void yes_exit(void);
extern void sii_set_boundary_pt_count(int frame_num, int num_points);
extern void se_clear_segments(int frame_num);
// NOTE:  This is defined in a million places!!
extern void solo_message(const char *message);
extern void solo_data_color_lut(int frme);
extern const char *sii_string_from_dtime(double dtime);
extern const struct solo_list_mgmt *sii_return_swpfi_radar_list (guint frame_num);
extern void sii_center_on_clicks(gint num_clicks);
extern void sii_set_geo_coords ( int frame_num, gdouble dx, gdouble dy,
				 SiiPoint &click);

/* c------------------------------------------------------------------------ */
/*     FORWARD DECLARATIONS                                                  */
/* c------------------------------------------------------------------------ */

void sii_draw_base_image(SiiFrameConfig *sfc, WW_PTR wwptr,
			 guchar *img, const GdkRectangle *area,
			 const gboolean blow_up);

void sii_calc_center_landmark_locations(SiiFrameConfig *sfc, WW_PTR wwptr,
					const gboolean time_series,
					const gdouble ts_start,
					const gdouble ts_span,
					gdouble &xctr, gdouble &yctr,
					gdouble &xlmrk, gdouble &ylmrk,
					gdouble &pixels_per_km);

void sii_draw_range_rings(GdkGC *gc, SiiFrameConfig *sfc,
			  GtkWidget*frame, WW_PTR wwptr,
			  GdkFont *font,
			  const SiiPoint *pt_max, const gdouble max_range,
			  const gdouble xlmrk, const gdouble ylmrk,
			  const gdouble pixels_per_km);

void sii_draw_azimuth_lines(GdkGC *gc, SiiFrameConfig *sfc,
			    GtkWidget *frame, WW_PTR wwptr,
			    GdkFont *font, const double max_range,
			    const gdouble xlmrk, const double ylmrk,
			    const double pixels_per_km);

void sii_draw_xy_tic_marks(GdkGC *gc, SiiFrameConfig *sfc, WW_PTR wwptr,
			   GdkFont *font, const gboolean time_series,
			   const gdouble ts_span,
			   const GdkRectangle &clip);

/* c------------------------------------------------------------------------ */
#endif
