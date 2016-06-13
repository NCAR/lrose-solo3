/* created using cproto */
/* Fri Jul 15 20:57:41 UTC 2011*/

#ifndef se_bnd_hh
#define se_bnd_hh

#include <string>
#include <vector>

#include <PointInSpace.hh>
#include <sed_shared_structs.h>
#include <UiCommandToken.hh>

#ifdef USE_RADX
#else
extern int se_use_bnd(const std::vector< UiCommandToken > &cmd_tokens);
#endif

extern struct bnd_point_mgmt *se_pop_bpms();
extern void se_push_all_bpms(struct bnd_point_mgmt **bpmptr);
extern void xse_absorb_bnd(void);
extern void xse_add_bnd_pt(struct solo_click_info *sci, struct one_boundary *ob,
			   const bool allow_duplicates = false);
extern void se_append_bpm(struct bnd_point_mgmt **top_bpm, struct bnd_point_mgmt *bpm);
extern void se_bnd_pt_atts(struct bnd_point_mgmt *bpm);
extern int xse_ccw(double x0, double y0, double x1, double y1);
extern void se_clr_all_bnds(void);
extern void se_clr_bnd(struct one_boundary *ob);
extern void se_clr_current_bnd(void);
extern void se_cycle_bnd(void);
extern void se_delete_bnd_pt(struct bnd_point_mgmt *bpm,
			     struct one_boundary *ob);
extern void se_draw_bnd(struct bnd_point_mgmt *bpm,
			const int num, const int erase);
extern struct one_boundary *se_malloc_one_bnd();
extern struct one_boundary *se_return_next_bnd();
extern char *absorb_zmap_bnd(const std::string &ifile, const int skip, int &nbytes );
extern void se_push_bpm(struct bnd_point_mgmt *bpm);
extern int se_radar_inside_bnd(struct one_boundary *ob);
extern void se_redraw_all_bnds();
extern struct one_boundary *se_return_next_bnd();
extern void xse_save_bnd ();
extern int xse_set_intxn(const double x, const double y, const double slope,
			 struct bnd_point_mgmt *bpm, struct one_boundary *ob);
extern void se_shift_bnd(struct one_boundary *ob,
			 const PointInSpace &boundary_radar, 
			 PointInSpace &current_radar, int scan_mode,
			 double current_tilt);
extern int se_sizeof_bnd_set();
extern void se_unpack_bnd_buf(char *buf, int size);
extern void xse_x_insert(struct bnd_point_mgmt *bpm, struct one_boundary *ob);
extern void xse_y_insert(struct bnd_point_mgmt *bpm, struct one_boundary *ob);
extern struct bnd_point_mgmt *se_zap_last_bpm(struct bnd_point_mgmt **top_bpm);
extern int absorb_zmap_pts(struct zmap_points **top, const std::string &ifile);
extern int se_rain_gauge_info(double dlat, double dlon);
extern int xabsorb_zmap_pts(struct zmap_points **top, char * ifile);
extern void se_prev_bnd_set();
extern void se_erase_all_bnds();
extern int xse_find_intxns(double angle, double range, struct one_boundary *ob);
extern int se_merge_intxn(struct bnd_point_mgmt *bpm,  struct one_boundary *ob);
extern void se_ts_find_intxns(double radar_alt, double d_max_range, struct one_boundary *ob, 
					   double d_time, double d_pointing, int automatic, int down, double d_ctr);
extern int xse_num_segments(struct one_boundary *ob);
extern void se_nab_segment(const int num, double &r0, double &r1,
			   struct one_boundary *ob);
extern void se_pack_bnd_set(char *buf);
extern bool se_compare_bnds(char *aa, char *bb, int size);
extern void se_erase_segments (int frame_num, int num_points, struct bnd_point_mgmt *bpm);
extern void se_draw_segments (int frame_num, int num_points, struct bnd_point_mgmt *bpm);

#endif
