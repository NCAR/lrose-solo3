/* created using cproto */
/* Tue Jun 21 22:05:50 UTC 2011*/

#ifndef hrd_dd_hh
#define hrd_dd_hh

#include <dd_io_structs.h>
#include <hrdh.h>
#include <dd_time.hh>
/* hrd_dd.c */

struct id_stack_entry {
    int radar_id;
    struct id_stack_entry *prev;
    struct id_stack_entry *next;
};

struct HRD_ray_que {
    double time;
    struct HRD_ray_que *next;
    struct HRD_ray_que *last;
    float rotation_angle;
    float elevation;
    int sweep_num;
    double diff;
    double real_diff;
};

struct HRD_for_each_radar {
    struct HRD_ray_que *ray_que;
    double delta_angle_sum;
    float fixed_angle;
    float fixed_angle_sum;
    float *range_bin[SRS_MAX_FIELDS];

    int hrd_ray_num;
    int hrd_sweep_num;
    int radar_num;
    int ray_num;
    int rays_since_crossover;
    int sweep_num;
    int this_radar_id;
    int ray_count;
    int sweep_count;
    int vol_count;

    unsigned short hrd_code;
    char radar_name[12];
};

struct hrd_info_que {
    struct hrd_info_que *next;
    struct hrd_info_que *last;
    int radar_id;
};

struct hrd_useful_items {
    struct hrd_info_que *info_que;
    double time0;
    double start_time;
    double stop_time;

    float azimuth_correction;
    float elevation_correction;
    float pitch_correction;
    float roll_correction;
    float drift_correction;
    float sweep_trip_angle;
    float sweep_trip_delta;

    int current_radar_ndx;
    int options;
    int hrd_fid;
    int io_type;
    int vol_count;
    int new_vol;
    int new_sweep;
    int radar_id;
    int hrd_range_delay;
    int max_rec_size;
    int min_ray_size;
    int ok_ray;
    int new_header;
    int header_count;
    int radar_count;
    int bang_zap;
    int run_time;
    int num_raw_fields;
    int raw_field_position[HRD_MAX_FIELDS];

    struct hrd_radar_info *hri;
    char *hrd_ray_pointer;
    char *select_radar;
    short *hrd_lut[HRD_MAX_FIELDS];

    unsigned short radar_id_bit;
    unsigned short hrd_code;
};


extern void hrd_dd_conv(int interactive_mode);
extern void hrd_fix_navs(void);
extern void hrd_gen_luts(void);
extern void hrd_grab_header_info(void);
extern void hrdx_ini(void);
extern int hrd_inventory(struct input_read_info *irq);
extern void hrd_nab_data(void);
extern void hrd_nab_info(void);
extern int hrd_next_block(void);
extern int hrd_next_ray(void);
extern void hrd_positioning(void);
extern void hrd_print_hdrh(void);
extern void hrd_print_head(void);
extern void hrd_print_hrh(char **ptrs);
extern void hrd_print_hri(struct hrd_radar_info *hri);
extern void hrd_print_stat_line(int count);
extern void hrd_reset(void);
extern int hrd_select_ray(void);
extern double hrd_time_stamp(struct hrd_ray_header *hrh, DD_TIME *dts);
extern void hrd_try_grint(struct generic_radar_info *gri, int verbose, char *preamble, char *postamble);
extern void hrd_update_header_info(void);
extern void hrd_upk_mc_data(void);
extern int upk_hrd16(unsigned short *dd, int bad_val, unsigned short *ss, unsigned short *zz, int *bads);
extern int upk_hrd16LE(unsigned short *dd, int bad_val, unsigned short *ss, unsigned short *zz, int *bads);

void hrd_upk_data (int fn, int nf, int nb, unsigned char *src, unsigned int *dst);
#endif
