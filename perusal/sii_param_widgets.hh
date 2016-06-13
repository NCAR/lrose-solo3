/* created using cproto */
/* Thu Jul  7 17:55:09 UTC 2011*/

#ifndef sii_param_widgets_hh
#define sii_param_widgets_hh

#include <string>
#include <vector>

#include <solo_window_structs.h>
#include "soloii.h"
#include "SiiPalette.hh"

// NOTE:  I added this to replace the use of 2's and 4's in the code.  I've put
// it in the places I've evaluated, but it'll probably need to be used in more
// places.

# define BORDER_PIXELS 2

/* c---------------------------------------------------------------------- */

typedef struct {
  gchar name[16];
  SiiPalette *pal;
} ParamFieldInfo;

/* c---------------------------------------------------------------------- */

extern void param_set_cb_loc(int frame_num, int loc);
extern void sii_param_dup_opal(struct solo_palette *opal);
extern void sii_param_set_plot_field(int frame_num, char *fname);
extern int solo_palette_color_numbers(int frame_num);
extern const gchar *set_cb_labeling_info (guint frame_num, gdouble *relative_locs);
extern void set_color_bar (SiiFrameConfig *sfc, WW_PTR wwptr);
extern void sii_initialize_parameter(const guint frame_num,
				     const std::string &name);
extern const gchar *set_cb_labeling_info (guint frame_num, gdouble *relative_locs);
extern void sii_param_set_plot_field (int frame_num, char *fname);
extern void sii_reset_image (guint frame_num);
extern void sii_set_param_names_list(const guint frame_num,
				     const std::vector< std::string> &param_names);
extern bool solo_hardware_color_table(gint frame_num);
extern int solo_palette_color_numbers(int frame_num);
extern GdkColor *sii_boundary_color (guint frame_num, gint exposed);
extern GdkColor *sii_grid_color (guint frame_num, gint exposed);
extern const gchar *sii_param_palette_name (guint frame_num);
extern void sii_colorize_image (SiiFrameConfig *sfc);
extern void sii_double_colorize_image (guint frame_num);

#endif
