/* 	$Id$	 */

#ifndef generic_radar_info_h
#define generic_radar_info_h

# define SRS_MAX_FIELDS 64
# define SRS_MAX_GATES 2048

#include <dd_time.h>

struct generic_radar_info {
    struct d_time_struct *dts;
    double time;
    float latitude;
    float longitude;
    float altitude;
    float altitude_agl;		/* the other is msl */
    float rotation_angle;
    float corrected_rotation_angle;
    float azimuth;
    float elevation;
    float fixed;
    float range_b1;		/* m. */
    float bin_spacing;		/* m. */
    float PRF;
    float radar_constant;
    float noise_power;
    float rcvr_gain;
    float ant_gain;
    float peak_power;
    float wavelength;		/* m. */
    float h_beamwidth;		/* deg. */
    float v_beamwidth;		/* deg. */
    float rcv_bandwidth;
    float pulse_width;		/* in meters */
    float heading;
    float drift;
    float roll;
    float pitch;
    float track;
    float tilt;
    float vns;
    float vew;
    float vud;
    float ui;
    float vi;
    float wi;
    float sweep_speed;
    float nyquist_vel;
    float unamb_range;
    float freq[16];
    float ipps[16];
    float range_value[SRS_MAX_GATES];
    float range_correction[SRS_MAX_GATES];
    float dd_scale[SRS_MAX_FIELDS];
    float dd_offset[SRS_MAX_FIELDS];
    float *range_bin[SRS_MAX_FIELDS];

    float aperature_size;
    float field_of_view;
    float aperature_eff;
    
    float primary_cop_baseln;
    float secondary_cop_baseln;
    float pc_xmtr_bandwidth;
    int32_t  pc_waveform_type;

    int actual_num_bins[SRS_MAX_FIELDS];
    int binary_format;
    int compression_scheme;
    int dd_radar_num;
    int field_id_num[SRS_MAX_FIELDS];
    int missing_data_flag;
    int nav_system;
    int num_freq;
    int num_ipps;
    int num_bins;
    int num_fields_present;
    int num_samples;
    int polarization;
    int radar_type;
    int scan_mode;
    int source_format;
    int source_ray_num;
    int sweep_num;
    int source_sweep_num;
    int vol_num;
    int source_vol_num;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    int millisecond;
    int ignore_this_sweep;
    
    short *scaled_data[SRS_MAX_FIELDS];
    unsigned char *byte_data[SRS_MAX_FIELDS];
    char *rdat_ptr[SRS_MAX_FIELDS];

    unsigned short fields_present[SRS_MAX_FIELDS];

    char field_name[SRS_MAX_FIELDS][12];
    char field_long_name[SRS_MAX_FIELDS][44];
    char field_units[SRS_MAX_FIELDS][12];
    char project_name[44];
    char radar_name[12];
    char site_name[12];
    char flight_id[12];
    char storm_name[20];
    char time_zone[12];
    char source_field_mnemonics[256];

    /* general purpose pointers */

    void * gpptr1;
    void * gpptr2;
    void * gpptr3;
    void * gpptr4;
    void * gpptr5;
    void * gpptr6;
    void * gpptr7;

    int int_missing_data_flag[SRS_MAX_FIELDS];
    int clutter_filter_val;
    int transition;
    int sizeof_gpptr1;
    int sizeof_gpptr2;
    int sizeof_gpptr3;
    int sizeof_gpptr4;
    int sizeof_gpptr5;
    int sizeof_gpptr6;
    int sizeof_gpptr7;
};

#endif
