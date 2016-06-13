/* created using cproto */
/* Thu Jul  7 17:55:10 UTC 2011*/

#ifndef sii_perusal_hh
#define sii_perusal_hh

#include <solo2.hh>

extern int displayq(int click_frme, int command);
//extern void sp_nex_sweep_set(int ns, struct linked_frames *sweep_list[]);
//extern int sp_data_loop(struct linked_frames *flink0);
extern int sp_diag_flag(void);
extern void sp_set_diag_flag(int flag);
extern void sp_replot_all(void);
extern int solo_worksheet(void);
extern int sp_nab_fixed_angle_from_fn(char *fn, double *fxd);
extern bool solo_parameter_setup(int frme);
extern void sp_landmark_offsets(int frme);
extern void sp_align_center(int frme);

#endif
