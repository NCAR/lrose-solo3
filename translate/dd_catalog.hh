/* created using cproto */
/* Tue Jun 21 22:05:07 UTC 2011*/

#ifndef dd_catalog_hh
#define dd_catalog_hh

#include <dd_general_info.h>
#include <time.h>

/* dd_catalog.c */
struct catalog_info {
    double d_cos_heading;
    double d_sin_heading;
    double d_cos_drift;
    double d_sin_drift;
    double d_cos_pitch;
    double d_sin_pitch;
    double d_cos_roll;
    double d_sin_roll;
    double prev_MB_count;
    float f_last_rotation_angle;
    float f_delta_angles;
    float f_swp_stop_angle;
    float f_stop_latitude;
    float f_stop_longitude;
    float f_stop_altitude;
    float f_stop_EW_wind;
    float f_stop_NS_wind;
    float f_ew_velocity;
    float f_ns_velocity;
    float f_fixed_angle;
    time_t stop_time;
    time_t time_stamp;
    time_t trip_time;

    int32_t lseek_vol_offset;
    int avg_rota_count;
    int vol_rec_mark;
    int prev_rec_count;
    int num_rays;
    int rays_per_volume;
    int num_good_cells;
    int tot_cells;
    int volumes_fid;
    int radar_type;
    int radar_num;
    int vol_num;
    int sweep_num;
    int flush_sweep_count;
    int vol_txt_size;
    int swp_txt_size;
    char *vol_stop_time;
    char *vol_txt_ptr;
    char *swp_txt_ptr;
    char volumes_file_name[256];
};

struct comment_mgmt {
    char *buf;
    char *at;
    int sizeof_buf;
    int sizeof_comments;
};

extern void dd_append_cat_comment(char *comm_str);
extern void dd_enable_cat_comments(void);
extern void cat_cat_att(char *name, int num_args, char *arg, char *cat);
extern void cat_cat_att(const std::string &name, int num_args,
			const std::string &arg, char *cat);
extern void cat_cat_att(const std::string &name, int num_args,
			const std::string &arg, std::string &cat);
extern void cat_cat_att_args(char *name, int num_args, char *arglist[], char *cat);
extern char *reverse_string(char *str, char *rev);
extern char *c_deblank(char *a, int n, char *clean);
extern void ddcat_close(int fid);
extern int ddcat_open(char *file_name);
extern void ddcat_write(struct catalog_info *cii, char *buf, int size, int func);
extern void cat_volume(struct dd_general_info *dgi, struct catalog_info *cii, time_t current_time, int finish);
extern void cat_sweep(struct dd_general_info *dgi, struct catalog_info *cii, time_t current_time, int finish); 
extern char *cat_time(time_t time, char *dst);
extern char *cat_date(time_t time, char *dst);
extern void dd_catalog(struct dd_general_info *dgi, time_t ignored_time, int flush);


#endif
