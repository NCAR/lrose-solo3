/* created using cproto */
/* Tue Jun 21 22:05:35 UTC 2011*/

#ifndef elda_dd_hh
#define elda_dd_hh

#include <dd_general_info.h>
#include <FieldLidar.h>
#include <dorade_share.h>
#include <dd_io_structs.h>
#include <time.h>
#include "eldh.h"
/* elda_dd.c */

extern double eld_fixed_angle(struct dd_general_info *dgi);
extern void eld_dd_conv(int interactive);
extern void cspd2celv(struct dds_structs *dds, double spacing);
extern int dde_desc_fwd_sync(char *aa, int ii, int jj);
extern void dor_print_flib(struct field_lidar_i *flib, struct solo_list_mgmt *slm);
extern void dor_print_lidr(struct lidar_d *lidr, struct solo_list_mgmt *slm);
extern void eld_dump_raw_header(struct dd_general_info *dgi, struct input_read_info *irq, struct io_packet *vol_dp, struct solo_list_mgmt *slm);
extern void eld_dump_this_ray(struct dd_general_info *dgi, struct solo_list_mgmt *slm);
extern void eldx_ini(void);
extern void eld_interest(struct dd_general_info *dgi, int verbosity, char *preamble, char *postamble);
extern int eld_inventory(void);
extern int eld_is_new_sweep(struct dd_general_info *dgi);
extern void eld_lidar_avg(DGI_PTR dgi);
extern void eld_lut(DGI_PTR dgi, int pn, short *dst);
extern int eld_next_ray(void);
extern int xml_set_version(char *buf, int *bsize);
extern int xml_open_element(char *buf, int *bsize, char *name);
extern int xml_close_element(char *buf, int *bsize, char *name);
extern void xml_append_attribute(char *buf, int *bsize,
				 const char *name, const char *value);
extern int xml_append_comment(char *buf, int *bsize, char *str);
extern int xml_end_attributes(char *buf, int *bsize, int empty_element);
extern void eld_positioning(void);
extern void eld_print_cspd(struct cell_spacing_d *cspd, struct solo_list_mgmt *slm);
extern void eld_print_frad(struct field_parameter_data *frad, struct solo_list_mgmt *slm);
extern void eld_print_ldat(struct lidar_parameter_data *ldat, struct solo_list_mgmt *slm);
extern void eld_print_stat_line(int count, struct dd_general_info *dgi);
extern void eld_radar_rename(char *name);
extern void eld_reset(void);
extern struct eldora_unique_info *eld_return_eui(void);
extern int eld_select_ray(struct dd_general_info *dgi);
extern int eld_stuff_data(struct dd_general_info *dgi, int parm_num);
extern void eld_init_sweep(struct dd_general_info *dgi,  time_t current_time,  int new_vol);
extern void eld_stuff_ray( struct dd_general_info *dgi, time_t current_time);
extern int ts_xml_out (struct dd_general_info *dgi, FILE *strm
		, struct time_series *tmsr, struct waveform_d *wave);

#endif
