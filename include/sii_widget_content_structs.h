/* 	$Id$	 */


#ifndef SII_WIDGET_CONTENT_STRUCTS_H
#define SII_WIDGET_CONTENT_STRUCTS_H

#include <solo_window_structs.h>
/* c------------------------------------------------------------------------ */

struct examine_widget_info {
  int window_num;
  int whats_selected;
  int scroll_increment;
  int ray_num;
  int at_cell;
  int ray_count;
  int cell_count;
  int change_count;
  int typeof_change;
  int row_annotation;
  int col_annotation;

  std::string fields_list;
  std::string modify_val;
  std::string display_format;
};

/* c------------------------------------------------------------------------ */

struct landmark_widget_info {
    int frame_num;
    int32_t linked_windows[SOLO_MAX_WINDOWS];
    int options;
    int reference_frame;
    int changed;
    double latitude;
    double longitude;
    double altitude;
};
/* c------------------------------------------------------------------------ */

struct centering_widget_info {
    int frame_num;
    int32_t linked_windows[SOLO_MAX_WINDOWS]; /* wwptr->frame_ctr_info->linked_windows */
    int options;		/* wwptr->frame_ctr_info->centering_options */
    int reference_frame;	/* wwptr->frame_ctr_info->reference_frame */
    int changed;
    double latitude;		/* wwptr->center_of_view->latitude */
    double longitude;		/* wwptr->center_of_view->longitude */
    double altitude;		/* wwptr->center_of_view->altitude */
    float az_of_ctr;		/* wwptr->center_of_view->azimuth */
    float rng_of_ctr;		/* wwptr->center_of_view->range */
};
/* c------------------------------------------------------------------------ */

struct lock_step_info {
    int frame_num;
    int changed;
    int32_t linked_windows[SOLO_MAX_WINDOWS]; /* wwptr->lock->linked_windows */
};

/* c------------------------------------------------------------------------ */

struct view_widget_info {
    int frame_num;
    int changed;
    int32_t linked_windows[SOLO_MAX_WINDOWS]; /* wwptr->view->linked_windows */
    int auto_clear;		/* YES or NO wwptr->view->auto_clear */
    int auto_grid;		/* YES or NO wwptr->view->auto_grid */
    int type_of_centering;
    int type_of_grid;		/* wwptr->view->type_of_grid */
    int type_of_plot;
    int frame_config_num;
    int ctr_from_last_click;	/* YES or NO */
    int width;
    int height;
    int left_attach;
    int right_attach;
    int top_attach;
    int bottom_attach;
    int magic_rng_annot;

    float magnification;	/* wwptr->view->magnification */
    float az_line_int_deg;	/* wwptr->view->az_line_int_deg */
    float rng_ring_int_km;	/* wwptr->view->rng_ring_int_km */
    float horiz_tic_mark_km;	/* wwptr->view->horiz_tic_mark_km */
    float vert_tic_mark_km;	/* wwptr->view->vert_tic_mark_km */
    /*
     * Grid size comes from wwptr->view->message
     */
    float az_annot_at_km;	/* wwptr->view->az_annot_at_km */
    float rng_annot_at_deg;	/* wwptr->view->rng_annot_at_deg */

    float rhi_x_ctr_km;		/* center for rhi plots */
    float rhi_y_ctr_km;		/* center for rhi plots */

    float ts_magnification;	/* time series has its own mag. */
    float ts_ctr_km;		/* center for time-series plots */
    float x_tic_mag;
    float y_tic_mag;
    float angular_fill_pct;
};
/* c------------------------------------------------------------------------ */




# endif 






