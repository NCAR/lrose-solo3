/* created using cproto */
/* Tue Jun 21 22:06:04 UTC 2011*/

#ifndef stdhrd_hh
#define stdhrd_hh

#include <generic_radar_info.h>
/* stdhrd.c */

struct type5_stuff {
    double time;
    struct type5_stuff *last;
    struct type5_stuff *next;

    float type_5;		 /* (  1) */     
    float words_are_106;	 /* (  2) */     
    float hour;			 /* (  3) */     
    float minute;		 /* (  4) */     
    float second;		 /* (  5) */     
    float record_count;		 /* (  6) */     
    float record_count_b;	 /* (  7) */     
    float switches;		 /* (  8) */     
    float switches_b;		 /* (  9) */     
    float error_flags;		 /* ( 10) */     
    float error_flags_b;	 /* ( 11) */     
    float latitude_deg;		 /* ( 12) */     
    float latitude_min;		 /* ( 13) */     
    float longitude_deg;	 /* ( 14) */     
    float longitude_min;	 /* ( 15) */     
    float radar_altitude;	 /* ( 16) */     
    float pressure;		 /* ( 17) */     
    float ambient_temp;		 /* ( 18) */     
    float dewpoint_sensor;	 /* ( 19) */     
    float down_radiometer;	 /* ( 20) */     
    float side_radiometer;	 /* ( 21) */     
    float ground_speed;		 /* ( 22) */     
    float true_air_speed;	 /* ( 23) */     
    float vert_grnd_speed;	 /* ( 24) */     
    float track;		 /* ( 25) */     
    float heading;		 /* ( 26) */     
    float pitch;		 /* ( 27) */     
    float roll;			 /* ( 28) */     
    float attack;		 /* ( 29) */     
    float side_slip;		 /* ( 30) */     
    float j_w_cloud_water;	 /* ( 31) */     
    float dynamic_pressure;	 /* ( 32) */     
    float ambient_dewpoint_temp; /* ( 33) */     
    float upward_radiometer;	 /* ( 34) */     
    float switches_c;		 /* ( 35) */     
    float e_w_tail_velocity;	 /* ( 36) */     
    float n_s_tail_velocity;	 /* ( 37) */     
    float u_d_tail_velocity;	 /* ( 38) */     
    float word_39;		 /* ( 39) */     
    float geopotential_altitude; /* ( 40) */     
    float pressure_altitude;	 /* ( 41) */     
    float d_value;		 /* ( 42) */     
    float standard_height;	 /* ( 43) */     
    float sfc_pressure_extrap;	 /* ( 44) */     
    float relative_humidity;	 /* ( 45) */     
    float virtual_temp;		 /* ( 46) */     
    float vertical_airspeed;	 /* ( 47) */     
    float ratio_specific_heats;	 /* ( 48) */     
    float mach_number;		 /* ( 49) */     
    float drift;		 /* ( 50) */     
    float e_w_ground_speed;	 /* ( 51) */     
    float n_s_ground_speed;	 /* ( 52) */     
    float e_w_true_airspeed;	 /* ( 53) */     
    float n_s_true_airspeed;	 /* ( 54) */     
    float e_w_wind_speed;	 /* ( 55) */     
    float n_s_wind_speed;	 /* ( 56) */     
    float vertical_wind_spd;	 /* ( 57) */     
    float wind_speed;		 /* ( 58) */     
    float wind_direction;	 /* ( 59) */     
    float word_60;		 /* ( 60) */     
    float vapor_pressure;	 /* ( 61) */     
    float mixing_ratio;		 /* ( 62) */     
    float potential_temp;	 /* ( 63) */     
    float equiv_pot_temp;	 /* ( 64) */     
    float e_w_ave_wind_spd;	 /* ( 65) */     
    float n_s_ave_wind_spd;	 /* ( 66) */     
    float ave_wind_spd;		 /* ( 67) */     
    float ave_wind_dir;		 /* ( 68) */     
    float ine1_vert_accel;	 /* ( 69) */     
    float ine2_vert_accel;	 /* ( 70) */     
    float sec_wind_ave;		 /* ( 71) */     
    float axbt1;		 /* ( 72) */     
    float axbt2;		 /* ( 73) */     
    float axbt3;		 /* ( 74) */     
    float nav_used;		 /* ( 75) */     
    float temperature_used;	 /* ( 76) */     
    float word_77;		 /* ( 77) */     
    float word_78;		 /* ( 78) */     
    float word_79;		 /* ( 79) */     
    float dpj_vert_gnd_speed;	 /* ( 80) */     
    float dpj_vert_air_speed;	 /* ( 81) */     
    float dpj_vert_wind_spd;	 /* ( 82) */     
    float temperature_1_orig;	 /* ( 83) */     
    float temperature_2_orig;	 /* ( 84) */     
    float dew_point_1_orig;	 /* ( 85) */     
    float dew_point_2_orig;	 /* ( 86) */     
    float gps_latitude_orig;	 /* ( 87) */     
    float gps_longitude_orig;	 /* ( 88) */     
    float ine1_latitude_orig;	 /* ( 89) */     
    float ine1_longitude_orig;	 /* ( 90) */     
    float ine2_latitude_orig;	 /* ( 91) */     
    float ine2_longitude_orig;	 /* ( 92) */     
    float radome_temp_1_orig;	 /* ( 93) */     
    float radome_temp_2_orig;	 /* ( 94) */     
    float radome_temp_3_orig;	 /* ( 95) */     
    float word_96;		 /* ( 96) */     
    float word_97;		 /* ( 97) */     
    float word_98;		 /* ( 98) */     
    float word_99;		 /* ( 99) */     
    float word_100;		 /* (100) */     
    float word_101;		 /* (101) */     
    float word_102;		 /* (102) */     
    float word_103;		 /* (103) */     
    float word_104;		 /* (104) */     
    float word_105;		 /* (105) */     
    float word_106;		 /* (106) */     
};


extern int hrd_ascii_read(void);
extern int hrd_merge_std(struct generic_radar_info *gri);
extern void hrd_std_init(void);
extern int hrd_sync_std(struct generic_radar_info *gri);
extern int hrd_sync_ascii(struct generic_radar_info *gri);
extern void hrd_dmp_std(struct type5_stuff *t5s, struct generic_radar_info *gri);
extern void hrd_ac_update_std(struct generic_radar_info *gri);
extern void type4_new(void);
extern void type5_new(void);
extern double cvt_hp1000fp(char *at);
extern int hrdstd_read(int fid, char *buf, int size, int io_type);
extern int main(void);

#endif
