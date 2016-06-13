/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <string.h>
#include <ddout_main.hh>

static char *Site_name;
static char *Project_name;
# define DAY_ZERO 0
# define ETERNITY 0x7fffffff
/* c------------------------------------------------------------------------ */

int 
main (void) {

    int i;
    int32_t start_time=DAY_ZERO, stop_time=ETERNITY;
    char *radar="TOGA", *dir="/steam/dt/oye";
    char *uf_file_name="toga_uf.tape";
    float range1=0., range2=250.;
    char *a;
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
	stop_time = start_time +atoi(a);
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
    if(a=getenv("SITE_NAME")) {
	Site_name = a;
    }
    if(a=getenv("PROJECT_NAME")) {
	Project_name = a;
    }
    if(a=getenv("OUTPUT_FILE")) {
	uf_file_name = a;
    }
    if(a=getenv("START_RANGE")) {
	range1 = (float)atof(a);
    }
    if(a=getenv("STOP_RANGE")) {
	range2 = (float)atof(a);
    }
    if(a=getenv("DD_DIR")) {
	dir = a;
    }

    dd_uf_conv( start_time, stop_time, radar, uf_file_name, dir
	       , range1, range2 );
}

/* c------------------------------------------------------------------------ */
