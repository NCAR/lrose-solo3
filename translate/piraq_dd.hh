/* created using cproto */
/* Tue Jun 21 22:05:56 UTC 2011*/

#ifndef piraq_dd_hh
#define piraq_dd_hh

#include <sys/types.h>
#include <dd_io_structs.h>
#include <dorade_share.h>
#include <piraq/piraqx.h>
#include <dd_defines.h>
#include "piraq.hh"
#include "run_sum.h"

/* piraq_dd.c */

#ifndef piraqx_dd_hh //Jul 26, 2011
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
    short subsec;
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
#endif  //Jul 26, 2011

extern struct input_read_info *return_pir_irq(void);
extern int pui_next_block(void);
extern void piraq_dd_conv(int interactive_mode);
extern int piraq_isa_new_vol(void);
extern int piraq_isa_new_sweep(void);
extern int piraq_select_ray(void);
extern void piraq_print_stat_line(int count);
extern void piraq_reset(void);
extern void piraq_name_replace(char *name, struct rename_str *top);
extern void piraq_name_aliases(char *aliases, struct rename_str **top);
extern double pc_swap4f(fourB *ov);
extern int32_t pc_swap4(fourB *ov);
extern void piraq_positioning(void);
extern int piraq_inventory(struct input_read_info *irq);
extern void piraq_dump_this_ray(struct solo_list_mgmt *slm);
extern void piraq_print_headers(struct solo_list_mgmt *slm);
extern void piraq_print_hdr(HEADERV *hdr, struct solo_list_mgmt *slm, RADARV *rhdr);
extern void piraq_print_rhdr(RADARV *rhdr, struct solo_list_mgmt *slm);
extern void polypp(DWELL *dwell, RADARV *radar, float *prods);
extern void dualprt(DWELL *dwell, RADAR *radar, float *pptr);
extern void dualprt2(DWELL *dwell, RADARV *radar, float *prods);
extern void dualpol1(DWELL *dwell, RADARV *radar, float *prods);
extern void dualpol3(DWELL *dwell, RADARV *radar, float *prods);
extern void dualpol12(DWELL *dwellness, RADARV *radar, float *prods);
extern void fullpolplus(DWELL *dwellness, RADARV *radar, float *prods);
extern void dualpolplus(DWELL *dwellness, RADARV *radar, float *prods);
extern void dualpolplusGf(DWELL *dwellness, RADARV *radar, float *prods);
extern int piraq_ts_stats(void);
extern short snarf2(char *bytes);
extern int32_t snarf4(char *bytes);
extern float snarf4f(char *bytes);
extern void update_prqx(PIRAQX *prqx, LeRADARV *rhdr, LeHEADERV *dwel);
extern void update_prqxx(PIRAQX *prqx, RADARV *rhdr, HEADERV *dwel);
extern void smallhvsimul(DWELL *dwell, RADAR *radar, float *prods);

void piraq_ini (void);
int piraq_next_ray (void);
void piraq_backwards_reset (int modest);
void piraq_map_hdr (char *aa, int gotta_header);
void piraq_header (void);
void piraq_fields (void);
static void simplepp(DWELL *dwell, RADARV *radar, float *prods);
void simplepp16(DWELL *dwell, RADARV *radar, float *prods);
static void products(DWELL *dwell, RADARV *radar, float *prods);
void piraq_dd_conv (int interactive_mode);

#endif
