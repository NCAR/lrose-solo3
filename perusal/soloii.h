/* 	$Id$	 */

# ifndef SOLOII_H
# define SOLOII_H

#include <gtk/gtk.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>

#include <DataWindow.hh>
#include <EditWindow.hh>
#include <ExamineWindow.hh>
#include <ParameterWindow.hh>
#include <sii_image.h>
#include <SiiPalette.hh>
#include <SweepfileWindow.hh>
#include <ViewWindow.hh>

#define yes TRUE
#define no FALSE

#define MAX_TOP_WINDOWS 64
#define MAX_FRAMES 12
#define MAX_CONFIG_ROWS 4
#define MAX_CONFIG_COLS 4
#define MAX_CONFIG_CELLS MAX_CONFIG_ROWS*MAX_CONFIG_COLS
#define MAX_LINK_SETS 16
#define CFG_QUE_SIZE 3

#define TASK_MODULO 100

#define DEFAULT_WIDTH 300
#define DEFAULT_HEIGHT 180

enum {				/* top level window ids for each frame */
   FRAME_ZERO,
   FRAME_MENU,

   FRAME_SWPFI_LINKS,
   FRAME_PARAM_LINKS,
   FRAME_VIEW_LINKS,
   FRAME_CTR_LINKS,
   FRAME_LMRK_LINKS,
   FRAME_LOCKSTEP_LINKS,

   FRAME_LAST_ENUM,

};

static const int FrameDragResize = 0x1;
static const int FrameDragAdjust = 0x2;
static const int FrameZoomResize = 0x4;
static const int FrameNewConfig  = 0x8;
static const int FrameReshape    = 0x10;

enum {
   LI_ZERO,
   LI_SWPFI,
   LI_LOCKSTEP,
   LI_PARAM,
   LI_VIEW,
   LI_CENTER,
   LI_LANDMARK,
   LI_LAST_ENUM,

};

typedef struct _GdkRectangle	  Rectangle;
typedef struct _GdkSegment	  Segment;
typedef struct _GdkPoint	  Point;

/* c---------------------------------------------------------------------- */

#define PARAM_MAX_WIDGETS PARAM_LAST_ENUM

enum {
   PARAM_HILIGHT,
   PARAM_CB_BOTTOM,		/* color bar */
   PARAM_CB_LEFT,
   PARAM_CB_RIGHT,
   PARAM_CB_TOP,
   PARAM_CB_SYMBOLS,
   PARAM_LAST_ENUM,

};

enum color_bar_orientation{
  CB_BOTTOM      = 1 << 0,
  CB_LEFT        = 1 << 1,
  CB_RIGHT       = 1 << 2,
  CB_TOP         = 1 << 3,
  CB_SYMBOLS     = 1 << 4,
  CB_ALL_SYMBOLS = 1 << 5,
};

/* c---------------------------------------------------------------------- */

typedef struct {
   double x;
   double y;
   double z;
} Dxyz;

/* c---------------------------------------------------------------------- */

typedef struct {

  guint frame_num;
  guint button_mask;
  gint x;			/* screen coordinates */
  gint y;			/* y = 0 => top & y = height => bottom */

  gdouble xx;			/* cartesian coords in km. */
  gdouble yy;
  gdouble zz;

  gdouble dtime;
  gdouble lat;
  gdouble lon;
  gdouble alt;

  gdouble rng_km;

} SiiPoint;

/* c---------------------------------------------------------------------- */

typedef struct _sii_linked_list SiiLinkedList;

struct _sii_linked_list {
  gpointer data;
  gpointer data2;
  SiiLinkedList *previous;
  SiiLinkedList *next;
};

/* c---------------------------------------------------------------------- */

typedef struct {

   guint left_attach;
   guint right_attach;
   guint top_attach;
   guint bottom_attach;

} sii_table_parameters;

/* c---------------------------------------------------------------------- */

typedef struct {
   gchar *name;
   guint frame_num;
   guint widget_id;
   guint lead_frame;
   guint num_links;
   gboolean link_set[MAX_FRAMES];
   GtkWidget *table;

} LinksInfo;

/* c---------------------------------------------------------------------- */

typedef struct {
   gint change_count;

   gboolean toggle[PARAM_MAX_WIDGETS];
   gboolean  electric_params;
   guint cb_loc;
   guint cb_labels_state;
   guint num_colors;

  // Color palette used for the rendering of the field.

   SiiPalette *pal;
   SiiPalette *orig_pal;
   LinksInfo *param_links;
  std::vector< std::string > param_names_list;

   int orientation;
   GString *cb_labels;
   GString *cb_symbols;

   guint field_toggle_count;
   SiiLinkedList *fields_list;
   SiiLinkedList *toggle_field;

} ParamData;

/* c---------------------------------------------------------------------- */

typedef struct {		/* frame info */
   
  /* parameters for each frame */
  guint frame_num;

  // NOTE: Look more closely at the blow-up information.  Perhaps we can move
  // this into a separate object.  Much of the information we also have for
  // regular frames so will need to be included in each of the frame_config
  // objects.

  // In frame_configs[0], this gives the frame number of the current blow-up
  // frame.  The blow-up frame is the frame that is being displayed in a
  // separate window, accessed by hitting the function key that has the same
  // number as the frame number.  My guess is that this field isn't used in
  // the other frame_config structures.

  guint blow_up_frame_num;
  SiiPoint click_loc;
  guint ncols;
  guint nrows;
   
  // Flag indicating whether this frame is currently in the blow-up window.

  gboolean blow_up;
  gboolean changed;
  gboolean local_reconfig;

  // Width and height of the frame, in pixels.

  guint width;
  guint height;

  guint data_width;
  guint data_height;
  guint lock_step;
  guint sync_num;
  gint reconfig_count;
  gint expose_count;
  gint prev_expose_num;
  gint new_frame_count;
  gint colorize_count;
  gint max_lbl_wdt;
  gint big_max_lbl_wdt;
  gint points_in_bnd;

  gint big_reconfig_count;
  gint big_expose_count;
  gint big_colorize_count;

  sii_table_parameters tbl_parms; /* left_attach, right_attach, etc */

  gchar *font;
  gchar *big_font;
  guint font_height;
  guint big_font_height;
  GdkGC *gc;            // Graphics context for regular frame rendering?
  GdkGC *big_gc;
  GdkGC *border_gc;
  GdkColormap *color_map;
  GtkAccelGroup *accel_group;

  gchar frame_title[128];
   
  // A pointer to the frame where the data is rendered.

  GtkWidget *frame;

  // A pointer to the window where the blow-up data is rendered.  This is
  // only set in frame_configs[0].

  GtkWidget *blow_up_window;

  // A pointer to the frame where the blow-up data is rendered.  This is only
  // set in frameConfigs[0].

  GtkWidget *blow_up_frame;

  // Pointers the the top level windows.  These all need to be replaced with
  // specific object pointers and handled the way widget objects really should
  // be handled.

  GtkWidget *toplevel_windows[MAX_TOP_WINDOWS];

  // Pointers to the individual top level windows associated with this frame.
  // These replace the toplevel_windows pointers.

  SweepfileWindow *sweepfile_window;
  ViewWindow *view_window;
  ParameterWindow *param_window;
  EditWindow *edit_window;
  ExamineWindow *examine_window;
  DataWindow *data_window;
  
  ParamData *param_data;
  gpointer view_data;

  LinksInfo *link_set[MAX_LINK_SETS];
  GdkPoint widget_origin[MAX_TOP_WINDOWS];
  GdkPoint frame_origin;
  GdkPoint big_frame_origin;

  GdkRectangle top_label;
  GdkRectangle color_bar_labels;
  GdkRectangle most_recent_expose;

  // The corners of the frame

  SiiPoint corner[4];		/* (ul,ur,lr,ll) */
  SiiPoint center;
  SiiPoint radar;
  SiiPoint ulc_radar;
  SiiPoint landmark;
  SiiPoint ulc_bnd;		/* corners of boundary rectangle */
  SiiPoint lrc_bnd;

  // The color bar pattern.  cb_pattern contains the data color LUT index for
  // the color that appears in that pixel position in the color bar.

  guint cb_pattern_len;
  guchar *cb_pattern;

  // This is the image information.  I don't know what the comment means

  SiiImage *image;		/* check each time for match with width x height */
  guint image_size;
  guint image_stride;
  guint max_image_size;
   
  guint big_sync_num;

  // The height and width of the blow-up window in pixels.  These values are
  // stored in frame_configs[0].  I'm guessing they're ignored in the other
  // frame_config structures.

  guint big_width;
  guint big_height;

  guint big_mag_multiple;

  SiiImage *big_image;
  guint big_image_size;
  guint big_image_stride;
  guint max_big_image_size;
  guint big_image_show_count;
   
  guint cfg_que_ndx;
  guint cfg_width[CFG_QUE_SIZE];
  guint cfg_height[CFG_QUE_SIZE];

  guint config_sync_num;
  guint drag_resize_count;
  guint expose_sync_num;
  gint reconfig_flag;
  gint x_drag_ref;
  gint y_drag_ref;

} SiiFrameConfig;

/* c---------------------------------------------------------------------- */

typedef struct {

  guint cell_num;
  guint row;
  guint col;

  guint frame_num;
  gboolean processed;

} sii_config_cell;


# endif
