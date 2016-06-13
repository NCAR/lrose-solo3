/* created using cproto */
/* Thu Jul  7 17:55:14 UTC 2011*/

#ifndef sp_basics_hh
#define sp_basics_hh

#include <solo_window_structs.h>

extern void solo_clear_busy_signal(void);
extern void solo_set_busy_signal(void);
extern int solo_busy(void);
extern std::string solo_return_radar_name(int frame_num);
extern void solo_activate_frame(int dst_frme);
extern struct solo_perusal_info *solo_return_winfo_ptr(void);
extern void sp_copy_tsri(struct ts_ray_info *a, struct ts_ray_info *b);
extern struct solo_window_ptrs *solo_return_wwptr(int ww_num);
extern void se_set_sfic_from_frame(int frme);
extern void solo_trigger_dir_rescans(const std::string &dir);
extern void solo_ww_top_line(int frme);
extern int sp_max_frames(void);

#endif
