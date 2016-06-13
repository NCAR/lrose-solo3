/* 	$Id$	 */
#ifndef eldh_h
#define eldh_h

# define     ELD_THRESHOLDING 0x0001
# define      ELD_DESPECKLING 0x0002    
# define    ELD_NOTCH_REMOVAL 0x0004
# define        ELD_UNFOLDING 0x0008
# define       ELD_AIR_MOTION 0x0010
# define       ELD_NO_REVERSE 0x0020
# define   ELD_INITIAL_UNFOLD 0x0040
# define    ELD_AZ_CONTINUITY 0x0080
# define    ELD_INCLUDE_DH_DP 0x0100
# define      ELD_FUNKY_SCANS 0x0200
# define       ELD_AC_COMPARE 0x0400
# define      ELD_DESC_SEARCH 0x0800
# define      LIMIT_SCAN_TIME 0x1000

# define      TOGACOARE_FIXES 0x8000
# define          ACE_1_FIXES 0x10000
# define         VORTEX_FIXES 0x20000
# define         FASTEX_FIXES 0x40000
# define      ROT_ANG_SECTORS 0x80000
# define      ELD_TIME_SERIES 0x100000

# define           LIDAR_DATA 0x0001
# define           NAILS_DATA 0x0002

# define    ROTANG_LIST_SIZE 11

# define            A_SPECKLE 3
# define     ELD_MIN_REC_SIZE 9000
# define   ELD_TILT_TOLERANCE 5.0
# define      FIRST_GOOD_GATE 9         /* also 24 */
# define    NCP_THRESHOLD_VAL 0.33
# define     OPTIMUM_AFT_TILT (-18.0)
# define    OPTIMUM_FORE_TILT 18.0
# define       VUF_FOLD_SHEAR 22.0
# define    VUF_MIN_BAD_COUNT 3
# define    VUF_NON_SHEAR_RUN 3
# define        VUF_NOTCH_MAX 6.3
# define      VUF_NOTCH_SHEAR 11.1
# define            VUFQ_SIZE 4
# define      TG_AFT_REFL_ERR 0
# define     TG_FORE_REFL_ERR 0
# define    TOGACOARE_VEL_ERR 1.010132996
# define      VORTEX_VEL_ERR1 1.0106326
# define      VORTEX_VEL_ERR2 1.0212653

# ifndef METERS_PER_MICROSECOND
# define METERS_PER_MICROSECOND 150.
# endif

# define ELD_SCALE(x,scale,offset) ((x)*scale+offset+.5)
# define ELD_UNSCALE(x,rcp_scale,offset) (((x)-offset)*rcp_scale)

#include <dd_defines.h>
#include <time.h>

struct individual_radar_info {
    float rotang_list[ROTANG_LIST_SIZE];
    int radar_num;
    int rotang_ndx;
    int prev_sweep_num;
    int ray_count;
    int sweep_count;
    int sweep_count_flag;
    int vol_count;
    int vol_count_flag;
    int new_radar_desc;
    int num_parms;
    int sizeof_gate;
    int sizeof_fparm[MAX_PARMS];
    int fparm_offset[MAX_PARMS];
    int use_this_sweep;
    int bad_ray_count;
    short *xlate_lut[MAX_PARMS];
    char *source_rdat[MAX_PARMS];
    char *dest_rdat[MAX_PARMS];
    char *threshold_rdat[MAX_PARMS];

    int ray_avg_count;
    int32_t *green_sums;
    int32_t *red_sums;
    struct running_avg_que *green_raq;
    struct running_avg_que *red_raq;
    double avg_time_ctr;
    double sweep_reference_time;
    double vol_reference_time;

    char parameter_name[MAX_PARMS][12];
    int num_parameter_des;
    float cell_spacing;
    float digitizer_rate;
};


struct eldora_unique_info {
    double time;
    double time_correction;
    double prior_time;
    double reference_time;
    struct dd_general_info *dgi;
    struct individual_radar_info *iri[MAX_SENSORS];
    float ncp_threshold;
    float cell_vector_bias;
    float funky_sweep_trip_angle;
    float funky_sweep_trip_delta;
    float lidar_sweep_time_limit;
    time_t current_time;
    int aft_info_ok;
    int beam_skip;
    int gen_thr_flds;
    int ignore_this_ray;
    int io_type;
    int julian_day_bias;
    int min_time_gap;
    int eldora_fid;
    int radar_num;
    int fore_radar_num;
    int aft_radar_num;
    int datum_size[MAX_BIN_FORMATS];
    int use_this_sweep[MAX_SENSORS];
    int volume_header_found;
    int options;
    int data_type;
    int fix_year;
    char *eld_buf;
    char site_name[12];

    int typeof_averaging;
    int max_cells;
    int num_avgd_fields;
    int num_cells_avgd;
    int num_rays_avgd;
    float rcp_rays_avgd;
    float rcp_cells_avgd;

    int fake_sweep_nums;
    float lidar_cell_spacing;
    int fx_count;
    struct time_dependent_fixes *fx;
};

struct ve_que_val {
    int vel;
    struct ve_que_val *last;
    struct ve_que_val *next;
};

#endif
