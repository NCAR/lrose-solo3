/* created using cproto */
/* Tue Jun 21 22:06:05 UTC 2011*/

#ifndef swp_file_acc_hh
#define swp_file_acc_hh

#include <dd_general_info.h>
#include <input_sweepfiles.h>

extern int dd_absorb_header_info(DGI_PTR dgi);
extern int dd_absorb_ray_info(DGI_PTR dgi);
extern int dd_absorb_rotang_info(DGI_PTR dgi, int gottaSwap);
extern int dd_absorb_seds(DGI_PTR dgi, int gottaSwap);
extern struct dd_input_sweepfiles_v3 *dd_return_sweepfile_structs_v3(void);
extern struct dd_input_sweepfiles_v3 *dd_setup_sweepfile_structs_v3(int *ok);
extern int dd_sweepfile_list_v3(struct unique_sweepfile_info_v3 *usi, double start_time, double stop_time);
extern double dd_ts_start(int frme, int radar_num, double start, int *version, char *info, char *name);
extern void dd_update_ray_info(DGI_PTR dgi);
extern void dd_update_sweep_info(DGI_PTR dgi);
extern int ddswp_last_ray(struct unique_sweepfile_info_v3 *usi);
extern int ddswp_new_sweep_v3(struct unique_sweepfile_info_v3 *usi);
extern struct cfac_wrap *ddswp_nab_cfacs(const char *radar_name);
extern int ddswp_next_ray_v3(struct unique_sweepfile_info_v3 *usi);
extern void ddswp_stuff_ray(DGI_PTR dgi);
extern void ddswp_stuff_sweep(DGI_PTR dgi);
extern int dd_create_dynamic_rotang_info(DGI_PTR dgi);
extern int dd_read_ray_for_dynamic_angle_table(DGI_PTR dgi, int rayNum);

#endif
