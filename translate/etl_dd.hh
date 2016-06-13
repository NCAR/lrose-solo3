/* created using cproto */
/* Tue Jun 21 22:05:39 UTC 2011*/

#ifndef etl_dd_hh
#define etl_dd_hh

#include <dorade_share.h>
#include <dd_io_structs.h>
#include "piraq_dd.hh"

# define PX4(x) (pc_swap4((&x)))

# define         MAX_MC_LIDARS 4
# define               MC_FORE 1
# define                MC_AFT 2
# define                MC_FXD 3
# define          MC_FORE_NAME "MCWS-FOR"
# define           MC_AFT_NAME "MCWS-AFT"
# define           MC_FXD_NAME "MCWS-FXD"

# define        MAX_ETL_FIELDS 7
# define         MAX_ETL_GATES 1024
# define          ETL_IO_INDEX 4
# define          RAY_QUE_SIZE 11
# define        SWEEP_QUE_SIZE 22

# define          REALLY_MCAWS -888
# define              _2MICRON 200
# define                 MCAWS 199
# define            OZONE_DIAL 198
# define         EXIMER_OZDIAL 197

# define       FT_TO_METERS(x) ((x) * 0.3048)
# define         X32BIT_ANG(x) ((x) * 8.381903172e-8) /* 180/2^31 */
# define KNOTS_TO_MS(x) ((x) * 0.51444444) /* (1852/3600) */

/* etl_dd.c */
struct etl_ray_que {
    struct etl_ray_que *last;
    struct etl_ray_que *next;
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
    int etl_beam_index;
    int lidar_id;
};

struct etl_swp_que {
    struct etl_swp_que *last;
    struct etl_swp_que *next;

    double time;
    double swpang_diff;
    double prf;
    double num_samples;
    double az_diff_sum;
    double el_diff_sum;
    double fxdang_sum;

    float swpang;

    int new_vol;
    int scan_mode;
    int num_rays;
    /* etl parameters */
    int etl_data_type;
    int etl_scan_type;
    int lidar_id;
};

struct mc_lidar_info {
   int dd_radar_num;
   int ray_count;
   int sweep_count;
   int sweep_count_flag;
   int vol_count;
   int vol_count_flag;
   char lidar_name[12];
};

struct etl_useful_info {

   double time;
   double ref_time;
   double trip_time;
   double sweep_time_limit;
   double sweep_reference_time;
   double time_correction;
   double az_diff_sum;
   double el_diff_sum;
   double az_diff_sq_sum;
   double el_diff_sq_sum;
   double fxdang_sum;
   double seconds_per_beam;

   int ignore_this_ray;
   int new_sweep;
   int new_vol;
   int ray_count;
   int sweep_count;
   int sweep_count_flag;
   int vol_count;
   int vol_count_flag;
   int count_rays;
   int its_macaws;
   unsigned int data_type;
   int max_field_size;
   int fake_sweep_nums;
   int sweep_ray_num;
   int swpang_num;
   int lidar_number;
   int count_since_trip;
   int data_file_header;
   int mc_lidar_ndx;

   struct mc_lidar_info *mc_lidars[MAX_MC_LIDARS];
   short *mom_lut[MAX_ETL_FIELDS];
   unsigned short *mom_data_ptr;
   char *mom_data;
   unsigned short *cov_data_ptr;
   char *cov_data;
   unsigned short *raw_data_ptr;
   char *raw_data;

   struct etl_ray_que *ray_que;
   struct etl_swp_que *swp_que;
   
};

extern void etl_dd_conv(int interactive_mode);
extern void etl_nab_data(void);
void etl_ini (void);
int etl_next_ray (void);
int etl_next_ray_mcaws (void);
int etl_next_ray_2micron (void);
int etl_select_ray (void);
void etl_reset (void);
void etl_positioning (void);
void etl_nab_mom_data (void);
void etl_print_stat_line (int count);
void etl_print_headers (struct solo_list_mgmt *slm);
void etl_dump_this_ray (struct solo_list_mgmt *slm);
int etl_inventory (struct input_read_info *irq);
void etl_moment_fields (void);


#endif
