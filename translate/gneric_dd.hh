/* created using cproto */
/* Tue Jun 21 22:05:44 UTC 2011*/

#ifndef gneric_dd_hh
#define gneric_dd_hh

#include <dd_general_info.h>
#include <generic_radar_info.h>

/* gneric_dd.c */

extern void dd_celv_update(struct dds_structs *dds);
extern void dd_init_sweep(struct dd_general_info *dgi, int new_vol);
extern void dd_parms_update(struct dd_general_info *dgi);
extern void dd_radd_update(struct dds_structs *dds);
extern void dd_stuff_ray(void);
extern void dd_vold_update(struct dd_general_info *dgi);
extern struct generic_radar_info *return_gri_ptr(void);

#endif
