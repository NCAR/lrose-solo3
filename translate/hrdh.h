/* 	$Id$	 */
#ifndef hrd_dd_h
#define hrd_dd_h

# define HRD_MAX_FIELDS 6
# define HRD_MAX_GATES 1024

# define  FIXED_ANGLE_TOLERANCE  5.
# define   AFT_TARGET_ELEVATION -22.
# define  FORE_TARGET_ELEVATION  22.
# define   ORTHOGONAL_TOLERANCE   4.

# define       LF_RADAR_ID 0x01	/* lower fuselage */
# define     TAIL_RADAR_ID 0x02	/* tail normal pointing */
# define    T_AFT_RADAR_ID 0x04	/* tail aft pointing */
# define    T_FOR_RADAR_ID 0x08	/* tail fore pointing */
# define LF_SECTOR_RADAR_ID 0x10 /* lower fuselage sector mode */

# define HRD_BANG_ZAP 5

struct hrd_header {
    /* tape header general information */
    short header_flag;		/* should be a 0 */
    short sizeof_header;
    short tape_num;
    short hd_fmt_ver;
    short word_5;
    short year;		
    short month;	
    short day;		
    short hour;		
    short minute;	
    short second;	
    char LF_menu[16];
    char TA_menu[16];
    char Data_menu[16];
    short word_36;
    short nav_system;		/* 0=ONE,1=INE1,2=INE2 */
    short LU_tape_drive;
    short aircraft_id;		/* 42,43 (0 for ground) */
    char flight_id[8];
    short data_header_len;	/* # 16-bit words */
    short ray_header_len;	/* # 16-bit words */
    short time_zone_offset;	/* minutes ahead of GMT */

    /* Power up test results */
    short word_47;
    short word_48;
    short word_49;
    short word_50;
    short word_51;
    short word_52;
    short word_53;
    short word_54;

    /* Display configuration when header written */
    short word_55;
    short word_56;
    short word_57;
    short word_58;
    short word_59;
    short word_60;
    short word_61;
    short word_62;
    short word_63;
    short word_64;
    short word_65;
    short word_66;
    short word_67;
    short word_68;
    short word_69;
    short word_70;
    short word_71;
    short word_72;
    short word_73;
    short word_74;
    short word_75;
    short word_76;
    short word_77;
    short word_78;
    short word_79;
    short word_80;
    char project_id[8];
    short word_85;
    short word_86;
    short word_87;
    short word_88;
    short word_89;
    short word_90;
    short word_91;
    short word_92;
    short word_93;
    short word_94;
    short word_95;
    short word_96;
    short word_97;
    short word_98;
    short word_99;
    short word_100;

    /* LF Radar Information words 101-400 */
    /* TA Radar Information words 401-700 */

};
/* c...mark */

struct hrd_radar_info {
    short sample_size;
    short DSP_flag;
    /*
     * 0: Range normalization 
     * 1: Doppler channel speckle remover 
     * 2: Log channel speckle remover 
     * 3: Pulse at end of ray 
     * 4: Pulse at beginning of ray 
     * 6: Use AGC (TA only) 
     * 8-9: 0:single PRF, 1:dual PRF 2/3, 2:dual PRF 3/4
     */
    short refl_slope_x4096;	/* dB per A/D count */
    short refl_noise_thr_x16;	/* dB above noise */
    short clutter_cor_thr_x16;	/* signed dB */
    short SQI_thr;		/* same units as DSP */
    short width_power_thr_x16;	/* dBZ */
    short calib_refl_x16;	/* dBZ */
    short AGC_decay_code;
    short dual_PRF_stabil_delay; /* pulses */
    short thr_flags_uncorr_refl;
    short word_112;
    short thr_flags_vel;
    short thr_flags_width;
    short data_mode;		/* 1:processed data, 2:time series */
    short word_116;
    short word_117;
    short word_118;
    short word_119;
    short word_120;
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

    /* Range Mask Configuration */
    short range_b1;		/* km. portion */
    short variable_spacing_flag; /* 1:var, 0:fixed */
    /*
     * if variable gate spacing then the gate spacing is
     *   75 m for the first 256 gates
     *  150 m for the next  128 gates
     *  300 m for the next  128 gates
     */
    short bin_spacing_xe3;
    short num_input_bins;
    short range_avg_state;	/* (1,2,3,4)(undefined in var mode) */
    short b1_adjust_xe4;
    short word_147;
    short word_148;
    short num_output_bins;
    short word_150;

    /* Noise sample information */
    short PRT_noise_sample;
    short range_noise_sample;	/* km. */
    short log_rec_noise_x64;	/* A2D units */
    short I_A2D_offset_x256;	/* A2D units */
    short Q_A2D_offset_x256;	/* A2D units */
    short word_156;
    short word_157;
    short word_158;
    short word_159;
    short word_160;

    /* DSP Diagnostics */
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

    /* Miscellaneous Information */
    short word_171;
    short waveln_xe4;		/* m. */
    short pulse_width_xe8;	/* sec. */
    short PRF;
    short word_175;
    short DSS_flag;		/* 0:off, 1:on */
    short trans_recv_number;
    short transmit_power;
    short gain_control_flag;	/* 0:full, 1:STC, 2:AGC */
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

    /* Antenna Scanning Information */
    short scan_mode;
    short sweep_speed_x10;	/* RPM */
    unsigned short tilt_angle;	/* binary angle */
    short sector_center;	/* degrees */
    short sector_width;		/* degrees */
    short word_196;
    short word_197;
    short word_198;
    short word_199;
    short word_200;

    /* Real Time Display Color Configuration */
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
    short word_301;
    short word_302;
    short word_303;
    short word_304;
    short word_305;
    short word_306;
    short word_307;
    short word_308;
    short word_309;
    short word_310;
    short word_311;
    short word_312;
    short word_313;
    short word_314;
    short word_315;
    short word_316;
    short word_317;
    short word_318;
    short word_319;
    short word_320;
    short word_321;
    short word_322;
    short word_323;
    short word_324;
    short word_325;
    short word_326;
    short word_327;
    short word_328;
    short word_329;
    short word_330;
    short word_331;
    short word_332;
    short word_333;
    short word_334;
    short word_335;
    short word_336;
    short word_337;
    short word_338;
    short word_339;
    short word_340;
    short word_341;
    short word_342;
    short word_343;
    short word_344;
    short word_345;
    short word_346;
    short word_347;
    short word_348;
    short word_349;
    short word_350;
    short word_351;
    short word_352;
    short word_353;
    short word_354;
    short word_355;
    short word_356;
    short word_357;
    short word_358;
    short word_359;
    short word_360;
    short word_361;
    short word_362;
    short word_363;
    short word_364;
    short word_365;
    short word_366;
    short word_367;
    short word_368;
    short word_369;
    short word_370;
    short word_371;
    short word_372;
    short word_373;
    short word_374;
    short word_375;
    short word_376;
    short word_377;
    short word_378;
    short word_379;
    short word_380;
    short word_381;
    short word_382;
    short word_383;
    short word_384;
    short word_385;
    short word_386;
    short word_387;
    short word_388;
    short word_389;
    short word_390;
    short word_391;
    short word_392;
    short word_393;
    short word_394;
    short word_395;
    short word_396;
    short word_397;
    short word_398;
    short word_399;
    short word_400;
};

/* c...mark */

struct hrd_data_rec_header {
    short data_record_flag;	/* should be a 1 */
    unsigned short sizeof_rec;
    short sweep_num;
    short rec_num;
    char radar_num;		/* 1:LF, 2:TA */
    char rec_num_flag;		/* 1:first, 0:middle, 2:last */
};

struct hrd_ray_header {
    unsigned short sizeof_ray;
    unsigned char code;
    /* bit 15 set: reflectivity */
    /* bit 14 set: velocity */
    /* bit 13 set: width */
    /* bit 12 set: data from TA DSP */
    /* bit 11 set: data from LF DSP */
    /* bit 10 set: time series */
    char year;
    char month;
    char day;
    unsigned char raycode;	/* word_4 */
    char hour;
    short minute;
    short seconds_x100;
    short latitude;	/* (binary angle) */
    short longitude;	/* (binary angle) word_8 */
    short altitude_xe3;
    /*
     * executables of the translaters earlier than 1200 Apr.18, 1994
     * will produce files with ns and ew velocities and winds
     * swapped
     */
    short ac_vew_x10;		/* east-west velocity */
    short ac_vns_x10;		/* north-south velocity */
    short ac_vud_x10;		/* vertical velocity */
    short ac_ui_x10;		/* east-west wind */
    short ac_vi_x10;		/* north-south wind */
    short ac_wi_x10;		/* vertical wind */
    short RCU_status;
    short elevation;	/* (binary angle) */
    short azimuth;	/* (binary angle) */
    short ac_pitch;	/* (binary angle) */
    short ac_roll;	/* (binary angle) */
    short ac_drift;	/* (binary angle) */
    short ac_heading;	/* (binary angle) */
};

struct hrd_generic_header {
    short header_id;
    short sizeof_rec;
};

struct mc_ray_header {
    float ntasweep;
    float latitude;	/* (binary angle) */
    float longitude;	/* (binary angle)  */
    float altitude_xe3;	/* word_4 */
    float ac_vew_x10;		/* east-west velocity */
    float ac_vns_x10;		/* north-south velocity */
    float ac_vud_x10;		/* vertical velocity */
    float ac_ui_x10;		/* east-west wind */
    float ac_vi_x10;		/* north-south wind */
    float ac_wi_x10;		/* vertical wind */
    float elevation;	/* (binary angle) */
    float azimuth;	/* (binary angle) word_12 */
    float ac_pitch;	/* (binary angle) */
    float ac_roll;	/* (binary angle) */
    float ac_drift;	/* (binary angle) */
    float ac_heading;	/* (binary angle) word_16 */
    float hour;
    float minute;
    float seconds_x100;	/* word_19 */
    float nwords16;
    float ndata;	/* word_21 */
};


struct hrd_mc_radar_info {
    short sample_size;
    short DSP_flag;
    /*
     * 0: Range normalization 
     * 1: Doppler channel speckle remover 
     * 2: Log channel speckle remover 
     * 3: Pulse at end of ray 
     * 4: Pulse at beginning of ray 
     * 6: Use AGC (TA only) 
     * 8-9: 0:single PRF, 1:dual PRF 2/3, 2:dual PRF 3/4
     */
    short refl_slope_x4096;	/* dB per A/D count */
    short refl_noise_thr_x16;	/* dB above noise */
    short clutter_cor_thr_x16;	/* signed dB */
    short SQI_thr;		/* same units as DSP */
    short width_power_thr_x16;	/* dBZ */
    short calib_refl_x16;	/* dBZ */
    short AGC_decay_code;
    short dual_PRF_stabil_delay; /* pulses */
    short thr_flags_uncorr_refl;
    short word_112;
    short thr_flags_vel;
    short thr_flags_width;
    short data_mode;		/* 1:processed data, 2:time series */
    short word_116;		/* 116/117 => BINARY */
    short word_117;
    short word_118;
    short word_119;
    short word_120;
    short word_121;
    short word_122;
    short word_123;
    short word_124;
    short word_125;
    short word_126;
    short word_127;		/*  */
    short word_128;		/*  */
    short word_129;		/*  */
    short word_130;		/*  */
    short word_131;		/*  */
    short word_132;
    short word_133;
    short word_134;
    short word_135;
    short word_136;
    short word_137;
    short word_138;
    short word_139;
    short word_140;

    /* Range Mask Configuration */
    short range_b1;		/* km. portion word_141 */
    short variable_spacing_flag; /* 1:var, 0:fixed */
    /*
     * if variable gate spacing then the gate spacing is
     *   75 m for the first 256 gates
     *  150 m for the next  128 gates
     *  300 m for the next  128 gates
     */
    short bin_spacing_xe3;	/* NRNS word_143 */
    short num_input_bins;	/*  */
    short range_avg_state;	/* (1,2,3,4)(undefined in var mode) */
    short b1_adjust_xe4;	/* word_146 */
    short word_147;
    short word_148;
    short num_output_bins;	/* NBINS */
    short word_150;

    /* Noise sample information */
    short PRT_noise_sample;
    short range_noise_sample;	/* km. */
    short log_rec_noise_x64;	/* A2D units */
    short I_A2D_offset_x256;	/* A2D units */
    short Q_A2D_offset_x256;	/* A2D units */
    short word_156;
    short word_157;
    short word_158;
    short word_159;		/* NOISEMTA */
    short word_160;

    /* DSP Diagnostics */
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

    /* Miscellaneous Information */
    short word_171;
    short waveln_xe4;		/* m. */
    short pulse_width_xe8;	/* sec. */
    short PRF;
    short word_175;
    short DSS_flag;		/* 0:off, 1:on */
    short trans_recv_number;
    short transmit_power;
    short gain_control_flag;	/* 0:full, 1:STC, 2:AGC */
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

    /* Antenna Scanning Information */
    short scan_mode;
    short sweep_speed_x10;	/* RPM */
    unsigned short tilt_angle;	/* binary angle */
    short sector_center;	/* degrees */
    short sector_width;		/* degrees */
    short word_196;
    short word_197;
    short word_198;
    short word_199;
    short word_200;

    /* Real Time Display Color Configuration */
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
    short word_301;
    short word_302;
    short word_303;
    short word_304;
    short word_305;
    short word_306;
    short word_307;
    short word_308;
    short word_309;
    short word_310;
    short word_311;
    short word_312;
    short word_313;
    short word_314;
    short word_315;
    short word_316;
    short word_317;
    short word_318;
    short word_319;
    short word_320;
    short word_321;
    short word_322;
    short word_323;
    short word_324;
    short word_325;
    short word_326;
    short word_327;
    short word_328;
    short word_329;
    short word_330;
    short word_331;
    short word_332;
    short word_333;
    short word_334;
    short word_335;
    short word_336;
    short word_337;
    short word_338;
    short word_339;
    short word_340;
    short word_341;
    short word_342;
    short word_343;
    short word_344;
    short word_345;
    short word_346;
    short word_347;
    short word_348;
    short word_349;
    short word_350;
    short word_351;
    short word_352;
    short word_353;
    short word_354;
    short word_355;
    short word_356;
    short word_357;
    short word_358;
    short word_359;
    short word_360;
    short word_361;
    short word_362;
    short word_363;
    short word_364;
    short word_365;
    short word_366;
    short word_367;
    short word_368;
    short word_369;
    short word_370;
    short word_371;
    short word_372;
    short word_373;
    short word_374;
    short word_375;
    short word_376;
    short word_377;
    short word_378;
    short word_379;
    short word_380;
    short word_381;
    short word_382;
    short word_383;
    short word_384;
    short word_385;
    short word_386;
    short word_387;
    short word_388;
    short word_389;
    short word_390;
    short word_391;
    short word_392;
    short word_393;
    short word_394;
    short word_395;
    short word_396;
    short word_397;
    short word_398;
    short word_399;
    short word_400;
};

/* c----------------------------------------------------------------------- */
/* c----------------------------------------------------------------------- */
/* c----------------------------------------------------------------------- */
/* c----------------------------------------------------------------------- */

#endif

