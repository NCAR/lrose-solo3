/* 	$Id$	 */

#ifndef SII_ENUMS_H
#define SII_ENUMS_H


/*
 * messages from the control buttons
 */
enum {
     BACKWARDS_FOREVER  = 1 << 1 ,
	BACKWARDS_ONCE  = 1 << 2 ,
      REPLOT_LOCK_STEP  = 1 << 3 ,
     REPLOT_THIS_FRAME  = 1 << 4 ,
       FORWARD_FOREVER  = 1 << 5 ,
	  FORWARD_ONCE  = 1 << 6 ,
	    REPLOT_ALL  = 1 << 7 ,
    FORWARD_NEXT_FIXED  = 1 << 8 ,
  BACKWARDS_NEXT_FIXED  = 1 << 9 ,
   RECENTER_AND_REPLOT  = 1 << 10,
};

/* types of plots
 */
enum {
      SOLO_R_THETA_PLOT = 0,
 SOLO_RECONSTRUCTED_RHI = 1 << 1,
 RHI_PLOT_RIGHT_TO_LEFT = 1 << 2,
       SOLO_TIME_SERIES = 1 << 3,
           TS_PLOT_DOWN = 1 << 4,
           TS_AUTOMATIC = 1 << 5,
  TS_PLOT_RIGHT_TO_LEFT = 1 << 6,
        TS_MSL_RELATIVE = 1 << 7,
     TS_CONTIGUOUS_TIME = 1 << 8,
};

# endif
