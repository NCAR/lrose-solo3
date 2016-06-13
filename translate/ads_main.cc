/* 	$Id: ads_main.c 2 2002-07-02 14:57:33Z oye $	 */

#ifndef lint
static char vcid[] = "$Id: ads_main.c 2 2002-07-02 14:57:33Z oye $";
#endif /* lint */
/* c----------------------------------------------------------------------- */

#include <dd_defines.h>
#include <ads_data.h>
#include <ads_data.hh>
#include <ads_main.hh>
#include <math.h>
#include <stdio.h>

static struct ads_data *adsd;
static int io_type;
static int gpro_fid;
static struct ads_raw_data_que *top_rdq;
static int ads_eof_count;

int 
main (void) {
    int i, j, k, n, fid, mark, status, count=0, trip=1234;
    int ads_max_eofs=2, doit=NO, loopcount;
    double d, time, theta, ew, ns;
    char buf[32768];
    char *a, *getenv();
    char *fn="/scr/steam/oye/rf27b.tape";
    float pct, pct_inc=1.;
    struct ads_raw_data_que *rdq;
    struct ads_data ads_data;
    adsd = &ads_data;

    gpro_header(buf);

    if(a=getenv("ADS_FILE_COUNT")) {
	if((i=atoi(a)) > 0)
	      ads_max_eofs = i;
    }
    printf("ADS file count = %d\n", ads_max_eofs);

    io_type = PHYSICAL_TAPE;
    if(a=getenv("ADS_IO_TYPE")) {
	if((i=atoi(a)) >= 0)
	      io_type = i;
    }
    printf("ADS io_type = %d\n", io_type);

    fn = "/scr/bock/oye/rf21.tape";
    if(a=getenv("ADS_TAPES")) {
	fn = a;
    }
    gpro_fid = open( fn, 0);
    if(gpro_fid < 1) {
	printf("Unable to open %s\n", fn);
    }
    printf("Opened ADS tape: %s\n\n", fn);
    /* c...mark */
    for(rdq=top_rdq;;rdq=rdq->next) {
	if(!(count++ % 100)) {
	    mark = 0;
	}
	if(count >= trip) {
	    mark = 0;
	}
	if(!(rdq->raw_data_buf = gpro_next_rec(rdq, &status))) {
	    printf("Last read stat: %d\n", status);
	    break;
	}
	if(ads_eof_count >= ads_max_eofs) {
	    printf( "EOF count break at %d\n", ads_eof_count);
	    break;
	}
	/*
	  pct+=0.016667;
	 */
# ifndef obsolete	
	if(adsd->itime > 213800)
	      break;
	doit = adsd->itime > 0 && adsd->itime < 245959;
	doit = adsd->itime > 5244 && adsd->itime < 5350;
	doit = fabs((double)adsd->roll) > 5.;
	doit = adsd->itime >= 212800;
	pct_inc = 0.016667;
	pct_inc = 1.;
# else
	doit = YES;
# endif

	if(doit)
	      for(loopcount=0,pct=0; pct < 1.0; pct+=pct_inc,loopcount++) {
		  gpro_assemble_vals(rdq, adsd, pct);

# ifdef obsolete
		  printf(
			 " %d %7.3f %7.3f %6.3f %.3f %.3f %.3f %.3f %.3f %.3f \
%7.3f %7.3f %.3f %.3f %4.0f\n"
			 , adsd->itime
			 , adsd->pitch
			 , adsd->roll
			 , adsd->thi
			 , adsd->alat
			 , adsd->alat2
			 , adsd->glat
			 , adsd->alon
			 , adsd->alon2
			 , adsd->glon
			 , adsd->vew
			 , adsd->gvew
			 , adsd->vns
			 , adsd->gvns
			 , adsd->galt
			 );
# else
		  theta = RADIANS(270.-adsd->wdir);
		  ew = adsd->wspd*cos(theta);
		  ns = adsd->wspd*sin(theta);
		  printf(
"ads %06d %.2f %5.1f %4.1f %6.2f %.4f %.4f %4.0f %5.1f %5.1f %5.1f %5.1f\n"
			 , adsd->itime
			 , pct
			 , adsd->pitch
			 , adsd->roll
			 , adsd->thi
			 , adsd->alat
			 , adsd->alon
			 , adsd->galt
			 , adsd->wspd
			 , adsd->wdir
			 , ew, ns
			 );
# endif
		  mark = 0;
	      }
    }
}
/* c------------------------------------------------------------------------ */
