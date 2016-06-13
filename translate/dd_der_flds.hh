/* created using cproto */
/* Tue Jun 21 22:05:09 UTC 2011*/

#ifndef dd_der_flds_hh
#define dd_der_flds_hh

/* dd_der_flds.c */
# define     MAX_ALTS 8
# define  DEFAULT_ALT 0
# define DUAL_PRF_ALT 1


#include <eldh.h>
#include <dd_general_info.h>
#include <dd_defines.h>


struct lidar_parameters {
    double baseline;
    double counts_per_db;
    double end_range;
    double fudge_factor;
    short *data_lut;
    float *rng_cor_lut;
    float cell_spacing;
    float ant_orientation;
    char *parameter_name;
    int id_num;
    int max_cells;
};

struct derived_fields {
    struct lidar_parameters *RDB_lps;
    struct lidar_parameters *GDB_lps;
    struct der_pair * alt_ders[MAX_ALTS];

    struct lidar_parameters *lps[MAX_PARMS];
};


extern void dd_derived_fields(DGI_PTR dgi);
extern double dd_ac_vel(DGI_PTR dgi);
extern void dd_BB_unfold(DGI_PTR dgi, int fn, double ac_vel, int qsize, double f_fold_shear, struct ve_que_val *vqv);
extern void dd_denotch(DGI_PTR dgi, int fn, double f_notch_max, double f_notch_shear);
extern int dd_despeckle(DGI_PTR dgi, int fn, int a_speckle);
extern int dd_single_prf_vortex_vels(DGI_PTR dgi, int fn, char *dname);
extern int dd_fix_vortex_vels(DGI_PTR dgi, int fn, char *dname);
extern int dd_gp_threshold(DGI_PTR dgi, int fn, char *thr_name, double thr_val1, double thr_val2, int thr_type);
extern int dd_legal_pname(char *name);
extern void dd_lidar_db(DGI_PTR dgi, int fnd, struct lidar_parameters *lps);
extern int dd_new_param(DGI_PTR dgi, char *srs_name, char *dst_name);
extern void dd_add_bias(DGI_PTR dgi, int fn, double add_bias);
extern void dd_nix_air_motion(DGI_PTR dgi, int fn, double ac_vel, int center_on_zero);
extern void dd_pct_stats(DGI_PTR dgi);
extern int dd_thr_parms(char *a, char *name, float *val);
extern int dder_lidar_opts(struct derived_fields *derfs, char *opts, char *dst_flds[], int num_dst);

int zdr_prime (DGI_PTR dgi, int fn);
int thr_ldr (DGI_PTR dgi, int fn, double threshold);
int setup_der_alts (char *list);
int get_kdp (DGI_PTR dgi, int fn);


#endif
