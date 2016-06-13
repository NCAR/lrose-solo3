/* created using cproto */
/* Thu Jul  7 17:55:13 UTC 2011*/

#ifndef sp_accepts_hh
#define sp_accepts_hh

#include <sii_widget_content_structs.h>

extern void sp_cpy_lmrk_info(int src_frme, int dst_frme);
extern void sp_cpy_ctr_info(int src_frme, int dst_frme);
extern void solo_cpy_lock_info(int src_frme, int dst_frme);
extern void solo_cpy_parameter_info(int src_frme, int dst_frme);
extern void solo_cpy_sweep_info(int src_frme, int dst_frme);
extern void solo_cpy_view_info(int src_frme, int dst_frme);
extern std::string solo_gen_file_info(int frme);
extern void sp_set_landmark_info(const struct landmark_widget_info &lwi);
extern void sp_set_center_of_view_info(struct view_widget_info &pvi,
				       struct centering_widget_info &cwi);
extern int solo_set_view_info(struct view_widget_info &pvi,
			      const struct landmark_widget_info &lwi,
			      struct centering_widget_info &cwi);

#endif
