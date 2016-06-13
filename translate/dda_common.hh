/* created using cproto */
/* Tue Jun 21 22:05:04 UTC 2011*/

#ifndef dda_common_hh
#define dda_common_hh

#include <string>

#include <dd_general_info.h>
#include "input_limits.hh"
#include <time_dependent_fixes.h>
#include <Comment.h>
/* dda_common.c */

extern void solo_set_halt_flag(void);
extern void solo_clear_halt_flag(void);
extern int solo_halt_flag(void);
extern void dd_set_control_c(void);
extern void dd_reset_control_c(void);
extern int dd_control_c(void);
extern int dd_catalog_only_flag(void);
extern struct dd_stats *dd_stats_reset(void);
extern char *dd_stats_sprintf(void);
extern int dd_solo_flag(void);
extern int dd_set_solo_flag(int val);
extern int dd_assign_radar_num(char *radar_name);
extern struct time_dependent_fixes *dd_clean_tdfs(int *fx_count);
extern void dd_close(int fid);
extern struct dd_comm_info *dd_first_comment(void);
extern struct dd_general_info *dd_get_structs(int radar_num);
extern void dd_intset(void);
extern struct dd_comm_info *dd_last_comment(void);
extern int dd_min_rays_per_sweep(void);
extern int dd_max_rays_per_sweep(void);
extern int dd_max_sweeps_per_volume(void);
extern int dd_min_sweeps_per_volume(void);
extern struct comment_d *dd_return_comment(int num);
extern int dd_return_num_comments(void);
extern void dd_reset_comments(void);
extern struct dd_comm_info *dd_next_comment(struct comment_d *comm, int little_endian);
extern struct time_dependent_fixes *dd_kill_tdf(struct time_dependent_fixes *fx, int *fx_count);
extern int dd_num_radars(void);
extern int dd_open(const std::string &dir, const std::string &file_name);
extern int dd_output_flag(int val);
extern int dd_position(int fid, int offset);
extern void dd_rename_swp_file(struct dd_general_info *dgi);
extern struct dd_input_filters *dd_return_difs_ptr(void);
extern int dd_return_radar_num(char *radar_name);
extern struct dd_stats *dd_return_stats_ptr(void);
extern struct time_dependent_fixes *dd_time_dependent_fixes(int *fx_count);
extern void dd_unlink(const std::string &full_path_name);
extern DGI_PTR dd_window_dgi(int window_num);
extern void dd_write(int fid, char *buf, int size);
extern DGI_PTR dgi_last(void);
extern void difs_terminators(DGI_PTR dgi, struct dd_input_filters *difs, struct dd_stats *dd_stats);
extern void do_print_dd_open_info(void);
extern void dont_print_dd_open_info(void);
extern std::string return_dd_open_info(void);

void dd_cchandl(int signal);
	
#endif
