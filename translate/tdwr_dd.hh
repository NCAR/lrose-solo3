/* created using cproto */
/* Tue Jun 21 22:06:07 UTC 2011*/

#ifndef tdwr_dd_hh
#define tdwr_dd_hh

#include <dd_io_structs.h>
#include <dorade_share.h>
/* tdwr_dd.c */

extern void tdwr_dd_conv(int interactive_mode);
extern void tdwr_dump_this_ray(struct solo_list_mgmt *slm);
extern void tdwr_ini(void);
extern int tdwr_inventory(struct input_read_info *iri);
extern void tdwr_map_structs(char *buf);
extern void tdwr_nab_data(void);
extern void tdwr_nab_dataLE(void);
extern int tdwr_next_ray(void);
extern void tdwr_positioning(void);
extern void tdwr_print_cbdh(struct solo_list_mgmt *slm);
extern void tdwr_print_stat_line(int count);
extern void tdwr_reset(void);
extern int tdwr_select_ray(void);
extern void tdwr_setup_fields(void);

#endif
