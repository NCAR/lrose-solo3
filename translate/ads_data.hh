/* created using cproto */
/* Tue Jun 21 22:04:58 UTC 2011*/

#ifndef ads_data_hh
#define ads_data_hh

#include <dd_time.h>
#include <Platform.h>
/* ads_data.c */

extern int eld_gpro_fix_asib(DD_TIME *dts, struct platform_i *asib);
extern int gpro_assemble_vals(struct ads_raw_data_que *rdq, struct ads_data *adsd, double pct);
extern double gpro_decode(struct letvar_info *this, double pct_samp, struct ads_raw_data_point *ardp);
extern int gpro_get_vbls(struct ads_raw_data_que *rdq);
extern int gpro_header(char *buf);
extern int gpro_mount_raw(void);
extern int gpro_nab_vbl(char *buf, struct letvar_info *this, struct ads_raw_data_point *ardp);
extern char *gpro_next_rec(struct ads_raw_data_que *rdq, int *status);
extern int gpro_position_err(struct ads_data *adsd);
extern struct ads_raw_data_que *gpro_sync(double time);

#endif
