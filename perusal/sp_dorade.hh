/* created using cproto */
/* Thu Jul  7 17:55:15 UTC 2011*/

#ifndef sp_dorade_hh
#define sp_dorade_hh

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

extern void solo_cell_lut(int frme);
#ifdef USE_RADX
extern void solo_color_cells(int frme, const RadxRay *ray);
#else
extern void solo_color_cells(int frme);
#endif

#endif
