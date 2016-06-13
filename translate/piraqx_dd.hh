/* created using cproto */
/* Tue Jun 21 22:05:58 UTC 2011*/

#ifndef piraqx_dd_hh
#define piraqx_dd_hh

#include <dd_io_structs.h>
#include <dorade_share.h>
/* piraqx_dd.c */

struct rename_str {
    struct rename_str *last;
    struct rename_str *next;
    char old_name[12];
    char new_name[12];
};

struct piraq_useful_items {
    double time;
    double prev_time;
    double time_correction;
    double range_correction;
    double rconst_correction;
    double zdr_offset_corr;
    double ldr_offset_corr;

    double az_diff_sum;
    double el_diff_sum;
    double az_diff_sq_sum;
    double el_diff_sq_sum;
    double x_az_diff_sum;
    double x_el_diff_sum;
    double fxdang_diff_sum;
    double swpang_diff_sum;
    double fxdang_sum;
    double az_diff_short_sum;
    double az_short_sum;
    double el_diff_short_sum;
    double el_short_sum;

    struct run_sum * az_rs;
    struct run_sum * el_rs;
    struct run_sum * az_diff_rs;
    struct run_sum * el_diff_rs;

    double az_diff_run_sum;
    double el_diff_run_sum;
    double az_run_sum;
    double el_run_sum;

    double max_az_diff;
    double max_el_diff;
    double rcp_short_len;

    double az_diff_avg_sum;
    double rcp_num_az_avgs;
    double el_diff_avg_sum;
    double rcp_num_el_avgs;

    double latitude;
    double longitude;
    double altitude;
    double seconds_per_beam;
    double antenna_movement;	/* degrees per ray */
    double prev_noise;
    
    float uniform_cell_spacing;
    float last_rcvr_pulsewidth;
    float last_prt;

    u_int32_t ref_time;
    u_int32_t trip_time;
    u_int32_t unix_day;
    
    int vol_num;
    int sweep_num;
    int vol_count_flag;
    int sweep_count_flag;
    int possible_new_sweep;
    int new_sweep;
    int possible_new_vol;
    int new_vol;
    int ray_count;
    int count_rays;
    int sweep_ray_num;
    int sweep_count;
    int transition_count;
    int vol_count;
    int options;
    int vol_sweep_count;
    int count_since_trip;
    int runaway;
    int inasweep;
    int ignore_this_ray;
    int skipped_backwards;
    int sweep_start_lag;
    int last_scan_mode;
    int last_dataformat;
    int last_gates;
    int lag_rays;
    int min_rays_per_sweep;
    int mark_selected_swp_count;

    int num_az_diffs_avgd;
    int num_el_diffs_avgd;
    int az_short_count;
    int el_short_count;
    int az_diff_short_count;
    int el_diff_short_count;
    int short_rq_len;
    int rhdr_count;
    int check_ugly;
    int time_glitch_count;
    float new_sweep_at_az;

    struct piraq_ray_que *ray_que;
    struct piraq_swp_que *swp_que;
    struct piraq_swp_que *vol_start;
    FILE *sl;
    char * raw_data;
    float * Noise_lut;
    float * hNoise_lut;
    float * vNoise_lut;
    float * cNoise_lut;
};

struct piraq_ray_que {
    struct piraq_ray_que *last;
    struct piraq_ray_que *next;
    double time;
    double time_diff;
    double az_diff;
    double el_diff;
    double fxdang_diff;
    double rcvr_pulsewidth;
    float fxdang;
    float az;
    float el;
    float prt;
    int hits;
    float source_fxdang;
    int scan_num;
    int vol_num;
    int ray_num;
    int scan_type;
    int transition;
    int num_gates;
    int ptime;
    int subsec;
};

struct piraq_swp_que {
    struct piraq_swp_que *last;
    struct piraq_swp_que *next;
    double swpang_diff;
    double rcvr_pulsewidth;
    double prf;
    float swpang;
    int new_vol;
    int scan_mode;
    int count;
};

extern void piraqx_dd_conv(int interactive_mode);
extern int piraqx_next_block(void);
extern int piraqx_isa_new_vol(void);
extern int piraqx_isa_new_sweep(void);
extern int piraqx_select_ray(void);
extern void piraqx_print_stat_line(int count);
extern void piraqx_reset(void);
extern void piraqx_name_replace(char *name, struct rename_str *top);
extern void piraqx_name_aliases(char *aliases, struct rename_str **top);
static void simplepp(void);
extern int simplepp2(void);
extern void dualprtfloat(void);
extern long long pc_swap8(char *ov);
extern void piraqx_positioning(void);
extern int piraqx_inventory(struct input_read_info *irq);
extern void piraqx_dump_this_ray(struct solo_list_mgmt *slm);
extern void piraqx_print_headers(struct solo_list_mgmt *slm);
extern void piraqx_print_hdr(struct solo_list_mgmt *slm);
extern int piraqx_ts_stats(void);

void piraqx_ini (void);
int piraqx_next_ray (void);
void piraqx_map_hdr (char *aa, int gotta_header);
void piraqx_backwards_reset (int modest);
void piraqx_fields (void);
static void productsx(void);
void xnewsimplepp (void);
size_t newsimplepp();

extern double pc_swap4f(fourB *ov);  //Jul 26, 2011
extern int32_t pc_swap4(fourB *ov);  //Jul 26, 2011

#endif
