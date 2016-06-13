/* created using cproto */
/* Tue Jun 21 22:05:40 UTC 2011*/

#ifndef fof_dd_hh
#define fof_dd_hh

#include <stdio.h>
#include <dd_io_structs.h>
#include <dd_time.h>
#include <dorade_share.h>
#include "rsf_house.h"
/* fof_dd.c */

struct param_desc {
    unsigned int id0  : 4;
    unsigned int id1  : 4;
    unsigned int bits : 8;
};

struct dual_pol_mode {
#ifdef LITTLENDIAN
    unsigned int dual_polar :  1; /* Dual Polarization Mode Enable */
    unsigned int zdr_limit  :  1; /* ZDR limit enable (0 = folding) */

    unsigned int zdr_scale  :  2; /* 00 +-  3 db
				   * 01 +-  6 db
				   * 10 +- 12 db
				   * 11 +- 24 db
				   */
    unsigned int ldr_limit  :  1; /* LDR limit enable (0 = folding) */
    unsigned int ldr_scale  :  2; /* 00 +-  6 db
				   * 01 +- 12 db
				   * 10 +- 24 db
				   * 11 +- 48 db
				   */

    unsigned int half_nyq   :  1; /* Half Nyquist Enable (Velocity interface
				   * 0 = HV pairs
				   * 1 = HH and VV pairs
				   */
    unsigned int unused     :  8;
#else
    unsigned int unused     :  8;
    unsigned int half_nyq   :  1; /* Half Nyquist Enable (Velocity interface
				   * 0 = HV pairs
				   * 1 = HH and VV pairs
				   */
    unsigned int ldr_scale  :  2; /* 00 +-  6 db
				   * 01 +- 12 db
				   * 10 +- 24 db
				   * 11 +- 48 db
				   */
    unsigned int ldr_limit  :  1; /* LDR limit enable (0 = folding) */
    unsigned int zdr_scale  :  2; /* 00 +-  3 db
				   * 01 +-  6 db
				   * 10 +- 12 db
				   * 11 +- 24 db
				   */
    unsigned int zdr_limit  :  1; /* ZDR limit enable (0 = folding) */
    unsigned int dual_polar :  1; /* Dual Polarization Mode Enable */
#endif

};

/*  e.g. 0x00df  Dual Mode, HH and VV pairs for velocity, ZDR limit at +- 24db,
 *          LDR limit at +- 24 db
 */

struct dual_pol_switch {
    unsigned int test_mode  :  1; /* Test mode enable (0=RP6 syncd) */
    unsigned int xmit_opp   :  1; /* Transmit opposite of receive */
    unsigned int rec_vert   :  1; /* Receive vertical only */
    unsigned int rec_horz   :  1; /* Receive horizontal only */
    unsigned int num_pulse  : 12; /* N-1 pulses/same polarization */
};

/* typical values
 * 
 *  0x0000  Alternating polarization pulse (HVHVHV...)
 *  0x003f  64 pulses each polarization
 *  0x1000  Tx and Rx horizontal only
 *  0x2000  Tx and Rx vertical only
 *  0x5000  Tx vertical, Rx horizontal
 *  0x6000  Tx horizontal, Rx vertical (LDR @ 10 cm)
 * 
 */

/*
 * 
        h = indexa( ched, i, j, 8, 'PULSE WIDTH' )
        call attarg( ched, h, j, 1, a, n )
        call dcode( ched, a, n, ityp, kint, zpls )
    	zpls = zpls*1.e-9
c
c   range to the first gate
	zr1 = ( zgs+zgse )*( 1+m ) +zr1t +zr1e  -.25*3.e5*zpls
c   calculations are in km.
c   zr1t is the range to the first gate from the raw housekeeping
c   zr1e is the error in this number
c   zgse is the error in the gate spacing

	r0 = gs*(1+m) +e0 - .25 * 3.e5 * pulse_width; 
 * 
 */

/*
 * 
            zrdcon = 168.8 +10.*alog10( zprf ) -2.*zgain
     a          -zatp +10.*alog10( zwvl**2 )
     a          -10.*alog10( zbmw**2 ) -10.*alog10( .93 )
     |          + zbwloss +zrinbys +zpwrcor
 * 
 */

struct final_field_info {	/* c...mark */
    struct final_field_info *last;
    struct final_field_info *next;

    struct field_derivation_info *derf;
    struct data_field_info *dataf;
    struct source_field_info *srsf;

    int id_num;
    char field_name[12];
    char *long_field_name;
};

struct data_field_info {
    struct data_field_info *last;
    struct data_field_info *next;

    struct data_field_info *thr_fld;
    struct data_field_info *src_fld;
    struct data_field_info *src_flda;
    struct data_field_info *src_fldb;

    struct field_derivation_info *derf;
    struct source_field_info *srsf;
    struct source_field_info *countsf;

//    void (*proc_ptr)();  //Jul 26, 2011
    void (*proc_ptr)(struct data_field_info *dataf);  //Jul 26, 2011

    float *lut;
    float *vals;

    float dd_scale;
    float dd_bias;
    float missing;
    float threshold;

    int hi_threshold;
    int setup;
    int id_num;
    int num_gates;
    int offset;
    int proc_id;
    int raw_counts;

    char *field_name;
    char *long_field_name;
};

struct source_field_info {
    struct source_field_info *last;
    struct source_field_info *next;

    struct calibration_info *cal_info;
    struct fof_system_attributes *sysatts;
    struct fof_rng_cor *rngc;

    float bias;
    float e0;
    float gs0;
    float gs;
    float prf;
    float r0;
    float rad_con;
    float scale;
    float zdr_scale;

    float *lut;
    float *lut2;
    float *lut3;
    float *rngcvals;

    int base_offset;
    int final_offset;
    int id_num;
    int mterm;
    int num_bits;
    int num_gates;
    int present;
    int x_band;

    char source_name[4];
    char field_name[4];
    char long_field_name[80];
};

struct field_derivation_info {
    struct field_derivation_info *last;
    struct field_derivation_info *next;
    int id_num;
    char source_name[4];
    char field_name[4];
    char long_field_name[80];
};

struct calibration_pair {
    struct calibration_pair *last;
    struct calibration_pair *next;
    int counts;
    float dbm;
};

struct calibration_info {
    struct calibration_info *last;
    struct calibration_info *next;
    struct calibration_pair *top_pair;

    float adjusted_mds;
    float saturation_threshold;
    float noise_power;
    float v_noise_power;
    float power_threshold;
    float *dbm_vals;

    int cal_type;
    int num_pairs;
    int threshold_counts;

    char name[64];
};

struct fof_system_attributes {
    struct fof_system_attributes *last;
    struct fof_system_attributes *next;

    double h_noise_power_watts;
    double v_noise_power_watts;

    float avg_trans_pwr;
    float h_beam_width;
    float h_cutoff;
    float h_ncp_threshold;
    float h_noise_power;
    float h_snr_threshold;
    float if_filter_loss;
    float integration_bias;
    float ldr_bias;
    float PRF;
    float pulse_width;
    float rec_power_corr;
    float system_gain;
    float sw_threshold;
    float v_beam_width;
    float v_cutoff;
    float v_noise_power;
    float wave_length;
    float zdr_bias;
    char name[12];
};

struct desc_info {
    int desc_offset;
    int scale_offset;
    int bias_offset;
    int id_num;
    int bits;
};

struct fof_ray_que {
    struct fof_ray_que *last;
    struct fof_ray_que *next;
    struct fof_ray_que *prev;

    double datp;
    double daz;
    double del;
    double delta_time;
    double dfx;
    double dprf;
    double drot;
    double time;

    float atp;
    float az;
    float el;
    float fx;
    float rotation_angle;
    float gs;
    float prf;

    unsigned short dual_pol_mode;
    unsigned short dual_pol_switch;
    int fields_present[16];
    int num_gates;
    int num_samples;
    int flags;
    int scan_mode;
    int sizeof_ray;
    int seq_num;
    int sweep_num;
    int vol_num;
    int num_fields;
    char *at;
    char *data;
};

struct fof_sweep_que {
    struct fof_sweep_que *last;
    struct fof_sweep_que *next;
    struct fof_ray_que *first_ray;
    int first_seq_num;
    int last_seq_num;
    double atp_sum;
    double datp_sum;
    double daz_sum;
    double del_sum;
    double dfx_sum;
    double dprf_sum;
    double fxd_sum;
    double drot_sum;

    float atp;
    float fx;
    float min_atp;
    float max_atp;
    float atp_start;
    float atp_end;
    float rotang_min_atp;

    int atp_changed;
    int min_atp_at;
    int max_atp_at;
    int ray_count;
    int sizeof_gate_info;
    int something_changed_at;
    int sweep_num;
};

struct fof_useful_info {
    double trip_time;
    double altitude;
    double latitude;
    double longitude;
    double time_correction;

    struct d_time_struct *dts;
    struct fof_ray_que *ray_que;
    struct fof_sweep_que *sweep_que;
    
    struct data_field_info *dataf;
    struct final_field_info *finalf;
    struct source_field_info *ref_srsf;

//    void (*scan_algorithm)();  //Jul 26, 2011
    void (*scan_algorithm)(int its_changed, struct fof_ray_que *rq);  //Jul 26, 2011

    float ref_fixed_angle;
    float gs0;

    int ascending_fxd_only;
    int cannot_generate_fields;
    int count_since_trip;	/* c...mark */
    int era;
    int fmt;
    int hsk64_flag;
    int hesitation_count;
    int ignore_transitions;
    int ignore_this_ray;
    int new_sweep;
    int new_sweep_flags;
    int new_vol;
    int num_final_fields;
    int presumed_new_sweep;
    int presumed_new_vol;
    int ray_count;
    int sizeof_gate_info;
    int sweep_count;
    int sweep_count_flag;
    int vol_count;
    int vol_count_flag;
    int rays_in_rec;
    int ray_in_rec;
    int transition_count;
    int x_band;

    char radar_name[12];
    char site[48];
};



extern void fof_dd_conv(int interactive_mode);
extern struct source_field_info *fof_check_srs_fld(char *field_name, struct field_derivation_info **derfx);
extern void fof_catalog_header(void);
extern double fof_crack_vms_date(char **sptrs, int nargs, DD_TIME *dts);
extern struct data_field_info *fof_data_field_spair(void);
extern void fof_dump_this_ray(struct solo_list_mgmt *slm);
extern struct data_field_info *fof_duplicate_field(char *field_name);
extern void fof_era(void);
extern struct data_field_info *fof_establish_data_field(char *field_name);
extern struct source_field_info *fof_find_srs_fld(char *field_name);
extern void fof_gen_counts_field(struct data_field_info *dataf);
extern void fof_gen_dbz_field(struct data_field_info *dataf);
extern void fof_gen_field(struct data_field_info *dataf);
extern void fof_gen_ldr_field(struct data_field_info *dataf);
extern void fof_gen_noiseless_zdr(struct data_field_info *dataf);
extern void fof_gen_unfolded_linear(struct data_field_info *dataf);
extern void fof_gen_quotient_field(struct data_field_info *dataf);
extern void fof_gen_snr(struct data_field_info *dataf);
extern void fof_gen_sw(struct data_field_info *dataf);
extern void fof_gen_thrd_field(struct data_field_info *dataf);
extern void fof_gen_unfolded_xvert(struct data_field_info *dataf);
extern char *fof_get_field_list_name(void);
extern void fof_ini(void);
extern struct data_field_info *fof_insert_data_field(struct data_field_info *this_info, struct data_field_info *that, int before);
extern int fof_inventory(struct input_read_info *iri);
extern int fof_isa_new_sweep(struct fof_ray_que *rq);
extern int fof_isa_new_vol(void);
extern void fof_link_cat_info(void);
extern void fof_luts(struct source_field_info *srsf);
extern void fof_malloc_vals(struct data_field_info *dataf);
extern void fof_MHR_or_cape(int its_changed, struct fof_ray_que *rq);
extern void fof_mist_cp3(int its_changed, struct fof_ray_que *rq);
extern void fof_nab_data(void);
extern struct fof_sweep_que *fof_new_sweep(struct fof_ray_que *rq);
extern void fof_new_vol(void);
extern int fof_next_ray(void);
extern void fof_positioning(void);
extern struct source_field_info *fof_possible_field(char *field_name, struct field_derivation_info **derf);
extern void fof_print_fieldlist(struct solo_list_mgmt *slm);
extern void fof_print_headers(struct solo_list_mgmt *slm);
extern void fof_print_hsk(struct solo_list_mgmt *slm);
extern void fof_print_stat_line(int a);
extern struct data_field_info *fof_push_data_field(struct data_field_info *dataf);
extern struct data_field_info *fof_que_data_field(struct data_field_info *dataf);
extern struct fof_ray_que *fof_que_ray_info(struct fof_ray_que **top, struct fof_ray_que *this_ray);
extern struct source_field_info *fof_raw_counts_field(char *field_name);
extern void fof_rayq_info(char *at, struct fof_ray_que *rq);
extern struct fof_ray_que *fof_ray_que_spair(void);
extern void fof_redo_srsfs(struct fof_ray_que *rq);
extern struct data_field_info *fof_remove_data_field(struct data_field_info *this_info);
extern void fof_reset(void);
extern void fof_rng_corr(struct source_field_info *srsf);
extern int fof_select_ray(void);
extern void fof_release_data_fields(struct data_field_info *this_info);
extern void fof_setup_field(struct data_field_info *dataf);
extern void fof_stack_ray_que_spairs(struct fof_ray_que *this_ray);
extern void fof_stack_sweep_que_spairs(struct fof_sweep_que *this_que);
extern struct fof_sweep_que *fof_sweep_que_spair(struct fof_sweep_que **top);
extern double fof_time(RSF_HOUSE *at);
extern int zatt_arg_ptrs(char *att, char *string_space, char **str_ptrs);
extern void ctypef(float *ff, int m, int n);
FILE *fof_catalog_header_stream();
int xnext_att(FILE *fp, char *att);
int znext_att(FILE *fp, char *att);
void fof_header_list(FILE *fp, char *name, double ztime, 
                     char *headername, char *experiment, char *instrument,
					 double *start, double *stop);
extern void fof_load_field_list(FILE *fp);
extern void fof_process_field_info(FILE *fp);
extern void fof_process_system_info(FILE *fp);
extern void fof_absorb_catalog_header(FILE *fp);
extern int fof_process_cal_info(FILE *fp);

#endif
