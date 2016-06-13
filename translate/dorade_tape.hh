/* created using cproto */
/* Tue Jun 21 22:05:30 UTC 2011*/

#ifndef dorade_tape_hh
#define dorade_tape_hh

#include <dd_general_info.h>

/* dorade_tape.c */
struct dd_sweep_list_entries {
    double start_time;
    double end_time;
    int32_t volume_time_stamp;
    int32_t sweep_time_stamp;
    int segment_num;
    int radar_num;
    int new_vol;
    int ignore_this_sweep;
    int num_rays;
    int num_parms;
    int source_sweep_num;
    struct dd_sweep_list_entries *next_segment;
    struct dd_sweep_list_entries *next;
    struct dd_sweep_list_entries *last;
    struct dd_sweep_list_entries *nv_track_last;
    char file_name[88];
};

struct track_new_vols {
    double start_time;
    double end_time;
    float sizeof_volume;
    int LF_radar_num;
    int list_radar_nums[MAX_SENSORS];
    int num_radars;
    int sweep_count[MAX_SENSORS];
    int vol_count;
    struct dd_sweep_list_entries *swle_last[MAX_SENSORS];
};

extern void dd_tape(DGI_PTR dgi, int flush);

#endif
