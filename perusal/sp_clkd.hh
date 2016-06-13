/* created using cproto */
/* Thu Jul  7 17:55:14 UTC 2011*/

#ifndef sp_clkd_hh
#define sp_clkd_hh

#include <solo_window_structs.h>
#include <sed_shared_structs.h>
#include <PointInSpace.hh>

extern void solo_new_list_field_vals(const int frame_index);
extern void solo_put_field_vals(struct dd_general_info *dgi, int param_num, double angle, double range, struct solo_field_vals *sfv, int state);
extern void sp_ts_seek_field_vals(int frme);
extern int sp_ts_ray_list(int frme, int xpos, int nr);
extern void sp_ts_data_click(int frme, int xpos, int ypos);
extern void sp_data_click(int frme, double x, double y);
extern void sp_locate_this_point(struct solo_click_info *sci, struct bnd_point_mgmt *bpm);
extern void sp_rtheta_screen_to_xyz(const int frme, PointInSpace &radar,
				    const PointInSpace &pisp);
extern void sp_seek_field_vals(int frme);
extern void sp_xyz_to_rtheta_screen(const int frme, const PointInSpace &radar,
				    PointInSpace &pisp);

#endif
