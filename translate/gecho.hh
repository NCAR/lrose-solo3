/* created using cproto */
/* Tue Jun 21 22:05:42 UTC 2011*/

#ifndef gecho_hh
#define gecho_hh

#include <dd_general_info.h>
/* gecho.c */

extern void dd_gecho(DGI_PTR dgi);
extern void dd_gecho_intxn(DGI_PTR dgi, int fid, double min_rot_angle, double max_rot_angle, double earth_radius, int min_gates, int min_gatex, double reasonable_gecho, char *gecho_refl, char *gecho_vel, int radar_num, char *gecho_ncp);

#endif
