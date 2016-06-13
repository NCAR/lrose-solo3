/* 	$Id$	 */


#ifndef xyraster_h
#define xyraster_h

# ifndef CART_ANGLE
# define CART_ANGLE(x) ((double)90.-(x))
# endif
# define RADIANZ(x) ((double)(x)*(double)0.017453292)
# define SIN45 .7071067812
# define SIN30 .5
# define SIN15 .258819045
# define GATE_NUM(r, r0, rcp_gs) (((r)-(r0))*(rcp_gs) +.5)
# define ROUND_IT(x) ((x)+.5)
# define IFIX(x) ((int)((x)+.5))
# define RXY_SCALE(x) ((int32_t)((x)*65536 +.5))
# define rxy_SCALE(x) (((x)*65536))
# define RXY_FUDGE 8		/* makes the view this much bigger on
				 * each side so the jaggies will
				 * not be visible
				 */
// union rxu {
//     int32_t scaled_val;
//     short val;
// } u;

struct rxy_coords {
    double x;
    double y;
    double gate_num;
};

struct line_segment {
    struct rxy_coords p0;
    struct rxy_coords p1;
    double sin;
    double cos;
    double x_inc;		/* as you move in the y direction */
    double y_inc;		/* as you move in the x direction */
    double gate_num;
    double major_gate_inc;
    double minor_gate_inc;
    int num_iters;
    int num_intxns;
};

/* c------------------------------------------------------------------------ */

struct xyras {

    struct line_segment edge0;
    struct line_segment edge1;

    double angle_ctr;
    double range_ctr;
    double pixels_per_meter;
    double meters_per_pixel;
    /*
     * coordinates in meters to the corners of the image
     */
    struct rxy_coords l_left;	/* view */
    struct rxy_coords u_right;	/* view */
    struct rxy_coords image_l_left;

    struct rxy_coords center;

    double r0;			/* center of first gate in meters */
    double r1;			/* center of last gate in meters */
    double gs;			/* gate spacing in meters */
    double rcp_gs;
    double sin_corner;
    int max_gate;

    int image_width;
    int image_height;
    int view_width;
    int view_height;
    int x_inner_loop;
    int radar_outside;
    int ray_out_of_frame;
    int traverse_negative;
    int ignore_this_ray;

    u_int32_t *colors;	/* one for each cell or gate */
    char *image;		/* where the colors go  */

    struct one_boundary *screen_boundary;
};

typedef union int32_t_short_align {
    int32_t scaled_val;
    short val;
} LS_ALIGN;

#endif
