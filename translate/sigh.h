/* 	$Id$	 */
/* c------------------------------------------------------------------------ */
# ifndef SIGH_H
# define SIGH_H

#include <dd_defines.h>

/* Miscellaneous def */
# define SIGMET_LAST_RECORD 0
# define SIGMET_END_OF_RAY 1
# define SIGMET_MAX_FIELDS 16
# define SIGMET_MAX_GATES 2048
# define SIGMET_REC_SIZE 6144	/* bytes */

# define X2(x) (sig_swap2(&(x)))
# define X4(x) (sig_swap4(&(x)))

# define SIGN16 0x8000
# define SIGN15 0x4000
# define MASK16 0xffff
# define MASK15 0x7fff

# define UNSCALE_LAT_LON(x) ((x)*8.381903173e-8)
# define S100(x) ((x)*100.0+0.5)

typedef char           SINT1;
typedef unsigned char  UINT1;
typedef twob           SINT2;
typedef twob           UINT2;
typedef fourb          SINT4;
typedef fourb          UINT4;

/* c------------------------------------------------------------------------ */

struct sigmet_useful_items {
    double base_time;
    double last_time;
    double start_time;
    double stop_time;
    double time0;

    int bad_date_time;
    int byte_count;
    int extended_header_flag;
    int header_count;
    int io_type;
    int ignore_rest_of_file;
    int irtotl;
    int isweep;
    int iwritn;
    int min_ray_size;
    int new_vol;
    int new_sweep;
    int num_fields;
    int ok_ray;
    int rays_in_scan;
    int run_time;
    int sigmet_fid;
    int sweep_num;

    int data_type_id[SIGMET_MAX_FIELDS];
    int field_word_count[SIGMET_MAX_FIELDS];

    struct sig_rec_struct *top_rsq;
    struct ingest_data_header *idh[SIGMET_MAX_FIELDS];
    char *sigmet_raw_field[SIGMET_MAX_FIELDS];
    short *sigmet_lut[SIGMET_MAX_FIELDS];
};
/* c------------------------------------------------------------------------ */
# define RAW_REC_QUE_SIZE 5
# define        SIG_RAW_DATA_PROD 0x0001
# define  SIG_INGEST_DATA_HEADERS 0x0002

struct sig_rec_struct {
    struct sig_rec_struct *last;
    struct sig_rec_struct *next;
    struct raw_prod_bhdr *rph;
    struct structure_header *sh;
    int bytes_left;
    int rec_type;
    char *rec_buf;
    char *cp;			/* current pointer */
};
/* c------------------------------------------------------------------------ */

# define DM_XHDR_P  0x0001	/* Extended Headers */
# define DB_XHDR_P  0
# define DM_UDBZ_P  0x0002	/* UnCorrected reflectivity */
# define DB_UDBZ_P  1
# define DM_CDBZ_P  0x0004	/* Corrected reflectivity */
# define DB_CDBZ_P  2
# define DM_VEL_P   0x0008	/* Velocity */
# define DB_VEL_P   3
# define DM_WIDTH_P 0x0010	/* Width */
# define DB_WIDTH_P 4
# define DM_ZDR_P   0x0020	/* Differential reflectivity */
# define DB_ZDR_P   5
# define DM_CRAIN_P 0x0040	/* Rainfall rate (stored as dBZ) */
# define DB_CRAIN_P 6
# define DB_LEVEL_P 7		/* Levels (stored as dBZ) */

/* c------------------------------------------------------------------------ */

struct raw_prod_bhdr {
    SINT2 irec;                 /* record # */
    SINT2 isweep;               /* sweep # */
    SINT2 iray_ptr;             /* byte offset (from bhdr) of first ray */
    SINT2 iray_num;             /* ray number of above first ray */
    SINT2 iflags;               /* flags */
# define APB_NOGOOD_P (0x0001)  /* block has invalid data */
    SINT2 ipad;
};
/* -------------------- Structure Header --------------------
 * The following five words appear at the beginning of many IRIS structures
 * to help identify and use them.
 */
#define STRUCT_HEADER_SZ_P   12          /* Size of header */
/*
 *
 * this structure has a sizeof 16 bytes on the Sun
 * until you break up the SINT4 variable into 2 SINT2 variables
 *
 */
struct structure_header 
{
  SINT2 id;                             /*Structure identifier */
#define ST_DEVICE_ST_P     1       /* Device_Status */
#define ST_TASK_CONF_P     2       /* Task_Configuration */
#define ST_INGEST_SUM_P    3       /* INGEST_summary */
#define ST_INGEST_DATA_P   4       /* INGEST_data_header */
#define ST_PRODUCT_CONF_P  6       /* Product_configuration */
#define ST_PRODUCT_HDR_P   7       /* Product_hdr */
#define ST_TAPE_HEADER_P   8       /* Tape_header_record */

  SINT4 ibytes;                        /*# of bytes in entire structure */
  SINT2 ivers;                         /*Format version # */
#define SVER_DEVICE_ST_P     2     /* Device_Status */
#define SVER_TASK_CONF_P     2     /* Task_Configuration */
#define SVER_INGEST_SUM_P    1     /* INGEST_summary */
#define SVER_INGEST_DATA_P   2     /* INGEST_data_header */
#define SVER_PRODUCT_CONF_P  3     /* Product_configuration */
#define SVER_PRODUCT_HDR_P   4     /* Product_hdr */
#define SVER_TAPE_HEADER_P   0     /* Tape_header_record */

  SINT2 reserved;
  SINT2 iflags;                        /* Flag Bits */
#define HD_STCOMP_P    0x0001      /*  Structure Complete */
};

/* c------------------------------------------------------------------------ */

struct ymds_time     
{
    /* Storage of time as Year, Month, day,
     * and # seconds into the day
     */ 
  SINT2 iyear, imon, iday;
  SINT4 isec;
};
/* c------------------------------------------------------------------------ */

struct ingest_data_header {
    struct structure_header hdr;
    struct ymds_time time;
    SINT2 idata;                /* Data code (See Task_DSP_Info.IDATA) */
    SINT2 isweep;               /* Sweep number */
    SINT2 inrev;                /* Number of rays in one revolution */
    SINT2 isndx;                /* Implies angle of first pointer */
    SINT2 irtotl;               /* Total number of rays */
    SINT2 iwritn;               /* # rays written [0 - IRTOTL] */
    SINT2 iangle;               /* Fixed angle for this sweep */
    SINT2 bits_bin;             /* # bits/bin for these data */
    SINT2 ipad[19];		/* should sum to 76 bytes */
};

/* c------------------------------------------------------------------------ */
/*
C -------------------- Ray Headers --------------------
C
C All radar rays are prefixed with the following short header.  The idea is
C that this is sufficient for most products, and thus takes up the least amount
C of space.  Products which require additional ray information must arrange to
C have the "Extended Header" recorded along with the rest of the data.  Note
C that even the "Extended Header" has this ray header attached.
*/

struct ray_header {
/*
C Antenna positions at the start and end of the ray are stored as 16-bit binary
C angles.
*/
    SINT2 iaz_start, iel_start;	/* AZ and EL at start of ray */
    SINT2 iaz_end  , iel_end;	/* AZ and EL at end of ray */
    
    SINT2 ibincount;		/* Actual number of bins */
    SINT2 itime;		/* Time offset in 1/10 secs, unsigned */
};
/* c------------------------------------------------------------------------ */
/*
C --- Version 1 Extended Header, and Extended Header Ray --- 

C Information about the radar site.  Latitude and Longitude are stored as 
C 32-bit binary angles, where 2000000 Hex is 45 degrees North Latitude or East
C Longitude.  All heights are in meters; all velocities are in cm/sec; all 
C angles are binary angles, all angular velocities are in binary angles/second.

C The exact time that the ray was acquired is stored to the nearest 
C millisecond.  The time is relative to the sweep starting time given 
C in the Ingest_Data_Header.
*/

struct extended_header_v1 {
    SINT4 itimems;		/* Time ray was initiated */
    
    SINT2 icallevel;		/* Calibration signal level */
    
    SINT2 iaz, iel;
    SINT2 itrain, ielor;
    SINT2 ipitch;
    SINT2 iroll;
    SINT2 iheading;
    
    SINT2 ivel_az;
    SINT2 ivel_el;
    SINT2 ivel_pitch;
    SINT2 ivel_roll;
    SINT2 ivel_heading;
    
    SINT4 ilatitude, ilongitude; /* Latitude and Longitude */

    SINT2 ialtitude;		/* Radar height (m above ground) */
    
    SINT2 ivel_east;		/* Radar Velocity East */
    SINT2 ivel_north;		/* Radar Velocity North */
    SINT2 ivel_up;		/* Radar Velocity Up */
    SINT4 iupdate_age;
    
    SINT2 iflag;
    SINT2 ivel_correction;
};

/* c------------------------------------------------------------------------ */
/* -------------------- Task Scheduling Information --------------------
 */
#define TASK_SCHED_INFO_SZ_P  120

struct task_sched_info
{
  /*There are six times stored for each task.  All times are in seconds offset
    from the start of a 24-hour day.  Invalid times are indicated by a value
    of -1.

    START and STOP are absolute times that define when the task can be running
    at all.  SKIP is the desired time between individual runs. Each time a task
    is run the beginning time is stored in LAST_RUN.  When a run finishes, the
    time used is written to TIME_USED.  LAST_RUN_DAY is a relative day number 
    that is used to resolve very old last run times.
  */

  SINT4 istart;                        /* Start time for task */
  SINT4 istop;                         /* Stop time for task */
  SINT4 iskip;                         /* Desired skip between runs */
  SINT4 ilast_run;                     /* Time that task was last run */
  SINT4 itime_used;                    /* Time used on last run */
  
  SINT4 ilast_run_day;                 /* Relative day # for ILAST_RUN */
  /*
    The flag word modifies the scheduling details.
    * ASAP indicates that the scheduling times should be ignored and that the
    task should be run once as soon as possible, after which the ASAP bit 
    will be cleared.
    * MAND indicates that the task can preempt a non-MAND task that is already
    running, and also defines a separate level of priority for normal time-
    based scheduling.
    * LSKIP is a late skip flag which indicates that the task must run within
    a certain tolerance of its expected starting times, else it will be 
    skipped.
    * DESCH indicates to INGEST that the task should be descheduled after 
    running it, i.e., its state should go to INACTIVE, rather than SCHEDULED.
    This bit is often used in conjunction with ASAP.
    */

  SINT2 iflag;
#define SCH_ASAP_P    0x0001       /* Start as_soon_as_possible */
#define SCH_ASAP_BIT_P   0
#define SCH_MAND_P    0x0002       /* Task has mandatory status */
#define SCH_MAND_BIT_P   1
#define SCH_LSKIP_P   0x0004       /* Late Skip */
#define SCH_LSKIP_BIT_P  2
#define SCH_MESTU_P   0x0008       /* TIME_USED is measured, else estimated. */
#define SCH_MESTU_BIT_P  3
#define SCH_DESCH_P   0x0010       /* Deschedule task after running it. */
#define SCH_DESCH_BIT_P  4
  char ipad[94];
};

/* -------------------- Task DSP Information -------------------- */

#define TASK_DSP_INFO_SZ_P  320
struct task_dsp_info
{
  char ipad1[2]; /* Unit number of DSP used to collect the data? */



  SINT2 idsptype; /*The type of DSP used to collect the data.  
                    See dsp.inc file for definitions*/

  /*
  The type(s) of data being recorded by the DSP are summarized in IDATA and
  IDATA_AUX.  IDATA is an array of 32 bit positions, indicating which major
  catagories of data are to be ingested.  For certain data types IDATA_AUX
  is available to give more specific information about a selected major
  data type.

  Note that the "Extended Header" type is included here, though strictly
  speaking it is not generated by the DSP.  Both the bit mask and the bit
  position parameters are given for types 0-31.  Higher number data types
  are also allocated to be used with the product generators.  For most
  definitions, see the dsp.inc file in the libs include directory.
  */
  UINT4 idata;
  UINT4 idata_aux[32];

  /* Trigger and related Info */
  SINT4 iprf;                          /* PRF in hertz */
  SINT4 ipw;                           /* Pulse Width in 1/100 usec. */


  SINT2 itrig;                          /*Trigger rate flag, see DSP.inc */

  SINT2 idual_delay;                /* Stabilization # pulses for multi-PRF */
  SINT2 iagc_code;                  /* Selected coefficient for AGC feedback */

  /* Miscellaneous */

  SINT2 isamp;                         /* # Pulses used (Sample Size) */

  SINT2 igain;
#define GAIN_FIXED_P   0           /* Fixed receiver gain */
#define GAIN_STC_P     1           /* STC Gain */
#define GAIN_AGC_P     2           /* AGC Gain */

  /*File defining clutter filter.  If the file name is blank, then the first
    filter code is used at all ranges. */

  char sfilter[12];
  SINT2 ifilter_first;                /* Filter used on first bin */

  /*Fixed Gain Level */
  SINT2 ifixed_gain;                   /* 1000*fixed gain level(0-1) */
  char ipad[150];
};

/* -------------------- Task Calibration Information -------------------- */

#define TASK_CALIB_INFO_SZ_P  320
struct task_calib_info
{
  /* Various calibration slopes and thresholds */
  SINT2 iz_slope;                /* LOG slope in dB*4096/ A/D count */
  
  SINT2 izns_thr;                /* LOG noise threshold in dB*16 */
  SINT2 iccr_thr;                /* Clutter Correction threshold in dB*16 */
  SINT2 isqi_thr;                /* SQI threshold * 256 */
  SINT2 isig_thr;                /* Signal power threshold in dBm*16 */
  SINT2 spare_thr[4];
  SINT2 iz_calib;                /* Calibration reflectivity, dBZ*16 @ 1Km */

  /* Threshold control flags for various parameters. */

  SINT2 iuz_tcf;                 /* UnCorrected Z flags */
  SINT2 icz_tcf;                 /* Corrected Z flags */
  SINT2 ivl_tcf;                 /* Velocity flags */
  SINT2 iwd_tcf;                 /* Width flags */
  SINT2 spare_tcf[4];

 /* Miscellaneous processing flags. */

  SINT2 iflags;
#define CF_USPKL_P   0x0001       /* UnCorrected Z speckle remover ON */
#define CF_CSPKL_P   0x0002       /* Corrected Z speckle remover ON */
#define CF_VSPKL_P   0x0004       /* Velocity speckle remover ON */
#define CF_WSPKL_P   0x0008       /* Width speckle remover ON */
#define CF_RNV_P     0x0010       /* Data is range normalized */
#define CF_BPLS_P    0x0020       /* DSP issues pulse at beginning of rays */
#define CF_EPLS_P    0x0040       /* DSP issues pulse at end of rays */
#define CF_VPL_P     0x0080       /* DSP varies # pulses in Dual PRF mode */
#define CF_3LAG_P    0x0100       /* Use 3-lag Doppler processing (else 2) */
#define CF_VCOR_P    0x0200       /* Velocities corrected for ship motion */

  SINT2 iz2_slope;                /* Slope and calibration reflectivity for */
  SINT2 iz2_calib;                /*  second DSP in dual DSP processor. */
  SINT2 izdr_bias;                /* ZDR bias in dB*16 */

  char ipad[276];
};

/* -------------------- Task Range Selection Information --------------------*/

#define TASK_RANGE_INFO_SZ_P  160
struct  task_range_info 
{
  SINT4 ibin_first;                    /* Range of first (input) bin in cm */
  SINT4 ibin_last;                     /* Range of last (input) bin in cm */

  SINT2 ibin_in_num;                   /* Number of input bins */
  SINT2 ibin_out_num;                  /* Number of output bins */

  /*
  If IBIN_VAR is zero the input bins are equally spaced, in which case the
  input and output bin spacings and any bin averaging are defined.  If
  IBIN_VAR is non-zero, then it indicates that some type of variable input
  spacing has been selected.  In that case, there are (up to) 48 bytes reserved
  to describe the variable format specifically.
  */

  SINT2 ibin_var;                   /* Non-Zero ==> variable resolution */
  SINT4 ibin_in_step;               /* Step between input bins in cm */
  SINT4 ibin_out_step;              /* Step between output bins in cm */

  SINT2 ibin_avg;                   /* 0:No Avg,  1:Avg Pairs, ... */
  char ipad[136];
};

/* -------------------- Task Antenna Scan Information --------------------*/
#define MAX_SWEEPS_P (40)
#define TASK_PSCAN_INFO_SIZE (200)

/* --- RHI Azimuth List ---
 *   Starting and ending elevation angles, followed by
 *   AZ list of binary angles 
 */
struct task_rhi_scan_info
{
  SINT2 istartel;
  SINT2 iendel;
  SINT2 iazlst[MAX_SWEEPS_P];
  char ipad[116];
};



/* --- PPI Elevation List ---
   Actual # of items in IELLST followed by
   EL list of binary angles */

struct task_ppi_scan_info
{

  SINT2 istartaz;
  SINT2 iendaz;
  SINT2 iellst[MAX_SWEEPS_P];
  char ipad[116];
};


/* --- File Scan Info ---
   First azimuth and elevation from the file, followed by
   the file name. */

struct task_file_scan_info
{
  SINT2 ifirstaz;
  SINT2 ifirstel;
  char scan_file[12];
  char ipad[184];
};   

/* --- Manual Scan Info --- */

struct task_manual_scan_info
{

  SINT2 iman_flags;               /*bit 0: continuous */
  char ipad[198];
};

/* c------------------------------------------------------------------------ */
#define TASK_SCAN_INFO_SZ_P  320
struct task_scan_info
{
  SINT2 iscan;                     /* Antenna Scan Mode */
#define SCAN_PPI_P    1            /* PPI (Sector) */
#define SCAN_RHI_P    2            /* RHI (Sector) */
#define SCAN_PPIMAN_P 3            /* PPI Manual */
#define SCAN_CON_P    4            /* Continuous AZ PPI's */
#define SCAN_FIL_P    5            /* File scan */
#define SCAN_RHIMAN_P 6            /* RHI Manual */

/*
  IRES is the desired angular resolution expressed as an integer number of
  hundredths of degrees.  This format was chosen (rather than binary angle)
  so that user-units of resolution could be expressed exactly.  In manual 
  scans, this is the number of rays to record.
*/

  SINT2 ires100;
  SINT2 idumyfill;
  SINT2 isweeps;                       /* # Sweeps to perform */

  /* The following Union contains scan information, the format of which varies
     according to the scan mode. */
  union
    {
      struct task_rhi_scan_info rhi;

      struct task_ppi_scan_info ppi;

      struct task_file_scan_info fil;

      struct task_manual_scan_info man;
    } u;
  char ipad[112];
};

/* -------------------- Task Miscellaneous Information -------------------- */

#define TASK_MISC_INFO_SZ_P  320
struct task_misc_info 
{
  SINT4 ilambda;                       /*Radar wavelength in cm*100 */

  char str_id[16];                   /*User's id of xmitter, receiver, etc */
  SINT4 ixmt_pwr;                      /*Transmit power in watts */

  SINT2 iflags;
#define MSC_DSSIM_P    0x0001      /*Digital Signal Simulator in use */
#define MSC_POLAR_P    0x0002      /*Polarization was in use */
#define MSC_NOFILE_P   0x0004      /*Do not store summary or data files */
#define MSC_RAW_P      0x0008      /*Schedule raw product to be made */
#define MSC_KEEP_P     0x0010      /*Keep this file (Watchdog info) */

  char  ipolar[24];             /* Reserved for polarization description */

  /*
  The parameter(s) that are displayed in real time during the INGEST of the 
  task are given in IRDISP.  These are parameter bit values in the range
  [0,31].  Any value out of this range will produce no display. */

  SINT2 irdisp[2];                  /*Real-Time-Display parameter(s) */

  /* If real time PPI or RHI product files are to be made from the displayed
   images, IRTPRODUCT tells what to do. */

  SINT2 irtproduct;                 /*Sweep # to store if positive, else: */
#define RTP_VOID_P    0            /*  Dont make any product files */
#define RTP_ALL_P    -1            /*  Make all possible product files */

  SINT2 idumyfill2;
  SINT4 itrunc;                    /*Truncation height in cm */

  /*
  Users comments are associated with tasks.  The comment buffer itself is
  part of the Task Configuration Structure, and the number of bytes
  inserted in that buffer so far is given below.
  */

  SINT2 icomnt_num;                /*# Bytes of comments */
  char ipad[256];
};

/* -------------------- Task ending structure --------------------------- 
  Remainder of structure (up to the comment buffer section) must consist of
  no more than TASK_CONF_END_SZ_P bytes. 
*/
/* Size of struct's end area: */
#define TASK_CONF_END_SZ_P  320 

struct task_end_info
{
  /*
  The task number consists of a "major" and "minor" number.  The major number
  is what we usually refer to the task by.  The minor number is used for 
  "hybrid" tasks i.e., when more than one task configuration is used to 
  define an overall task.  If the minor number is zero, then there are no
  additional tasks. Otherwise the minor numbers represent an ordering of a
  set of task configurations, all of which have the same major number. The
  minor numbers always run from 1...N, where N is the total number of tasks
  involved in this hybrid.
  */

  SINT2 id_major, id_minor;

  /*
  There are two character strings associated with a task.  One is the task
  name, i.e.,  file name of the .TCO file.  The other is an optional brief
  description of what the task does.  Both strings are end-padded with spaces.
  */

  char sname[12], sdscript[80];

  /*
  STATE is the current state of the task.  It can be modified by the EXEC
  or INGEST processes.
  */

  SINT2 istate;
#define TASK_VOID_P       0        /* No task defined here (empty slot) */
#define TASK_MODIFY_P     1        /* Being modified by someone */
#define TASK_INACTIVE_P   2        /* Exists, but not used in any way */
#define TASK_SCHED_P      3        /* Waiting to run */
#define TASK_RUNNING_P    4        /* Running */
  char ipad[222];

};
        

/* -------------------- Task Configuration Structure -------------------- */

#define TASK_COMNT_SZ_P     720          /* Size of Comment buffer */

struct  task_configuration 
{
  struct structure_header    hdr; 
  struct task_sched_info     sch;
  struct task_dsp_info       dsp;
  struct task_calib_info     cal;
  struct task_range_info     rng;
  struct task_scan_info      scan;
  struct task_misc_info      misc;
  struct task_end_info       end;

  /*The comment buffer is the last part of the Task Configuration Stucture.*/
  UINT1  comnts[ TASK_COMNT_SZ_P ];
};


/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */
/* c------------------------------------------------------------------------ */
/*
 * -------------------- Summary File Concepts --------------------
 *
 * An INGEST summary file is written for each volume scan, and is updated each
 * time a sweep is finished.  This allows product generators to get data as each
 * sweep completes, without having to wait for the full volume scan.
 *
 * The file contains the ingest_summary structure, which consists of:
 *
 *     * INGEST Header Information
 *     * Task Configuration Structure
 *     * Device Status Table
 *     * GPARM from DSP#1
 *     * GPARM from DSP#2
 *
 * Note that the INGEST summary file is also the Volume Header Record for INGEST
 * data that are archived onto tape.
 *
 * INGEST summary file names have the form  YYMMDDSSSSS. where
 *     YYMMDD  are year, month, and day in decimal.  The year is the last two
 *             decimal digits of a value between 1950 and 2049.
 *     SSSSS   is # seconds into the day in decimal.
 *
 * The data file names associated with a summary file are YYMMDDSSSSS.WWP where
 *     WW    is the sweep number in decimal.
 *     P     is the parameter number recorded in the file, where the numbers
 *           correspond to bits in Task_DSP_Info.IDATA.  P takes on values
 *           0,1,2...9,A,B,...U,V.
 *
 * On machines that limit file name length to 14 characters, the dot can be
 * eliminated from the file name.
 */


#define INGEST_SUMRY_HSZ_P  (480)          /* Size of Header Information */


struct ingest_header
{

  char sfile[80];                       /* Name of File on disk */

  /*
   * IDFNUM indicates the current number of data files that are associated with 
   * this summary file.  As sweeps are performed and more data files are added
   * to the INGEST directory, IDFNUM is bumped accordingly.  Likewise, IDFSIZE
   * tallies up the total size in bytes of those data files.
   */
  SINT2 idfnum;                      /* # of associated data files extant */
  SINT4 idfsize;                     /* Total size of those files */

  /*
   * Date and time that the Volume Scan was started.  This is not necessarilly
   * equal to the starting time of the first sweep, but is always .LE. to it.
   */
  struct ymds_time time;

  /*
   * The following character string will contain a tape drive name to indicate
   * which drive was used for writing.  This string is filled in only when the
   * INGEST summary is actually written out to a tape.  Note that the INGEST 
   * summary is exactly the same as the tape volume header.
   */
  char stape[16];

  /* Sizes of various things */

  SINT2 ib_rayhed;                   /* # bytes in ray headers */
  SINT2 ib_xhdr;                     /* # bytes in Extended Header */
  SINT2 ib_task;                     /* # bytes in task config table */
  SINT2 ib_devst;                    /* # bytes in device status table */
  SINT2 ib_gparm;                    /* # bytes in each GPARM */
  SINT2 spare_b[14];

  /*
   * Information about the radar site.  Latitude and Longitude are stored as 
   * 32-bit binary angles, where 2000000 Hex is 45 degrees North Latitude or 
   * East Longitude.  All heights are in meters.  Some of these values will
   * vary according in multi-antenna installations.
   */
  char sitename[16];
  SINT2 igmt_min;                    /* # minutes ahead of GMT */
  SINT4 ilat, ilon;                  /* Latitude and Longitude */
  SINT2 ignd_hgt;                    /* Ground height (m above sea level) */
  SINT2 irad_hgt;                   /* Radar height (m above ground) */

  /*
   * Information about the organization of the ray pointers in the data files
   * for this summary file.  The ray pointers at the beginning of each INGEST
   * data file point to IRTOTL rays whose nominal angles are taken from the set
   * of INREV equally spaced angles around the circle starting from zero degrees.
   * ISNDX is an index in the range [0 - INREV-1] that specifies the nominal angle
   * of the first ray.  For some types of scans the pointer table is not sorted
   * by angle, but rather by order of arrival of the rays.  In these cases INREV
   * and ISNDX will be zero.
   */
  SINT2 inrev;                       /* Number of rays in one revolution. */
  SINT2 isndx;                       /* Implies angle of first pointer */
  SINT2 irtotl;                      /* Total number of rays */

  SINT4 ialtitude;                   /* Altitude of radar in cm */
  SINT4 inu_vel[3];
/* The offset between the INU and the radar pedistal on moving platform system: */
  SINT4 inu_offset[3];

  char ipad[266];
};

/*
 * -------------------- INGEST Summary --------------------
 *
 * This structure is provided mainly as a means of collecting together the 
 * various structures and buffers that comprise it.  In general, the "working 
 * versions" of the summary's components will be elsewhere, but are 
 * copied here for I/O purposes.
 *
 */

/*
 * -------------------- Device Status Structure --------------------
 *
 * Status of the various devices is recorded here.
 *
 * Structure for each individual test, i.e. each unit within any catagory of
 * Devices.
 *
 */

struct one_device
{
  UINT2 istat;
#define DEV_NULL_P   (0)         /*Not applicable*/
#define DEV_OK_P     (1)         /*OK*/
#define DEV_ERROR_P  (2)         /*Error has occured*/
#define DEV_SHUT_P   (3)         /*Device shutdown*/
#define DEV_TEST_P   (4)         /*Device failed self test*/
#define DEV_REMOTE_P (5)         /*Remote device unavailable*/
#define DEV_IRIS_P   (6)         /*Remote IRIS unavailable*/
         
/* This number indicates which process is using the device. */
  UINT2 iuser;
#define PROC_NONE_P    (0)       /*Noone is using it*/
#define PROC_RTDISP_P  (1)       /*Real time display*/
#define PROC_INGEST_P  (2)       /*Ingest*/
#define PROC_INGFIO_P  (3)       /*Ingest file output*/
#define PROC_EXEC_P    (4)       /*Exec*/
#define PROC_OUTFMT_P  (5)       /*Output Formatter*/
#define PROC_PRODUCT_P (6)       /*Product Generator*/
#define PROC_WATCH_P   (7)       /*Watchdog process*/
#define PROC_QUICK_P   (8)       /*Quick look menu(part of out)*/
#define PROC_TAPE_P    (9)       /*Tape process*/
#define PROC_WINDOW_P  (10)      /*Window process*/

/* Node name or user name */
  char sname[15];

/* Number of characters in the name */
  SINT1 iname_length;
  char ipad[10];
};
/*
 * Structure for full summary of results
 */

#define DEV_STAT_SZ_P (STRUCT_HEADER_SZ_P+720)

struct device_status
{
  struct structure_header hdr;
  struct one_device dsp[4];
  struct one_device antenna[4];
  struct one_device output[12];
  char ipad[120];
};


struct  ingest_summary
{
  struct structure_header hdr;
  struct ingest_header igh;

  struct task_configuration tcf;
  struct device_status dst; 

  /* Not sure which GPARM structure will be here, so just allocate space. */

#define GPARM_SZ_P (524)
  SINT2 gparm_a[ GPARM_SZ_P / 2 ];
  SINT2 gparm_b[ GPARM_SZ_P / 2 ];
  
};

/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */
/* c------------------------------------------------------------------------ */

#endif

