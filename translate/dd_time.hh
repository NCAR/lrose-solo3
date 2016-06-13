/* created using cproto */
/* Tue Jun 21 22:05:25 UTC 2011*/

#ifndef dd_time_hh
#define dd_time_hh

#include <dd_time.h>

/* dd_time.c */

extern void dd_clear_dts(struct d_time_struct *dts);
extern double d_time_stamp(DD_TIME *dts);
extern DD_TIME *d_unstamp_time(DD_TIME *dts);
void init_d_time (void);

#endif
