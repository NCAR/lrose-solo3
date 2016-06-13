
#include <dd_defines.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>


int main (void)
{
    int i, j, n, volume_interval=600, output_fid=-1, io_type=PHYSICAL_TAPE;
    time_t start_time=DAY_ZERO, stop_time=ETERNITY;
    char *dir="/steam/dt/oye";
    char *output_file="/steam/dt/oye/dorade.tape";
    char *radar_names="ELDR_TA ELDR_TF";
    char *a;
    char *output_flags="DATA";
    char *interleave="SWEEPS";
    short yy, mon, dd, hh, mm, ss, ms;
    struct tm tm;
#ifdef SYSV
    char tz[20];
#endif


    if(a=getenv("START_TIME")) {
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

    if(a=getenv("DD_DIR")) {
	dir = a;
    }
    if(a=getenv("VOLUME_INTERVAL")) {
	volume_interval = atoi(a); /* seconds */
    }
    if(a=getenv("INTERLEAVE")) {
	interleave = a;
    }
    if(a=getenv("OUTPUT_FLAGS")) {
	output_flags = a;
    }
    if(a=getenv("OUTPUT_FILE")) {
	output_file = a;
    }
    if(a=getenv("IO_TYPE")) {
	if(strstr(a, "FB_IO"))
	      io_type = FB_IO;
    }
# ifdef obsolete
    printf( "Creating DORADE file %s\n", output_file );
    if(!strstr(output_flags,"NO_DATA")) {
	if((output_fid = creat( output_file, PMODE )) < 0 ) {
	    printf( "Unable to open %s %d\n", output_file, output_fid );
	    return(1);
	}
    }
# endif
    ddout_loop( start_time, stop_time, radar_names, volume_interval
	       , interleave, output_flags, dir, output_file
	       , output_fid, io_type);
}
/* c------------------------------------------------------------------------ */
