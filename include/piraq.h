/*
 * $Date: 2011-11-07 08:09:11 -0700 (Mon, 07 Nov 2011) $
 * $Id: piraq.h 482 2011-11-07 15:09:11Z rehak $
 * 
 */
# ifndef PIRAQ_H
# define PIRAQ_H
#include "piraq/dd_types.h"
#include "piraq/dd_defines.h"
#include "piraq/piraqx.h" /* separate definitions to ease sharing with Rapid DOW DRX */

struct radar_consts {
  float h_rconst;
  float v_rconst;
};



#define MAXNUM 10000

/* structure to completely define the operation of the PIRAQ */
/* and data system at the lowest level */
typedef struct
	{
	int     iobase,membase;
	int     gates,hits,pulsewidth,timeseries,gate0mode;
	int     watchdog,prt,timingmode,delay;
	int     tsgate,pcorrect,clutterfilter;
	char    dspfilename[80],outfilename[80],outpath[80];
	float   afcgain,afchi,afclo,locktime;
	} CONFIG;

/* structure to define all parameters that PIRAQ uses in DPRAM */
typedef struct
	{
	int     *membase;
	int     *gates,*hits,*pulsewidth,*timeseries,*gate0mode;
	int     *tsgate,*pcorrect,*clutterfilter;
	volatile int *flag,*numdiscrim,*dendiscrim,*g0invmag;
	volatile int *s1,*s2,*s3,*s4;   /* spare numbers for misc use */
	int     *tsptr[2],*bufptr[2];
	} PIRAQ;

/* structure that defines communication parameters to the display */
typedef struct
	{
	int     displayparm,fakeangles,recording;
	float   threshold;
	float   dbzhi,dbzlo,powlo,powhi;
	char    title[40];
	} DISPLAY;

/* header for each dwell describing parameters which might change dwell by dwell */
/* this structure will appear on tape before each abp set */
typedef struct  {
		char            desc[4];
		short           recordlen;
		short           gates,hits;
		float           rcvr_pulsewidth,prt,delay; /* delay to first gate */
		char            clutterfilter,timeseries;
		short           tsgate;
		unsigned int    time;      /* seconds since 1970 */
		short           subsec;    /* fractional seconds (.1 mS) */
		float           az,el;
		float           radar_longitude; 
		float           radar_latitude;
		float           radar_altitude;
		float           ew_velocity;
		float           ns_velocity;
		float           vert_velocity;
		char            dataformat; /* 0 = abp, 1 = abpab (poly),
					     * 2 = abpab (dual prt)
					     * 3 = abpabp (dual pol)
					     * 4 = ? (s-pol) */
		float           prt2;
		float           fxd_angle;
		unsigned char   scan_type; 
		unsigned char   scan_num; /* bumped by one for each new scan */
		unsigned char   vol_num; /* bumped by one for each new vol */
		char            reserved[8];
		} HEADER;

/* this structure gets recorded for each dwell */
typedef struct  {                
		HEADER          header;
		float           abp[MAXNUM * 4]; /* a,b,p + time series */
		} DWELL;

/* this structure gets recorded once per volume (when a parameter changes) */
typedef struct  {
		char    desc[4];
		short   recordlen;
		short   rev;
		short   year;           /* this is also in the dwell as sec from 1970 */
		char    radar_name[8];
		char    polarization;   /* H or V */
		float   test_pulse_pwr; /* power of test pulse (refered to antenna flange) */
		float   test_pulse_frq; /* test pulse frequency */
		float   frequency;      /* transmit frequency */
		float   peak_power;     /* typical xmit power (at antenna flange) */
		float   noise_figure;
		float   noise_power;    /* for subtracting from data */
		float   receiver_gain;  /* gain from antenna flange to PIRAQ input */
		float   data_sys_sat;   /* PIRAQ input power required for full scale */
		float   antenna_gain;
		float   horz_beam_width;
		float   vert_beam_width;
		float   xmit_pulsewidth; /* transmitted pulse width */
		float   rconst;         /* radar constant */
		float   phaseoffset;    /* offset for phi dp */
		float   misc[9];        /* 6 more misc floats */
		char    text[960];
		} RADAR;

/* this is what the top of either the radar or dwell struct looks like */
/* it is used for recording on disk and tape */
typedef struct
	{
	char    desc[4];
	unsigned short   recordlen;
	} TOP;

#define MEMREGISTER     (config->iobase+4)
#define STATUSREGISTER  (config->iobase+4)
#define CONTROLREGISTER (config->iobase+5)

#define K2      0.93
#ifndef SPEED_OF_LIGHT
#define SPEED_OF_LIGHT       2.99792458E8
#endif
#define SMALL   -4.0
#define SMALLplus .0183156389	/* e**(-4) */

typedef struct  {float x,y;}    complex;

typedef twob    LeShort;
typedef fourB   LeLong;
typedef fourB   LeFloat;


/* header for each dwell describing parameters which might change dwell by dwell */

/* this structure will appear on tape before each abp set */
typedef struct  {
		char            desc[4];
		LeShort           recordlen;
		LeShort           gates,hits;
		LeFloat           rcvr_pulsewidth,prt,delay; /* delay to first gate */
		char            clutterfilter,timeseries;
		LeShort           tsgate;
		LeLong          time;      /* seconds since 1970 */
		LeShort           subsec;    /* fractional seconds (.1 mS) */
		LeFloat           az,el;
		LeFloat           radar_longitude; 
		LeFloat           radar_latitude;
		LeFloat           radar_altitude;
		LeFloat           ew_velocity;
		LeFloat           ns_velocity;
		LeFloat           vert_velocity;
		char              dataformat; /* 0 = abp, 1 = abpab (poly),
					       * 2 = abpab (dual prt) */
		LeFloat           prt2;
		LeFloat           fxd_angle;
		unsigned char     scan_type; 
		unsigned char     scan_num; /* bumped by one for each new scan */
		unsigned char     vol_num; /* bumped by one for each new vol */
		char              reserved[8];
		} LeHEADER;

/* this structure gets recorded for each dwell */
typedef struct  {                
		LeHEADER          header;
		LeFloat           abp[MAXNUM * 4]; /* a,b,p + time series */
		} LeDWELL;

/* this structure gets recorded once per volume (when a parameter changes) */
typedef struct  {
		char    desc[4];
		LeShort   recordlen;
		LeShort   rev;
		LeShort   year;           /* this is also in the dwell as sec from 1970 */
		char    radar_name[8];
		char    polarization;   /* H or V */
		LeFloat   test_pulse_pwr; /* power of test pulse (refered to antenna flange) */
		LeFloat   test_pulse_frq; /* test pulse frequency */
		LeFloat   frequency;      /* transmit frequency */
		LeFloat   peak_power;     /* typical xmit power (at antenna flange) */
		LeFloat   noise_figure;
		LeFloat   noise_power;    /* for subtracting from data */
		LeFloat   receiver_gain;  /* gain from antenna flange to PIRAQ input */
		LeFloat   data_sys_sat;   /* PIRAQ input power required for full scale */
		LeFloat   antenna_gain;
		LeFloat   horz_beam_width;
		LeFloat   vert_beam_width;
		LeFloat   xmit_pulsewidth; /* transmitted pulse width */
		LeFloat   rconst;         /* radar constant */
		LeFloat   phaseoffset;    /* offset for phi dp */
		LeFloat   misc[9];        /* 9 more misc floats */
		char    text[960];
		} LeRADAR;



/* SPOL structs */


/* structure to completely define the operation of the VIRAQ */
/* there will be one of these structures for each VIRAQ in the system */
typedef struct
	{
	int     membase;  /* actual VIRAQ VME base address */
	int     intenable,intvector,intlevel,intautoclear;
	int     numfred,vfifo,tpflag,trigflag,tpdelay,tpwidth;
	int     prt,prt2,timingmode,delay,velsign,viraqdsp;
	char    dspfilename[80],outfilename[80],outpath[80];
	} VIRAQCFG;

/* structure to completely define the operation of the QUAD */
/* there will be one of these structures for each QUAD in the system */
typedef struct
	{
	int     membase;  /* actual VIRAQ VME base address */
	int     intenable,intvector,intlevel,intautoclear;
	int     boardnumber,velsign,fifointsel;
	char    dspfilename[80],outfilename[80],outpath[80];
	} QUADCFG;

/* structure to specify the data to be put in the DPRAM */
typedef struct
	{
	int     gates,hits,pulsewidth,gate0mode;
	int     pcorrect,clutterfilter,timeseries,tsgate;
	int     afcflag,xmitpulse; /* xmitpulse used for discrim only */
	float   afcgain,dacval;
	float   dacmin,dacmax,locktime;
	} DPRAMCFG;

/* structure to define all parameters in any DPRAM */
typedef struct
	{
	int     *membase;  /* effective VME address for VIRAQ base */
	int     *gates,*hits,*pulsewidth,*timeseries,*gate0mode;
	int     *tsgate,*pcorrect,*clutterfilter;
	int     *afcflag,*afcgain,*locktime,whoami;
	int     *eofstatus,*xmitpulse,*dacmax,*dacmin;
	volatile int *flag,*dacval,*gate0power,*s1,*s2,*s3,*s4,*status,*raycount;   
	int     *dataformat,*tsptr[2],*bufptr[2];
	} DPRAM;
	
/* structure containing all information about a VIRAQ */
typedef struct
	{
	short           controlimg;
	short           *counter1,*counter2,*counter3;     
	char            *intcontrol,*intvector;
	short           *difcontrol,*intclear;
	VIRAQCFG        config;         /* desired viraq operation */
	DPRAMCFG        dpconfig;       /* desired DSP operation */
	DPRAM           dpram[2];       /* dpram mem pointers */
	} VIRAQ;

/* structure to define all registers in a QUAD */
typedef struct
	{
	char             *dspcontrol,*tclk0ctrl,*tclk1ctrl;     
	char             *intcontrol,*intvector;
	QUADCFG         config;         /* desired quad operation */
	DPRAMCFG        dpconfig;       /* desired DSP operation */
	DPRAM           dpram[4];
	} QUAD;

/* structure to completely define the operation of FRED */
typedef struct
	{
	VIRAQ   *viraq[20];     /* up to 20 viraqs */
	QUAD    *quad[20];      /* up to 20 quad boards */
	int     viraqnum,quadnum,gates,hits,prt,prt2,timingmode;
	int     delay,pulsewidth,xmitpulse,clutterfilter;
	int     timeseries,tsgate,velsign;
	char    outpath[80],vfile[20][80],qfile[20][80];
	} FRED;

/* structure that defines communication parameters to the display */
typedef struct
	{
	int     displayparm,fakeangles,recording,type;
	double  threshold,pprg;
	double  dbzhi,dbzlo,powlo,powhi,zdrlo,zdrhi;
	int     mousex,mousey,mousebutton,mousegate;
	double  mouserange,mouseangle;
	char    title[40];
	} DISPLAYX;

/* header for each dwell describing parameters which might change dwell by dwell */
/* this structure will appear on tape before each abp set */
typedef struct  {
		char            desc[4];
		unsigned short  recordlen;
		short           gates,hits;
		float           rcvr_pulsewidth,prt,delay; /* delay to first gate */
		char            clutterfilter,timeseries;
		short           tsgate;
		unsigned int    time;      /* seconds since 1970 */
		short           subsec;    /* fractional seconds (.1 mS) */
		float           az,el;
		float           radar_longitude; 
		float           radar_lattitude;
		float           radar_altitude;
		float           ew_velocity;
		float           ns_velocity;
		float           vert_velocity;
		char            dataformat;     /* 0 = abp, 1 = abpab (poly), 2 = abpab (dual prt) */
		float           prt2;
		float           fxd_angle;
		unsigned char   scan_type;
		unsigned char   scan_num;
		unsigned char   vol_num;
		unsigned int    ray_count;
		char            transition;
		float           hxmit_power;    /* on the fly hor power */
		float           vxmit_power;    /* on the fly ver power */
		float           yaw;            /* platform heading in degrees */
		float           pitch;          /* platform pitch in degrees */
		float           roll;           /* platform roll in degrees */
		float           gate0mag;       /* magnetron sample amplitude in dB rel to sat */
		float           dacv;           /* transmit frequency */

		char            spare[80];
		} HEADERV;

/* this structure gets recorded for each dwell */
typedef struct  {                
		HEADERV          header;
		short           abp[MAXNUM * 10]; /* a,b,p + time series */
		} DWELLV;

/* this structure gets recorded once per volume (when a parameter changes) */
typedef struct  {
		char    desc[4];
		unsigned short   recordlen;
		short   rev;
		short   year;           /* this is also in the dwell as sec from 1970 */
		char    radar_name[8];
		char    polarization;   /* H or V */
		float   test_pulse_pwr; /* power of test pulse (refered to antenna flange) */
		float   test_pulse_frq; /* test pulse frequency */
		float   frequency;      /* transmit frequency */
		float   peak_power;     /* typical xmit power (at antenna flange) read from config.rdr file */
		float   noise_figure;
		float   noise_power;    /* for subtracting from data */
		float   receiver_gain;  /* hor chan gain from antenna flange to VIRAQ input */
		float   data_sys_sat;   /* VIRAQ input power required for full scale */
		float   antenna_gain;
		float   horz_beam_width;
		float   vert_beam_width;
		float   xmit_pulsewidth; /* transmitted pulse width */
		float   rconst;         /* radar constant */
		float   phaseoffset;    /* offset for phi dp */
		float   vreceiver_gain; /* ver chan gain from antenna flange to VIRAQ */
		float   vtest_pulse_pwr; /* ver test pulse power refered to antenna flange */
		float   vantenna_gain;  
		float   vnoise_power;   /* for subtracting from data */
		float   zdr_fudge_factor; /* what else? */
                float   mismatch_loss;
		float   misc[3];        /* 3 more misc floats */
		char    text[960];
		} RADARV;

/* this is what the top of either the radar or dwell struct looks like */
/* it is used for recording on disk and tape */


#define K2      0.93
#define TWOPI   6.283185307



typedef struct  {
		char            desc[4];
		LeShort           recordlen;
		LeShort           gates,hits;
		LeFloat           rcvr_pulsewidth,prt,delay; /* delay to first gate */
		char            clutterfilter,timeseries;
		LeShort           tsgate;
		LeLong          time;      /* seconds since 1970 */
		LeShort           subsec;    /* fractional seconds (.1 mS) */
		LeFloat           az,el;
		LeFloat           radar_longitude; 
		LeFloat           radar_lattitude;
		LeFloat           radar_altitude;
		LeFloat           ew_velocity;
		LeFloat           ns_velocity;
		LeFloat           vert_velocity;
		char              dataformat; /* 0 = abp, 1 = abpab (poly),
					       * 2 = abpab (dual prt) */
		LeFloat           prt2;
		LeFloat           fxd_angle;
		unsigned char     scan_type; 
		unsigned char     scan_num; /* bumped by one for each new scan */
		unsigned char     vol_num; /* bumped by one for each new vol */
		LeLong            ray_count;
		char              transition;
		LeFloat           hxmit_power;    /* on the fly hor power */
		LeFloat           vxmit_power;    /* on the fly ver power */
		char              spare[100];
		} LeHEADERV;

/* this structure gets recorded for each dwell */
typedef struct  {                
		LeHEADERV          header;
		LeFloat           abp[MAXNUM * 4]; /* a,b,p + time series */
		} LeDWELLV;

/* this structure gets recorded once per volume (when a parameter changes) */
typedef struct  {
		char    desc[4];
		LeShort   recordlen;
		LeShort   rev;
		LeShort   year;           /* this is also in the dwell as sec from 1970 */
		char    radar_name[8];
		char    polarization;   /* H or V */
		LeFloat   test_pulse_pwr; /* power of test pulse (refered to antenna flange) */
		LeFloat   test_pulse_frq; /* test pulse frequency */
		LeFloat   frequency;      /* transmit frequency */
		LeFloat   peak_power;     /* typical xmit power (at antenna flange) */
		LeFloat   noise_figure;
		LeFloat   noise_power;    /* for subtracting from data */
		LeFloat   receiver_gain;  /* gain from antenna flange to PIRAQ input */
		LeFloat   data_sys_sat;   /* PIRAQ input power required for full scale */
		LeFloat   antenna_gain;
		LeFloat   horz_beam_width;
		LeFloat   vert_beam_width;
		LeFloat   xmit_pulsewidth; /* transmitted pulse width */
		LeFloat   rconst;         /* radar constant */
		LeFloat   phaseoffset;    /* offset for phi dp */
		LeFloat   vreceiver_gain; /* ver chan gain from antenna flange to VIRAQ */
		LeFloat   vtest_pulse_pwr; /* ver test pulse power refered to antenna flange */
		LeFloat   vantenna_gain;  
		LeFloat   misc[6];        /* 7 more misc floats */
		char    text[960];
		} LeRADARV;



typedef struct {
   char            desc[4];
   short           recordlen;
   short           gates,hits;
   short           spad1;
   float           rcvr_pulsewidth,prt,delay; /* delay to first gate */
   char            clutterfilter,timeseries;
   short           tsgate;
   unsigned int    time;	/* seconds since 1970 */
   short           subsec; /* fractional seconds (.1) */
   short           spad2;
   float           az,el;
   float           radar_longitude; 
   float           radar_latitude;
   float           radar_altitude;
   float           ew_velocity;
   float           ns_velocity;
   float           vert_velocity;
   char            dataformat;	/* 0 = abp, 1 = abpab (poly), 2 
				   = abpab (dual prt) */
   char            pad1[3];
   float           prt2;
   float           fxd_angle;
   unsigned char   scan_type;
   unsigned char   scan_num;
   unsigned char   vol_num;
   char            pad2;
   unsigned int    ray_count;
   char            transition;
   char            pad3[3];
   float           hxmit_power;	/* on the fly hor power */
   float           vxmit_power;	/* on the fly ver power */
   float           yaw;		/* platform heading in degrees */
   float           pitch;	/* platform pitch in degrees */
   float           roll;	/* platform roll in degrees */
   char            spare[88];
} HEADERNU;

typedef struct  {
   char    desc[4];
   short   recordlen;
   short   rev;
   short   year;		/* this is also in the dwell as sec from
				   1970 */
   short   spad;
   char    radar_name[8];
   char    polarization;	/* H or V */
   char    pad1[3];
   float   test_pulse_pwr;	/* power of test pulse (refered to anten
				   na flange) */
   float   test_pulse_frq;	/* test pulse frequency */
   float   frequency;		/* transmit frequency */
   float   peak_power;		/* typical xmit power (at antenna flange
				   ) read from config.rdr file */
   float   noise_figure;
   float   noise_power;		/* for subtracting from data */
   float   receiver_gain;	/* hor chan gain from antenna flange to 
				   VIRAQ input */
   float   data_sys_sat;	/* VIRAQ input power required for full scale */
   float   antenna_gain;
   float   horz_beam_width;
   float   vert_beam_width;
   float   xmit_pulsewidth;	/* transmitted pulse width */
   float   rconst;		/* radar constant */
   float   phaseoffset;		/* offset for phi dp */
   float   vreceiver_gain;	/* ver chan gain from antenna flange to 
				   VIRAQ */
   float   vtest_pulse_pwr;	/* ver test pulse power refered to antenna flange */
   float   vantenna_gain;
    float   vnoise_power;   /* for subtracting from data */
    float   zdr_fudge_factor; /* what else? */
    float   misc[4];        /* 4 more misc floats */
   char    text[960];
} RADARNU;

/* c------------------------------------------------------------------------ */



/* c------------------------------------------------------------------------ */

/* LOG:
    $Log$
    Revision 1.1  2003/05/08 16:25:11  oye
    First commit.

    Revision 1.2  2003/04/03 23:05:11  vanandel
    changed speed of light from 'C", which too short and ambigiuos

    Revision 1.1  2003/03/14 14:37:58  vanandel

    : Added Files:
    : 	dd_defines.h dd_types.h piraq.h piraqx.h
    : ----------------------------------------------------------------------

    Revision 1.21  2003/03/12 15:07:59  vanandel
    put Rapid DOW definitions in piraqx.h for ease of
    sharing.

    Revision 1.20  2003/03/04 00:23:23  vanandel
    minor changes to remove redundant field,
    add definitions of maximum size of various fields

    Revision 1.19  2003/02/20 21:20:10  vanandel
    spelling correction on latitude
    change union to inline for timeseries and polarization
    parameters

    add XDWELL structure

    Revision 1.18  2003/01/31 21:33:06  vanandel
    Current 'draft', with input from Josh, Eric, Mitch.

 */


# endif /* PIRAQ_H */
