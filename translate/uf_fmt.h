/* 	$Id$	 */
#ifndef uf_fmt_h
#define uf_fmt_h

#include <sys/types.h>

/*
 * Header for generating and unpacking Universal Format
 * (Common Doppler Exchange Format) data
 */

# define CAL 0
# define PPI 1
# define COP 2
# define RHI 3
# define VER 4
# define TAR 5
# define MAN 6
# define IDL 7
# define SUR 8

# define DELETE -32768
# define USE_ALT_SCALE -1
# define NYQ_VEL_OFFSET 19
# define RAD_CON_OFFSET 19
# define MAX_UF_FLDS 64

# define UF_SCALE(x,scale) ((x)*(float)(scale) +.5)
#ifndef S1000  //Jul 26, 2011
# define S1000(x) ((x)*1000.0+0.5)
#endif  //Jul 26, 2011
# define S100(x) ((x)*100.0+0.5)
#ifndef S64  //Jul 26, 2011
# define S64(x) ((x)*64.0+0.5)
#endif  //Jul 26, 2011
# ifndef S10
# define S10(x) ((x)*10.0+0.5)
# endif
# define S8(x) ((x)*8.0+0.5)
# define RCP64 0.015625
# define RCP6400 0.00015625
# define RCP128 0.0078125
# define RCP60 0.01666667
# define RCP3600 0.0002777778
# define FW(n) (n)-1		   /* fortran subscript to C subscript */
# define FsCs(n) ((n)-1)	   /* fortran subscript to C subscript */
# ifndef BYTES_TO_SHORTS
# define BYTES_TO_SHORTS(x) ((((x)-1)>>1)+1)
# endif
# ifndef SHORTS_TO_BYTES
# define SHORTS_TO_BYTES(x) ((x)<<1)
# endif
# define US64(x) ((x)*RCP64)
# define US100(x) ((x)*.01)
//# define US10(x) ((x)*.1)
# define POWER_FIELDS "DM|DZ|DB|DY|DL|DO|XM|XL|SN|ZD|ZN|ZR|LL|LM|LZ|LY"
# define VELOCITY_FIELDS "VF|VE|VT|VQ|VR|VS|VS|VG|SR|SW"

typedef struct {
  char name_struct[4];		/* "UFFN" */
  int32_t sizeof_struct;
  int32_t num_orig_fields;
  int32_t offset_to_mh;		/* manditory header */

  char orig_names[128];
} UF_ORIG_FIELDS;

/*
 * for "XSTF" "data_hed_ptr" in the manditory header
 *   points to first field header
 * for the field header "fld_data_ptr" is the pointer
 *   to the next field header.
 *
 *
 */

typedef struct {		   /* UF MANDITORY HEADER */
    /*
     * numbers in comments are the corresponding word numbers from the
     * Nov.'80 BAMS SESAME News writeup
     */
    char  id[2];                   /* 1 */
    short rec_len;		   /* 2 */
    /*
     * UF_FMT pointers are integer*2 (FORTRAN) type subscripts
     */
    short opt_hed_ptr;		   /* 3 */
    short loc_hed_ptr;		   /* 4 */
    short data_hed_ptr;		   /* 5 */
    short rec_num;		   /* 6 */
    short vol_num;		   /* 7 */
    short ray_num;		   /* 8 */
    short prec_in_ray;		   /* 9 */
    /*
     * the original spec provided for rays to occupy more than one record
     */
    short swp_num;		   /* 10 */
    char  radar[8];		   /* 11 */
    char  site[8];		   /* 15 */
    short lat_deg;		   /* 19 */
    short lat_min;		   /* 20 */
    short lat_sec_x64;		   /* 21 */
    /*
     * x64 implies that this parameter has been scaled (multiplied) by 64
     */
    short lon_deg;		   /* 22 */
    short lon_min;		   /* 23 */
    short lon_sec_x64;		   /* 24 */
    short altitude_msl;		   /* 25 */
    short year;			   /* 26 */
    short month;		   /* 27 */
    short day;			   /* 28 */
    short hour;			   /* 29 */
    short minute;		   /* 30 */
    short second;		   /* 31 */
    char  time_zone[2];		   /* 32 */
    short azimuth_x64;		   /* 33 */
    short elevation_x64;	   /* 34 */
    short sweep_mode;		   /* 35 */
    /*
     * 0 = Calibration
     * 1 = PPI
     * 2 = Coplane
     * 3 = RHI
     * 4 = Vertical
     * 5 = Target
     * 6 = Manual
     * 7 = Idle
     * 8 = Surveilance
     */
    short fixed_angle_x64;	   /* 36 */
    short sweep_rate_x64;	   /* 37 */
    short gen_date_yy;		   /* 38 */
    short gen_date_mm;		   /* 39 */
    short gen_date_dd;		   /* 40 */
    char  facility_id[8];	   /* 41 */
    short missing_data_flag;	   /* 45 */
    short word_46;		/* filler to an even # of 16-bit words */

} UF_MANDITORY;


typedef struct {		   /* UF OPTIONAL HEADER */
    char  project_id[8];	   /* 1 */
    short baseline_az_x64;	   /* 5 */
    short baseline_el_x64;	   /* 6 */
    short vol_hour;		   /* 7 */
    short vol_minute;		   /* 8 */
    short vol_second;		   /* 9 */
    char  fld_tape_id[8];	   /* 10 */
    short range_info_flag;	   /* 14 */
    /*
     * 0 = range info the same for all fields in this volume
     * 1 = range info the same only within each sweep
     * 2 = range info the same only within each ray
     */
    short seconds_xe3;		   /* 15 */
    short software_rev;		   /* 16 */

} UF_OPTIONAL;

typedef struct {		   /* UF OPTIONAL HEADER */
    char  project_id[8];	   /* 1 */
    short baseline_az_x64;	   /* 5 */
    short baseline_el_x64;	   /* 6 */
    short vol_hour;		   /* 7 */
    short vol_minute;		   /* 8 */
    short vol_second;		   /* 9 */
    char  fld_tape_id[8];	   /* 10 */
    short range_info_flag;	   /* 14 */
    /*
     * 0 = range info the same for all fields in this volume
     * 1 = range info the same only within each sweep
     * 2 = range info the same only within each ray
     */

} OLD_UF_OPTIONAL;


typedef struct {
    char id[4];
    short array[6];		   /* reserve space for 6 words */
} UF_LOC_USE;

typedef struct {
    /*
     * Aircraft postitioning information
     */
    char id[4];			/* should contain "AIR" NULL */
    short ac_pitch_x64;		/* (3) */
    short ac_roll_x64;		/* (4) */

    short ac_phdg_x64;		/* (5) platform heading */
    short ac_vns_x10;		/* (6) north/south velocity */
    short ac_vew_x10;		/* (7) east/west velocity */
    short ac_wp3_x10;		/* (8) corrected vertical velocity */

    short ac_hgme;		/* (9) geometric altitude in meters */
    short ac_ui_x10;		/* (10) east-west wind */
    short ac_vi_x10;		/* (11) north-south wind */
    short ac_wi_x10;		/* (12) vertical wind */

    short ac_drift_x64;		/* (13) drift angle */
    short ac_rotation_x64;	/* (14) rotation angle */
    short ac_tilt_x64;		/* (15) tilt angle */
    short ac_phdg_change_x64;	/* (16) platform heading change deg per sec */

    short ac_pitch_change_x64;	/* (17) platform pitch change deg per sec */
    short ac_ant_rot_angle_x64;	/* (18) rotation angle corrected for roll */
    short ac_tilt_angle_x64;	/* (19) corrected tilt angle */
    short word_20;		/* filler */

} UF_LOC_USE_AC;

typedef struct {
    /*
     * UCLA aircraft postitioning information
     */
    char id[10];		/* should contain "INE VALUES"  */
    char flight_id[8];

    short word_10;		/* filler */

    short word_11;		/* filler */
    short ac_phdg_x64;		/* (12) platform heading */
    short ac_roll_x64;		/* (13) */
    short ac_pitch_x64;		/* (14) */
    short ac_drift_x64;		/* (15) drift angle */
    short ac_vud_x64;		/* (16) vertical velocity */
    short tail_vew_x64;		/* (17) east/west velocity */
    short tail_vns_x64;		/* (18) north/south velocity */
    short tail_vud_x64;		/* (19) up/down velocity */
    short word_20;		/* filler */

    short word_21;		/* filler */
    short ac_ant_rot_angle_x64;	/* (22) rotation angle corrected for roll */
    short ac_tilt_angle_x64;	/* (23) corrected tilt angle */
    short word_24;		/* filler */
    short word_25;		/* filler */

} UCLA_LOC_USE_AC;


typedef struct {
    char id[4];			/* should be "SHIP" */
    short  train_ord;
    short  elev_ord;

    short  pitch;
    short  roll;
    short  heading;
    short  azm_rate;

    short  elev_rate;
    short  pitch_rate;
    short  roll_rate;
    short  heading_rate;

    short  vel_e;
    short  vel_n;
    short  vel_u;

    short  nav_sys_flag;
    short  rad_vel_cor;
} UF_LOC_USE_SHIP;

typedef struct {
    char  id[2];		   /* 1 */
    short fld_hed_ptr;		   /* 2 */
} UF_FLD_ID;


typedef struct {
    UF_FLD_ID field[1];
} UF_FLD_ID_ARRAY;


typedef struct {
    short num_flds_this_ray;	   /* 1 */
    short num_recs_this_ray;	   /* 2 */
    short num_flds_this_rec;	   /* 3 */
    /*
     * followed by the requisit number of field ids
     */
} UF_DATA_HED;

/*
 * xe6 is short hand for scaling by 1.0e6
 */

typedef struct {		   /* UF FIELD DATA HEADER */
    unsigned short fld_data_ptr;   /* 1 */
    short scale;		   /* 2 (if -1 use alt_scale and alt_bias) */
    short range_g1;		   /* 3 */
    short g1_adjust_xe3;	   /* 4 */
    short gate_spacing_xe3;	   /* 5 */
    short num_gates;		   /* 6 */
    unsigned short sample_vol_xe3; /* 7 */
    unsigned short h_beam_width_x64; /* 8 */
    unsigned short v_beam_width_x64; /* 9 */
    unsigned short rec_bw_xe6;	   /* 10 */
    short polarization;		   /* 11 */
    unsigned short waveln_x6400;   /* 12 */
    short num_samples;		   /* 13 */
    char  thr_fld[2];		   /* 14 */
    short thr_val;		   /* 15 */
    short thr_val_scale;	   /* 16 */
    char  edit_code[2];		   /* 17 */
    unsigned short prT_xe6;	   /* 18 */
    short bits_per_datum;	   /* 19 */
/*
 * place holders for radar parameters that are field type dependent
 */
    short word_20;
    short word_21;
    short word_22;
    short word_23;
    short word_24;
    short word_25;
/*
 * more field specific radar parameters (FSRPs)
 */
    short word_26;
    short word_27;
    short az_adjust_xe3;	   /* 28 */
    short el_adjust_xe3;	   /* 29 */
    short gt_sp_adjust_xe6;	   /* 30 */
    short alt_scale;               /* 31 (+ or - power of 2) */
    short alt_bias;		   /* 32 (scaled by alt_scale) */

} UF_FLD_HED;

/*
 * many of the parameters in the field specific parameters need
 * to be divide by the scale to get scientific units.
 * 
 */
struct fsrpg {
    short word_20;
    short word_21;
    short word_22;
    short word_23;
    short word_24;
    short word_25;
};
/*
 * for velocity data:
 * word 20: short nyq_vel;
 * word 21: char  flagged[2];
 *   contains "FL" if flagged velocities
 *   i.e if the low order bit of this field is
 *     0 than the velocity is unreliable and if it is
 *     1 than the velocity is considered ok
 */
struct fsrpv {
    short nyq_vel;		/* scaled */
    char flagged[2];		/* could contain an "FL" */
    short word_22;
    short word_23;
    short word_24;
    short word_25;
};
 
/*
 * for power fields:
 * word 20: short rad_const;
 * word 21: short noise_power;
 * word 22: short rec_gain;
 * word 23: short peak_power;
 * word 24: short ant_gain;
 * word 25: short pulse_dur_x64e6;
 */
struct fsrpp {
    short rad_const;		/* scaled */
    short noise_power;		/* scaled */
    short rec_gain;		/* scaled */
    short peak_power;		/* scaled */
    short ant_gain;		/* scaled */
    short pulse_dur_x64e6;
};

typedef struct {		   /* UF FIELD DATA HEADER */
    short fld_data_ptr;		   /* 1 */
    short scale;		   /* 2 (if -1 use alt_scale and alt_bias) */
    short range_g1;		   /* 3 */
    short g1_adjust_xe3;	   /* 4 */
    short gate_spacing_xe3;	   /* 5 */
    short num_gates;		   /* 6 */
    short sample_vol_xe3;	   /* 7 */
    short h_beam_width_x64;	   /* 8 */
    short v_beam_width_x64;	   /* 9 */
    short rec_bw_xe6;		   /* 10 */
    short polarization;		   /* 11 */
    short waveln_x6400;		   /* 12 */
    short num_samples;		   /* 13 */
    char  thr_fld[2];		   /* 14 */
    short thr_val;		   /* 15 */
    short thr_val_scale;	   /* 16 */
    char  edit_code[2];		   /* 17 */
    short prT_xe6;		   /* 18 */
    short bits_per_datum;	   /* 19 */
    union { /* the next six words are field type specdific */
	struct fsrpg fsrpg;
	struct fsrpv fsrpv;
	struct fsrpp fsrpp;
    } fsrp;
/*
 * more field specific radar parameters (FSRPs)
 */
    short word_26;
    short word_27;
    short az_adjust_xe3;	   /* 28 */
    short el_adjust_xe3;	   /* 29 */
    short gt_sp_adjust_xe6;	   /* 30 */
    short alt_scale;               /* 31 (+ or - power of 2) */
    short alt_bias;		   /* 32 (scaled by alt_scale) */

} UF_FLD_HEDU;

 
struct uf_structs {		/* struct to manage UF structs */
    UF_MANDITORY *man;
    UF_OPTIONAL *opt;
    union {			/* local use structs */
	UF_LOC_USE *lug;	/* local use generic */
	UF_LOC_USE_AC *luac;	/* RDP aircraft header */
	UCLA_LOC_USE_AC *luucla; /* UCLA's aircraft header */
	UF_LOC_USE_SHIP *luship; /* RDP shipboard loc use header */
    } lus;
    UF_DATA_HED *dhed;		/* first part of data header */
    UF_FLD_ID_ARRAY *fida[MAX_UF_FLDS];
    UF_FLD_HED *fhed[MAX_UF_FLDS];
    short *fdata[MAX_UF_FLDS];
};

#endif
