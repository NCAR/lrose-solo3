# ifndef RSF_HOUSE_H
# define RSF_HOUSE_H

/*
 * Modified structures for FOF housekeeping and beams.
 *
 * Mods made in an attempt to improve generality, without loss of 
 * description.   RAR 6/27/91
 *
 * Major changes include the "unstructuring" of the beam to treat the data
 * portion separately from the housekeeping (this will necessitate the 
 * passing of two pointers when doing a "get_beam"); also, essential 
 * housekeeping is expanded to encompass CP2 256-word housekeeping.
 *
 * NOTE: 680X0/SPARC byte ordering is assumed throughout these
 * structure definitions.
 *
 */

#define CORR_FACT 182.044444
#define MAX_GATES 2048
#define MAX_FIELDS 8

/* 
 * house keeping header */
 
typedef struct {
    	unsigned short pscale;
	short pbias;
}SCALING;
typedef struct {
/* 1*/	unsigned short rec_num;		/* log rec# for this beam, MOD 32768 */
/* 2*/	unsigned short field_tape_seq;	/* field tape sequence # =0 */
/* 3*/	unsigned short rec_type;	/* record type; 0 = data */
/* 4*/	unsigned short year;		/* last two digits */
/* 5*/	unsigned short month;
/* 6*/	unsigned short day;
/* 7*/	unsigned short hour;
/* 8*/	unsigned short minute;
/* 9*/	unsigned short second;
/*10*/	unsigned short az_xCF;		/* degrees * CF */
/*11*/	unsigned short el_xCF;		/* degrees * CF */
/*12*/	unsigned short rhozero1;	/* range to leading edge of 1st gate*/
/*13*/	unsigned short rhozero2;	/* = rhozero1 + rhozero2/1000 (in km)*/
/*14*/	unsigned short gs;		/* gate spacing (m) = */
/*15*/	unsigned short num_gates;	/* gates per beam */
/*16*/	unsigned short samples_per_beam;
/*17*/	unsigned short test_pulse_level; /* 0 */
/*18*/	unsigned short atp;		/* average transmitted power   */
/*19*/	unsigned short pulse_width;	/* 1.67 us */
/*20*/	unsigned short prfx10;		/* PRF (Hz * 10), typ. 1235 Hz */
/*21*/	unsigned short wavelength;	/* wavelength (cm * 100) */
/*22*/	unsigned short swp_num;		/* running counter of elevation scans
					 * since last start of operations */
/*23*/	unsigned short sweep_index;	/* identifies the sweep in the volume*/
/*24*/	unsigned short unused1[2];
/*26*/	unsigned short scan_mode;	/* 8 = Surveillance */
/*27*/	unsigned short cw_az_lim;	/* azimuth angle of first dwell */
/*28*/	unsigned short ccw_az_lim;	/* azimuth angle of last dwell */
/*29*/	unsigned short up_elev_lim;	/* 0 */
/*30*/	unsigned short lo_elev_lim;	/* 0 */
/*31*/	unsigned short fixed_ang;	/* the elevation angle from the
					 * scan strategy table */
/*32*/	unsigned short sig_source;	/*  0 = radar */
/*33*/	unsigned short coupler_loss;
/*34*/	unsigned short tp_strt;
/*35*/	unsigned short tp_width;
/*36*/	unsigned short pri_co_bl;
/*37*/	unsigned short scnd_co_bl;
/*38*/	unsigned short tp_atten;
/*39*/	unsigned short sys_gain;
/*40*/	unsigned short fix_tape;
/*41*/	unsigned short tp_freq_off;
/*42*/	unsigned short log_bw;
/*43*/	unsigned short lin_bw;
/*44*/	unsigned short ant_bw;
/*45*/	unsigned short ant_scan_rate;
/*46*/	unsigned short unused2[2];
/*48*/	unsigned short vol_num;		/* running count of full or partial
					 * volume scans since last start of 
					 * operations */
/*49*/	unsigned short clut_filt;
/*50*/	unsigned short polarization;	/*0 = horizontal */
/*51*/	unsigned short prf1;
/*52*/	unsigned short prf2;
/*53*/	unsigned short prf3;
/*54*/	unsigned short prf4;
/*55*/	unsigned short prf5;
/*56*/	unsigned short unused3;
/*57*/	unsigned short rec_num_d32768;	/* record count overflow */
/*58*/	unsigned short altitude;
/*59*/	unsigned short latitude;
/*60*/	unsigned short longitude;
/*61*/	unsigned short transit;		/* 0 = in a scan */
/*62*/	unsigned short ds_id;		/* -1 */
/*63*/	unsigned short rs_id;		/* 0x4d48 - 'MH' */
/*64*/	unsigned short proj_num;	
/*65*/	unsigned short hsk_len;		/* # of words of housekeeping = 100 */
/*66*/	unsigned short log_rec_len;	/* 100 + 4*512 */
/*67a*/	unsigned char  num_lrecs;	/* number of log recs in phys rec */
/*67b*/ unsigned char  lrec_num;	/* number of current logical rec  */
/*68*/	unsigned short num_fields;
/*69-74*/	unsigned short parm_desc1[6];
/*75*/	unsigned short tp_max;
/*76*/	unsigned short tp_min;
/*77*/	unsigned short tp_step;
/*78*/	unsigned short vol_scan_prg;
/*79-90*/
	SCALING cal_info1[6];
/*91*/	unsigned short rnoise_bdcal;	/* words 91-100 are MHR words */
/*92*/	unsigned short rsolar;
/*93*/	unsigned short rgcc;
/*94*/	unsigned short rvtime;
/*95*/	unsigned short rcrec;
/*96*/	unsigned short unused4[4];
/*100*/	unsigned short live_or_sim;	/* LIVE or SIM */
/*101-160*/ unsigned short unused5[60];
/*161-170*/ unsigned short parm_desc2[10];
/*171*/ unsigned short src_test_bus;
/*172*/ unsigned short add_test_bus;
/*173*/ unsigned short half_prf;
/*174*/ unsigned short ptape_unit;
/*175*/ unsigned short stape_unit;
/*176*/ unsigned short word_176;
/*177*/ unsigned short word_177;
/*178*/ unsigned short word_178;
/*179*/ unsigned short cal_attn_step;
/*180*/ unsigned short cal_freq_step;
/*181*/ unsigned short r_sq_offset;
/*182*/ unsigned short refl_thres;
/*183*/ unsigned short shifter_cnts;
/*184*/ unsigned short attn_setting;
/*185*/ unsigned short swp_center;
/*186*/ unsigned short cp2_mode;
/*187*/ unsigned short non_dual_mode;
/*188*/ unsigned short word_188;
/*189-200*/ unsigned short unused6[12];
/*201*/ unsigned short wavelength2;	/* wavelength, secondary system */
/*202*/ unsigned short atp2;		/* average tx pwr, secondary wavlen */
/*203*/ unsigned short pulse_width2;
/*204*/ unsigned short prf_secondary;
/*205*/ unsigned short sys_gain2;
/*206*/ unsigned short log_bw2;
/*207*/ unsigned short ant_bw2;
/*208*/ unsigned short polarization2;
/*209-211*/ unsigned short unused7[3];
/*212-231*/
 	SCALING cal_info2[10];
/*232-240*/ unsigned short unused8[9];
/*241-246*/ unsigned short aircraft_info[6];
/*247*/ unsigned short dual_pol_mode;
/*248*/ unsigned short dual_pol_switch;
/*249-256*/ unsigned short unused9[8];

} RSF_HOUSE;

#endif
