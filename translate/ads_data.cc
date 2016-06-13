/* 	$Id: ads_data.c 2 2002-07-02 14:57:33Z oye $	 */

#ifndef lint
static char vcid[] = "$Id: ads_data.c 2 2002-07-02 14:57:33Z oye $";
#endif /* lint */
/*
 * This file contains the following routines
 * 
 * eld_gpro_fix_asib
 * gpro_assemble_vals
 * gpro_decode
 * gpro_get_vbls
 * gpro_header
 * gpro_mount_raw
 * gpro_nab_vbl
 * gpro_next_rec
 * gpro_position_err
 * gpro_sync
 * 
 * 
 * 
 * 
 * 
 */

# include <stdlib.h>
# include <string.h>
# include <stdio.h>
# include <errno.h>
# include "dorade_headers.h"
# include <eldh.h>
# include <ads_data.h>
# include <ads_data.hh>

static int Bitkey, logical_rec_size, num_logical_recs, physical_rec_size;
static int ltvi_count=0, gpro_fid= -1, io_type=FB_IO;
static int ads_merge=YES, ads_eof_count=0;
static struct requested_variables_list *rvl_top;
static struct letvar_info *ltvi_top, *ltvi_list[MAX_VBLS];
static struct ads_raw_data_que *top_rdq;
static struct ads_data *adsd;
static struct ins_errors *ins_err;
static DD_TIME dtsg;
static char *next_raw_tape=NULL;
static short yy, mon, dd, hh, mm, ss, ms;

/* c----------------------------------------------------------------------- */
# ifdef notyet

int 
main (void) {
    int i, j, k, n, fid, mark, status, count=0, trip=1234;
    int ads_max_eofs=2, doit=NO, loopcount;
    double d, time, fmod(), fabs(), sin(), theta, cos(), ew, ns;
    char buf[32768], *gpro_next_rec();
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
# endif
/* c------------------------------------------------------------------------ */

int 
eld_gpro_fix_asib (DD_TIME *dts, struct platform_i *asib)
{
    static char *hbuf=NULL;
    static int count=0, trip_count=500;
    char *a, *get_tagged_string(), *gpro_next_rec();
    struct ads_raw_data_que *rdq;
    struct ads_raw_data_que *gpro_sync();
    float delta;
    int i, status, mark;
    double d, d_time_stamp();

    if(!ads_merge)
	  return;

    if(!(count++ % trip_count)) {
	mark = 0;
    }
    if(!hbuf) {
	adsd = (struct ads_data *)malloc(sizeof(struct ads_data));
	hbuf = (char *)malloc(BLOCK_SIZE);
	dtsg.time_stamp = dts->time_stamp;
	d_unstamp_time(&dtsg);
	gpro_header(hbuf);

	if((a=get_tagged_string("ADS_START_DATE"))) {
	    dcdatime(a, strlen(a), &yy, &mon, &dd, &hh, &mm, &ss, &ms );
	    dtsg.julian_day=0;
	    dtsg.day_seconds=0;
	    dtsg.year = yy > 1900 ? yy : yy+1900;
	    dtsg.month = mon;
	    dtsg.day = dd;
	    d = d_time_stamp(&dtsg);
	}
	if(!(next_raw_tape=get_tagged_string("ADS_TAPES"))) {
	    printf("No ADS raw tape list\n");
	    ads_merge = NO;
	    return;
	}
	if(dd_itsa_physical_dev(next_raw_tape)) {
	    io_type = PHYSICAL_TAPE;
	}
# ifdef obsolete
	if((a=get_tagged_string("ADS_IO_TYPE"))) {
	    if(strstr(a, "FB_IO")) {
		io_type = FB_IO;
	    }
	    else if(strstr(a,"PHYSICAL_TAPE")) {
		io_type = PHYSICAL_TAPE;
	    }
	}
# endif
	/* mount the first raw tape */
	if((gpro_fid = gpro_mount_raw()) < 0) {
	    printf("Unable to mount first raw tape: %s %d\n"
		   , next_raw_tape, gpro_fid);
	    exit(1);
	}
	/* load up the que */
	for(rdq=top_rdq;;) {
	    if(!gpro_next_rec(rdq, &status)) {
		printf("Trouble doing inital reads: %d\n", status);
		exit(1);
	    }
	    if((rdq=rdq->next) == top_rdq)
		  break;		/* back at the top of the que */
	}
    }
    if(!(rdq=gpro_sync(dts->time_stamp))) {
	/* return without correcting anything */
	return;
    }
    /* lets do some correcting */
    delta = dts->time_stamp - rdq->time;
    gpro_assemble_vals(rdq, adsd, delta);
    asib->roll = adsd->roll;
    asib->latitude = adsd->alat - ins_err->alat_err;
    asib->longitude = adsd->alon - ins_err->alon_err;

}
/* c------------------------------------------------------------------------ */

int 
gpro_assemble_vals (struct ads_raw_data_que *rdq, struct ads_data *adsd, double pct)
{
    double gpro_decode();
    /*
     * assemble the requested values for the sample
     * this involves reading the records that will bracket the time
     * usually an interpolation between the two records that bracket the time
     */
    adsd->time = rdq->time;
    adsd->pitch = gpro_decode(ltvi_list[GPRO_ID_PITCH]
			      , pct, rdq->at[GPRO_ID_PITCH]);
    adsd->roll = gpro_decode(ltvi_list[GPRO_ID_ROLL]
			      , pct, rdq->at[GPRO_ID_ROLL]);
    adsd->thi = gpro_decode(ltvi_list[GPRO_ID_THI]
			      , pct, rdq->at[GPRO_ID_THI]);
    if(adsd->thi < 0 ) adsd->thi += 360.;
    adsd->alat = gpro_decode(ltvi_list[GPRO_ID_ALAT]
			      , pct, rdq->at[GPRO_ID_ALAT]);
    adsd->alon = gpro_decode(ltvi_list[GPRO_ID_ALON]
			      , pct, rdq->at[GPRO_ID_ALON]);

    adsd->alat2 = gpro_decode(ltvi_list[GPRO_ID_ALAT2]
			      , pct, rdq->at[GPRO_ID_ALAT2]);
    adsd->alon2 = gpro_decode(ltvi_list[GPRO_ID_ALON2]
			      , pct, rdq->at[GPRO_ID_ALON2]);

    adsd->glat = gpro_decode(ltvi_list[GPRO_ID_GLAT]
			      , pct, rdq->at[GPRO_ID_GLAT]);
    adsd->glon = gpro_decode(ltvi_list[GPRO_ID_GLON]
			      , pct, rdq->at[GPRO_ID_GLON]);
    adsd->galt = gpro_decode(ltvi_list[GPRO_ID_GALT]
			      , pct, rdq->at[GPRO_ID_GALT]);
    adsd->vns = gpro_decode(ltvi_list[GPRO_ID_VNS]
			      , pct, rdq->at[GPRO_ID_VNS]);
    adsd->vew = gpro_decode(ltvi_list[GPRO_ID_VEW]
			      , pct, rdq->at[GPRO_ID_VEW]);
    adsd->gvns = gpro_decode(ltvi_list[GPRO_ID_GVNS]
			      , pct, rdq->at[GPRO_ID_GVNS]);
    adsd->gvew = gpro_decode(ltvi_list[GPRO_ID_GVEW]
			      , pct, rdq->at[GPRO_ID_GVEW]);
    adsd->wspd = gpro_decode(ltvi_list[GPRO_ID_WSPD]
			      , pct, rdq->at[GPRO_ID_WSPD]);
    adsd->wdir = gpro_decode(ltvi_list[GPRO_ID_WDIR]
			      , pct, rdq->at[GPRO_ID_WDIR]);

    adsd->itime = rdq->at[GPRO_ID_SEC]->ivalue
	  + rdq->at[GPRO_ID_MIN]->ivalue*100
		+ rdq->at[GPRO_ID_HR]->ivalue*10000;

    adsd->etime = rdq->at[GPRO_ID_SEC]->ivalue
	  + rdq->at[GPRO_ID_MIN]->ivalue*60
		+ rdq->at[GPRO_ID_HR]->ivalue*3600;
    return;
}
/* c------------------------------------------------------------------------ */

double 
gpro_decode (struct letvar_info *this, double pct_samp, struct ads_raw_data_point *ardp)
{
    int i, inc, j, k, mark, mask;
    int sample;
    float *f, rcp_factor=this->rcp_factor;
    char *a;
    
    if(this->sample > 1) {	/* don't have an ivalue for this sample yet */
	sample = pct_samp*this->sample;
	if(this->bits == 16)
	      ardp->ivalue = ardp->shorts[sample];
	else if(this->bits > 16)
	      ardp->ivalue = ardp->longs[sample];
    }	
    
    if(this->bits > 16 && this->bits < 32) {
	ardp->ivalue >>= 32-this->bits;

	if(this->conkey == GPRO_UNSIGNED_INT) {
	    mask = ~((~0) << this->bits);
	    ardp->ivalue &= mask;
	}
    }
    else if(this->bits == 16 && this->conkey == GPRO_UNSIGNED_INT) {
	ardp->ivalue &= 0xffff;
    }
    else if(this->conkey == GPRO_IEEE_FP) {
	f = (float *)&ardp->ivalue;
	ardp->value = GPRO_UNSCALE(*f, this->rcp_factor, this->term);
	return(ardp->value);
    }
    ardp->value = GPRO_UNSCALE(ardp->ivalue, rcp_factor, this->term);
    return(ardp->value);
}
/* c------------------------------------------------------------------------ */

int 
gpro_get_vbls (struct ads_raw_data_que *rdq)
{
    int i, mark;
    double d, diff, gpro_decode(), d_time_stamp(), fabs();
    struct letvar_info *this;

    for(this=ltvi_top; ; this=this->next) {
	gpro_nab_vbl(rdq->raw_data_buf, this, rdq->at[this->id_num]);
	if(this->next == ltvi_top)
	      break;
    }
    dtsg.julian_day=0;
    dtsg.day_seconds = d = D_SECS(rdq->at[GPRO_ID_HR]->ivalue
			     , rdq->at[GPRO_ID_MIN]->ivalue
			     , rdq->at[GPRO_ID_SEC]->ivalue
			     , 0);
    rdq->time = d_time_stamp(&dtsg);
    /* see we've crossed midnight */
    if(fabs(rdq->time -rdq->last->time) > ALMOST_ONE_DAY) {
	if(rdq->last->time > 0 ) { /* not the first record */
	    dtsg.time_stamp += SECONDS_PER_DAY +.001; /* klooge! */
	    rdq->time = dtsg.time_stamp;
	    d_unstamp_time(&dtsg);
	}
    }
}
/* c------------------------------------------------------------------------ */

int 
gpro_header (char *buf)
{

    char *a, *b, *c, *s=buf;
    char *get_tagged_string();
    int i, j, m, n, count=0;
    double d, atof();
    FILE *fp;
    
    struct requested_variables_list *next, *last;
    struct letvar_info *ltvi, *ltvi_last;
    struct ads_raw_data_que *rdq, *last_rdq;
    struct running_avg *ravg, *lastravg;
    char *letgen, *ordgen;
    static char *vlist[MAX_VBLS];
    int gpro_ids[MAX_VBLS];
    int rvl_count;
    int Logbit, Datlog, Datsiz;
    int nchar=0;

    a = "/home/steam/oye/radar/dorade/genpro.header";
    if(!(fp=fopen( a, "r"))) {
	printf("Unable to open %s\n", a);
    }

    i = 0;
    gpro_ids[i] = GPRO_ID_HR;
    vlist[i] = "HR";
    i++;
    gpro_ids[i] = GPRO_ID_MIN;
    vlist[i] = "MIN";
    i++;
    gpro_ids[i] = GPRO_ID_SEC;
    vlist[i] = "SEC";
    i++;
    gpro_ids[i] = GPRO_ID_PITCH;
    vlist[i] = "PITCH";
    i++;
    gpro_ids[i] = GPRO_ID_ROLL;
    vlist[i] = "ROLL";
    i++;
    gpro_ids[i] = GPRO_ID_THI;
    vlist[i] = "THI";
    i++;
    gpro_ids[i] = GPRO_ID_ALAT;
    vlist[i] = "ALAT";
    i++;
    gpro_ids[i] = GPRO_ID_ALON;
    vlist[i] = "ALON";
    i++;
    gpro_ids[i] = GPRO_ID_GMOD1;
    vlist[i] = "GMOD1";
    i++;
    gpro_ids[i] = GPRO_ID_GSTAT;
    vlist[i] = "GSTAT";
    i++;
    gpro_ids[i] = GPRO_ID_GLAT;
    vlist[i] = "GLAT";
    i++;
    gpro_ids[i] = GPRO_ID_GLON;
    vlist[i] = "GLON";
    i++;
    gpro_ids[i] = GPRO_ID_GALT;
    vlist[i] = "GALT";
    i++;
    gpro_ids[i] = GPRO_ID_ALAT2;
    vlist[i] = "ALAT2";
    i++;
    gpro_ids[i] = GPRO_ID_ALON2;
    vlist[i] = "ALON2";
    i++;
    gpro_ids[i] = GPRO_ID_VEW;
    vlist[i] = "VEW";
    i++;
    gpro_ids[i] = GPRO_ID_VNS;
    vlist[i] = "VNS";
    i++;
    gpro_ids[i] = GPRO_ID_GVEW;
    vlist[i] = "GVEW";
    i++;
    gpro_ids[i] = GPRO_ID_GVNS;
    vlist[i] = "GVNS";
    i++;
    gpro_ids[i] = GPRO_ID_WDIR;
    vlist[i] = "WDIR";
    i++;
    gpro_ids[i] = GPRO_ID_WSPD;
    vlist[i] = "WSPD";
    i++;

    rvl_count = i;

    for(i=0; i < rvl_count; i++) {
	/* requested_variables_list que */
	next = (struct requested_variables_list *)
	      malloc(sizeof(struct requested_variables_list));
	if(!i) {
	    rvl_top = next;
	}
	else {
	    last->next = next;
	    next->last = last;
	}
	next->name = vlist[i];
	next->id_num = gpro_ids[i];
	rvl_top->last = next;
	next->next = rvl_top;
	last = next;
    }

    /*
     * load in the header and establish pointers to critical info
     * 
     */

    printf( "\n" );

    for(;;) {
	count++;

	if( !fgets(s, 81, fp) || count > 2222 ) {
	    printf( "gpro_header--break! n,count=%d,%d\n", n, count );
	    break;
	}
	for(n=0,a=s; *a && *a != '\n' && n < 80; n++,a++); *a = '\0';;
	nchar += n;

	if(*s == '/') {
	    continue;
	}
	else if(!strncmp(s+1,"LETVAR",6)) {

	    /* get the variable name */
	    for(b=s+n-1; s < b && *(b-1) == ' '; b--); /* get to last char */
	    for(a=b-1; s < a && *(a-1) != ' '; a--); /* get to first char */
	    *b = '\0';
	    /*
	     * see if this is one of the requested variables
	     */
	    for(i=0,next=rvl_top; i < rvl_count; i++,next=next->next) {
		if(!strcmp(a,next->name)) {
		    /* found one! */
		    ltvi = (struct letvar_info *)
			  malloc(sizeof(struct letvar_info));
		    if(!ltvi_count++) {	/* first one */
			ltvi_top = ltvi;
		    }
		    else {
			ltvi_last->next = ltvi;
			ltvi->last = ltvi_last;
		    }
		    ltvi->name = a;
		    for(c=s; *c != '=' && c < b; c++); /* get past the = */
		    ltvi->letvar = ++c;

		    /* be able to reference this struct by variable id num */
		    ltvi_list[next->id_num] = ltvi;

		    ltvi->id_num = next->id_num;
		    ltvi_last = ltvi;
		    ltvi->next = ltvi_top;
		    ltvi_top->last = ltvi;

		    /* remove it from the variable list */
		    next->last->next = next->next;
		    next->next->last = next->last;
		    if(next == rvl_top)
			  rvl_top = next->next;
		    free(next);
		    s += n+1;	/* save this line */
		    rvl_count--;
		    break;
		}
	    }
	    if(!rvl_count)	/* all variables have been found */
		  break;
	    if(i == rvl_count) {
		/* requested variable not found! */
	    }
	}
	else if(!strncmp(s+1,"LETGEN",6)) {
	    for(c=s; *c != '='; c++); c++; /* get past the = */
	    letgen = c;
	    s += n+1;
	}
    }
# ifdef obsolete
    printf( "Read %d characters\n", count*LINE_LEN );
# endif
    fclose(fp);


    /*
     * unpack the decoding constants
     */

    for(a=c=letgen,b=a+strlen(a); a < b && *(b-1) == ' '; b--); *b = '\0';
    for(; c < b && *c != ','; c++); *c = '\0';
    Logbit = atoi(a);
    for(a= ++c; c < b && *c != ','; c++); *c = '\0';
    Datlog = atoi(a);
    for(a= ++c; c < b && *c != ','; c++); *c = '\0';
    Datsiz = atoi(a);
    for(a= ++c; c < b && *c != ','; c++); *c = '\0';
    Bitkey = atoi(++c);
    logical_rec_size = BITS_TO_BYTES(Logbit);
    num_logical_recs = Datlog;
    physical_rec_size = BITS_TO_BYTES(Datsiz);

    for(i=0,ltvi=ltvi_top; i < ltvi_count; i++,ltvi=ltvi->next) {
	a = c = ltvi->letvar;
	b = ltvi->name;
	for(     ; c < b && *c != ','; c++); *c = '\0';	/* next comma */
	ltvi->fstbit = atoi(a);
	for(a= ++c; c < b && *c != ','; c++); *c = '\0';
	ltvi->bits = atoi(a);
	for(a= ++c; c < b && *c != ','; c++); *c = '\0';
	ltvi->skip = atoi(a);
	for(a= ++c; c < b && *c != ','; c++); *c = '\0';
	ltvi->sample = atoi(a);
	for(a= ++c; c < b && *c != ','; c++); *c = '\0';
	ltvi->conkey = atoi(a);
	for(a= ++c; c < b && *c != ','; c++); *c = '\0';
	ltvi->term = atof(a);
	for(a= ++c; c < b && *c != ','; c++); *c = '\0';
	ltvi->rcp_factor = 1./atof(a);

	ltvi->offset = FSTBIT_TO_OFFSET(ltvi->fstbit);
	ltvi->binc = BITS_TO_BYTES(ltvi->bits + ltvi->skip);
	ltvi->sinc = BYTES_TO_SHORTS(ltvi->binc);
	ltvi->linc = BYTES_TO_LONGS(ltvi->binc);
    }

    /* set up the raw data que */
    for(j=0; j < MAX_RDQ; j++) {
	rdq = (struct ads_raw_data_que *)
	      malloc(sizeof(struct ads_raw_data_que));
	if(!j) {
	    top_rdq = rdq;
	}
	else {
	    last_rdq->next = rdq;
	    rdq->last = last_rdq;
	}
	top_rdq->last = rdq;
	rdq->next = top_rdq;
	last_rdq = rdq;

	for(i=0,ltvi=ltvi_top; i < ltvi_count; i++,ltvi=ltvi->next) {
	    rdq->at[ltvi->id_num] = (struct ads_raw_data_point*)
	      malloc(sizeof(struct ads_raw_data_point));
	}
    }
    /* set up the lat/lon error structures */
    ins_err = (struct ins_errors *)malloc(sizeof(struct ins_errors));
    memset((char *)ins_err, 0, sizeof(struct ins_errors));

    /* c...mark */
    /* set up the lat error que */
    for(i=0; i < MAX_POS_AVG; i++) {
	ravg = (struct running_avg *)malloc(sizeof(struct running_avg));
	if(!i) {
	    ins_err->top_alat_err = ravg;
	}
	else {
	    lastravg->next = ravg;
	    ravg->last = lastravg;
	}
	lastravg = ravg;
	ravg->next = ins_err->top_alat_err;
	ins_err->top_alat_err->last = ravg;
    }
    /* set up the lon error que */
    for(i=0; i < MAX_POS_AVG; i++) {
	ravg = (struct running_avg *)malloc(sizeof(struct running_avg));
	if(!i) {
	    ins_err->top_alon_err = ravg;
	}
	else {
	    lastravg->next = ravg;
	    ravg->last = lastravg;
	}
	lastravg = ravg;
	ravg->next = ins_err->top_alon_err;
	ins_err->top_alon_err->last = ravg;
    }
}
/* c------------------------------------------------------------------------ */

int 
gpro_mount_raw (void)
{
    int n;
    char *a, *b, *c, *dd_whiteout(), *dd_delimit(), str[256];

    a = dd_whiteout(next_raw_tape);
    if(!strlen(a))
	  return(-1);
    b = dd_delimit(a+1);
    for(c=str; a < b;)
	  *c++ = *a++;
    *c = '\0';
    printf("Opening ads raw data tape: %s\n", str);
    if((n = open( str, 0)) < 0)
	  return(n);
    next_raw_tape = b;
    return(n);
}
/* c------------------------------------------------------------------------ */

int 
gpro_nab_vbl (char *buf, struct letvar_info *this, struct ads_raw_data_point *ardp)
{
    /* just nab the raw bits
     */
    int i, inc, j, k, n, mark;
    char *a=buf+this->offset;
    short sint, *ss;
    int32_t ivalue, *ll;

    if(this->sample == 1) {
	/* just nab the data point */
	if(this->bits == 8) {
	    ardp->ivalue = *a;
	}
	else if(this->bits == 16) {
	    ardp->ivalue = *((short *)a);
	}
	else if(this->bits > 16) {
	    ardp->ivalue = *((int32_t *)a);
	}
    }
    else {
	/* assume sample > 1 and just copy out the data
	 */
	if(this->bits == 16 && this->skip == 0) {
	    /* contiguous shorts */
	    memcpy((char *)ardp->shorts, a, this->sample*sizeof(short));
	}
	else if(this->bits == 16) { /* interleaved shorts */
	    inc = this->sinc;
	    n = this->sample;
	    for(ss=(short *)a,i=0; i < n; ss+=inc) {
		ardp->shorts[i++] = *ss;
	    }
	}
	else if(this->bits > 16 && this->skip < 16) {
	    /* contiguous longs */
	    memcpy((char *)ardp->longs, a, this->sample*sizeof(int32_t));
	}
	else {			/* assume interleaved longs */
	    inc = this->linc;
	    n = this->sample;
	    for(ll=(int32_t *)a,i=0; i < n; ll+=inc)
		  ardp->longs[i++] = *ll;
	}
    }
}
/* c------------------------------------------------------------------------ */

char *
gpro_next_rec (struct ads_raw_data_que *rdq, int *status)
{
    /* This routine returns a pointer to the start of then next
     * logical record of ads data
     */
    int i, n=0, mark, eof_count=0, bad_rec=0;
    short *ss;
    static unsigned short *us;
    static char *buf=NULL, *gpr;
    static int lrec_num=0, count=0, trip=10;

    *status = 0;
    if(!buf) {
	buf = (char *)malloc(BLOCK_SIZE);
	us = (unsigned short *)buf;
    }
    for(;;) {
	if(lrec_num % num_logical_recs) {
	    /* no need to read in another record */
	    gpr = buf +(lrec_num*logical_rec_size);
	    if(*((unsigned short *)gpr) == Bitkey) {
		break;
	    }
	}
	count++;
	lrec_num = 0;
	*status = n =
	      gp_read(gpro_fid, buf, BLOCK_SIZE, io_type);
	if(n < 0) {
	    bad_rec++;
	    printf("ADS funky rec at: %d  status: %d  errno: %d\n"
		   , count, n, errno);
	    if(bad_rec > 21) {
		return(NULL);
	    }
	}
	else if( n == 0 ) {
	    ads_eof_count++;
	    printf(" ADS EOF\n");
	    if(++eof_count > 2) {
		return(NULL);
	    }
	}
	else if(*us == Bitkey) {
	    gpr = buf;
	    break;
	}
	else {
	    if(n > physical_rec_size) {
		mark = 0;
	    }
	    bad_rec = 0;
	    eof_count = 0;
	}
	/* otherwise keep looping */
    }
    lrec_num++;
    rdq->raw_data_buf = gpr;
    gpro_get_vbls(rdq);
    gpro_assemble_vals(rdq, adsd, 0);
    gpro_position_err(adsd);
    return(gpr);
}
/* c------------------------------------------------------------------------ */

int 
gpro_position_err (struct ads_data *adsd)
{
    /* keep the average error in lat/lon by
     * keeping a running average of n seconds of non-sever roll data
     */
    double fabs();
    float err;

    if(fabs((double)adsd->roll) > ELD_ROLL_LIMIT) {
	return;
    }
    if(ins_err->num < MAX_POS_AVG) {
	ins_err->num++;
	ins_err->rcp_num = 1./(float)ins_err->num;
    }
    else {
	ins_err->sum_alat_err -= ins_err->top_alat_err->val;
	ins_err->sum_alon_err -= ins_err->top_alon_err->val;
    }
    ins_err->sum_alat_err +=
	  (ins_err->top_alat_err->val = adsd->alat - adsd->glat);
    ins_err->top_alat_err = ins_err->top_alat_err->next;
    ins_err->alat_err = ins_err->rcp_num * ins_err->sum_alat_err;

    ins_err->sum_alon_err +=
	  (ins_err->top_alon_err->val = adsd->alon - adsd->glon);
    ins_err->top_alon_err = ins_err->top_alon_err->next;
    ins_err->alon_err = ins_err->rcp_num * ins_err->sum_alon_err;
}
/* c------------------------------------------------------------------------ */

struct ads_raw_data_que *
gpro_sync (double time)
{
    /* c...mark */
    struct ads_raw_data_que *rdq=NULL, *this;
    static int count=0, trip=100, no_count=0, keep_trying=YES, sync_count=0;
    int where=AFTER, status, mark;
    DD_TIME dts;
    char str[32], str0[32], *dts_print();
    
    if(keep_trying) {
	for(;;) {
	    if(!(count % trip)) {
		mark = 0;
	    }
	    /* try to keep reading in records until we have
	     * bracketed the requested time
	     */
	    for(this=top_rdq;;) { /* loop through the que */
		if(time >= this->time && time < this->time +1.) {
		    /* we have bracketed the time requested! */
		    if(!(sync_count++ % 1500)) {
			dts.time_stamp = time;
			d_unstamp_time(&dts);
			printf("  ADS at %.2f sync'd with %.2f  %s\n"
			       , this->time, time, dts_print(&dts, str));
		    }
		    return(this);
		}
		else if(this->time < time) {
		    where = BEFORE;
		}
		else {
		    where = AFTER;
		}
		if((this=this->last) == top_rdq) {
		    break;
		}
	    }
	    if(where == BEFORE) {
		count++;
		/* read in the next record and search again */
		top_rdq = top_rdq->next;
		if(!(gpro_next_rec(top_rdq, &status))) {
		    printf("Last read stat: %d\n", status);
		    close(gpro_fid);
		    /* try mounting another tape */
		    if((gpro_fid = gpro_mount_raw()) < 0) {
			keep_trying = NO;
			break;
		    }
		}
	    }
	    else {		/* haven't sync'd up with ac data yet */
		mark = 0;
		break;
	    }
	}
    }
    if(!(no_count++ % 1500)) {
	dts.time_stamp = time;
	d_unstamp_time(&dts);
	printf("  No ADS sync for time: %s\n", dts_print(&dts, str));
    }
    return(NULL);
}
/* c----------------------------------------------------------------------- */

/* c------------------------------------------------------------------------ */

/* c----------------------------------------------------------------------- */

