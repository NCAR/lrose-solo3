/* 	$Id$	 */


#ifndef SOLO_WINDOW_STRUCTS_H
#define SOLO_WINDOW_STRUCTS_H

#ifdef USE_RADX
#include <Radx/Radx.hh>
#endif

#include <ColorTable.hh>
#include <PointInSpace.hh>
#include <SiiPalette.hh>
#include <sii_image.h>
#include <solo_list_mgmt.hh>

#define          SOLO_MAX_WINDOWS 12
#define        SOLO_TOTAL_WINDOWS SOLO_MAX_WINDOWS+1
#define           MAX_EXM_COLUMNS 48

#define         SOLO_BAD_DIR_NAME 1

#define                        K64 65536
#define                        K32 32768
#define                          K 1024

// Frame configs

#define            SMALL_RECTANGLE 0
#define          DOUBLE_WIDE_FRAME 1    
#define               SQUARE_FRAME 2
#define          LARGE_SLIDE_FRAME 3
#define         LARGE_SQUARE_FRAME 4
#define    LARGE_HALF_HEIGHT_FRAME 5
#define     LARGE_HALF_HEIGHT_REPL 6

#ifndef NULL
#define NULL 0
#endif

#define     SOLO_LOCAL_POSITIONING 0
#define     SOLO_FIXED_POSITIONING 1
#define    SOLO_LINKED_POSITIONING 2


#include <gtk/gtk.h>
#include "sii_widget_content_structs.h"
#include "sii_enums.h"

/* c------------------------------------------------------------------------ */
/* Define the structures used to save the soloii configuration to a file.    */

struct solo_sweep_file {
    char name_struct[4];	/* "SSFI" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    double start_time;
    float fixed_angle;
    int32_t linked_windows[SOLO_MAX_WINDOWS];
    int32_t sweep_count;
    int32_t sweep_skip;

    int32_t time_stamp;
    int32_t latest_version;
    int32_t input_fmt;
    int32_t io_type;
    int32_t millisecond;
    int32_t radar_type;
    int32_t version_num;
    char directory_name[128];
    char file_name[128];
    char scan_type[16];
    char radar_name[16];
    int32_t radar_num;
    int32_t sweep_keep_count;
    double stop_time;
};
/* c------------------------------------------------------------------------ */

struct solo_plot_lock {
    char name_struct[4];	/* "SPTL" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    int32_t linked_windows[SOLO_MAX_WINDOWS];
};
/* c------------------------------------------------------------------------ */

struct solo_plot_lock0 {
    char name_struct[4];	/* "SPTL" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    int32_t linked_windows[6];
};
/* c------------------------------------------------------------------------ */

struct solo_parameter_info {
    char name_struct[4];	/* "SPMI" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    int32_t linked_windows[SOLO_MAX_WINDOWS];
    char parameter_name[16];
    char palette_name[16];
};
/* c------------------------------------------------------------------------ */

struct solo_view_info {
    char name_struct[4];	/* "SWVI" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    int32_t height_in_pix;
    int32_t width_in_pix;

    int32_t auto_clear;
    int32_t auto_grid;
    int32_t frame_config;
    int32_t linked_windows[SOLO_MAX_WINDOWS];
    int32_t type_of_annot;
    int32_t type_of_grid;
    int32_t type_of_plot;

    float angular_fill_deg;
    float angular_fill_pct;
    float az_line_int_deg;
    float az_annot_at_km;
    float rng_ring_int_km;
    float rng_annot_at_deg;
    float grid_line_width_pix;
    float horiz_tic_mark_km;
    float vert_tic_mark_km;
    float radar_altitude_km;
    float border_minor_tic_km;
    float border_major_tic_km;

    float magnification;
    float vertical_mag;

    /* there are also three points in space associtated with view info.
     * they are the radar_location, center_of_view, and a landmark.
     * they reside in the solo_window_ptrs struct.
     */
    int32_t offset_to_pisps;
    int32_t num_pisps;
    char message[48];		/* display length of x and y axes in km. */
    /*
     * "SOL_RAD_LOC" id contains the radar location for this window
     * "SOL_WIN_LOC" id contains the location of the center of this window
     * "SOL_LANDMARK" id contains a landmark for this window
     */
    int32_t type_of_centering;
    float rhi_x_ctr_km;		/* center for rhi plots */
    float rhi_y_ctr_km;		/* center for rhi plots */

    float ts_magnification;	/* time series has its own mag. */
    float ts_ctr_km;		/* center for time-series plots */
    float x_tic_mag;
    float y_tic_mag;

};
/* c------------------------------------------------------------------------ */

struct landmark_info {
    char name_struct[4];	/* "SLMK" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    int32_t linked_windows[SOLO_MAX_WINDOWS];
    int32_t landmark_options;
    int32_t reference_frame;
};
/* c------------------------------------------------------------------------ */

struct frame_ctr_info {
    char name_struct[4];	/* "SCTR" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    int32_t linked_windows[SOLO_MAX_WINDOWS];
    int32_t centering_options;
    int32_t reference_frame;
};

/* c------------------------------------------------------------------------ */

struct se_changez {
    struct se_changez *last;
    struct se_changez *next;
    float f_old_val;
    float f_new_val;

    int cell_num;
    int col_num;
    int field_num;
    int flagged_for_deletion;	/* display "-------" */
    int ray_num;
    int row_num;
    int sweep_num;
    int window_num;
    int typeof_change;
    int second_cell_num;
};

/* c------------------------------------------------------------------------ */

struct solo_examine_info {
  char name_struct[4];	/* "SXMN" */
  int32_t sizeof_struct;
  int32_t window_num;
  int32_t time_modified;
  int32_t changed;
  int32_t always_popup;

  float modify_val;
  int32_t whats_selected;	/* 0 for data */
  int32_t scroll_increment;	/* 19 default */

  // The index in the data of the first ray being displayed in the
  // examine window.  Defaults to 0.

  int32_t ray_num;

  // The number of rays being displayed in the examine window.  Defaults to 5.

  int32_t ray_count;
  int32_t cell_count;		/* 21 default */
  int32_t field_count;		/* 3 or less */

  // I believe this is the index of the first gate being displayed in the
  // examine window currently.  Defaults to 0.

  int32_t at_cell;
  int32_t change_count;
  int32_t typeof_change;
  int32_t row_annotation;
  int32_t col_annotation;
  char fields_list[128];
  char display_format[16];	/* 7.2f */
  int32_t sweep_num;
};
/* c------------------------------------------------------------------------ */

struct examine_control {
  int window_num;
  int actual_ray_num;
  int actual_at_cell;
  int actual_num_cells;
  int actual_ray_count;
  int actual_num_fields;
  int actual_field_num[16];
  int num_data_cols;		/* actual_num_rays * actual_num_fields */
  int num_cols;		/* = non_data_col_count + num_data_cols
				 */
  int col_lims[MAX_EXM_COLUMNS];
  char *col_ptrs[MAX_EXM_COLUMNS];

  int bad_click;
  int heading_row_count;
  int non_data_col_count;
  int num_chars_rng_label;
  int num_chars_per_row;
  int num_chars_per_cell;
  int rays_in_sweep;
  int cells_in_ray;
  int ctr_on_click;

  struct se_changez *tmp_change;

  float click_angle;
  float click_range;

  float rotation_angles[MAX_EXM_COLUMNS];
#ifdef USE_RADX
  Radx::fl32 bad_data[MAX_EXM_COLUMNS]; /* bad data flag for each field */
  double ac_vel[MAX_EXM_COLUMNS];
  Radx::fl32 *data_ptrs[MAX_EXM_COLUMNS];
#else
  float bad_data[MAX_EXM_COLUMNS]; /* bad data flag for each field */
  float ac_vel[MAX_EXM_COLUMNS];
  float *data_ptrs[MAX_EXM_COLUMNS];
#endif
  float *data_space;
  int max_cells;

  float *ranges;
  int max_rngs;

  std::string fields_list;
  std::vector< std::string > fld_ptrs;

  std::string rng_fmt;
  char data_line[256];
  std::string actual_data_format;
  std::string del_str;
};

/* c------------------------------------------------------------------------ */

struct landmark_info0 {
    char name_struct[4];	/* "SLMK" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    int32_t linked_windows[6];
    int32_t landmark_options;
    int32_t reference_frame;
};
/* c------------------------------------------------------------------------ */

struct res_landmark_info0 {
    int32_t landmark_options;
    int32_t reference_frame;
};
/* c------------------------------------------------------------------------ */

struct frame_ctr_info0 {
    char name_struct[4];	/* "SCTR" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    int32_t linked_windows[6];
    int32_t centering_options;
    int32_t reference_frame;
};

/* c------------------------------------------------------------------------ */

struct res_frame_ctr_info0 {
    int32_t centering_options;
    int32_t reference_frame;
};

/* c------------------------------------------------------------------------ */

struct solo_generic_window_struct {
    char name_struct[4];
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
};

typedef struct solo_generic_window_struct *GWS_PTR;

/*
 * most of these descriptors here are meant to go in a
 * save solo window info file
 */
/* c------------------------------------------------------------------------ */

struct solo_sweep_file0 {
    char name_struct[4];	/* "SSFI" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    double start_time;
    float fixed_angle;
    int32_t linked_windows[6];
    int32_t sweep_count;
    int32_t sweep_skip;

    int32_t time_stamp;
    int32_t latest_version;
    int32_t input_fmt;
    int32_t io_type;
    int32_t millisecond;
    int32_t radar_type;
    int32_t version_num;
    char directory_name[128];
    char file_name[128];
    char scan_type[16];
    char radar_name[16];
    int32_t radar_num;
    int32_t sweep_keep_count;
    double stop_time;
};
/* c------------------------------------------------------------------------ */

struct res_solo_sweep_file0 {
    int32_t sweep_count;
    int32_t sweep_skip;

    int32_t time_stamp;
    int32_t latest_version;
    int32_t input_fmt;
    int32_t io_type;
    int32_t millisecond;
    int32_t radar_type;
    int32_t version_num;
    char directory_name[128];
    char file_name[128];
    char scan_type[16];
    char radar_name[16];
    int32_t radar_num;
    int32_t sweep_keep_count;
    double stop_time;
};
/* c------------------------------------------------------------------------ */

struct solo_parameter_info0 {
    char name_struct[4];	/* "SPMI" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    int32_t linked_windows[6];
    char parameter_name[16];
    char palette_name[16];
};
/* c------------------------------------------------------------------------ */

struct res_solo_parameter_info0 {
    char parameter_name[16];
    char palette_name[16];
};

/* c------------------------------------------------------------------------ */

struct solo_radar_name {
    char name_struct[4];	/* "SRDN" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    int32_t linked_windows[SOLO_MAX_WINDOWS];
    char radar_name[16];
};
/* c------------------------------------------------------------------------ */

struct solo_radar_name0 {
    char name_struct[4];	/* "SRDN" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    int32_t linked_windows[6];
    char radar_name[16];
};
/* c------------------------------------------------------------------------ */

struct res_solo_radar_name0 {
    char radar_name[16];
};
/* c------------------------------------------------------------------------ */

struct solo_view_info0 {
    char name_struct[4];	/* "SWVI" */
    int32_t sizeof_struct;
    int32_t window_num;
    int32_t time_modified;
    int32_t changed;
    int32_t always_popup;

    int32_t height_in_pix;
    int32_t width_in_pix;

    int32_t auto_clear;
    int32_t auto_grid;
    int32_t frame_config;
    int32_t linked_windows[6];
    int32_t type_of_annot;
    int32_t type_of_grid;
    int32_t type_of_plot;

    float angular_fill_deg;
    float angular_fill_pct;
    float az_line_int_deg;
    float az_annot_at_km;
    float rng_ring_int_km;
    float rng_annot_at_deg;
    float grid_line_width_pix;
    float horiz_tic_mark_km;
    float vert_tic_mark_km;
    float radar_altitude_km;
    float border_minor_tic_km;
    float border_major_tic_km;

    float magnification;
    float vertical_mag;

    /* there are also three points in space associtated with view info.
     * they are the radar_location, center_of_view, and a landmark.
     * they reside in the solo_window_ptrs struct.
     */
    int32_t offset_to_pisps;
    int32_t num_pisps;
    char message[48];		/* display length of x and y axes in km. */
    /*
     * "SOL_RAD_LOC" id contains the radar location for this window
     * "SOL_WIN_LOC" id contains the location of the center of this window
     * "SOL_LANDMARK" id contains a landmark for this window
     */
    int32_t type_of_centering;
    float rhi_x_ctr_km;		/* center for rhi plots */
    float rhi_y_ctr_km;		/* center for rhi plots */

    float ts_magnification;	/* time series has its own mag. */
    float ts_ctr_km;		/* center for time-series plots */
    float x_tic_mag;
    float y_tic_mag;

};
/* c------------------------------------------------------------------------ */

struct res_solo_view_info0 {
    int32_t type_of_annot;
    int32_t type_of_grid;
    int32_t type_of_plot;

    float angular_fill_deg;
    float angular_fill_pct;
    float az_line_int_deg;
    float az_annot_at_km;
    float rng_ring_int_km;
    float rng_annot_at_deg;
    float grid_line_width_pix;
    float horiz_tic_mark_km;
    float vert_tic_mark_km;
    float radar_altitude_km;
    float border_minor_tic_km;
    float border_major_tic_km;

    float magnification;
    float vertical_mag;

    /* there are also three points in space associtated with view info.
     * they are the radar_location, center_of_view, and a landmark.
     * they reside in the solo_window_ptrs struct.
     */
    int32_t offset_to_pisps;
    int32_t num_pisps;
    char message[48];		/* display length of x and y axes in km. */
    /*
     * "SOL_RAD_LOC" id contains the radar location for this window
     * "SOL_WIN_LOC" id contains the location of the center of this window
     * "SOL_LANDMARK" id contains a landmark for this window
     */
    int32_t type_of_centering;
    float rhi_x_ctr_km;		/* center for rhi plots */
    float rhi_y_ctr_km;		/* center for rhi plots */

    float ts_magnification;	/* time series has its own mag. */
    float ts_ctr_km;		/* center for time-series plots */
    float x_tic_mag;
    float y_tic_mag;

};
/* c------------------------------------------------------------------------ */
 
struct ts_ray_info {
    short ray_num;
    short sweep_num;
};
/* c------------------------------------------------------------------------ */
 
struct ts_sweep_info {
  int dir_num;
  int ray_count;
  char directory[128];
  char filename[88];
};
/* c------------------------------------------------------------------------ */
 
struct ts_ray_table {
    int max_ray_entries;
    int max_sweep_entries;
    int max_list_entries;
    int max_sxm_entries;
    int ray_count;
    int sweep_count;
    struct ts_ray_info *tsri_top;
    struct ts_sweep_info *tssi_top;
    struct ts_ray_info **tsri_list;
    struct ts_ray_info *tsri_sxm;
};
/* c------------------------------------------------------------------------ */
 
struct solo_window_ptrs {
  int32_t window_num;
  int32_t time_modified;

  float parameter_scale;
  float parameter_bias;

  int32_t active;
  int32_t changed;
  int32_t ddir_radar_num;
  int32_t file_id;
  int32_t menus_popped_up;
  int32_t no_more_data;
  int32_t num_colors_in_table;
  int32_t parameter_num;
  int32_t parameter_bad_val;
  int32_t prev_time_stamp;
  int32_t selected_sweep_num;

  int32_t annotation_color_num;
  int32_t background_color_num;
  int32_t bnd_alert_num;
  int32_t bnd_color_num;
  int32_t bnd_last_num;
  int32_t emphasis_color_num;
  int32_t exceeded_color_num;
  int32_t grid_color_num;
  int32_t lock_color_num;
  int32_t missing_data_color_num;

  struct solo_plot_lock *lock;
  struct solo_sweep_file *sweep;
  struct solo_radar_name *radar;
  struct solo_parameter_info parameter;
  SiiPalette *palette;
  struct solo_view_info *view;

  PointInSpace radar_location;
  PointInSpace center_of_view;
  PointInSpace landmark;

  struct solo_field_vals *field_vals;

  // Look-up tables that are used to relate the image information with the
  // color to use.  The algorithm used for rasterizing the data requires that
  // the data rays have uniform gate spacing.  The data_cell_lut table maps
  // the uniform gate-spacing ray to the actual data gate (the index is the
  // uniform gate-spacing ray index, the value is the actual data gate index).
  // This table is used to get the data value for each rasterized ray.  This
  // data value is then used as the index in the data_color_lut table to get
  // the color value for that gate in the rasterized ray.

  u_int32_t *data_color_lut;	/* 64K*sizeof(int32_t) */
  short *data_cell_lut;

  // This will replace data_color_lut above when the Radx refactoring is
  // finished

  ColorTable color_table;
  
  std::string show_file_info;
  float uniform_cell_spacing;
  float uniform_cell_one;
  int number_cells;
  u_int32_t *cell_colors;
  std::string top_line;
  unsigned char *color_bar;
  float clicked_angle;
  float clicked_range;
  PointInSpace *pisp_click;
  int lock_num;
  int clicked_frame_num;
  int clicked_list_id;
  int clicked_list_char_num;
  int clicked_button_id;
  int clicked_list_entry_num;

  struct solo_window_ptrs *next_lock;
  struct solo_window_ptrs *next_sweep;
  struct solo_window_ptrs *lead_sweep;
  SiiImage *image;
  SiiImage *big_image;

  double landmark_x_offset;	/* meters */
  double landmark_y_offset;	/* meters */

  struct landmark_info *landmark_info;
  struct frame_ctr_info *frame_ctr_info;
  PointInSpace clicked_ctr;
  int clicked_ctr_of_frame;
  int sweep_file_modified;
  int scan_mode;

  struct solo_examine_info *examine_info; 
  std::vector< std::string > examine_list;
  struct se_changez *changez_list;
  int num_changez;
  struct examine_control *examine_control;

  std::vector< std::string > list_field_vals;
  std::string start_time_text;
  std::string stop_time_text;
  struct ts_ray_table *tsrt;	/* ray table for time series plots */
  struct ts_ray_info *clicked_tsri;
  int clicked_x_pxl;
  int clicked_y_pxl;

  float color_bar_min_val;
  float color_bar_max_val;
  float emphasis_min;
  float emphasis_max;
  double d_prev_time_stamp;
  double d_sweepfile_time_stamp;
  double d_sweepfile_time_inc;

  int swpfi_filter_set;
  bool swpfi_time_sync_set;
  double clicked_time;
  int magic_rng_annot;
  float filter_fxd_ang;
  float filter_tolerance;
  std::string filter_scan_modes;
  int color_bar_location;
  int color_bar_symbols;
};

typedef struct solo_window_ptrs *WW_PTR;

/* c------------------------------------------------------------------------ */

struct solo_perusal_info {
  int active_windows[SOLO_MAX_WINDOWS];

  struct solo_window_ptrs *solo_windows[SOLO_TOTAL_WINDOWS];

  std::vector< std::string > list_sweeps;
  std::vector< std::string > list_radars;
  std::vector< std::string > list_parameters;

  int num_locksteps;
  int first_ww_for_lockstep[SOLO_MAX_WINDOWS];
  int sweeps_in_lockstep[SOLO_MAX_WINDOWS];
  std::vector< std::string > list_all_files;
  std::vector< std::string > list_select_files;
};

/* c------------------------------------------------------------------------ */

# define   SFV_GARBAGE 0x1
# define    SFV_CENTER 0x2

struct solo_field_vals {
    struct solo_field_vals *next;
    struct solo_field_vals *last;
    float earth_r;
    float lat;
    float lon;
    float alt;
    float az;
    float el;
    float rot_ang;
    float rng;
    float bad_flag;
    float field_vals[7];
    int num_rays;
    int num_vals;
    int state;
    int items_per_ray;
    int sizeof_item;
    double time;
    char **list;
    float tilt;
    float ranges[7];
    float heading;
    float agl;
};

/* c------------------------------------------------------------------------ */

/* c...mark */

/*
 * zeb colortables are in "/zeb/lib/colortables/"
 * colortable input routines and ui code are in "/zeb/src/dm/"
 *
 * ui_perform(char *cmd); 
 *     e.g. ui_perform("read /rdss/solo/colors/original_solo.colors");
 *
 * see state DMC_COLORTABLE (22) in dm.state plus the following code in dm.c
 *
 *	   case DMC_COLORTABLE:
 *	   	dc_Define (UPTR (cmds[1]));
 *		break;
 * 
 * dc_Define is in dm_color.c as is dc_InTable and dc_ColorValue
 * 
 */


# ifdef FYI

/*
 * for information only--do not use!
 */

/*
 * The internal format of a zeb color table.
 */
# define MAXCT 256		/* Max number of colors			*/
typedef struct color_table
{
	char	ct_name[40];		/* The name of this table	*/
	int	ct_ncolor;		/* The number of colors		*/
	XColor	*ct_colors;		/* The actual color entries	*/
} CTable;

/* c------------------------------------------------------------------------ */
typedef struct _XImage {
    int width, height;          /* size of image */
    int xoffset;                /* number of pixels offset in X direction */
    int format;                 /* XYBitmap, XYPixmap, ZPixmap */
    char *data;                 /* pointer to image data */
    int byte_order;             /* data byte order, LSBFirst, MSBFirst */
    int bitmap_unit;            /* quant. of scanline 8, 16, 32 */
    int bitmap_bit_order;       /* LSBFirst, MSBFirst */
    int bitmap_pad;             /* 8, 16, 32 either XY or ZPixmap */
    int depth;                  /* depth of image */
    int bytes_per_line;         /* accelarator to next line */
    int bits_per_pixel;         /* bits per pixel (ZPixmap) */
    u_int32_t red_mask;     /* bits in z arrangment */
    u_int32_t green_mask;
    u_int32_t blue_mask;
    XPointer obdata;            /* hook for the object routines to hang on */
    struct funcs {              /* image manipulation routines */
        struct _XImage *(*create_image)();
#if NeedFunctionPrototypes
        int (*destroy_image)        (struct _XImage *);
        u_int32_t (*get_pixel)  (struct _XImage *, int, int);
        int (*put_pixel)            (struct _XImage *, int, int, u_int32_t);
        struct _XImage *(*sub_image)(struct _XImage *, int, int, unsigned int, unsigned 
int);
        int (*add_pixel)            (struct _XImage *, int32_t);
#else
        int (*destroy_image)();
        u_int32_t (*get_pixel)();
        int (*put_pixel)();
        struct _XImage *(*sub_image)();
        int (*add_pixel)();
#endif
        } f;
} XImage;

# endif



#endif /* ifndef SOLO_WINDOW_STRUCTS_H */



