/* created using cproto */
/* Tue Jun 21 22:05:14 UTC 2011*/

#ifndef ddin_hh
#define ddin_hh

#include <dd_general_info.h>
#include <dd_io_structs.h>
#include <dd_io_mgmt.hh>
#include <dorade_share.h>
#include <time.h>
#include <dd_defines.h>

/* ddin.c */

struct dor_inidividual_radar_info {
    struct dd_general_info *dgi;
    int radar_num;
    int ray_count;
    int sweep_count;
    int vol_count;
    int sweep_count_flag;
    int vol_count_flag;
    int sweep_skip_num;
    int use_this_sweep;
    char radar_name[12];
    char *parm_names[MAX_PARMS][12];
    struct sweepinfo_d swib;
    float fixed_angle;
    float ref_fixed_angle;
    int raw_sweep_count;
    int raw_vol_count;
};


extern void dgi_interest(struct dd_general_info *dgi, int verbosity, char *preamble, char *postamble);
extern void ddin_reset(void);
extern void ddin_dd_conv(int interactive);
extern void ddin_dump_this_ray(struct dd_general_info *dgi, struct solo_list_mgmt *slm);
extern int ddin_ini(int interactive);
extern int ddin_inventory(void);
extern int ddin_next_ray(void);
extern void ddin_positioning(void);
extern void dorade_reset_offset(struct input_read_info *iri);
extern int ddin_select_ray(DGI_PTR dgi);
extern int ddin_clip_data(DGI_PTR dgi);
extern void ddin_init_sweep(DGI_PTR dgi, time_t current_time, int new_vol);
extern void ddin_stuff_ray(DGI_PTR dgi, time_t current_time);

#endif
