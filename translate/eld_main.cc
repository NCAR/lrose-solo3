
#include <dd_defines.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <eld_main.hh>

main (void)
{
    int beam_count=0, run_time=0, sweep_count=0, sweep_skip=1;
    int input_file_id, io_type=PHYSICAL_TAPE;
    time_t start_time=DAY_ZERO, stop_time=ETERNITY;
    char *dir="/steam/dt/oye";
    char *srs_file="/steam/dt/oye/eldora.tape";
    char *select_radar=NULL;
    char *a;
    char *output_flags="DATA & CATALOG";
    short yy, mon, dd, hh, mm, ss, ms;
    struct tm tm;
    


    if(a=getenv("START_TIME")) {
	dcdatime( a, strlen(a), &yy, &mon, &dd, &hh,&mm, &ss, &ms );
	tm.tm_wday = 0;
	tm.tm_yday = 0;
	tm.tm_isdst = 0;
	tm.tm_zone = "GMT";
	tm.tm_gmtoff = 0;
	tm.tm_sec = ss;
	tm.tm_min = mm;
	tm.tm_hour = hh;
	tm.tm_mday = dd;
	tm.tm_mon = mon-1;
	tm.tm_year = yy;
	start_time = timegm(&tm);
    }
    if(a=getenv("RUN_TIME")) {
	run_time = atoi(a);
    }
    if(a=getenv("STOP_TIME")) {
	dcdatime( a, strlen(a), &yy, &mon, &dd, &hh,&mm, &ss, &ms );
	tm.tm_wday = 0;
	tm.tm_yday = 0;
	tm.tm_isdst = 0;
	tm.tm_zone = "GMT";
	tm.tm_gmtoff = 0;
	tm.tm_sec = ss;
	tm.tm_min = mm;
	tm.tm_hour = hh;
	tm.tm_mday = dd;
	tm.tm_mon = mon-1;
	tm.tm_year = yy;
	stop_time = timegm(&tm);
    }
    if(a=getenv("BEAM_COUNT")) {
	beam_count = atoi(a);
    }
    if(a=getenv("SWEEP_COUNT")) {
	sweep_count = atoi(a);
    }
    if(a=getenv("SWEEP_SKIP")) {
	sweep_skip = atoi(a);
	if( sweep_skip < 1 )
	      sweep_skip = 1;
    }
    if(a=getenv("SOURCE_FILE")) {
	srs_file = a;
    }
    if(a=getenv("IO_TYPE")) {
	if(strstr(a, "FB_IO"))
	      io_type = FB_IO;
	else if(strstr(a,"BINARY_IO"))
	      io_type = BINARY_IO;
    }
    if(a=getenv("DD_DIR")) {
	dir = a;
    }
    if(a=getenv("SELECT_RADAR")) {
	select_radar = a;
    }
    if(a=getenv("OUTPUT_FLAGS")) {
	output_flags = a;
    }
# ifdef obsolete
    /* open the file */
    if((input_file_id = open( srs_file, 0 )) < 0) { 
	printf( "Could not open input file %s  error=%d\n"
	       , srs_file, input_file_id );
	exit(1);
    }
    printf( "Input file name: %s\n", srs_file );
# endif

    eld_dd_conv( start_time, stop_time, dir, srs_file
		   , output_flags, beam_count, sweep_count, run_time
		   , sweep_skip, input_file_id, select_radar, io_type);
}
/* c------------------------------------------------------------------------ */
