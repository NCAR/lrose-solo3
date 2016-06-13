#include <time.h>
#include <sys/time.h>
#include <dd_defines.h>
#include <gnd_main.hh>
#include <string.h>

/* c------------------------------------------------------------------------ */

int 
main (void) {

    int i;
    int run_time=0, sweep_count=0, beam_count=0, min_gates=11;
    int32_t start_time=DAY_ZERO, stop_time=ETERNITY;
    char *radar="ELDR_TF", *dir="/steam/dt/oye";
    char *vel_name="VR", *refl_name="DBZ";

    float earth_radius=6366810.;
    float min_rot_angle=100.;
    float max_rot_angle=260.;

    char *a, *getenv();
    double atof();
    short yy, mon, dd, hh, mm, ss, ms;
    struct tm tm;
#ifdef SYSV
    char tz[20];
#endif
    
    if(a=getenv("START_TIME")) {
	/*
	 * assume times are
	 * of the form mm/dd/yy:hh:mm:ss.ms
	 */
	dcdatime( a, strlen(a), &yy, &mon, &dd, &hh,&mm, &ss, &ms );
	tm.tm_wday = 0;
	tm.tm_yday = 0;
	tm.tm_sec = ss;
	tm.tm_min = mm;
	tm.tm_hour = hh;
	tm.tm_mday = dd;
	tm.tm_mon = mon-1;
	tm.tm_year = yy;
#ifdef SYSV
        strcpy (tz, "TZ=GMT");
        putenv (tz);
#ifdef SVR4
        altzone = 0;
#endif
        timezone = 0;
        daylight = 0;
        tm.tm_isdst = -1;
        start_time = (int32_t) mktime (&tm);
#else
        tm.tm_zone = "GMT";
        tm.tm_gmtoff = 0;
        tm.tm_isdst = 0;
        start_time = timegm (&tm);
#endif
    }
    if(a=getenv("RUN_TIME")) {
	run_time = atoi(a);
    }
    if(a=getenv("STOP_TIME")) {
	dcdatime( a, strlen(a), &yy, &mon, &dd, &hh,&mm, &ss, &ms );
	tm.tm_wday = 0;
	tm.tm_yday = 0;
	tm.tm_sec = ss;
	tm.tm_min = mm;
	tm.tm_hour = hh;
	tm.tm_mday = dd;
	tm.tm_mon = mon-1;
	tm.tm_year = yy;
#ifdef SYSV
        strcpy (tz, "TZ=GMT");
        putenv (tz);
#ifdef SVR4
        altzone = 0;
#endif
        timezone = 0;
        daylight = 0;
        tm.tm_isdst = -1;
        stop_time = (int32_t) mktime (&tm);
#else
        tm.tm_zone = "GMT";
        tm.tm_gmtoff = 0;
        tm.tm_isdst = 0;
        stop_time = timegm (&tm);
#endif
    }
    if(a=getenv("RADAR_NAME")) {
	radar = a;
    }
    if(a=getenv("DD_DIR")) {
	dir = a;
    }
    if(a=getenv("VELOCITY_NAME")) {
	vel_name = a;
    }
    if(a=getenv("DBZ_NAME")) {
	refl_name = a;
    }
    if(a=getenv("SWEEP_COUNT")) {
	sweep_count = atoi(a);
    }
    if(a=getenv("BEAM_COUNT")) {
	beam_count = atoi(a);
    }
    if(a=getenv("MIN_GATES")) {
	min_gates = atoi(a);
    }

    loop_ground_echo( start_time, stop_time, run_time
		     , radar, dir
		     , min_rot_angle, max_rot_angle, earth_radius
		     , sweep_count, beam_count, min_gates
		     , vel_name, refl_name );
}

/* c------------------------------------------------------------------------ */
