/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NTRP
# define THE_REST

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dd_math.h>
#include <dd_defines.h>
#include "dorade_share.hh"
#include "ddb_common.hh"
#include "stdhrd.hh"

# define HOUR                             0
# define MINUTE                           1
# define SECOND                           2
# define LATITUDE_D                       3
# define LATITUDE_M                       4
# define LATITUDE_S                       5
# define LONGITUDE_D                      6
# define LONGITUDE_M                      7
# define LONGITUDE_S                      8
# define STATIC_PRESSURE                  9
# define AMBIENT_TEMPERATURE             10
# define DOWNWARD_RADIOMETER             11
# define SIDE_RADIOMETER                 12
# define GROUND_SPEED                    13
# define TRUE_AIRSPEED                   14
# define TRACK_ANGLE                     15
# define HEADING_ANGLE                   16
# define PITCH_ANGLE                     17
# define ROLL_ANGLE                      18
# define JW_CLOUD_LIQUID_WATER           19
# define DEW_POINT_TEMPERATURE           20
# define UPWARD_RADIOMETER               21
# define GEOPOTENTIAL_ALTITUDE           22
# define PRESSURE_ALTITUDE               23
# define WIND_SPEED                      24
# define WIND_DIRECTION                  25
# define INE_1_VERTICAL_ACCLEROMETER     26
# define INE_2_VERTICAL_ACCELEROMETER    27
# define VERTICAL_GROUND_SPEED           28
# define VERTICAL_AIR_SPEED              29
# define VERTICAL_WIND                   30

# define NUM_ASCII_VALUES                31


# define FsCo(x) ((x)-1)	/* fortran subscript to C offset */
# define TYPE5_COUNT 106
# define TYPE4_COUNT 222
# define NEW_TYPE4_COUNT 20
# define TYPE2_COUNT 202
# define MAX_STD_REC_SIZE 5000
# define RCP60 .01666666666

struct generic_std {
    short type;
    short num_short;
};

struct type1_stuff {
    short type;
    short num_short;
    short ac_num;
    short year;
    short month;
    short day;
};

struct type4_stuff {
    struct type4_stuff *last;
    struct type4_stuff *next;

    float GPS_Latitude;		 /* ( 1) */
    float GPS_Longitude;	 /* ( 2) */ 
    float INE1_Latitude;	 /* ( 3) */ 
    float INE1_Longitude;	 /* ( 4) */  
    float INE2_Latitude;	 /* ( 5) */ 
    float INE2_Longitude;	 /* ( 6) */  
    float Rosemount_1;		 /* ( 7) */ 
    float Rosemount_2;		 /* ( 8) */  
    float Dew_Point_1;		 /* ( 9) */ 
    float Dew_Point_2;		 /* (10) */ 
    float Radome_Temp_1;	 /* (11) */ 
    float Radome_Temp_2;	 /* (12) */  
    float Radome_Temp_3;	 /* (13) */  
};

struct hrd_std {
    FILE *stream;
    int in_fid;
    int io_type;
    int hrd_ascii_tape;
    int year;
    int month;
    int day;
    int ac_num;
    char project[20];
    char *buff;
    struct generic_std *gs;

    struct type1_stuff *t1s;
    double *type2_vals;
    float *type5_unscale_values;
    struct type4_stuff *t4s;
    struct type5_stuff *t5s;
};

/* ASCII tape parameters
 */
static int hour                        ;
static int minute                      ;
static int second                      ;
static int latitude_d                  ;
static int latitude_m                  ;
static int latitude_s                  ;
static int longitude_d                 ;
static int longitude_m                 ;
static int longitude_s                 ;
static int static_pressure             ;
static int ambient_temperature         ;
static int downward_radiometer         ;
static int side_radiometer             ;
static int ground_speed                ;
static int true_airspeed               ;
static int track_angle                 ;
static int heading_angle               ;
static int pitch_angle                 ;
static int roll_angle                  ;
static int jw_cloud_liquid_water       ;
static int dew_point_temperature       ;
static int upward_radiometer           ;
static int geopotential_altitude       ;
static int pressure_altitude           ;
static int wind_speed                  ;
static int wind_direction              ;
static int ine_1_vertical_acclerometer ;
static int ine_2_vertical_accelerometer;
static int vertical_ground_speed       ;
static int vertical_air_speed          ;
static int vertical_wind               ;

static int cwidth[NUM_ASCII_VALUES];

static struct hrd_std *hs;
static char *next_tape=NULL;
static DD_TIME dts;
static char *deep6=0;

/* c------------------------------------------------------------------------ */

int 
hrd_ascii_read (void)
{
    int ii, jj;
    float value_of[NUM_ASCII_VALUES];
    struct type5_stuff *t5s=hs->t5s->next;
    char tmp_val[16], *aa, line[256];
# ifdef obsolete
    static int col[NUM_ASCII_VALUES] =
	  { 2,2,2,4,4,7,5,4,7,6
	   ,5,5,5,8,5,7
	   ,5,5,5,5,5,5,5,5
	   ,7,7,5,5,5,5,5};
# endif

    /* get the next line of ascii input
     */
    if(!(aa = fgets(line, sizeof(line), hs->stream))) {
	return(0);
    }

    for(ii=jj=0; ii < NUM_ASCII_VALUES; ii++) {
	/* grab 31 items of varying column widths
	 */
	strncpy(tmp_val, line+jj, cwidth[ii]);
	tmp_val[cwidth[ii]] = '\0';
	sscanf(tmp_val, "%f", value_of +ii);
	jj += cwidth[ii];
    }

    t5s->hour = value_of[hour];
    t5s->minute = value_of[minute];
    t5s->second = value_of[second];
    t5s->latitude_deg = value_of[latitude_d];
    t5s->latitude_min = value_of[latitude_m] + RCP60 * value_of[latitude_s];
    t5s->longitude_deg = value_of[longitude_d];
    t5s->longitude_min = value_of[longitude_m] + RCP60 * value_of[longitude_s];
    t5s->pressure = value_of[static_pressure];
    t5s->ambient_temp = value_of[ambient_temperature];
    t5s->down_radiometer = value_of[downward_radiometer];
    t5s->side_radiometer = value_of[side_radiometer];
    t5s->ground_speed = value_of[ground_speed];
    t5s->true_air_speed = value_of[true_airspeed];
    t5s->track = value_of[track_angle];
    t5s->heading = value_of[heading_angle];
    t5s->pitch = value_of[pitch_angle];
    t5s->roll = value_of[roll_angle];
    t5s->j_w_cloud_water = value_of[jw_cloud_liquid_water];
    t5s->ambient_dewpoint_temp = value_of[dew_point_temperature];
    t5s->upward_radiometer = value_of[upward_radiometer];
    t5s->geopotential_altitude = value_of[geopotential_altitude];
    t5s->pressure_altitude = value_of[pressure_altitude];
    t5s->wind_speed = value_of[wind_speed];
    t5s->wind_direction = value_of[wind_direction];
    t5s->ine1_vert_accel = value_of[ine_1_vertical_acclerometer];
    t5s->ine2_vert_accel = value_of[ine_2_vertical_accelerometer];
    t5s->dpj_vert_gnd_speed = value_of[vertical_ground_speed];
    t5s->dpj_vert_air_speed = value_of[vertical_air_speed];
    t5s->dpj_vert_wind_spd = value_of[vertical_wind];

    dts.day_seconds = D_SECS((int)t5s->hour, (int)t5s->minute
			     , (int)t5s->second, 0);
    t5s->time = d_time_stamp(&dts);

    return(NUM_ASCII_VALUES);
}
/* c------------------------------------------------------------------------ */

int 
hrd_merge_std (struct generic_radar_info *gri)
{
    static int count=0,  merge_it=YES;
    int ii, mark;
    char *a;
    /*
     * routine to manage the merging of P3 standard tape data with the
     * airborne radar data
     * 
     */
    if(!merge_it)
	  return(NO);

    if(!count++) {
	hrd_std_init();

	if(a=get_tagged_string("HRD_STD_TAPE")) {
	    next_tape = a;
	}
	else if(a=get_tagged_string("HRD_ASCII_TAPE")) {
	    hs->hrd_ascii_tape = YES;
	    next_tape = a;
	}
	else {
	    merge_it = NO;
	    return(NO);
	}
    }

    if((ii = hs->hrd_ascii_tape
	? hrd_sync_ascii(gri) : hrd_sync_std(gri)) != 1) {
	/* lost or never found sync
	 */
	merge_it = NO;
	d_unstamp_time(gri->dts);
	printf("HRD ac sync for %s lost or never found\n"
	       , dts_print(gri->dts));
    }
    return(YES);
}
/* c------------------------------------------------------------------------ */

void 
hrd_std_init (void)
{
    
    int ii, jj, mark;
    struct type4_stuff *t4s, *last_t4s;
    struct type5_stuff *t5s, *last_t5s;
    float *fptr;

    hs = (struct hrd_std *)malloc(sizeof(struct hrd_std));
    memset(hs, 0, sizeof(struct hrd_std));

    /* initialize a circular queue of 20 type 5 records
     * or two physical records worth of data
     */
    for(ii=0; ii < 20; ii++) {
	t5s = (struct type5_stuff *)malloc(sizeof(struct type5_stuff));
	memset(t5s, 0, sizeof(struct type5_stuff));
	if(!ii) {
	    hs->t5s = t5s;
	}
	else {
	    t5s->last = last_t5s;
	    last_t5s->next = t5s;
	}
	t5s->next = hs->t5s;
	hs->t5s->last = t5s;
	last_t5s = t5s;
    }
    /* set up for 10 logical records of type 4 data
     */
    for(ii=0; ii < 10; ii++){
	t4s = (struct type4_stuff *)malloc(sizeof(struct type4_stuff));
	memset(t4s, 0, sizeof(struct type4_stuff));
	if(!ii) {
	    hs->t4s = t4s;
	}
	else {
	    t4s->last = last_t4s;
	    last_t4s->next = t4s;
	}
	t4s->next = hs->t4s;
	hs->t4s->last = t4s;
	last_t4s = t4s;
    }
    /* set up for the unscale factors for type 5 data (type 3 stuff)
     */
    hs->type5_unscale_values = fptr = 
	  (float *)malloc(TYPE5_COUNT*sizeof(float));
    for(ii=0; ii < TYPE5_COUNT; ii++, *fptr++ = 1.0);

    /* set up for type 2 data
     */
    hs->type2_vals = (double *)malloc(TYPE2_COUNT*sizeof(double));
    memset(hs->type2_vals, 0, TYPE2_COUNT*sizeof(double));

    /* type 1 header
     */
    hs->t1s = (struct type1_stuff *)malloc(sizeof(struct type1_stuff));
    memset(hs->t1s, 0, sizeof(struct type1_stuff));

    /* the record buffer
     */
    hs->buff = (char *)malloc(MAX_STD_REC_SIZE);
    hs->gs = (struct generic_std *)hs->buff;
    
    /* initialize data position variables
     * for the ac ASCII data
     */
    ii = 0;
    hour                         = ii++;
    minute                       = ii++;
    second                       = ii++;
    latitude_d                   = ii++;
    latitude_m                   = ii++;
    latitude_s                   = ii++;
    longitude_d                  = ii++;
    longitude_m                  = ii++;
    longitude_s                  = ii++;
    static_pressure              = ii++;
    ambient_temperature          = ii++;
    downward_radiometer          = ii++;
    side_radiometer              = ii++;
    ground_speed                 = ii++;
    true_airspeed                = ii++;
    track_angle                  = ii++;
    heading_angle                = ii++;
    pitch_angle                  = ii++;
    roll_angle                   = ii++;
    jw_cloud_liquid_water        = ii++;
    dew_point_temperature        = ii++;
    upward_radiometer            = ii++;
    geopotential_altitude        = ii++;
    pressure_altitude            = ii++;
    wind_speed                   = ii++;
    wind_direction               = ii++;
    ine_1_vertical_acclerometer  = ii++;
    ine_2_vertical_accelerometer = ii++;
    vertical_ground_speed        = ii++;
    vertical_air_speed           = ii++;
    vertical_wind                = ii++;
    /*
     * initialize column widths for the ac ASCII data
     */
    ii = 0;
    
    cwidth[ii++] = 2;
    cwidth[ii++] = 2;
    cwidth[ii++] = 2;
    cwidth[ii++] = 4;
    cwidth[ii++] = 4;
    cwidth[ii++] = 7;
    cwidth[ii++] = 5;
    cwidth[ii++] = 4;
    cwidth[ii++] = 7;
    cwidth[ii++] = 6;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    cwidth[ii++] = 8;
    cwidth[ii++] = 5;
    cwidth[ii++] = 7;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    cwidth[ii++] = 7;
    cwidth[ii++] = 7;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    cwidth[ii++] = 5;
    
}
/* c------------------------------------------------------------------------ */

int 
hrd_sync_std (struct generic_radar_info *gri)
{
    /* c...mark */
    int ii, jj, nn, mark, gotta_type4=NO, gotta_type5=NO;
    static int count=0, counta=0, countb=0;  //Jul 26, 2011 add int type
    static int end_of_tape=YES;
    char *a, *b, *c, str[256];
    short *ss, *zz;
    struct type1_stuff *t1s;
    double d, *dptr;
    float f, *fptr;


    for(;;) {
	count++;
	/* keep looping until sync'd somewhere in a type 5 record
	 */
	for(ii=0;; ii++, hs->t5s=hs->t5s->next) {
	    counta++;
	    if(ii > 20) {
		mark = 0;
		*str = *deep6;
	    }
	    if(hs->t5s->next->time <= hs->t5s->time) {
		/* we are at the end of the current record
		 * read in the next set of data
		 */
		break;
	    }
# ifdef NTRP
	    if(hs->t5s->time <= gri->time && hs->t5s->next->time > gri->time) {
		if(! hs->t5s->time) {		
		    /* we are before the time on the tape */
		    return(1);
		}
		/* we have bracketed the target time!
		 */
		hrd_ac_update_std(gri);
		return(1);
	    }
# else
	    if(hs->t5s->time-.5 <= gri->time && hs->t5s->time+.5 > gri->time) {
		/* we have bracketed the target time!
		 */
		hrd_ac_update_std(gri);
		return(1);
	    }
# endif
	}
	gotta_type4 = gotta_type5 = NO;

	for(;;) {
	    countb++;
	    /* loop to read in the next set of ac data
	     */
	    if(end_of_tape) {
		end_of_tape = NO;
		/* try to open the next file */
		a = dd_whiteout(next_tape);
		if(!strlen(a)) {
		    printf("No more HRD std tapes/files!\n");
		    return(0);
		}
		next_tape = b = dd_delimit(a);
		for(c=str; a < b; *c++ = *a++); *c++ = '\0';
		hs->io_type = strstr(str, "/dev") ? PHYSICAL_TAPE : FB_IO;

		printf("Opening P3 standard tape: %s\n", str);
		if((hs->in_fid = open(str, 0)) < 0) {
		    perror("Unable to open file");
		    return(0);
		}
	    }
	    if((nn = hrdstd_read
		(hs->in_fid, hs->buff, (int)10000, hs->io_type)) <= 0) {
		end_of_tape = YES;
		continue;
	    }
/*
C unpack OAO standard tapes
 
c  The tape consists of several types of records:
c   Type 1: Header Record
c   Type 2: Calibration record, used to convert the "raw" voltages
c           (type 4 records) to "scientific units" (type 5)
c   Type 3: Data scaling record for science units (where to place the
c           decimal point)
c   Type 4: Original 1 s data (voltages from various sensors)
c   Type 5: Processed record, i.e., useful observations
c   Type 6: Trailer record

c  On output the real*4 array Buff contains the processed flight level
c  data.  Format of Buff follows the AOC type 5 records fairly closely
c  with a few exceptions of some raw (type 4) data.
 */

	    switch(hs->gs->type) {

	    case 1:
		t1s = hs->t1s;
		memcpy(hs->t1s, hs->buff, sizeof(struct type1_stuff));
		dts.year = t1s->year > 1900 ? t1s->year : t1s->year+1900;
		dts.month = t1s->month;
		dts.day = t1s->day;
		dts.day_seconds=0;
		d = d_time_stamp(&dts);
		printf("**** AC: %d  yy: %d  mm: %d  dd: %d ****\n"
		       , t1s->ac_num
		       , t1s->year
		       , t1s->month
		       , t1s->day
		       );
		break;

	    case 16384:
		dptr = hs->type2_vals;
		a = hs->buff;
		for(ii=0,jj=nn/4; ii < jj; ii++,a+=4) {
		    /* nab a bunch of HP1000 floating point numbers */
		    *dptr++ = cvt_hp1000fp(a);
# ifdef obsolete
		    printf(" %3d) %10.4f\n", ii, *(dptr-1));
# endif
		}
		break;

	    case 3:
		/* scale values for select type 5 parameters */
		fptr = hs->type5_unscale_values;
		ss = (short *)hs->buff;
		zz = ss +hs->gs->num_short;
		for(ss+=2; ss < zz;) {
		    ii = FsCo(*ss++);
		    if((jj = *ss++)) {
			*(fptr+ii) = 1./(float)jj;
		    }
		    else {
			*(fptr+ii) = 1.;
		    }
		}
		break;

	    case 4:
		/* unpack all 10 logical records of type 4 data
		 */
		type4_new();
		gotta_type4 = YES;
		break;

	    case 5:
		if(gotta_type4) { /* make sure we've read a type 4 before
				   * reading a type 5 record
				   */
		    type5_new(); /* unpack all 10 logical records
				  * of type 5 data
				  */
		    gotta_type5 = YES;
		}
		break;

	    case 6:
		end_of_tape = YES;
		break;
	    default:
		break;
	    }
	    if(gotta_type5)
		  break;
	}
	mark = 0;
    }
    return(0);			/* do we ever get here? and
				 * why was a return of 1 commented out?
				 */
}
/* c------------------------------------------------------------------------ */

int 
hrd_sync_ascii (struct generic_radar_info *gri)
{
    /* c...mark */
    int ii, jj, nn, mark, gotta_type4=NO, gotta_type5=NO;
    static int count=0, counta=0, countb=0;  //Jul 26, 2011 add int type
    static int end_of_tape=YES;
    char *a, *b, *c, str[256];
    char name[16], dummy[16];
    short *ss, *zz;
    struct type1_stuff *t1s;
    double d,*dptr;
    float f, *fptr;


    for(;;) {
	count++;
	
	for(;hs->t5s->time ;) {	/* this "for" statement should make it read
				 * two records before doing any checking
				 */
# ifdef NTRP
	    if(hs->t5s->time <= gri->time && hs->t5s->next->time > gri->time) {
		if(! hs->t5s->time) {		
		    /* we are before the time on the tape */
		    return(1);
		}
		/* we have bracketed the target time!
		 */
		hrd_ac_update_std(gri);
		return(1);
	    }
# else
	    if(hs->t5s->time-.5 <= gri->time && hs->t5s->time+.5 > gri->time) {
		/* we have bracketed the target time!
		 */
		hrd_ac_update_std(gri);
		return(1);
	    }
# endif
	    if(hs->t5s->time > gri->time)
		  return(1);

	    break;
	}

	hs->t5s = hs->t5s->next;

	for(;;) {
	    countb++;
	    /* loop to read in the next set of ac data
	     */
	    if(end_of_tape) {
		end_of_tape = NO;
		/* try to open the next file */
		a = dd_whiteout(next_tape);
		if(!strlen(a)) {
		    printf("No more HRD ASCII tapes/files!\n");
		    return(0);
		}
		next_tape = b = dd_delimit(a);
		for(c=str; a < b; *c++ = *a++); *c++ = '\0';

		printf("Opening P3 ascii tape: %s\n", str);
		if(!(hs->stream = fopen(str, "r")) < 0) {
		    perror("Unable to open file");
		    return(0);
		}
		/* read in the header record
		 */
		a = fgets(str, sizeof(str), hs->stream);
		sscanf(str, "%2d%2d%2d %d %s %s %s"
		      , &dts.year, &dts.month, &dts.day, &hs->ac_num
		      , name, dummy, hs->project);
		if(dts.year < 1900) dts.year += 1900;
		dts.day_seconds = 0;
		d_time_stamp(&dts);
		printf("**** AC: %d  %s ****\n"
		       , hs->ac_num, dts_print(d_unstamp_time(&dts)));
	    }

	    if((nn = hrd_ascii_read()) > 0) {
		break;
	    }
	    end_of_tape = YES;
	}
    }
    return(0);
}
/* c------------------------------------------------------------------------ */

void 
hrd_dmp_std (struct type5_stuff *t5s, struct generic_radar_info *gri)
{
    double d1, d2;
    char mess[256];
    static int count=0;

    if( count++ % 1000 )
	  return;
    dts.time_stamp = t5s->time;
    d_unstamp_time(&dts);
    d1 = t5s->latitude_deg +RCP60 * t5s->latitude_min;
    d2 = t5s->longitude_deg +RCP60 * t5s->longitude_min;

    if(count == 1) {
	printf("\n");
    }

    printf(
"%02d%02d%02d %9.4f %7.4f %7.4f %9.4f %7.4f %7.4f %6.3f %5.2f %5.2f\n"
	   , dts.hour
	   , dts.minute
	   , dts.second
	   , d1
	   , gri->latitude -d1
	   , t5s->gps_latitude_orig -d1
	   , d2
	   , gri->longitude -d2
	   , t5s->gps_longitude_orig -d2
	   , gri->altitude -t5s->pressure_altitude
	   , gri->ui -t5s->e_w_wind_speed
	   , gri->vi -t5s->n_s_wind_speed
	   );
    return;
}
/* c------------------------------------------------------------------------ */

void 
hrd_ac_update_std (struct generic_radar_info *gri)
{
    /* c...mark */
    static int count=0;
    int ii, mark, ok_movement;
    struct type5_stuff *t5s=hs->t5s;
    double d, d1, d2, frac, movement;
    float lat_err=0, lon_err=0;
    static struct running_avg_que *raq_lat, *raq_lon;
    static struct que_val *qv_lat, *qv_lon;
				/* c...mark */


    if(!count++) {		/* set up a running average queue
				 * for 10 items (secs) for the
				 * differences between type 5 and gps
				 */
	raq_lat = se_return_ravgs(10);
	raq_lon = se_return_ravgs(10);
	qv_lat = raq_lat->at;
	qv_lon = raq_lon->at;
    }

    frac = gri->time -(int32_t)gri->time;
    count++;

    d1 = t5s->latitude_deg +RCP60 * t5s->latitude_min;
    d2 = t5s->next->latitude_deg +RCP60 * t5s->next->latitude_min;

    if(!hs->hrd_ascii_tape) {	/*  */
	d = t5s->next->gps_longitude_orig -t5s->gps_longitude_orig;
	d *= COS(RADIANS(d1)) * 111000.; /* meters! */
	movement = SQ(d);
	d = 111000. * (t5s->next->gps_latitude_orig -t5s->gps_latitude_orig);
	movement += SQ(d);

	if(ok_movement = (movement > 2500. && movement < 40000)) {
	    /* equivalent to 50 m/s and 200 m/s squared
	     * either not enough movement or more than possible
	     */
	    d = t5s->gps_latitude_orig -d1;
	    raq_lat->sum -= qv_lat->f_val;
	    raq_lat->sum += d;
	    qv_lat->f_val = d;
	    qv_lat = qv_lat->next;
	}
	lat_err = raq_lat->sum * raq_lat->rcp_num_vals;
    }

    gri->latitude = d1 + lat_err
# ifdef NTRP
	  + frac * (d2-d1)
# endif
		;
    d1 = t5s->longitude_deg +RCP60*t5s->longitude_min;
    d2 = t5s->next->longitude_deg +RCP60 * t5s->next->longitude_min;

    if(!hs->hrd_ascii_tape && ok_movement) {
	d = t5s->gps_longitude_orig -d1;
	
	raq_lon->sum -= qv_lon->f_val;
	raq_lon->sum += d;
	qv_lon->f_val = d;
	qv_lon = qv_lon->next;
	lon_err = raq_lon->sum * raq_lon->rcp_num_vals;
    }

    gri->longitude = d1 + lon_err
# ifdef NTRP
	  + frac * (d2-d1)
# endif
		;
    gri->altitude = t5s->pressure_altitude
# ifdef NTRP
	  + frac * (t5s->next->pressure_altitude -t5s->pressure_altitude)
# endif
		;
    gri->altitude_agl = t5s->geopotential_altitude
# ifdef NTRP
	  + frac * (t5s->next->geopotential_altitude -t5s->geopotential_altitude)
# endif
		;

# ifdef THE_REST

    gri->heading = t5s->heading
# ifdef NTRP
	  + frac * (t5s->next->heading -t5s->heading)
# endif
		;
    gri->roll = t5s->roll
# ifdef NTRP
	  + frac * (t5s->next->roll -t5s->roll)
# endif
		;
    gri->pitch = t5s->pitch
# ifdef NTRP
	  + frac * (t5s->next->pitch -t5s->pitch)
# endif
		;
    gri->drift = t5s->drift
# ifdef NTRP
	  + frac * (t5s->next->drift -t5s->drift)
# endif
		;
    gri->vew = t5s->e_w_ground_speed
# ifdef NTRP
	  + frac * (t5s->next->e_w_ground_speed -t5s->e_w_ground_speed)
# endif
		;
    gri->vns = t5s->n_s_ground_speed
# ifdef NTRP
	  + frac * (t5s->next->n_s_ground_speed -t5s->n_s_ground_speed)
# endif
		;
    gri->vud = t5s->dpj_vert_gnd_speed
# ifdef NTRP
	  + frac * (t5s->next->dpj_vert_gnd_speed -t5s->dpj_vert_gnd_speed)
# endif
		;
    gri->ui = t5s->e_w_wind_speed
# ifdef NTRP
	  + frac * (t5s->next->e_w_wind_speed -t5s->e_w_wind_speed)
# endif
		;
    gri->vi = t5s->n_s_wind_speed
# ifdef NTRP
	  + frac * (t5s->next->n_s_wind_speed -t5s->n_s_wind_speed)
# endif
		;
    gri->wi = t5s->vertical_wind_spd
# ifdef NTRP
	  + frac * (t5s->next->vertical_wind_spd -t5s->vertical_wind_spd)
# endif
		;

# endif   /* for THE_REST */


    d = RADIANS(CART_ANGLE(t5s->wind_direction +180.));

    gri->ui = t5s->wind_speed * cos(d);
    gri->vi = t5s->wind_speed * sin(d);
    gri->wi = t5s->vertical_wind_spd;

    if(!hs->hrd_ascii_tape) {
       hrd_dmp_std(t5s, gri);
    }
    mark = 0;
}
/* c------------------------------------------------------------------------ */

void 
type4_new (void)
{
    int ii, jj, kk, mark;
    static int count=0;
    short *ss=(short *)hs->buff;
    struct type4_stuff *t4s=hs->t4s->next;
    float a3, a2, a1, a0, volts, volts2, volts3;
    double *dptr=hs->type2_vals, Big=2147483647.0, Convt;
    int Length;

    
    Convt = 4.0*180.0/Big;
    Length = (int) (*(dptr+1)/2); dptr += 2;

    for(ii=0; ii < 10; ii++,ss+=TYPE4_COUNT,t4s=t4s->next) {
	count++;

	memcpy(&kk, ss+FsCo(22), 4); /* orig gps lat */
	t4s->GPS_Latitude = kk *Convt;

	memcpy(&kk, ss+FsCo(24), 4); /* orig gps lon */
	t4s->GPS_Longitude = kk *Convt;

	memcpy(&kk, ss+FsCo(59), 4); /* orig ine1 lat */
	t4s->INE1_Latitude = kk *Convt;

	memcpy(&kk, ss+FsCo(61), 4); /* orig ine1 lon */
	t4s->INE1_Longitude = kk *Convt;

	memcpy(&kk, ss+FsCo(79), 4); /* orig ine2 lat */
	t4s->INE2_Latitude = kk *Convt;

	memcpy(&kk, ss+FsCo(81), 4); /* orig ine2 lon */
	t4s->INE2_Longitude = kk *Convt;

	dptr = hs->type2_vals+2;

	for(jj=2; jj < Length; jj+=5) {
	    kk = (int) *dptr++;
	    if(kk > 404) {
		mark = 0;
	    }
	    a3 = *dptr++;
	    a2 = *dptr++;
	    a1 = *dptr++;
	    a0 = *dptr++;
	    volts = (float)*(ss+FsCo(kk));
	    volts2 = volts * volts;
	    volts3 = volts2 * volts;

	    switch(kk) {

	    case 143:		/* Rosemount temperature #1 */
		t4s->Rosemount_1 = a3*volts3 + a2*volts2 + a1*volts + a0;
		break;

	    case 144:		/* Rosemount temperature #2 */
		t4s->Rosemount_2 = a3*volts3 + a2*volts2 + a1*volts + a0;
		break;

	    case 145:		/* Dew Point #1 */
		t4s->Dew_Point_1 = a3*volts3 + a2*volts2 + a1*volts + a0;
		break;

	    case 166:		/* Dew Point #2 */
		t4s->Dew_Point_2 = a3*volts3 + a2*volts2 + a1*volts + a0;
		break;

	    case 162:		/* Radome Temp #1 Starboard side */
		t4s->Radome_Temp_1 = a3*volts3 + a2*volts2 + a1*volts + a0;
		break;

	    case 163:		/* Radome Temp #2 Starboard side */
		t4s->Radome_Temp_2 = a3*volts3 + a2*volts2 + a1*volts + a0;
		break;

	    case 164:		/* Radome Temp #3 Starboard side */
		t4s->Radome_Temp_3 = a3*volts3 + a2*volts2 + a1*volts + a0;
		break;
	    }
		  
	}
	dptr = hs->type2_vals+2;

	for(jj=2; jj < Length; jj+=5,dptr+=5) {
	    kk = (int) *dptr;

	    switch(kk) {
		
	    case -143:		/* Rosemount temperature #1 */
		t4s->Rosemount_1 = t4s->Rosemount_1 * (*(dptr+FsCo(2)))
		      + (*(dptr+FsCo(4)));
		break;
		
	    case -144:		/* Rosemount temperature #2 */
		t4s->Rosemount_2 = t4s->Rosemount_1 * (*(dptr+FsCo(2)))
		      + (*(dptr+FsCo(4)));
		break;
		
	    case -145:		/* Dew Point #1 */
		t4s->Dew_Point_1 += *(dptr+FsCo(4));
		break;
		
	    case -146:		/* Dew Point #2 */
		t4s->Dew_Point_2 += *(dptr+FsCo(4));
		break;
	    }
	}
    }
    mark = 0;
}
/* c------------------------------------------------------------------------ */

void 
type5_new (void)
{
    int ii, jj, nn, mark;
    static int count=0;
    short *ss=(short *)hs->buff;
    struct type4_stuff *t4s=hs->t4s;
    struct type5_stuff *t5s=hs->t5s->next;
    float *fptr, *fus;
    double d, T, Te, Ee, AlogEe;


    for(nn= *(ss+1),ii=0; ii < 10; ii++,t4s=t4s->next,t5s=t5s->next) {
	count++;
	ss = (short *)hs->buff +ii*TYPE5_COUNT;	/* point to current lrec */
	fus = hs->type5_unscale_values;	/* point to scale factors */
	fptr = &t5s->type_5;	/* point to fp array */

	dts.day_seconds = D_SECS(*(ss+2), *(ss+3), *(ss+4), 0);
	t5s->time = d_time_stamp(&dts);

	for(jj=0; jj < nn; jj++) /* unscale all values first */
	      *fptr++ = (*fus++) * (float)(*ss++);
	    
	Te = pow((double)t5s->pressure/(t5s->pressure +t5s->dynamic_pressure)
		 , (double)(t5s->ratio_specific_heats-1.)
		 /t5s->ratio_specific_heats);
	T = t4s->Rosemount_1 +273.16;

	t5s->temperature_1_orig = T * Te -273.16; /* ( 83) */     

	T = t4s->Rosemount_2 +273.16;
	t5s->temperature_2_orig = T * Te -273.16; /* ( 84) */     

	t5s->dew_point_1_orig = t4s->Dew_Point_1;	/* ( 85) */     
	if(t4s->Dew_Point_1 < 0.) {
	    Ee = 6.1078*exp((double)(22.4716*t4s->Dew_Point_1)
			    /(272.722+t4s->Dew_Point_1));
	    AlogEe = log(Ee);
	    t5s->dew_point_1_orig = 243.17 *
		  ((AlogEe-1.8096)/(19.4594-AlogEe));
	}
	t5s->dew_point_2_orig = t4s->Dew_Point_2;	/* ( 85) */     
	if(t4s->Dew_Point_2 < 0.) {
	    Ee = 6.1078*exp((double)(22.4716*t4s->Dew_Point_2)
			    /(272.722+t4s->Dew_Point_2));
	    AlogEe = log(Ee);
	    t5s->dew_point_2_orig = 243.17 *
		  ((AlogEe-1.8096)/(19.4594-AlogEe));
	}
	t5s->gps_latitude_orig = t4s->GPS_Latitude; /* ( 87) */     
	t5s->gps_longitude_orig = t4s->GPS_Longitude; /* ( 88) */     
	t5s->ine1_latitude_orig = t4s->INE1_Latitude; /* ( 89) */     
	t5s->ine1_longitude_orig = t4s->INE1_Longitude; /* ( 90) */     
	t5s->ine2_latitude_orig = t4s->INE2_Latitude; /* ( 91) */     
	t5s->ine2_longitude_orig = t4s->INE2_Longitude; /* ( 92) */     
	
	T = t4s->Radome_Temp_1 +273.16;
	t5s->radome_temp_1_orig = T * Te -273.16; /* ( 93) */     

	T = t4s->Radome_Temp_2 +273.16;
	t5s->radome_temp_2_orig = T * Te -273.16; /* ( 94) */     

	T = t4s->Radome_Temp_3 +273.16;
	t5s->radome_temp_3_orig = T * Te -273.16; /* ( 95) */     
	
    }
    mark = 0;
}
/* c------------------------------------------------------------------------ */

double 
cvt_hp1000fp (char *at)
{
# define  MASK23 0x7fffff
# define   MASK7 0x7f
    /*
     * convert HP1000 floating point to local floating point
     */
    int32_t ii, ival;
    double d, xp_val, exponent, fraction;
    double two_to_the_minus_23rd = 1.192092896e-7; /* 2**-23) */

    memcpy(&ival, at, 4);

    if(!ival)
	  return((double)0);
    /*
     * eight bits of exponent in lowest bits
     * sign of the exponent is in the right most (or low order) bit
     * the sign of the value is in the leftmost or highest order bit
     * followed by the 23-bit mantissa
     */
    exponent = ival & 1 ? ((ival >> 1) & MASK7) -128 : (ival >> 1) & MASK7;
    xp_val = pow((double)2.0, exponent);

    if((ival >> 31) & 1) {	/* negative value! */
	if((ival >> 8) & MASK23) {
	    /* non-zero mantissa...better make sure we have sign extension
	     */
	    fraction = ((int32_t)0xff000000 | (ival >> 8)) *
		  two_to_the_minus_23rd;
	}
	else {
	    /* a zero mantissa here implies some negative power of 2
	     */
	    fraction = -1.;
	}
    }
    else
	  fraction = (ival >> 8) * two_to_the_minus_23rd;

    return(fraction * xp_val);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

int 
hrdstd_read (int fid, char *buf, int size, int io_type)
{
    static int count=0, t_count=0, std_fid=-1;
    int ii, nn, mark;
    char *a;

    count++;

    if(t_count < 3) {
	if(t_count == 0) {	/* the idea here is to set up to read
				 * the first 3 records (types 1,2,3)
				 * from a file so you can start somewhere
				 * other than the beginning of the tape
				 */
	    if(a=getenv("HRDSTD_VOLUME_HEADER")) {
		std_fid = open(a, 0);
	    }
	    else
		  t_count = 3;
	}
	if(t_count < 3) {
	    t_count++;
	    nn = fb_read(std_fid, buf, 10000);
	    return(nn);
	}
    }
    nn = gp_read(fid, buf, size, io_type);
    return(nn);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

# ifdef obsolete
    d = cvt_hp1000fp(&ival);

    ival = 0x80000002;
    d = cvt_hp1000fp(&ival);

    ival = 0x40000002;
    d = cvt_hp1000fp(&ival);

    ival = 0x80000000;
    d = cvt_hp1000fp(&ival);

    ival = 0x400000ff;
    d = cvt_hp1000fp(&ival);

    ival = 0x800000FD;
    d = cvt_hp1000fp(&ival);

    ival = 0x70000006;
    d = cvt_hp1000fp(&ival);

    ival = 0x7FFFFFFE;
    d = cvt_hp1000fp(&ival);

    ival = 0x800000FE;
    d = cvt_hp1000fp(&ival);

    ival = 0x40000007;
    d = cvt_hp1000fp(&ival);

    ival = 0x80000005;
    d = cvt_hp1000fp(&ival);

    ival = 0x40000005;
    d = cvt_hp1000fp(&ival);

    ival = 0xc0000005;
    d = cvt_hp1000fp(&ival);
# endif


# ifdef obsolete
/* c------------------------------------------------------------------------ */

int 
main (void)
{
    struct generic_radar_info *gri;

    int32_t ival=0x40000004;
    double d;
    int ii, jj, kk, nn;
    int yy=93, mon=2, dd=9, hh=13, mm=15, ss=1;
    char *a;
    

    if(!(a=getenv("HRD_STD_TAPE"))) {
	  a = "/scr/amber/oye/p3_standard.tape";
    }
    put_tagged_string("HRD_STD_TAPE", a);
    gri = return_gri_ptr();
    if((a=getenv("HRD_TIME"))) {
	sscanf(a, "%d %d %d %d %d %d"
	       , &yy, &mon, &dd, &hh, &mm, &ss);
    }
    else {
    }

    gri->dts->year = yy < 1900 ? yy+1900 : yy;
    gri->dts->month = mon;
    gri->dts->day = dd;
    gri->dts->day_seconds = D_SECS(hh, mm, ss, 0);
    gri->time = d_time_stamp(gri->dts);
    for(ii=0; ii < 6000; ii++) {
	hrd_merge_std(gri);
	gri->time += 1.;
    }

    /* c...mark */
}
# endif

