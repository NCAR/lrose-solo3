# ifndef ETL_H
# define ETL_H
/* ======== Defines ============================ */

#define ANC_HEADER	0x4141
#define COV_HEADER	0x4343
#define MOM_HEADER	0x4d4d
#define RAW_HEADER	0x5252
#define FILE_HEADER	0x4646
#define CLOSEFILE	0x4346
#define END_HEADER 	0xffffffff


#define MC_ANC_HEADER	0x4142
#define MC_COV_HEADER	0x4344
#define MC_MOM_HEADER	0x4d4e
#define MC_RAW_HEADER	0x5253
#define MC_FILE_HEADER	0x4647

/* other ETL defs */
#define SM_GATE_WIDTH 	30
#define MAX_LAGS	7	
#define AD_RATE		50
#define NI_QUEST	AD_RATE/2	
#define INT_RANGE	70.0
#define CHAR_MOM_GATE	6	
#define CHAR_COV_GATE	MAX_LAGS*4
#define CHAR_RAW_GATE	4	


/* 2 micron structs for LIFT 1996 */

struct  data_file_info {
   short file_number;
   short version_number;
   short lidar_number;
   short data_type;
   short number_data_fields;
   short start_year;
   short start_month;
   short start_day;
   short start_hour;
   short start_minute;
   short start_second;
   short start_hund_sec;
   int32_t time_utc;
   int32_t number_of_records;
   short spare ;
   short platform_type;
   float filler1[4];
};

struct  descriptive_info {
   char exp_name[32];
   char metalog[248];
};

struct  gps_info_parm {
   int32_t gps_time_sec;
   int32_t gps_time_nsec;
   float gps_heading;
   float gps_pitch;
   float gps_roll;
   float gps_mrms;
   float gps_brms;
   int32_t  gps_bad_altitude_flag;
   int32_t filler;
   float gps_stddev_delta_heading;
   float gps_stddev_delta_pitch;
   float gps_stddev_delta_roll;
   float gps_utc_time;
   float gps_latitude;
   float gps_longitude ;
   float gps_altitude;
   float gps_course_over_ground;
   float gps_speed_over_ground;
   float gps_vertical_velocity;
   float gps_pdop;
   float gps_hdop;
   float gps_vdop;
   float gps_tdop;
   float spare[2];
};

struct  lidar_info_parm {
   float rep_rate;
   float operating_wavelength;
   float zero_beat_freq;
   int32_t mode;
   short actual_num_bad_pulses;
   short filler[3];
   float pulse_frequency;
   float qs_but;
   float pzt_volt;
   float pulse_energy;
   float freq_mean;
   float freq_variance;
   int32_t spare[4];
};

struct  ocs_info_parm {
   short ocs_status;
   short data_recording;
   short current_file_no;
   short num_gates;
   short gate_width;
   short range_to_first_gate;
   short num_to_integrate;
   short spare1;
   int32_t current_rec;
   int32_t current_beam_rec;
   short spare2[10];
};

struct  plat_env_parm {
   float static_temp;
   float dewpt_temp;
   float sfc_temp;
   float wind_speed;
   float wind_direction;
   float pressure;
   float spare[4];
};

struct  position_info_parm {
   int32_t utc_time;
   float latitude;
   float longitude;
   float spare1;
   short month;
   short day;
   short year;
   short hour;
   short minute;
   short second;
   short millisecond;
   short spare2[3];
   int32_t microsecond;
};

struct  scan_info_parm {
   float azimuth;
   float elevation;
   short scan_type;
   short spare[5];
};

struct  inu_info_parm {
   int32_t inu_time_sec;
   int32_t inu_time_nsec;
   float inu_pitch;
   float inu_roll;
   float inu_yaw;
   float inu_pitch_rate;
   float inu_roll_rate;
   float inu_yaw_rate;
   float inu_x_velocity;
   float inu_y_velocity;
   float inu_z_velocity;
   float inu_x_accel;
   float inu_y_accel;
   float inu_z_accel;
   float spare1[2];
   short inu_ang_tilt;
   short spare2[3];
};

struct  scan_control_parm {
   short scan_type;
   short scanner;
   short compensation_flag;
   short coord_file_input;
   short profiler_sync_mode;
   short spare1;
   char scan_name[24];
   float beg_azimuth;
   float end_azimuth;
   float azimuth_step;
   float beg_elevation;
   float end_elevation;
   float elevation_step;
   float coord_buf[120];
   int32_t num_steps;
   int32_t spare2[5];
};

struct  noise_block_parm {
   float noise_values[14];
};

struct  dsp_control_parm {
   int32_t num_beams_to_send;
   short data_type;
   short AD_rate;
   short number_lags;
   short num_pulses_avg;
   short gate_width;
   short range_to_first_gate;
   short num_gates_pulse;
   short num_to_integrate;
   short freq_corr;
   short start_pt_freq_corr;
   short num_freq_corr_pts;
   short raw_start_pt;
   short raw_num_samples;
   short enable_bad_pul_reject;
   short pulse_rejection_thres;
   short num_bad_pulses;
   short noise_corr_flag;
   short start_noise_gate ;
   short num_noise_gates;
   short spare[5];
};

struct  site_control_parm {
   short lidar_number;
   short altitude;
   char sitename[16];
   short latitude_deg;
   short latitude_min;
   short latitude_sec;
   short longitude_deg;
   short longitude_min;
   short longitude_sec;
   short spare[4];
};

struct  cov_header_struct {
   int32_t record_type;
   struct  position_info_parm   position_info;
   struct  scan_info_parm   scan_info;
   struct  gps_info_parm   gps_info;
   struct  inu_info_parm   inu_info;
   struct  plat_env_parm   plat_env;
   struct  lidar_info_parm   lidar_info;
   struct  noise_block_parm   noise_block;
   struct  ocs_info_parm   ocs_info;
   short spare[8];
};

struct  raw_header_struct {
   int32_t record_type;
   struct  position_info_parm   position_info;
   struct  scan_info_parm   scan_info;
   struct  gps_info_parm   gps_info;
   struct  inu_info_parm   inu_info;
   struct  lidar_info_parm   lidar_info;
   struct  ocs_info_parm   ocs_info;
   short raw_word_size;
   short spare[9];
};

struct  file_header_struct {
   int32_t record_type;
   int32_t data_ident_key;
   struct  data_file_info   data_file;
   struct  scan_control_parm   scan_control;
   struct  dsp_control_parm   dsp_control;
   struct  site_control_parm   site_control;
   struct  descriptive_info   descriptive;
   short data_recording;
   short spare[9];
};

struct moment_header_struct {
   int32_t record_type;
   struct  position_info_parm   position_info;
   struct  scan_info_parm   scan_info;
   struct  gps_info_parm   gps_info;
   struct  inu_info_parm   inu_info;
   struct  plat_env_parm   plat_env;
   struct  lidar_info_parm   lidar_info;
   struct  noise_block_parm   noise_block;
   struct  ocs_info_parm   ocs_info;
   short spare[8];
};

struct anc_header_struct {
   int32_t record_type;
   struct  position_info_parm   position_info;
   struct  data_file_info   data_file;
   struct  ocs_info_parm   ocs_info;
   struct  lidar_info_parm   lidar_info;
   short spare[10];
};

struct  moment_record_struct {
   struct  moment_header_struct   mom_header;
   short intby2_buf[1200];
};

struct  raw_record_struct {
   struct  raw_header_struct   raw_header;
   short intby2_buf[7200];
};



/* MCAWS structs for Conrad Zieglers data */

/*== flight information ========================================================*/

struct flight_info_parm_mcws {
	u_int32_t	end_time;	/* Time in seconds GMT from 1970	*/
	unsigned short	flight_number;	/* Flight number assigned to flight	*/
	unsigned short	run_number;	/* Run number assigned to each run 1-999*/
	unsigned short	start_msec;	/* Time at start in milliseconds	*/
	unsigned short	end_msec;	/* Time at end in milliseconds	*/
};

/*== Gegraphic information =====================================================*/

struct location_info_parm_mcws {
	int 	lat;			/* Latitude in 32 bit angle -180 to 180	*/ 
	int 	lati;			/* Latitude in 32 bit angle -180 to 180	*/ 
	int 	lng;			/* Latitude in 32 bit angle -180 to 180	*/ 
	int 	lngi;			/* Latitude in 32 bit angle -180 to 180	*/ 
	unsigned short p_alitude;	/* Pressure alitude (ft)  at end 	*/
	unsigned short r_alitude;	/* Radar alitude (ft) at end	 	*/
};

/*== Platform atitude and velocity =============================================*/

struct plat_info_parm_mcws {
	short	pitch;			/* Pitch angle, signed, postive nose up */
					/* fixed pt 1/10 degree			*/
	short	pitchi;			/* Pitch angle, signed, postive nose up */
					/* fixed pt 45/8192 degree		*/
	short	roll;			/* Roll angle, signed, + roll right	*/
					/* fixed pt 1/10 degree			*/
	short	rolli;			/* Roll angle, signed, + roll right	*/
					/* fixed pt 45/8192 degree		*/
	unsigned short	true_hdg;	/* True heading 0-359.9 fixed pt 1/10	*/
	unsigned short	true_hdgi;	/* True heading 0-359.95 		*/
					/* fixed pt 45/8192 degree		*/
	short	drift_angle;		/* Drift angle, signed, + wind from left*/ 
					/* fixed pt 1/10 degree			*/
	short	drift_anglei;		/* Drift angle, signed, + wind from left*/ 
					/* fixed pt 45/8192 degree		*/
	unsigned short	gnd_speed;	/* Ground speed 1kt			*/
	unsigned short 	gnd_speedi;	/* Ground speed 0.01 m/s		*/
	unsigned short	track;		/* track angle relative to N 1/10 deg	*/
	unsigned short	true_aspeed;	/* True airspeed measured kt		*/
	unsigned short	true_aspeedc;	/* True airspeed computed kt		*/
	short	u_roll;			/* Rolled that is used fx_pt 1/100	*/
	short	u_pitch;		/* Pitch that is used fx_pt 1/100	*/
	short	u_drift;		/* Drift that is used fx_pt 1/100	*/
	unsigned short	u_trhdg;	/* TRHDG that is used fx_pt 1/100	*/
	short	u_gndspd;		/* Ground speed 0.01 fx_pt m/s		*/
	short 	u_tas;			/* True airspeed .01 fx_pt m/s		*/
	short 	spare;			/* Spare word				*/
};

struct arinc_info_parm_mcws {
	int utc;			/* Time in hr:min:sec int 1.0 sec units	*/
	int gps_lat;			/* Lat in 32 bit angle +-180 degrees	*/
	int gps_lng;			/* Lat in 32 bit angle +-180 degrees	*/
	int gps_alt;			/* Gps altitude in 0.125 ft		*/
	unsigned short gps_trk;		/* Gps track in 0.0055 units		*/
	unsigned short gps_gnspd;	/* Ground speed in knots 0.125 units	*/
};

struct scanner_info_parm_mcws {
	double PITCH;
	double ROLL;
	double DRIFT;
	double THDG;
	double TAS;
	double GNDSPD;
	double LOSD;
	double LOSE;
	double ASIM;
	double ELEV;
	double DEVN;
	double THETA;
	double T1A;
	double T2A;
	double T1B;
	double T2B;
	double DTHETA;
	double THETA1;
	double THETA2;
	double LOMHZ;
	double VSIGN;
	double VCORN;
	double ASCOMP;
	double GSCOMP;
	double OFFSET;
	float  FREQ;
	int	LO20KHZ;
	int	TTPI;
	int	TTPO;
	int	WTIMEO;
	int	WTIMEI;
	int 	bad_pulse;
	int 	lo_read;
};

/*== Platform environment ======================================================*/

struct plat_env_parm_mcws {
	short	static_temp;		/* Static air temp, signed 1/10 C	*/
	short	static_tempc;		/* Static air temp, signed calc 1/10 C	*/
	short 	total_temp;		/* Total air temperature, sigend 1/10 C */
	short	dewpt_ge;		/* Dew/frost point, signed GE 1/10 C	*/
	short	dewpt_egg;		/* Dew/frost point, signed EGG 1/10 C 	*/
	short	sfc_temp;		/* IR surface temperature, signed 1/10 C*/
	short	wind_speed;		/* INS wind speed magnitude	kt	*/
	short	wind_dirn;		/* INS wind speed direction 0 to 3599   */
					/* fixed pt 1/10 degree			*/
};
	
/*== Beam location relative to run =============================================*/

struct beam_loc_parm_mcws {
	unsigned short 	beam_index;	/* Beam position in scan cycle		*/
	short		spare1;		/* East distance from start 20m 	*/
	short		spare2;		/* North distance from start 20m 	*/
	unsigned short	beam_losa;	/* az angle rel north	.010000 deg	*/
	short		beam_lose;	/* el angle rel horz+up	.010000 deg	*/
	short		scan_type;
};

/*== House keeeping info OCS system ============================================*/

struct ocs_info_parm_mcws {
	unsigned short	ocs_status;	/* OCS status Coded			*/
	unsigned short	sources;	/* Souces for g_spd,pitch,roll,etc...	*/
	unsigned short	theta1;		/* Inner wedge position BCD 1/10 deg	*/
	unsigned short	theta2;		/* Outer wedge position BCD 1/10 deg	*/
	unsigned short	wedge_ttp;	/* Wedge time to position in ms		*/
	unsigned short	rfif_lo;	/* RF/IF unit LO freq 1/100 MHZ		*/
	unsigned short  rfif_lo_out;	/* RF/IF unit LO freq 1/100 MHZ as read	*/
	unsigned short	rfif_gain;	/* RF/IF unit gain in units 0 - 1023	*/
	unsigned short 	num_bad_pulses;	/* Number of bad pulses allowed		*/
	unsigned short	num_pulses;	/* Number of pulses to integrate	*/
	unsigned short	num_lags;	/* Number of lags			*/
	unsigned short	num_gates;	/* Number of range gates		*/
	unsigned short	gate_width;	/* Range gate width in meters		*/
	unsigned short	num_posns;	/* Number of beam positions per scan	*/
	unsigned short	bad_data;	/* Indicates a bad data sets		*/
	unsigned short  bad_pulses_set;	/* Number of bad pulses for data set	*/
	unsigned short	off_gscomp;	/* OFFSET + GSCOMP 1/100		*/
	unsigned short	spare[3];	/* Spare				*/
};

/*== scanner inu data ==========================================================*/

struct scan_inu_parm_mcws {
	int   time[10];			/* Inu time 1/100 of second 		*/
	int   lati[10];			/* Latitude in 32bit angle -180 to 180	*/
	int   lngi[10];			/* Latitude in 32bit angle -180 to 180	*/
	short   pitchi[20];             /* Pitch angle, signed, postive nose up */
					/* fixed pt 45/8192 degree              */
	short   rolli[20];              /* Roll angle, signed, + roll right     */
					/* fixed pt 45/8192 degree              */
	unsigned short   true_hdgi[20]; /* True heading 0-359.95                */
					/* fixed pt 45/8192 degree              */
	short   drift_anglei[10];       /* Drift angle, signed, + wind from left*/
					/* fixed pt 45/8192 degree              */
	unsigned short 	gnd_speedi[10];	/* Ground speed 0.01 m/s		*/
};

/*== Dads data =================================================================*/

struct dads_inu_parm_mcws {
	int   time;			/* Inu time second			*/
	int   lat;			/* Latitude in 32bit angle -180 to 180	*/
	int   lng;			/* Latitude in 32bit angle -180 to 180	*/
	unsigned short p_alitude;       /* Pressure alitude (ft)  at end        */
	unsigned short r_alitude;       /* Radar alitude (ft) at end            */
	short   pitch;                  /* Pitch angle, signed, postive nose up */
					/* fixed pt 1/10 degree                 */
	short   roll;                   /* Roll angle, signed, + roll right     */
					/* fixed pt 1/10 degree                 */
	short   true_hdg;               /* True heading 0-359.9 fixed pt 1/10   */
	short   drift_angle;            /* Drift angle, signed, + wind from left*/
					/* fixed pt 1/10 degree                 */
	unsigned short  gnd_speed;      /* Ground speed 1kt                     */
	unsigned short  track;          /* track angle relative to N 1/10 deg   */
	unsigned short  true_aspeed;    /* True airspeed measured kt            */
	unsigned short  true_aspeedc;   /* True airspeed computed kt            */
};

/*== Lidar housekeeping info ===================================================*/

struct lidar_info_parm_mcws {
	int io_pzt;		/* injection oscillator pzt drive voltage */
	int lo_pzt;		/* local oscillator pzt drive voltage */
	int po_pzt;		/* power oscillator pzt drive voltage */
	int io_error;
	int lo_error;
	int po_error;
	int hv_on;		/* High voltage status: 0 = hv off, 1 = on */
	int loops;		/* loop status: 0 = loops off, 1 = on 2 locking */
	int setup;		/* setup file number 				*/
	int amb_temp;		/* ambient temperature				*/
	int gas_temp;		/* gas temperature				*/
	int fan_volt;		/* fan voltage					*/
	float io_det;		/* injection oscillator detctor			*/
	float lo_det;		/* locat oscillator detector			*/
	float po_det;		/* power oscillator detector			*/
	float rep_rate;		/* Laser prf	(hz)				*/
	float laser_power;	/* Laser power (watts)				*/
};

/*== Defination of coded OCS status ============================================*/

#define	SCANNER_AFT	0x8000		/* Scanner in aft position		*/
#define OUT_WEDGE_TIM	0x4000		/* Outer wedge time out			*/
#define IN_WEDGE_TIM	0x2000		/* Inner wedge time out			*/
#define LIDAR_FAULT	0x1000		/* Lidar fault				*/
#define INU_FAULT	0x0800		/* Scanner INU fault			*/
#define SIG_FAULT	0x0400		/* Signal process fault			*/
#define DATA_SYS_FAULT	0x0200		/* Data system fault			*/
#define TAPE1_FAULT	0x0100		/* Tape number one fault		*/
#define TAPE2_FAULT	0x0080		/* Tape number two faultt		*/
#define DADS_FAULT	0x0040		/* DADS data stream fault		*/
#define SAM_70_FAULT	0x0020		/* Fault with SAM70			*/
#define OCS_SCANNING	0x0010		/* OCS is currently taking data		*/
#define DUM_a		0x0008		/* Dummy				*/
#define DUM_b		0x0004		/* Dummy				*/
#define DUM_c		0x0002		/* Dummy				*/
#define DUM_d		0x0001		/* Dummy				*/

/*== moment header structure ===================================================*/

struct moment_header_struct_mcws {
	unsigned int record_type;
	struct flight_info_parm_mcws flight_info;
	struct location_info_parm_mcws location_info;
	struct plat_info_parm_mcws plat_info;
	struct plat_env_parm_mcws plat_env;
	struct beam_loc_parm_mcws beam_loc;
	struct ocs_info_parm_mcws ocs_info;
	struct lidar_info_parm_mcws lidar_info;
	struct arinc_info_parm_mcws arinc_info;
	short spare[8];
	/* Here is where the moment data goes */
};

/* This size of the momement data is this plus 6 * the number of gates		*/

/*== raw header structure ======================================================*/

struct raw_header_struct_mcws {
	unsigned int record_type;
	struct flight_info_parm_mcws flight_info;
	struct location_info_parm_mcws location_info;
	struct plat_info_parm_mcws plat_info;
	struct plat_env_parm_mcws plat_env;
	struct beam_loc_parm_mcws beam_loc;
	struct ocs_info_parm_mcws ocs_info;
	struct lidar_info_parm_mcws lidar_info;
	struct arinc_info_parm_mcws arinc_info;
	short spare[8];
	/* Here is where the moment data goes */
};

/*== covariance header structure ===============================================*/

/* Currently this is identicle to the momement data record.  I don't know if it
   will stay that way.								*/

struct cov_header_struct_mcws {
	unsigned int record_type;
	struct flight_info_parm_mcws flight_info;
	struct location_info_parm_mcws location_info;
	struct plat_info_parm_mcws plat_info;
	struct plat_env_parm_mcws plat_env;
	struct beam_loc_parm_mcws beam_loc;
	struct ocs_info_parm_mcws ocs_info;
	struct lidar_info_parm_mcws lidar_info;
	struct arinc_info_parm_mcws arinc_info;
	short spare[8];
	/* Here is where the covariance data goes */
};

/*== Ancillary data record =====================================================*/

struct anc_header_struct_mcws {
	unsigned int record_type;
	struct flight_info_parm_mcws flight_info;
	struct dads_inu_parm_mcws dads_inu;
	struct scan_inu_parm_mcws scan_inu;
	struct plat_env_parm_mcws plat_env;
	struct lidar_info_parm_mcws lidar_info;
	struct ocs_info_parm_mcws ocs_info;
	short az_angle;				/* Az angle to			*/
						/* plane.  Fixed pt 1/100	*/
	short el_angle;				/* Elevation angle Fx pt 1/100 	*/
		/* Be very carefull because of double alignments		*/
	struct scanner_info_parm_mcws scanner_info[10];
	struct arinc_info_parm_mcws arinc_info[10];
	int num_scan_info;
	int i_spare;
	short scan_type;
	short spare[39];
};

#endif
