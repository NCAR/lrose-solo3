/* 	$Id$	 */
# ifndef TOGA_H
# define TOGA_H

# define  TOGA_CW_AZ_CORR 0
# define TOGA_CCW_AZ_CORR 0
# define  TOGA_CW_EL_CORR 0
# define TOGA_CCW_EL_CORR 0

/* Status bits */
# define RANDOM_CONTROLLED        0x0001
# define COHERENT_PHASE           0x0002
# define RANDOM_PHASE             0x0004
# define DIGITAL_SIGNAL_SIMULATOR 0x0008
# define KLYSTRON                 0x0010
# define MAGNETRON                0x0020
# define COAXIAL_MAG              0x0040

/* Threshold flags */
# define CLUTTER_FILTER           0x0001
# define SPECKLE_REMOVER          0x0002
# define CSR_THRESHOLD            0x0004
# define SNR_THRESHOLD            0x0008
# define RANGE_NORMALIZATION      0x0010
# define HAMMING_WINDOW           0x0020
# define PLATFORM_MOTION_CORR     0x0040

/* TOGA scan modes */
# define TOGA_PPI 1
# define TOGA_RHI 2
# define TOGA_MAN 3
# define SCAN_DEF_FILE 4

/* Signal processor configuration */
# define NUM_DSP_MASK             0x003f	/* low 6 bits */
# define DSP_CONFIG_MASK          0x0007
# define SINGLE_POLARIZATION      0x0001
# define SINGLE_POL_INTERLEAVED   0x0002
# define DUAL_POLARIZATION        0x0004

/* Extra ray header info bits */
# define IFF_DATA_AVAILABLE       0x0001
# define ROLL_AVAILABLE           0x0002
# define PITCH_AVAILABLE          0x0004
# define HEADING_AVAILABLE        0x0008

/* Navigator flags */
# define NAV_INPUT_OK             0x0001
# define NAV_LAT_LON              0x0002
# define NAV_ALTITUDE             0x0004
# define NAV_SPEED_HEAD           0x0008
# define NAV_SET_DRIFT            0x0010

/* Toga parameter id numbers */
# define TOGA_SNR 1
# define TOGA_VELOCITY 2
# define TOGA_SPECTRAL_WIDTH 3
# define TOGA_DBZ 4
# define TOGA_Z_ 5

/* Toga data types */
# define TOGA_DOPPLER_DATA 1
# define SLOW_ALTERNATING_DOPPLER 2
# define DUAL_RECEIVER_DOPPLER 3
# define TOGA_REFLECTIVITY_DATA 19

/* Miscellaneous def */
# define TOGA_LAST_RECORD 0
# define TOGA_END_OF_RAY 1
# define TOGA_MAX_FIELDS 16
# define TOGA_MAX_GATES 2048

# define SIGN16 0x8000
# define SIGN15 0x4000
# define MASK16 0xffff
# define MASK15 0x7fff
# define SIGN12 0x0800
# define MASK12 0x0fff
# define MASK11 0x07ff
# define SIGN10 0x0200
#ifndef MASK10  //Jul 26, 2011
# define MASK10 0x03ff
#endif  //Jul 26, 2011

# define RCP16 0.0625
# define RCP60 0.01666667
# define RCP512 0.001953125
# define S100(x) ((x)*100.0+0.5)
#ifndef S10  //Jul 26, 2011
# define S10(x) ((x)*10.0+0.5)
#endif  //Jul 26, 2011


/* c...mark */

struct toga_header {
    short storm_year;
    short storm_month;
    short storm_day;
    short storm_num;
    short storm_map_num;
    short year;
    short month;
    short day;
    short hour;
    short minute;
    short seconds_x100;
    short data_set;
    /* polarization info
     */
    short pol11_axial_ratio_x100; /* for the ellipse */
    short pol11_orientation;	/* degrees ccw */
    short pol11_power_div_bits;	/* low 7 bits */
    short pol11_delay_bits;	/* low 7 bits */

    short pol12_axial_ratio_x100; /* for the ellipse */
    short pol12_orientation;	/* degrees ccw */
    short pol12_power_div_bits;	/* low 7 bits */
    short pol12_delay_bits;	/* low 7 bits */

    short status_bits;
    short bin1_range_x40;	/* km. */
    short num_bins;
    short bin_spacing;		/* m. */
    short range_jitter;		/* 1=jitter by 1/2 bin_spacing */
    short num_cal_bins;
    short cal_bin1_range_x40;	/* km. */
    short cal_bin2_range_x40;	/* km. */
    short cal_bins_step;	/* m. */
    short min_angle_x10;
    short max_angle_x10;
    short min_elevation_x10;
    short max_elevation_x10;
    short angular_resolution_x100;
    short num_fixed_angles;

    /* 36-55 contain fixed angles_x10
     */
    short fixed_angle_x10[20];
    /* end fixed angles */

    short word_56;
    short signal2noise_thr_x16;	/* dB */
    short signal2clutter_thr_x16; /* dB */
    short threshold_flags;
    short num_dop_sig_procs;
    short ray_header_size;	/* in 16-bit words */
    short scan_mode;

    /* 63-70 name of scan def file
     */
    char scan_def_file_name[16];

    short PRF;
    short num_samples;
    short sig_proc_config;
    short word_74;
    short rec_sat_level1;	/* in dBZ */
    short rec_sat_level2;	/* in dBZ */
    /* 0 sat_level means standard range dependent STC */

    /* Bias levels */
    short word_77;
    short word_78;
    short word_79;
    short word_80;
    short word_81;
    short word_82;
    short word_83;
    short word_84;
    short word_85;
    short word_86;
    short word_87;
    short word_88;

    short wavelength_xe4;	/* m. */
    short pulse_width_xe8;	/* sec. */
    short horz_xmit_power;
    short vert_xmit_power;
    short height_of_zeroing;	/* km. */
    short coarse_latitude_x100;	/* deg. */
    short coarse_longitude_x100; /* deg. */
    short time_zone_offset;	/* minutes ahead of GMT */

    /* Z slopes and MDZs */
    short word_97;
    short word_98;
    short word_99;
    short word_100;
    short word_101;
    short word_102;
    short word_103;
    short word_104;

    short num_polar_pairs;

    short xtra_info;		/* flags for additional nav data */

    short extended_ray_header;
    /* 0 implies 20 words long
     * assume > 0 implies 20 +extend value
     */
    short latitude_degrees;
    short latitude_minutes_x100;
    short longitude_degrees;
    short longitude_minutes_x100;
    short ant_altitude_msl;
    short gnd_altitude_msl;
    short speed_x100; /* m/s */
    short velocity_correction;
    short heading_x10;		/* deg */
    short direction_x10;	/* deg */
    short set_of_platform_x10;	/* deg */
    short drift_x100;		/* m/s */
    short navigator_flags;

    /* 121-199 spare words */
    short word_121;
    short word_122;
    short word_123;
    short word_124;
    short word_125;
    short word_126;
    short word_127;
    short word_128;
    short word_129;
    short word_130;
    short word_131;
    short word_132;
    short word_133;
    short word_134;
    short word_135;
    short word_136;
    short word_137;
    short word_138;
    short word_139;
    short word_140;
    short word_141;
    short word_142;
    short word_143;
    short word_144;
    short word_145;
    short word_146;
    short word_147;
    short word_148;
    short word_149;
    short word_150;
    short word_151;
    short word_152;
    short word_153;
    short word_154;
    short word_155;
    short word_156;
    short word_157;
    short word_158;
    short word_159;
    short word_160;
    short word_161;
    short word_162;
    short word_163;
    short word_164;
    short word_165;
    short word_166;
    short word_167;
    short word_168;
    short word_169;
    short word_170;
    short word_171;
    short word_172;
    short word_173;
    short word_174;
    short word_175;
    short word_176;
    short word_177;
    short word_178;
    short word_179;
    short word_180;
    short word_181;
    short word_182;
    short word_183;
    short word_184;
    short word_185;
    short word_186;
    short word_187;
    short word_188;
    short word_189;
    short word_190;
    short word_191;
    short word_192;
    short word_193;
    short word_194;
    short word_195;
    short word_196;
    short word_197;
    short word_198;
    short word_199;

    /* More polarization info */
    short word_200;
    short word_201;
    short word_202;
    short word_203;
    short word_204;
    short word_205;
    short word_206;
    short word_207;
    short word_208;
    short word_209;
    short word_210;
    short word_211;
    short word_212;
    short word_213;
    short word_214;
    short word_215;
    short word_216;
    short word_217;
    short word_218;
    short word_219;
    short word_220;
    short word_221;
    short word_222;
    short word_223;
    short word_224;
    short word_225;
    short word_226;
    short word_227;
    short word_228;
    short word_229;
    short word_230;
    short word_231;
    short word_232;
    short word_233;
    short word_234;
    short word_235;
    short word_236;
    short word_237;
    short word_238;
    short word_239;
    short word_240;
    short word_241;
    short word_242;
    short word_243;
    short word_244;
    short word_245;
    short word_246;
    short word_247;
    short word_248;
    short word_249;
    short word_250;
    short word_251;
    short word_252;
    short word_253;
    short word_254;
    short word_255;

    /* 256-300 spare words */
    short word_256;
    short word_257;
    short word_258;
    short word_259;
    short word_260;
    short word_261;
    short word_262;
    short word_263;
    short word_264;
    short word_265;
    short word_266;
    short word_267;
    short word_268;
    short word_269;
    short word_270;
    short word_271;
    short word_272;
    short word_273;
    short word_274;
    short word_275;
    short word_276;
    short word_277;
    short word_278;
    short word_279;
    short word_280;
    short word_281;
    short word_282;
    short word_283;
    short word_284;
    short word_285;
    short word_286;
    short word_287;
    short word_288;
    short word_289;
    short word_290;
    short word_291;
    short word_292;
    short word_293;
    short word_294;
    short word_295;
    short word_296;
    short word_297;
    short word_298;
    short word_299;
    short word_300;

    char comments[680];
};

/* c...mark */

struct toga_rec_header {
    short first_rec_ptr;	/* 0 if no ray begins in this rec */
    short rec_num;
    short last_rec_flag;
    short word_4;
};

struct toga_ray_header {
    unsigned short azimuth;	/* binary angle */
    unsigned short elevation;	/* binary angle */
    short year;
    short month;
    short day;
    short hour;
    short minute;
    short seconds_x100;
    short tilt_num;
    short step_attn_output;
    short data_type;
    short range_bin1_x40;	/* km */
    short first_killed_bin;
    short last_killed_bin;

    unsigned short roll;			/* binary angle */
    unsigned short pitch;		/* binary angle */
    unsigned short heading;		/* binary angle */
    short ships_speed_x100;	/* m/s */
    short latitude_minutes_x100;
    short longitude_minutes_x100;
};

struct toga_useful_items {
    double start_time;
    double stop_time;

    double sum_delta_az;
    double sum_delta_el;

    float last_az;
    float last_el;
    float cw_az_corr;
    float ccw_az_corr;
    float base_latitude;
    float base_longitude;
    float toga_data_scale[TOGA_MAX_FIELDS];
    float toga_data_offset[TOGA_MAX_FIELDS];

    int radar_id;
    int fid;
    int io_type;
    int min_ray_size;
    int ok_ray;
    int header_count;
    int new_vol;
    int new_sweep;
    int sweep_ray_count;

    int prev_data_fmt;
    int num_fields;
    int data_type_id[TOGA_MAX_FIELDS];
    int relative_offset[TOGA_MAX_FIELDS];
    int gate_width;

    short *toga_lut[TOGA_MAX_FIELDS];

    int ray_count;
    int sweep_count_flag;
    int sweep_count;
    int vol_count_flag;
    int vol_count;
};

#endif
