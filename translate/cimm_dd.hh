/* created using cproto */
/* Tue Jun 21 22:05:01 UTC 2011*/

#ifndef cimm_dd_hh
#define cimm_dd_hh

/* cimm_dd.c */

extern void cimm_dd_conv(int interactive_mode);
extern void cimm_positioning(void);
extern void cimm_print_header(struct solo_list_mgmt *slm);
extern void cimm_dump_this_ray(struct solo_list_mgmt *slm);
extern int cimm_inventory(struct input_read_info *irq);
extern void cimm_ini(void);
extern void cimm_reset(void);
extern int cimm_select_ray(void);
extern void cimm_nab_data(void);
extern int cimm_next_ray(void);
extern void cimm_print_stat_line(int count);
extern int cimm_isa_new_sweep(void);

#endif
