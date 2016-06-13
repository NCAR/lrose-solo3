/* 	$Id$	 */
/******************************************************************************
*  INCLUDE:           solo_io_defs.h
*
*  DESCRIPTION:       Defines used for correcting data from ELDORA field tapes
*******************************************************************************/
# ifndef SOLO_DEFS_H
# define SOLO_DEFS_H

# define RADIANS(x)  ((x)*0.017453292)
# define DEGREES(x)  ((x)*57.29577951)
# define CART_ANGLE(x) ((double)90.-(x))
# define PI 3.141592654
# define TWOPI 6.283185307
# define MAX_BUFF 32768
# define FORE_RADAR      70
# define AFT_RADAR       65
# define MAX_RAY_NO      1600
# define MAXFIELD 20
# define BOUNDARY_ADD_PT      1
# define BOUNDARY_DELETE_PT   2
# define BOUNDARY_MOVE_PT     3
# define BOUNDARY_INSERT_PT   5
# define PT_DELETION          4
# define HEADER_BLOCKS        25
# define RAY_BLOCKS           15

# define SOLO_EXIT 0x1
# define SOLO_HALT 0x2
# ifndef SET_IN
# define SET_IN &
# endif

/*
 * A macro to make function prototypes a little easier across both STDC and
 * non-STDC implementations.
 */
# ifdef __STDC__
#  define FP(stuff) stuff
# else
#  define FP(stuff) ()
# endif

#endif
