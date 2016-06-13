/* created using cproto */
/* Tue Jun 21 22:05:52 UTC 2011*/

#ifndef nex_dd_hh
#define nex_dd_hh

#include <nexh.h>
#include <dorade_share.h>
#include <dd_io_structs.h>

/* nex_dd.c */

double nex_conc_to_fp(float *ival);
extern void nex_dd_conv(int interactive_mode);
extern void nex_dump_this_ray(struct solo_list_mgmt *slm);
extern void nexx_ini(void);
extern int nex_nab_site_info(struct solo_list_mgmt *slm);
extern int nex_inventory(struct input_read_info *iri);
extern void nex_nab_data(void);
extern void nexx_nab_data(void);
extern void nexx_new_parms(void);
extern void nex_new_parms(void);
extern char *nex_next_block(void);
extern int nex_next_ray(void);
extern int nex_process_cal_info(void);
extern void nex_update_xtnded_headers(void);
extern void nex_fake_xtnded_headers(void);
extern void nex_positioning(void);
extern void nex_print_dhed(struct digital_radar_data_header *dh, struct solo_list_mgmt *slm);
extern void nex_print_headers(struct solo_list_mgmt *slm);
extern void nex_print_nmh(struct nexrad_message_header *mh, struct solo_list_mgmt *slm);
extern void nex_print_rda(struct rda_status_info *rda, struct solo_list_mgmt *slm);
extern void nex_print_nvst(struct nexrad_vol_scan_title *vst, struct solo_list_mgmt *slm);
extern void nex_print_stat_line(int count);
extern void nex_reset(void);
extern int nex_select_ray(void);
extern double nexrad_time_stamp(int jday, int ms, DD_TIME *dts);

#endif
