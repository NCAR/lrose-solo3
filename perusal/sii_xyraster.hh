/* created using cproto */
/* Thu Jul  7 17:55:12 UTC 2011*/

#ifndef sii_xyraster_hh
#define sii_xyraster_hh

#include <xyraster.h>

extern int frme_intxns(struct xyras *rxy, struct line_segment *edge);
extern void init_xyraster(int frme);
extern int ray_raster_setup(int frme, double angle0, double angle1, struct xyras *rxy);
extern struct xyras *return_xyras(int frme);
extern double sp_meters_per_pixel(void);
extern void swap_edges(struct line_segment *edge0, struct line_segment *edge1);
extern void swap_ends(struct line_segment *edge);
extern void xx_inner_loop(struct xyras *rxy);
extern void yy_inner_loop(struct xyras *rxy);

#endif
