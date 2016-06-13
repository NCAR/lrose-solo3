/* 	$Id$	 */
#ifndef input_limits_hh
#define input_limits_hh

# include <dd_defines.h>

struct d_limits {
    double lower;
    double upper;
    struct d_limits *last;
    struct d_limits *next;
};

struct dd_input_filters {
    double final_stop_time;
    float min_free_MB;

    int abrupt_start_stop;
    int altitude_truncations;
    int beam_count;
    int beam_skip;
    int catalog_only;
    int compression_scheme;
    int max_beams;
    int max_sweeps;
    int max_vols;
    int modes[MAX_MODES];
    int num_az_sectors;
    int num_el_sectors;
    int num_az_xouts;
    int num_el_xouts;
    int num_fixed_lims;
    int num_modes;
    int num_prf_lims;
    int num_ranges;
    int num_sectors;
    int num_selected_radars;
    int num_time_lims;
    int radar_selected[MAX_SENSORS];
    int run_time;
    int stop_flag;
    int sweep_count;
    int sweep_skip;
    int vol_count;

    struct d_limits *altitude_limits;
    struct d_limits *fxd_angs[MAX_LIMS];
    struct d_limits *prfs[MAX_LIMS];
    struct d_limits *sectors[MAX_LIMS];
    struct d_limits *times[MAX_LIMS];
    struct d_limits *ranges[MAX_LIMS];
    struct d_limits *azs[MAX_LIMS];
    struct d_limits *els[MAX_LIMS];
    struct d_limits *xout_azs[MAX_LIMS];
    struct d_limits *xout_els[MAX_LIMS];
};
# endif
