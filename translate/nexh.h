/*      $Id$         */
# ifndef NEXH_H
# define NEXH_H

# define NEXRAD_FILE_NAME "ARCHIVE2"

/* message types
 */
# define            DIGITAL_RADAR_DATA 1
# define               RDA_STATUS_DATA 2
# define  PERFORMANCE_MAINTENANCE_DATA 3
# define           CONSOLE_MESSAGE_A2G 4
# define          MAINTENANCE_LOG_DATA 5
# define          RDA_CONTROL_COMMANDS 6
# define       VOLUME_COVERAGE_PATTERN 7
# define          CLUTTER_SENSOR_ZONES 8
# define              REQUEST_FOR_DATA 9
# define           CONSOLE_MESSAGE_G2A 10
# define         LOOPBACK_TEST_RDA_RPG 11
# define         LOOPBACK_TEST_RDG_RPA 12
# define     CLUTTER_FILTER_BYPASS_MAP 13
# define     EDITED_CLUTTER_FILTER_MAP 14

# define        UNDEFINED_CLUTTER_DATA 15
# define       CLUTTER_NOTCH_WIDTH_MAP 15

# define           CONSOLE_MESSAGE_A2U 20
# define           CONSOLE_MESSAGE_U2G 21
# define         LOOPBACK_TEST_RDG_A2U 22
# define         LOOPBACK_TEST_RDG_U2A 22

# define        NEXRAD_EXTENDED_FORMAT 1995

# define          SITE_ADAPTATION_DATA 101 /* each VCP */
# define           ONLINE_RECEIVER_CAL 102 /* each VCP */
# define        ONLINE_TRANSMITTER_CAL 103 /* each 8 hours */
# define            SUN_CHECK_CAL_DATA 104
# define      SUPPLEMENTAL_VOLUME_DATA 111
# define       SUPPLEMENTAL_SWEEP_DATA 112
# define               RIDDS_VOLUME_ID 201
# define              RIDDS_TIME_STAMP 202

/* Weirdness
 */
# define KRGX_TYPE_1 257
# define KRGX_TYPE_2 513
# define KRGX_TYPE_3 514

/* radial status
 */
# define  START_OF_NEW_ELEVATION 0
# define     INTERMEDIATE_RADIAL 1
# define        END_OF_ELEVATION 2
# define   BEGINNING_OF_VOL_SCAN 3
# define         END_OF_VOL_SCAN 4

/* doppler velocity resolution
 */
# define POINT_FIVE_METERS_PER_SEC 2
# define ONE_METER_PER_SEC 4

/* volume coverage patterns
 */
# define SCAN_16_PER_5_MIN 11
# define SCAN_11_PER_5_MIN 21
# define SCAN_8_PER_10_MIN 31
# define SCAN_7_PER_10_MIN 32



# ifndef MAXCVGATES
#define MAXCVGATES 1500
# endif

# define   NEX_HORZ_BEAM_WIDTH 0.95
# define   NEX_VERT_BEAM_WIDTH 0.95
# define        NEX_NUM_DELTAS 5 /* num frequencies? */
# define    NEX_NUM_UNAMB_RNGS 8
# define       NEX_FIXED_ANGLE 0.005493164063
# define           NEX_AZ_RATE 0.001373291016
# define    NEX_NOMINAL_WAVELN 0.1053
# define       NEX_PACKET_SIZE 2432
# define        NEX_MAX_FIELDS 16
# define         NEX_MAX_GATES 960
# define     NEX_MAX_REF_GATES 920
# define             NEX_DZ_ID 1
# define             NEX_VE_ID 2
# define             NEX_SW_ID 4

/* options! */
# define         NEX_REFL_ONLY 0x0001
# define          NEX_VEL_ONLY 0x0002
# define            RAP_NEXRAD 0x0004
# define           NEX_RNG_AMB 0x0008
# define           NEX_MAX_RNG 0x0010
# define          NEX_RAW_VALS 0x0020
# define     NEX_NCDC_BLOCKING 0x0040


struct nexrad_id_rec {          /* "ARCHIVE2" */
    char filename[8];
};

struct nexrad_vol_scan_title {  /* "ARCHIVE2.001,..." */
    char filename[9];
    char extension[3];
    int32_t julian_date;           /* from 1/1/70 */
    int32_t milliseconds_past_midnight;
    int32_t filler_1;
};

struct CTM_info {               /* ignored */
    short CTM_word_1;
    short CTM_word_2;
    short CTM_word_3;
    short CTM_word_4;
    short CTM_word_5;
    short CTM_word_6;
};

struct nexrad_message_header {
    short message_len;          /* in 16-bit words */
    unsigned char channel_id;
    unsigned char message_type;
    short seq_num;              /* mod 0x7fff */
    short julian_date;          /* from 1/1/70 */
    int32_t milliseconds_past_midnight;
    short num_message_segs;
    short message_seg_num;
};

struct rda_status_info {
   short rda_status;		/* ( 1) halfword location */
   short oper_status;		/* ( 2) */
   short cntrl_status;		/* ( 3) */
   short aux_pwr_gen_state;	/* ( 4) */
   short atp;			/* ( 5) */
   short ref_cal_corr;		/* ( 6) */
   short dte;			/* ( 7) */
   short vcp;			/* ( 8) */
   short rds_cntl_auth;		/* ( 9) */
   short intefr_det_rate;	/* (10) */
   short op_mode;		/* (11) */
   short intefr_suppr_unit;	/* (12) */
   short arch2status;		/* (13) */
   short arch2vols;		/* (14) */
   short rda_alarms;		/* (15) */
   short command_ak;		/* (16) */
   short ch_cntrl_stat;		/* (17) */
   short spol_blnk_stat;	/* (18) */
   short bypass_map_date;	/* (19) */
   short bypass_map_time;	/* (20) */
   short notch_map_date;	/* (21) */
   short notch_map_time;	/* (22) */
   short tps_stat;		/* (23) */
   short spare1;		/* (24) */
   short spare2;		/* (25) */
   short spare3;		/* (26) */
   short alarm_codes[14];	/* (27-40) */
};

struct digital_radar_data_header {
    int32_t milliseconds_past_midnight; /* (15-16) */
    short julian_date;          /* (17) from 1/1/70 */
    short unamb_range_x10;      /* (18) km. */
    unsigned short azimuth;     /* (19) binary angle */
    short radial_num;           /* (20) */
    short radial_status;        /* (21) */
    unsigned short elevation;   /* (22) binary angle */
    short elev_num;             /* (23) */
    short ref_gate1;            /* (24) meters */
    short vel_gate1;            /* (25) meters */
    short ref_gate_width;       /* (26) meters */
    short vel_gate_width;       /* (27) meters */
    short ref_num_gates;        /* (28) */
    short vel_num_gates;        /* (29) */
    short sector_num;           /* (30) */
    float sys_gain_cal_const;   /* (31-32) */
    short ref_ptr;              /* (33) byte count from start of drdh */
    short vel_ptr;              /* (34) byte count from start of drdh */
    short sw_ptr;               /* (35) byte count from start of drdh */
    short velocity_resolution;  /* (36) */
    short vol_coverage_pattern; /* (37) */
    short VNV1;                 /* V & V simulator reserved */
    short VNV2;
    short VNV3;
    short VNV4;
    short ref_data_playback;    /* (42) */
    short vel_data_playback;    /* (43) */
    short sw_data_playback;     /* (44) */
    short nyquist_vel_x100;     /* (45)m/s */
    short atmos_atten_factor_x1000; /* (46) dB/km */
    short threshold_parameter;  /* (47) */
    /* c...mark */
    /* word_?? are meant as fillers and correspond to the documentation */
    short word_48;
    short word_49;
    short word_50;
    short word_51;
    short word_52;
    short word_53;
    short word_54;
    short word_55;
    short word_56;
    short word_57;
    short word_58;
    short word_59;
    short word_60;
    short word_61;
    short word_62;
    short word_63;
    short extended_header_ptr;	/* byte count from start of drdh */
};

/* new headers for extended format
 */

struct nexrad_field_header {
    short bytes_in_this_header;
    char  name[8];
    short ptr_to_next_header;	/* byte count from start of drdh */
    short data_ptr;             /* byte count from start of drdh */
    short bytes_per_gate;

    int32_t scale;                 /* might contain an IEEE float */
    int32_t bias;                  /* might contain an IEEE float */
    short scale_mult;		/* = 0 => scale is IEEE float
				 * otherwise the scale has been multiplied
				 * by this number */
    short bias_mult;		/* = 0 => bias is IEEE float */

    short num_gates;
    short meters_to_gate1;
    short meters_per_gate;
    unsigned short base_prf_in_hz_x100;
    unsigned short km_unambiguous_range_x100;
    unsigned short meters_unambiguous_vel_x100;
    short snr_thresh_db_x8;
    short xmit_power_db_x8;
};


struct nexrad_extended_header {
    short bytes_in_this_header;
    short bytes_in_this_packet;
    short volume_count;
    short num_fields;
    short first_field_header_ptr; /* byte count from start of drdh */
};

struct nexrad_xtra_volume_info { /* maps SUPPLEMENTAL_VOLUME_DATA */

    char station_name[8];
    int32_t deg_latitude_x100000;
    int32_t deg_longitude_x100000;

    short meters_altitude_msl;
    short frequency_mhz;
    short deg_beamwidth_x100;
};

struct nexrad_xtra_sweep_info { /* maps SUPPLEMENTAL_SWEEP_DATA */
    short fixed_angle_x64;
    short pulse_width_xe9;      /* nanoseconds */
    short optimal_scan_rate_x64; /* degrees per second */
    short clutter_filtering;
    short num_prfs;
    short ptr_first_prf_set;    /* byte count from start of drdh */
};

struct nexrad_prf_set {
    short bytes_in_this_header;
    short ptr_to_next_prf_set;	/* byte count from start of drdh */
    unsigned short prf_in_hz_x100;
};

# ifndef S100
# define S64(x) ((x) * 64. +.5)
# define US64(x) ((x) * .015625)
# define S100(x) ((x) * 100. +.5)
# define US100(x) ((x) * .01)
# endif

# define NEX_SCALE(x, scale, bias) (((x) + (bais)) * (scale))
# define NEX_UNSCALE(x, rcp_scale, bias) ((x) * (rcp_scale) - (bias))









/* translator structs only--proceed with caution! */

struct nex_unique_field_info {
    char field_name[12];
    char long_field_name[48];
    char *data_ptr;
    short *data_lut;
    short sizeof_data_lut;
    short *gate_lut;
    short sizeof_gate_lut;
    struct nexrad_field_header nfh;
    int gri_ndx;
};


struct nexrad_useful_items {
    float latitude;
    float longitude;
    float altitude;
    float nyquist_vel;
    float range_ambiguity_flag;
    int fid;
    int io_type;
    int ref_ndx;
    int vel_ndx;
    int sw_ndx;
    int new_vol;
    int new_sweep;
    int ray_count;
    int sweep_count;
    int sweep_count_flag;
    int vol_count;
    int vol_count_flag;
    int bytes_left;
    int initial_missing_gates;
    int dz_offset;
    int previous_parameters;
    int current_parameters;
    int max_gates;

    short dz_replicate[NEX_MAX_GATES];
    short *lut[NEX_MAX_FIELDS]; /* look up tables */
    char *ref_ptr;
    char *vel_ptr;
    char *sw_ptr;
    int nex_packet_size;
    int num_fields;
    struct nex_unique_field_info nufi[NEX_MAX_FIELDS];
    int options;
};


struct nexrad_site_stuff {
    int site_number;
    char radar_name[8];
    char city[20];
    char state[4];
    double latitude;
    double longitude;
    float altitude;
    float frequency_mhz;
    float short_pulse_ns;
    float long_pulse_ns;
};


struct nexrad_VCP_items {
    int fixed_angle;
    int wave_type;
    int is_prf_num;
    int is_pulse_count;
    int az_rate;
    int item_6;
    int id_prf_num;
    int id_pulse_count;
    int item_9;
    int item_10;
    int item_11;
    int item_12;
    int item_13;
    int item_14;
    int item_15;
    int item_16;
    int item_17;
};


struct nexrad_VCP_header {
    int item_1;
    int pattern_type;
    int VCP_num;
    int num_sweeps;
    int item_5;
    int pulse_flag;
    struct nexrad_VCP_items *swp_info;
    struct nexrad_VCP_header *next;
};

#endif
