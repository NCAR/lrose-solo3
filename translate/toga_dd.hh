/* created using cproto */
/* Tue Jun 21 22:06:09 UTC 2011*/

#ifndef toga_dd_hh
#define toga_dd_hh

#include <toga.h>
#include <dd_time.hh>
/* toga_dd.c */

extern void toga_data_types(void);
extern void toga_dd_conv(void);
extern void toga_gen_luts(void);
extern void toga_grab_header_info(void);
extern void toga_nab_data(void);
extern void toga_nab_info(void);
extern int toga_next_ray(void);
extern int toga_select_ray(void);
extern int toga_ok_ray(void);
extern void toga_print_stat_line(int count);
extern double toga_time_stamp(struct toga_ray_header *trah, DD_TIME *dts);
extern void toga_update_header_info(void);

//extern void toga_ini(int32_t start_time, int32_t stop_time, char *dir, char *toga_file, char *project_name, char *site_name, char *output_flags);
void toga_ini(void);

#endif
