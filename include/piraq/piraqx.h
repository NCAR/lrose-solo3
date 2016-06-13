#ifndef PIRAQX_H
#define PIRAQX_H
/**
 * @file   piraqx.h

 * $Date$
 * $Id$

 *
 *
 *  @author Joseph VanAndel <vanandel@ucar.edu>
 */
#include "piraq/dd_types.h"
#include "piraq/dd_defines.h"

/* definition of several different data formats */
#ifndef DATA_SIMPLEPP

#define DATA_SIMPLEPP    0 /* simple pulse pair ABP */
#define DATA_POLYPP      1 /* poly pulse pair ABPAB */
#define DATA_DUALPPFLOAT 2 /* ARC HiQ dual PRT (float ABP,ABP) */
#define DATA_POL1        3 /* dual polarization pulse pair ABP,ABP */
#define DATA_POL2        4 /* more complex dual polarization ??????? */
#define DATA_POL3        5 /* almost full dual polarization with log integers */
#define DATA_SIMPLEPP16  6 /* simple pulse pair ABP (16-bit ints not floats) */
#define DATA_DOW        7 /* dow data format */
#define DATA_POL12       8 /* simple pulse pair ABP (16-bit ints not floats) */
#define DATA_POL_PLUS    9 /* full pol plus */
#define DATA_MAX_POL    10 /* same as full plus plus more gates */
#define DATA_HVSIMUL    11 /* simultaneous transmission of H and V */
#define DATA_SHRTPUL    12 /* same as MAX_POL with gate averaging */
#define DATA_SMHVSIM    13 /* 2000 DOW4 copolar matrix for simultaneous H-V (no iq average) */
#define DATA_DUALPP     15 /* DOW dual prt pulse pair ABP,ABP */
/* ABP data computed in piraq3: rapidDOW * project */
#define	DATA_ABPDATA   16

/* Staggered PRT ABP data computed  in piraq3: rapidDOW project */
#define    PIRAQ_ABPDATA_STAGGER_PRT 17


#define DATA_TYPE_MAX PIRAQ_ABPDATA_STAGGER_PRT /* limit of data types */

#define DATA_POL_PLUS_CMP 29	/* full pol plus */
#define DATA_MAX_POL_CMP  30	/* same as full plus plus more gates */
#define DATA_HVSIMUL_CMP  31	/* simultaneous transmission of H and V */
#define DATA_SHRTPUL_CMP  32	/* same as MAX_POL with gate averaging */

#define PIRAQX_CURRENT_REVISION 1


#define PIRAQ3_MAX_GATES 2000
#define PIRAQ3_MAX_TS_GATES 20  /* maximum # of time-series gates */
#define PIRAQ3_MAX_TS_SAMPLES 256 /* maximum # of time-series samples/gate */

#endif /* DATA_SIMPLEPP */



typedef struct piraqX_header_rev1
{		/* /code/oye/solo/translate/piraq.h
         * all elements start on 4-byte boundaries
         * 8-byte elements start on 8-byte boundaries
         * character arrays that are a multiple of 4
         * are welcome
         */
    char desc[4];			/* "DWLX" */
    uint4 recordlen;        /* total length of record - must be the second field */
    uint4 channel;          /* e.g., RapidDOW range 0-5 */
    uint4 rev;		        /* format revision #-from RADAR structure */
    uint4 one;			    /* always set to the value 1 (endian flag) */
    uint4 byte_offset_to_data;
    uint4 dataformat;

    uint4 typeof_compression;	/*  */
/*
      Pulsenumber (pulse_num) is the number of transmitted pulses
since Jan 1970. It is a 64 bit number. It is assumed
that the first pulse (pulsenumber = 0) falls exactly
at the midnight Jan 1,1970 epoch. To get unix time,
multiply by the PRT. The PRT is a rational number a/b.
More specifically N/Fc where Fc is the counter clock (PIRAQ_CLOCK_FREQ),
and N is the divider number. So you can get unix time
without roundoff error by:
secs = pulsenumber * N / Fc. The
computations is done with 64 bit arithmatic. No
rollover will occur.

The 'nanosecs' field is derived without roundoff
error by: 100 * (pulsenumber * N % Fc).

Beamnumber is the number of beams since Jan 1,1970.
The first beam (beamnumber = 0) was completed exactly
at the epoch. beamnumber = pulsenumber / hits.
*/


#ifdef _TMS320C6X   /* TI doesn't support long long */
    uint4 pulse_num_low;
    uint4 pulse_num_high;
#else
    uint8 pulse_num;   /*  keep this field on an 8 byte boundary */
#endif
#ifdef _TMS320C6X   /* TI doesn't support long long */
    uint4 beam_num_low;
    uint4 beam_num_high;
#else
    uint8 beam_num;	/*  keep this field on an 8 byte boundary */
#endif
    uint4 gates;
    uint4 start_gate;
    uint4 hits;
/* additional fields: simplify current integration */
    uint4 ctrlflags; /* equivalent to packetflag below?  */
    uint4 bytespergate;
    float4 rcvr_pulsewidth;
#define PX_NUM_PRT 4
    float4 prt[PX_NUM_PRT];
    float4 meters_to_first_gate;

    uint4 num_segments;  /* how many segments are we using */
#define PX_MAX_SEGMENTS 8
    float4 gate_spacing_meters[PX_MAX_SEGMENTS];
    uint4 gates_in_segment[PX_MAX_SEGMENTS]; /* how many gates in this segment */



#define PX_NUM_CLUTTER_REGIONS 4
    uint4 clutter_start[PX_NUM_CLUTTER_REGIONS]; /* start gate of clutter filtered region */
    uint4 clutter_end[PX_NUM_CLUTTER_REGIONS];  /* end gate of clutter filtered region */
    uint4 clutter_type[PX_NUM_CLUTTER_REGIONS]; /* type of clutter filtering applied */

#define PIRAQ_CLOCK_FREQ 10000000  /* 10 Mhz */

/* following fields are computed from pulse_num by host */
    uint4 secs;     /* Unix standard - seconds since 1/1/1970
                       = pulse_num * N / ClockFrequency */
    uint4 nanosecs;  /* within this second */
    float4 az;   /* azimuth: referenced to 9550 MHz. possibily modified to be relative to true North. */
    float4 az_off_ref;   /* azimuth offset off reference */
    float4 el;		/* elevation: referenced to 9550 MHz.  */
    float4 el_off_ref;   /* elevation offset off reference */

    float4 radar_longitude;
    float4 radar_latitude;
    float4 radar_altitude;
#define PX_MAX_GPS_DATUM 8
    char gps_datum[PX_MAX_GPS_DATUM]; /* e.g. "NAD27" */

    uint4 ts_start_gate;   /* starting time series gate , set to 0 for none */
    uint4 ts_end_gate;     /* ending time series gate , set to 0 for none */

    float4 ew_velocity;

    float4 ns_velocity;
    float4 vert_velocity;

    float4 fxd_angle;		/* in degrees instead of counts */
    float4 true_scan_rate;	/* degrees/second */
    uint4 scan_type;
    uint4 scan_num;
    uint4 vol_num;

    uint4 transition;
    float4 xmit_power;

    float4 yaw;
    float4 pitch;
    float4 roll;
    float4 track;
    float4 gate0mag;  /* magnetron sample amplitude */
    float4 dacv;
    uint4  packetflag;

    /*
    // items from the depricated radar "RHDR" header
    // do not set "radar->recordlen"
    */

    uint4 year;             /* e.g. 2003 */
    uint4 julian_day;

#define PX_MAX_RADAR_NAME 16
    char radar_name[PX_MAX_RADAR_NAME];
#define PX_MAX_CHANNEL_NAME 16
    char channel_name[PX_MAX_CHANNEL_NAME];
#define PX_MAX_PROJECT_NAME 16
    char project_name[PX_MAX_PROJECT_NAME];
#define PX_MAX_OPERATOR_NAME 12
    char operator_name[PX_MAX_OPERATOR_NAME];
#define PX_MAX_SITE_NAME 12
    char site_name[PX_MAX_SITE_NAME];


    uint4 polarization;
    float4 test_pulse_pwr;
    float4 test_pulse_frq;
    float4 frequency;

    float4 noise_figure;
    float4 noise_power;
    float4 receiver_gain;
    float4 E_plane_angle;  /* offsets from normal pointing angle */
    float4 H_plane_angle;


    float4 data_sys_sat;
    float4 antenna_gain;
    float4 H_beam_width;
    float4 V_beam_width;

    float4 xmit_pulsewidth;
    float4 rconst;
    float4 phaseoffset;

    float4 zdr_fudge_factor;

    float4 mismatch_loss;
    float4 rcvr_const;

    float4 test_pulse_rngs_km[2];
    float4 antenna_rotation_angle;   /* S-Pol 2nd frequency antenna may be 30 degrees off vertical */

#define PX_SZ_COMMENT 64
    char comment[PX_SZ_COMMENT];
    float4 i_norm;  /* normalization for timeseries */
    float4 q_norm;
    float4 i_compand;  /* companding (compression) parameters */
    float4 q_compand;
    float4 transform_matrix[2][2][2];
    float4 stokes[4];

    float4 vxmit_power;
    float4 vtest_pulse_pwr;
    float4 vnoise_power;
    float4 vreceiver_gain;
    float4 vantenna_gain;
    float4 h_rconst;
    float4 v_rconst;
    float4 peak_power;            /* added by JVA -  needed for
                                     v/h_channel_radar_const */
    float4 spare[12];

    /*
    // always append new items so the alignment of legacy variables
    // won't change
    */
} PIRAQX;


/*
 * PIRAQX rev2 structure
 */
typedef struct piraqX_header_rev2 {
		/* all elements start on 4-byte boundaries
         * 8-byte elements start on 8-byte boundaries
         * character arrays that are a multiple of 4
         * are welcome
         */
/*
The first int in any potential FIFO record should be a size
That way recording and ethernet functions can deal with them and all future revisions
including totally new record types in a uniform way.  MAR 2/24/05
Therefore, I switched the order of the first two 32bit entities.
*/
	uint4 recordlen;        /* total length of record - must be the second field */
    char desc[4];			/* "DWLX" */
    uint4 channel;          /* e.g., RapidDOW range 0-5 */
    uint4 rev;		        /* format revision #-from RADAR structure */
    uint4 one;			    /* always set to the value 1 (endian flag) */
    uint4 byte_offset_to_data;
    uint4 dataformat;

    uint4 typeof_compression;
/*
Pulsenumber (pulse_num) is the number of transmitted pulses
since Jan 1970. It is a 64 bit number. It is assumed
that the first pulse (pulsenumber = 0) falls exactly
at the midnight Jan 1,1970 epoch. To get unix time,
multiply by the PRT. The PRT is a rational number a/b.
More specifically N/Fc where Fc is the counter clock (PIRAQ_CLOCK_FREQ),
and N is the divider number. So you can get unix time
without roundoff error by:
secs = pulsenumber * N / Fc. The
computations is done with 64 bit arithmetic. No
rollover will occur.

The 'nanosecs' field is derived without roundoff
error by: 100 * (pulsenumber * N % Fc).

Beamnumber is the number of beams since Jan 1,1970.
The first beam (beamnumber = 0) was completed exactly
at the epoch. beamnumber = pulsenumber / hits.
*/
    uint8 pulse_num;   /*  keep this field on an 8 byte boundary */
    uint8 beam_num;	/*  keep this field on an 8 byte boundary */
    uint4 gates;
    uint4 start_gate;
    uint4 hits;
/* additional fields: simplify current integration */
    uint4 ctrlflags; /* equivalent to packetflag below?  */
    uint4 bytespergate;
    float4 rcvr_pulsewidth;
#define PX_NUM_PRT 4
    float4 prt[PX_NUM_PRT];
    float4 meters_to_first_gate;

    uint4 num_segments;  /* how many segments are we using */
#define PX_MAX_SEGMENTS 8
    float4 gate_spacing_meters[PX_MAX_SEGMENTS];
    uint4 gates_in_segment[PX_MAX_SEGMENTS]; /* how many gates in this segment */

#define PX_NUM_CLUTTER_REGIONS 4
    uint4 clutter_start[PX_NUM_CLUTTER_REGIONS]; /* start gate of clutter filtered region */
    uint4 clutter_end[PX_NUM_CLUTTER_REGIONS];  /* end gate of clutter filtered region */
    uint4 clutter_type[PX_NUM_CLUTTER_REGIONS]; /* type of clutter filtering applied */

#define PIRAQ_CLOCK_FREQ 10000000  /* 10 Mhz */

/* following fields are computed from pulse_num by host */
    uint4 secs;     /* Unix standard - seconds since 1/1/1970
                       = pulse_num * N / ClockFrequency */
    uint4 nanosecs;  /* within this second */
    float4 az;   /* azimuth: referenced to 9550 MHz. possibily modified to be relative to true North. */
    float4 az_off_ref;   /* azimuth offset off reference */
    float4 el;		/* elevation: referenced to 9550 MHz.  */
    float4 el_off_ref;   /* elevation offset off reference */

    float4 radar_longitude;
    float4 radar_latitude;
    float4 radar_altitude;
#define PX_MAX_GPS_DATUM 8
    char gps_datum[PX_MAX_GPS_DATUM]; /* e.g. "NAD27" */

    uint4 ts_start_gate;   /* starting time series gate , set to 0 for none */
    uint4 ts_end_gate;     /* ending time series gate , set to 0 for none */

    float4 ew_velocity;

    float4 ns_velocity;
    float4 vert_velocity;

    float4 fxd_angle;		/* in degrees instead of counts */
    float4 true_scan_rate;	/* degrees/second */
    uint4 scan_type;
    uint4 scan_num;
    uint4 vol_num;

    uint4 transition;
    float4 hxmit_power;

    float4 yaw;
    float4 pitch;
    float4 roll;
    float4 track;
    float4 gate0mag;  /* magnetron sample amplitude */
    float4 dacv;
    uint4  packetflag;

    /*
    items from the deprecated radar "RHDR" header
    do not set "radar->recordlen"
    */

    uint4 year;             /* e.g. 2003 */
    uint4 julian_day;

#define PX_MAX_RADAR_NAME 16
    char radar_name[PX_MAX_RADAR_NAME];
#define PX_MAX_CHANNEL_NAME 16
    char channel_name[PX_MAX_CHANNEL_NAME];
#define PX_MAX_PROJECT_NAME 16
    char project_name[PX_MAX_PROJECT_NAME];
#define PX_MAX_OPERATOR_NAME 12
    char operator_name[PX_MAX_OPERATOR_NAME];
#define PX_MAX_SITE_NAME 12
    char site_name[PX_MAX_SITE_NAME];

    char   polarization[4];	// H, V, +45, -45, RC, LC, etc.--3 character limit. H, V are used in DOS
    float4 test_pulse_pwr;
    float4 test_pulse_frq;
    float4 frequency;		// Radar transmit frequency
	float4 stalofreq;		// Radar local oscillator frequency

    float4 noise_figure;
    float4 noise_power;
    float4 receiver_gain;
    float4 E_plane_angle;  /* offsets from normal pointing angle */
    float4 H_plane_angle;

    float4 data_sys_sat;
    float4 antenna_gain;
    float4 H_beam_width;
    float4 V_beam_width;

    float4 xmit_pulsewidth;
    float4 rconst;
    float4 phaseoffset;

    float4 zdr_fudge_factor;

    float4 mismatch_loss;
    float4 rcvr_const;

    float4 test_pulse_rngs_km[2];
    float4 antenna_rotation_angle;   /* S-Pol 2nd frequency antenna may be 30 degrees off vertical */

#define PX_SZ_COMMENT 64
    char comment[PX_SZ_COMMENT];
    float4 i_norm;  /* normalization for timeseries */
    float4 q_norm;
    float4 i_compand;		// companding (compression) parameters
    float4 q_compand;
    float4 transform_matrix[2][2][2];
    float4 stokes[4];

	float4	vxmit_power;	/* These added 7/25/05 MAR */
	float4	vreceiver_gain;	// Vertical receiver gain
	float4	vantenna_gain;	// Vertical antenna gain
	float4	vnoise_power;	// Vertical noise power
	float4	h_measured_xmit_power; // Measured horizontal peak transmitted power
	float4	v_measured_xmit_power; // Measured vertical peak transmitted power

	uint4	angle_source;	// Changed float4 spare[16] to spare[15] to compensate
							// See Angle source definitions.
	uint4	timingmode;		// Added for compatability with config.dsp. Spare[15]->spare[14]
	float4	tpwidth;		// trigger width as in DOS code but expressed in seconds
	float4	tpdelay;		// trigger delay as in DOS code but expressed in seconds
	float4	delay;			// delay as in DOS code	but expressed in seconds
	float4	pllfreq;		// Frequency command to LO expressed in Hz.
	float4	pllalpha;		// AFC time constant 0 < pllalpha < 1. Averaging time ~1./(1. - pllalpha)
	float4	afclo;			// AFC parameters: typical afclo = 95e6  Hz
	float4	afchi;			// typical afchi = 120e6 Hz
	float4	afcgain;		// typical afcgain = 3.0
	float4	velsign;		// +/- 1 to reverse sense of velocities, if necessary. Value depends on radar characteristics.
	float4 spare[3];		/* this used to be 20 spare floats */
	float4 xmitcoupler;		// Calibrates G0 vs measured xmit power. Changes same sign as power reading.
    /* always append new items so the alignment of legacy variables won't change */
} PIRAQX_REV2;

typedef PIRAQX PIRAQX_REV1;

/*
 * Set these to the minimum and maximum PIRAQX header length.
 * Currently, rev1 is the shorter header and rev2 is the longer.
 */
static const int PIRAQX_MINHEADERLEN = sizeof(PIRAQX_REV1);
static const int PIRAQX_MAXHEADERLEN = sizeof(PIRAQX_REV2);

/* to ease the transition for legacy DRX code */
typedef struct piraqX_header_rev1 INFOHEADER;


/*
 * Accessors which deal with either rev1 or rev2 headers.
 */
static inline uint4 px_rev(PIRAQX *px) {
	/*
	 * Fortunately, rev is in the same place and is the same size
	 * for both PIRAQX rev1 and rev2...
	 */
	return px->rev;
}

static inline char* px_desc(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return((px_rev(px) == 1) ? px->desc : px2->desc);
}

static inline uint4 px_recordlen(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return((px_rev(px) == 1) ? px->recordlen : px2->recordlen);
}

static inline uint4 px_channel(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->channel : px2->channel;
}

static inline uint4 px_one(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->one : px2->one;
}

static inline uint4 px_byte_offset_to_data(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->byte_offset_to_data : px2->byte_offset_to_data;
}

static inline uint4 px_dataformat(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->dataformat : px2->dataformat;
}

static inline uint4 px_typeof_compression(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->typeof_compression : px2->typeof_compression;
}

static inline uint8 px_pulse_num(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->pulse_num : px2->pulse_num;
}

static inline uint8 px_beam_num(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->beam_num : px2->beam_num;
}

static inline uint4 px_gates(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->gates : px2->gates;
}

static inline uint4 px_start_gate(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->start_gate : px2->start_gate;
}

static inline uint4 px_hits(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->hits : px2->hits;
}

static inline uint4 px_ctrlflags(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->ctrlflags : px2->ctrlflags;
}

static inline uint4 px_bytespergate(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->bytespergate : px2->bytespergate;
}

static inline float4 px_rcvr_pulsewidth(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->rcvr_pulsewidth : px2->rcvr_pulsewidth;
}

/*
 * Return our *array* of PRTs
 */
static inline float4* px_prt(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->prt : px2->prt;
}

static inline float4 px_meters_to_first_gate(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->meters_to_first_gate : px2->meters_to_first_gate;
}

static inline uint4 px_num_segments(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->num_segments : px2->num_segments;
}

static inline float4* px_gate_spacing_meters(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->gate_spacing_meters : px2->gate_spacing_meters;
}

static inline uint4* px_gates_in_segment(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->gates_in_segment : px2->gates_in_segment;
}

static inline uint4* px_clutter_start(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->clutter_start : px2->clutter_start;
}

static inline uint4* px_clutter_end(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->clutter_end : px2->clutter_end;
}

static inline uint4* px_clutter_type(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->clutter_type : px2->clutter_type;
}

static inline uint4 px_secs(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->secs : px2->secs;
}

static inline uint4 px_nanosecs(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->nanosecs : px2->nanosecs;
}

static inline float4 px_az(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->az : px2->az;
}

static inline float4 px_az_off_ref(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->az_off_ref : px2->az_off_ref;
}

static inline float4 px_el(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->el : px2->el;
}

static inline float4 px_el_off_ref(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->el_off_ref : px2->el_off_ref;
}

static inline float4 px_radar_longitude(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->radar_longitude : px2->radar_longitude;
}

static inline float4 px_radar_latitude(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->radar_latitude : px2->radar_latitude;
}

static inline float4 px_radar_altitude(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->radar_altitude : px2->radar_altitude;
}

static inline char* px_gps_datum(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->gps_datum : px2->gps_datum;
}

static inline uint4 px_ts_start_gate(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->ts_start_gate : px2->ts_start_gate;
}

static inline uint4 px_ts_end_gate(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->ts_end_gate : px2->ts_end_gate;
}

static inline float4 px_ew_velocity(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->ew_velocity : px2->ew_velocity;
}

static inline float4 px_ns_velocity(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->ns_velocity : px2->ns_velocity;
}

static inline float4 px_vert_velocity(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->vert_velocity : px2->vert_velocity;
}

static inline float4 px_fxd_angle(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->fxd_angle : px2->fxd_angle;
}

static inline float4 px_true_scan_rate(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->true_scan_rate : px2->true_scan_rate;
}

static inline uint4 px_scan_type(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->scan_type : px2->scan_type;
}

static inline uint4 px_scan_num(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->scan_num : px2->scan_num;
}

static inline uint4 px_vol_num(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->vol_num : px2->vol_num;
}

static inline uint4 px_transition(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->transition : px2->transition;
}

/*
 * Rev2 changed the name of xmit_power to hxmit_power.  We treat them as
 * synonyms for both revsions.
 */
static inline float4 px_xmit_power(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->xmit_power : px2->hxmit_power;
}
static inline float4 px_hxmit_power(PIRAQX *px) {
	return px_xmit_power(px);
}

static inline float4 px_yaw(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->yaw : px2->yaw;
}

static inline float4 px_pitch(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->pitch : px2->pitch;
}

static inline float4 px_roll(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->roll : px2->roll;
}

static inline float4 px_track(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->track : px2->track;
}

static inline float4 px_gate0mag(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->gate0mag : px2->gate0mag;
}

static inline float4 px_dacv(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->dacv : px2->dacv;
}

static inline uint4 px_packetflag(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->packetflag : px2->packetflag;
}

static inline uint4 px_year(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->year : px2->year;
}

static inline uint4 px_julian_day(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->julian_day : px2->julian_day;
}

static inline char* px_radar_name(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->radar_name : px2->radar_name;
}

static inline char* px_channel_name(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->channel_name : px2->channel_name;
}

static inline char* px_project_name(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->project_name : px2->project_name;
}

static inline char* px_operator_name(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->operator_name : px2->operator_name;
}

static inline char* px_site_name(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->site_name : px2->site_name;
}

/*
 * Polarization changed from a uint4 in rev1 to a char[4] in rev2.
 * For rev2, we return the char[4] cast into a uint4.
 */
static inline uint4 px_polarization(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	if (px_rev(px) == 1)
		return px->polarization;
	else {
		uint4 *uintval = (uint4*)px2->polarization;
		return *uintval;
	}
}

static inline float4 px_test_pulse_pwr(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->test_pulse_pwr : px2->test_pulse_pwr;
}

static inline float4 px_test_pulse_frq(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->test_pulse_frq : px2->test_pulse_frq;
}

static inline float4 px_frequency(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->frequency : px2->frequency;
}

/*
 * For rev1, there is no stalofreq, so we return -1.
 */
static inline float4 px_stalofreq(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? -1 : px2->stalofreq;
}

static inline float4 px_noise_figure(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->noise_figure : px2->noise_figure;
}

static inline float4 px_noise_power(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->noise_power : px2->noise_power;
}

static inline float4 px_receiver_gain(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->receiver_gain : px2->receiver_gain;
}

static inline float4 px_E_plane_angle(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->E_plane_angle : px2->E_plane_angle;
}

static inline float4 px_H_plane_angle(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->H_plane_angle : px2->H_plane_angle;
}

static inline float4 px_data_sys_sat(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->data_sys_sat : px2->data_sys_sat;
}

static inline float4 px_antenna_gain(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->antenna_gain : px2->antenna_gain;
}

static inline float4 px_H_beam_width(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->H_beam_width : px2->H_beam_width;
}

static inline float4 px_V_beam_width(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->V_beam_width : px2->V_beam_width;
}

static inline float4 px_xmit_pulsewidth(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->xmit_pulsewidth : px2->xmit_pulsewidth;
}

static inline float4 px_rconst(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->rconst : px2->rconst;
}

static inline float4 px_phaseoffset(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->phaseoffset : px2->phaseoffset;
}

static inline float4 px_zdr_fudge_factor(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->zdr_fudge_factor : px2->zdr_fudge_factor;
}

static inline float4 px_mismatch_loss(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->mismatch_loss : px2->mismatch_loss;
}

static inline float4 px_rcvr_const(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->rcvr_const : px2->rcvr_const;
}

static inline float4* px_test_pulse_rngs_km(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->test_pulse_rngs_km : px2->test_pulse_rngs_km;
}

static inline float4 px_antenna_rotation_angle(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->antenna_rotation_angle : px2->antenna_rotation_angle;
}

static inline char* px_comment(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->comment : px2->comment;
}

static inline float4 px_i_norm(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->i_norm : px2->i_norm;
}

static inline float4 px_q_norm(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->q_norm : px2->q_norm;
}

static inline float4 px_i_compand(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->i_compand : px2->i_compand;
}

static inline float4 px_q_compand(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->q_compand : px2->q_compand;
}

static inline float4* px_stokes(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->stokes : px2->stokes;
}

static inline float4 px_vxmit_power(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->vxmit_power : px2->vxmit_power;
}

static inline float4 px_vreceiver_gain(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->vreceiver_gain : px2->vreceiver_gain;
}

static inline float4 px_vantenna_gain(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->vantenna_gain : px2->vantenna_gain;
}

static inline float4 px_vnoise_power(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? px->vnoise_power : px2->vnoise_power;
}

/*
 * These three only exist in rev1, so zero is returned for these values
 * in rev2.
 */
static inline uint4 px_h_rconst(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return (uint4)((px_rev(px) == 1) ? px->h_rconst : 0);
}

static inline uint4 px_v_rconst(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return (uint4)((px_rev(px) == 1) ? px->v_rconst : 0);
}

static inline uint4 px_peak_power(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return (uint4)((px_rev(px) == 1) ? px->peak_power : 0);
}

/*
 * The remaining struct members only exist in rev2, so zero will be
 * returned for all of these values in rev1.
 */
static inline float4 px_h_measured_xmit_power(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->h_measured_xmit_power;
}

static inline float4 px_v_measured_xmit_power(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->v_measured_xmit_power;
}

static inline uint4 px_angle_source(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->angle_source;
}

static inline uint4 px_timingmode(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->timingmode;
}

static inline float4 px_tpwidth(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->tpwidth;
}

static inline float4 px_tpdelay(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->tpdelay;
}

static inline float4 px_delay(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->delay;
}

static inline float4 px_pllfreq(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->pllfreq;
}

static inline float4 px_pllalpha(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->pllalpha;
}

static inline float4 px_afclo(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->afclo;
}

static inline float4 px_afchi(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->afchi;
}

static inline float4 px_afcgain(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->afcgain;
}

static inline float4 px_velsign(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->velsign;
}

static inline float4 px_xmitcoupler(PIRAQX *px) {
	PIRAQX_REV2 *px2 = (PIRAQX_REV2*)px;
	return(px_rev(px) == 1) ? 0 : px2->xmitcoupler;
}

#endif /* PIRAQX_H */
