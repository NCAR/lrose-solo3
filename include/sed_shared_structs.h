/* 	$Id$	 */
# ifndef SED_SHARED_STRUCTS_H
# define SED_SHARED_STRUCTS_H

#include <dd_defines.h>
#include <PointInSpace.hh>

/* c------------------------------------------------------------------------ */

struct solo_click_info {
    int which_widget;		/* widget id */
    int which_list;		/* list id */
    int which_widget_button;
    int which_mouse_button;
    int which_list_entry;
    int which_character;

    char click_string[88];
    char command_buffer[88];

    int frame;
    int32_t x;			/* in meters */
    int32_t y;			/* in meters */
    int32_t z;			/* in meters */

    int second_list_entry;
    int32_t ival0;
    int32_t ival1;
    float f_val0;
    float f_val1;
};
/* c------------------------------------------------------------------------ */

struct swp_file_input_control
{
  double start_time;
  double stop_time;
  int frame;			/* from which frame the source
				 * data widget was invoked */
  
  int version;		/* -1 to catch earliest version
			 * 99999 to catch latest version
			 */
  std::string radar_name;
  std::string directory;
  int radar_num;
  /*
   * items from the source data info widget
   */
  int new_version_flag;	/* non-zero creates a new version
			 * need to be able to toggle on and of
			 * in case it's hit inadvertantly
			 * i.e. need an on/off indicator */
  std::string directory_text;
  std::string radar_names_text;
  std::string first_sweep_text;
  std::string last_sweep_text;
  std::string version_text;
  int clicked_frame;		/* used as a flag (frame number plus one)
				 * nonzero indicates sweep file
				 * information has come from the frame
				 * from which the editor widget was called up
				 * If it's set to zero we are either in batch
				 * mode or the user has modified the sfic
				 * widget */
};
/* c------------------------------------------------------------------------ */

struct sed_command_files
{
  std::string directory_text;
  std::string file_name_text;
  std::string comment_text;
  int omit_source_file_info;
};
/* c------------------------------------------------------------------------ */

struct solo_frame_state_files
{
  std::string directory_text;
  std::string file_name_text;
  std::string comment_text;
  int omit_sweep_info;
};
/* c------------------------------------------------------------------------ */

# define	  VERT_XY_PLANE 0
# define	  HORZ_XY_PLANE 1
  
# define   BND_FIX_RADAR_INSIDE 0x0001
# define  BND_FIX_RADAR_OUTSIDE 0x0002

struct boundary_header {
    char name_struct[4];	/* "BDHD" */
    int32_t sizeof_struct;
    int32_t time_stamp;
    int32_t offset_to_origin_struct; /* should be a PISP */
    int32_t type_of_info;
    int32_t num_points;
    int32_t offset_to_first_point;
    int32_t sizeof_boundary_point;
    int32_t xy_plane_horizontal;
    int32_t open_boundary;
    int32_t version;
    char radar_name[12];
    char user_id[16];
    int32_t force_inside_outside;

};

struct bnd_point_mgmt {
    struct bnd_point_mgmt *last;
    struct bnd_point_mgmt *next;
    int32_t x;			/* meters */
    int32_t y;			/* meters */
    int32_t z;			/* meters */
    int32_t r;			/* range in meters */

    struct bnd_point_mgmt *last_intxn;
    struct bnd_point_mgmt *next_intxn;

    struct bnd_point_mgmt *x_parent;
    struct bnd_point_mgmt *x_left;
    struct bnd_point_mgmt *x_right;

    struct bnd_point_mgmt *y_parent;
    struct bnd_point_mgmt *y_left;
    struct bnd_point_mgmt *y_right;

    float slope;
    float slope_90;		/* slope of line perpendicular
				 * to this line*/
    float len;

    int32_t x_mid;			/* midpoint of the line */
    int32_t y_mid;			/* midpoint of the line */
    int32_t dy;			/* last->y - this->y */
    int32_t dx;			/* last->x - this->x */
    int32_t rx;			/* intersection with a ray */

    int bnd_num;
    int segment_num;
    int what_happened;
    int which_frame;
    int which_side;
    int mid_level;
    PointInSpace *pisp;

    int32_t _x;		/* x shifted */
    int32_t _y;		/* y shifted */
    int32_t _z;		/* z shifted */
    /*
     * time series stuff...range and time will be in pisp.
     */
    double t_mid;
    float dt;
    float r_mid;
    float dr;
  
    int screen_x;
    int screen_y;
};

struct one_boundary {
    struct one_boundary *last;
    struct one_boundary *next;

    struct bnd_point_mgmt *top_bpm;
    struct bnd_point_mgmt *x_mids;
    struct bnd_point_mgmt *y_mids;
    struct bnd_point_mgmt *first_intxn;
    struct bnd_point_mgmt *next_segment;
    struct bnd_point_mgmt *last_line;
    struct bnd_point_mgmt *last_point;

    float r0;
    float r1;
    int num_segments;
    int num_intxns;
    int num_points;
    double min_x;			/* meters */
    double max_x;			/* meters */
    double min_y;			/* meters */
    double max_y;			/* meters */
    double min_z;			/* meters */
    double max_z;			/* meters */
    int radar_inside_boundary;
    int open_boundary;
    struct boundary_header *bh;
};

# endif

