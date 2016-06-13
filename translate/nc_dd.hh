/* created using cproto */
/* Tue Jun 21 22:05:50 UTC 2011*/

#ifndef nc_dd_hh
#define nc_dd_hh

#include <dd_time.h>
/* nc_dd.c */

extern void nc_dd_conv(int interactive_mode);
extern int nc_ini(void);
extern double ncswp_time_stamp(char *fname, DD_TIME *dts);
extern int nc_access_swpfi(char *full_path_name);
extern int set_gri_stuff(void);
extern int field_index_num(char *name);
extern int alias_index_num(char *name);

#endif
