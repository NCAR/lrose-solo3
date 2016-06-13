/* created using cproto */
/* Tue Jun 21 22:06:02 UTC 2011*/

#ifndef sigm_dd_hh
#define sigm_dd_hh

#include <dd_defines.h>
/* sigm_dd.c */

extern void sigmet_dd_conv(void);
extern void sigmet_ini(void);
extern void sigmet_nab_data(void);
extern void sigmet_nab_info(void);
extern int sigmet_next_ray(void);
extern int sigmet_select_ray(void);
extern void sigmet_print_stat_line(int count);
extern int ingest_ingest_headers(char *tbuf);
extern void grab_summary_header(char *buf);
extern int sig_read(int fid, char *buf, int size);
extern short sig_swap2(twob *ov);
extern int32_t sig_swap4(fourb *ov);

#endif
