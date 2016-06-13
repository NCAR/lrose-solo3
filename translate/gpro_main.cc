/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */
/* c----------------------------------------------------------------------- */
/* c...mark */

#include <dd_defines.h>
#include <gpro_data.h>
#include <gpro_data.hh>
#include <gpro_main.hh>
#include <stdio.h>
#include <stdlib.h>
#include <dd_math.h>


static struct ads_data *adsd;
static char *tbuf;     /* input tape record buffer */
static char *hbuf;     /* header record buffer */
static int io_type;
static char *next_raw_tape;
static int gpro_fid;
static struct ads_raw_data_que *top_rdq;
static int ads_eof_count;


int 
main (void) {
    int i, j, k, n, fid, mark, status, count=0, trip=1234;
    int ads_max_eofs=2, doit=NO, loopcount;
    double d, time;
    char buf[32768];
    char *a;
    char *fn="/scr/steam/oye/rf27b.tape";
    float pct;
    struct ads_raw_data_que *rdq;
    struct ads_data ads_data;
    adsd = &ads_data;


    hbuf = (char *)malloc(BLOCK_SIZE);
    tbuf = (char *)malloc(BLOCK_SIZE);

    if(a=getenv("ADS_FILE_COUNT")) {
	if((i=atoi(a)) > 0)
	      ads_max_eofs = i;
    }
    printf("ADS file count = %d\n", ads_max_eofs);

    if(a=getenv("ADS_IO_TYPE")) {
	if((i=atoi(a)) >= 0)
	      io_type = i;
    }
    printf("ADS io_type = %d\n", io_type);

    fn = "/scr/stout/oye/rf29a";
    if(a=getenv("ADS_TAPES")) {
	fn = a;
    }
    put_tagged_string("GPRO_TAPES", fn);
    rdq = gpro_initialize(&adsd);

    for(;;rdq=rdq->next) {
	if(!(count++ % 100)) {
	    mark = 0;
	}
	if(count >= trip) {
	    mark = 0;
	}
	if(!(gpro_next_rec(rdq, &status))) {
	    printf("Last read stat: %d\n", status);
	    break;
	}
	if(ads_eof_count >= ads_max_eofs) {
	    printf( "EOF count break at %d\n", ads_eof_count);
	    break;
	}
# ifndef obsolete
	if(adsd->itime > 185000)
	      break;

	doit = adsd->itime > 0 && adsd->itime < 245959;
	doit = adsd->itime > 5244 && adsd->itime < 5350;
	doit = fabs((double)adsd->roll) > 5.;
	doit = adsd->itime >= 165050;
# else
	doit = YES;
# endif

	if(doit)
	      for(loopcount=0,pct=0; pct < 1.0; pct+=1.,loopcount++) {
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
		  printf(
"gpro %06d %.2f %5.1f %5.1f %6.2f %.4f %.4f %4.0f %4.0f %5.1f %4.0f %4.0f \
%5.1f %5.1f %5.1f\n"
			 , adsd->itime
			 , pct
			 , adsd->pitch
			 , adsd->roll
			 , adsd->thi
			 , adsd->alat
			 , adsd->alon
			 , adsd->palt
			 , adsd->hgme
			 , adsd->drift
			 , adsd->vew
			 , adsd->vns
			 , adsd->ui
			 , adsd->vi
			 , adsd->wi
			 );
# endif
	      }
    }
}
/* c------------------------------------------------------------------------ */
