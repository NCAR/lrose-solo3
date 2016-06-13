/* created using cproto */
/* Tue Jun 21 22:05:46 UTC 2011*/

#ifndef gpro_data_hh
#define gpro_data_hh

#include "gpro_data.h"
#include <Platform.h>
#include "dd_time.hh"
#define	AC_NC_VALS_RING 11
#define	MAX_NC_NDX 16
/* gpro_data.c */

struct ac_vals {
    struct ac_vals *last;
    struct ac_vals *next;
    void *vptr;
    short sval;
    int32_t lval;
    float fval;
    double dval;
    double delta;
    double time;
    int que_ndx;
};

struct nc_var_info {
    struct nc_var_info *last;
    struct nc_var_info *next;
    int var_ndx;
    int var_id;
    int data_type;
    int ndims;
    int dims[4];
    int natts;
    char var_name[16];
    struct ac_vals *vals;
    struct ac_vals *vals_ptrs[AC_NC_VALS_RING];
};

struct nc_ac_info {
    double reference_time;
    double next_time;
    double base_time;
    double delta_time;
    double time_correction;
    double last_radar_time;

    struct nc_var_info *vars_top; 
    int count;
    int max_count;
    int nc_fid;
    int file_count;
    int file_ndx;
    int last_radar_num;
    char *file_names_ptrs[16];
    char file_names[256];
    char file_directory[128];

    char *ac_var_names[MAX_NC_NDX];
    struct nc_var_info *ac_var_ptrs[MAX_NC_NDX];
};


extern char *eld_nimbus_fix_asib(DD_TIME *dts, struct platform_i *asib, int options, int radar_num);
extern void ac_nc_nab_this_time_step(void);
extern void ac_nc_apply_fixes(DD_TIME *dts, struct platform_i *asib, int options, int ndx);
extern int ac_nc_find_tag(char *tag);
extern int ac_nc_init(void);
extern int main(void);
extern void eld_gpro_fix_asib(DD_TIME *dts, struct platform_i *asib, int options);
extern void gpro_assemble_vals(struct ads_raw_data_que *rdq, struct ads_data *adsd, double pct);
extern void setup_ltvi(void);
extern int gpro_mount_raw(void);
extern void gp_header(char *buf);
extern int is_gp_var(char *name);
extern int len_arg(char *a, char *b);
extern int gp_get_line(char *s);
extern void gp_date(char *s, int *year, int *mon, int *day);
extern float gp_time(char *s);
extern char *gp_arg_ptr(char *s, int nth);
extern double gpro_decode(struct letvar_info *this_info, double pct_samp, int32_t *at); //Jul 26, 2011 *this issue
extern char *gpro_next_rec(struct ads_raw_data_que *rdq, int *status);
extern void gpro_position_err(struct ads_data *adsd);
extern struct ads_raw_data_que *gpro_sync(double time);
extern struct ads_raw_data_que *gpro_initialize(struct ads_data **adsd_at);

#endif
