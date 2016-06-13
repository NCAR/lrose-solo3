/* created using cproto */
/* Tue Jun 21 22:06:10 UTC 2011*/

#ifndef uf_dd_hh
#define uf_dd_hh

#include <dd_io_structs.h>
#include <dd_time.hh>
#include <uf_fmt.h>

/* uf_dd.c */


typedef struct {
    char file_id[8];	/* e.g. "FOFS 1.0" */
    int32_t format;
    int32_t num_tables;
    int32_t bytes2header[16]; /* bytes to table header from BOF */
    int32_t header_size[16];
} FILE_HEADER;


typedef struct {
    int32_t bytes2table; /* bytes to first table entry from BOF */
    int32_t num_entries;
    size_t table_entry_size;
    char table_id[8];
    char dependency[8];	/* pointers are dependent on the indicated table id */
    char element_id[16][4]; /* 4 character id for each entry element */
} TABLE_HEADER;


typedef struct {
    int32_t bytes2rec; /* bytes to "physical record" from BOF */
    u_short num_recs; /* number of logical records packed into "phys.rec." */
    u_short place_holder;
} RECORD_TABLE_ENTRY;


typedef struct {
    RECORD_TABLE_ENTRY record[1];
} RECORD_TABLE;


typedef struct {
    TABLE_HEADER header;
} RECORD_TABLE_HEADER;	/* "RADARREC" id */


typedef struct {
     TABLE_HEADER header;
} SWEEP_TABLE_HEADER; /* "RADARSWP" id */

#if !defined(piraqx_dd_hh) && !defined(piraq_dd_hh) //Jul 26, 2011
struct rename_str {
    struct rename_str *last;
    struct rename_str *next;
    char old_name[12];
    char new_name[12];
};
#endif  //Jul 26, 2011

struct uf_useful_items {
    double time0;
    double start_time;
    double stop_time;
    double sweep_time_limit;
    double trip_time;
    double subsec_increment;

    int count_since_trip;
    int do_subsecond_times;
    int watch_fxd_angle;
    int io_type;
    int max_num_bins;
    int min_ray_size;
    int moving_platform;
    int new_sweep;
    int new_vol;
    int ok_ray;
    int prev_radar_num;
    int current_radar_ndx;
    int radar_count;
    int run_time;
    int uf_fid;
    int sizeof_loc_use;
    int options;
    int sizeof_this_ray;


    struct rename_str *top_ren;
};

struct UF_for_each_radar {
    float *range_bin[SRS_MAX_FIELDS];
    float prev_fxd_angle;
    double sweep_reference_time;
    double sum_rotang_diff;
    float prev_rotang;

    int radar_num;
    int srs_sweep_num;
    int srs_vol_num;
    int vol_num;
    int ray_count;
    int sweep_count;
    int vol_count;
    int sweep_ray_num;

    char radar_name[12];
    char project_name[12];
};

extern void uf_dd_conv(int interactive);
extern void map_uf_ptrs(short *sbuf);
extern void rdp_ac_header(int rdn);
extern void ucla_ac_header(void);
extern void ufx_ini(void);
extern int uf_inventory(struct input_read_info *irq);
extern int uf_logical_read(void);
extern int uf_next_ray(void);
extern void uf_positioning(void);
extern char **uf_print_data(short *uf, char **ptrs);
extern char **uf_print_dhed(short *dh, char **ptrs);
extern char **uf_print_fldh(UF_FLD_HED *fld, char **ptrs, char *name);
extern void uf_print_headers(short *uf, char **ptrs);
extern char **uf_print_lus(short uf[], char **ptrs);
extern char **uf_print_lus_ac(UF_LOC_USE_AC *lus, char **ptrs);
extern char **uf_print_lus_gen(short *lus, int lundx, int len, char **ptrs);
extern char **uf_print_lus_ucla(UCLA_LOC_USE_AC *lus, char **ptrs);
extern char **uf_print_man(UF_MANDITORY *mh, char **ptrs);
extern char **uf_print_opt(UF_OPTIONAL *opt, char **ptrs);
extern void uf_print_stat_line(int count);
extern void uf_rename_str(char *a);
extern void uf_reset(void);
extern int uf_select_ray(void);
extern double uf_time_stamp(DD_TIME *dts);
extern void uf_vol_update(void);

#endif
