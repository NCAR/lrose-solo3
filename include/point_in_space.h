/* 	$Id$	 */
#ifndef point_in_space_h
#define point_in_space_h

# ifndef PISP
# define PISP
/* set this bit in state if there is a lat/lon/alt/earthr */
# define      PISP_EARTH   0x00000001

/* set this bit in state if x,y,z vals relative to lat/lon */
# define      PISP_XYZ     0x00000002

/* set this bit in state if az,el,rng relative to lat/lon */
# define      PISP_AZELRG  0x00000004

/* set this bit in state for aircraft postitioning relative to lat/lon
 * rotation angle, tilt, range, roll, pitch, drift, heading
 */
# define       PISP_AIR     0x00000008
# define PISP_PLOT_RELATIVE 0x00000010
# define   PISP_TIME_SERIES 0x00000020

struct point_in_space {
    char name_struct[4];	/* "PISP" */
    int32_t sizeof_struct;

    double time;		/* unix time */
    double earth_radius;
    double latitude;
    double longitude;
    double altitude;

    float roll;
    float pitch;
    float drift;
    float heading;
    float x;
    float y;
    float z;
    float azimuth;
    float elevation;
    float range;
    float rotation_angle;
    float tilt;

    int32_t cell_num;
    int32_t ndx_last;
    int32_t ndx_next;
    int32_t state;			/* which bits are set */

    char id[16];
};
# endif
#endif
