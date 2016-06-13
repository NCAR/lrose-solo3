/* created using cproto */
/* Tue Jun 21 22:05:21 UTC 2011*/

#ifndef ddout_hh
#define ddout_hh

#include <input_sweepfiles.h>
/* ddout.c */

extern void ddout_loop(void);
extern int ddout_ini(void);
extern void ddout_sweeps_loop(int time_span);
extern int ddout_really(int fid, char *buf, int len);
extern void ddout_sweep_dump(struct unique_sweepfile_info_v3 *usi);
extern void ddout_vol_data(struct unique_sweepfile_info_v3 *usi, int time_span);
extern int ddout_vol_detector(int volume_interval);
extern void ddout_vols_loop(int volume_interval);
extern int ddout_write(int fid, char *buf, int len);
extern int ddout_hedr_saver(int fid, char *buf, int len);
extern void ddout_headers_dump(struct unique_sweepfile_info_v3 *usix, int all);

#endif
