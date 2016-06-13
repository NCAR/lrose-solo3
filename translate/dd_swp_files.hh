/* created using cproto */
/* Tue Jun 21 22:05:24 UTC 2011*/

#ifndef dd_swp_files_hh
#define dd_swp_files_hh

#include <dd_general_info.h>
/* dd_swp_files.c */

extern void dd_set_unique_vol_id(void);
extern int dd_sizeof_prev_file(void);
extern void dd_dump_headers(struct dd_general_info *dgi);
extern void dd_dump_ray(struct dd_general_info *dgi);
extern void dd_flush(int flush_radar_num);
extern void dd_insert_headers(struct dd_general_info *dgi);
extern void dd_new_vol(struct dd_general_info *dgi);

#endif
