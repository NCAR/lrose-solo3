/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

#include <string.h>
#include <stdlib.h>
#include <dd_defines.h>
#include <dd_time.h>
#include <dd_math.h>
#include "dorade_share.hh"
#include "ddb_common.hh"
#include "xwsrqc.hh"

# ifndef SEEK_SET
# define SEEK_SET 0
# define SEEK_CUR 1
# define SEEK_END 2
# endif

/* c------------------------------------------------------------------------ */
static struct wsrqc_stuff_ptrs *top_scope1=NULL;
static struct wsrqc_stuff_ptrs *top_scope2=NULL;
static struct wsrqc_stuff_ptrs *top_scope3=NULL;
static struct wsrqc_stuff_ptrs *wsp1=NULL, *wsp2=NULL, *wsp3=NULL;
static char *last_line=NULL, *look_for=NULL, *look_for_found=NULL;
# define SIZEOF_BUF 1024
static char buf[SIZEOF_BUF];
static int wsr_correct=YES;
static int32_t current_offset=0;
static FILE *stream;
static float db_off, attenuation, log_intercept, log_slope;
static float min_curve, max_value;

# ifdef obsolete
/* c------------------------------------------------------------------------ */

int 
main (void)
{
    struct generic_radar_info *gri;

    int32_t ival=0x40000004;
    double d;
    int ii, jj, kk, nn;
    int yy=92, mon=11, dd=06, hh=4, mm=1, ss=0;
    char *a;
    

    gri = return_gri_ptr();

    if((a=getenv("WSR_QC_FILE"))) {
	put_tagged_string("WSR_QC_FILE", a);
    }
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
	wsrqc_correct(gri, "DZ");
	gri->time += 1.;
    }

    /* c...mark */
}
# endif

/* c------------------------------------------------------------------------ */

int 
wsrqc_correct (struct generic_radar_info *gri, char *name)
{
    static int count=0;//Jul 26, 2011
    int ii, mm, mark;

    if(!wsr_correct)
	  return(1);
    /*
     * routine to correct the power fields for the TOGA/SIGMET radar
     * in TOGA-COARE
     *
     * get the designated corrections file and open it
     * then initialize on the file
     */
    if(!count++) {
	if(!wsrqc_init(gri))
	      return(0);
	if(!wsr_correct)
	      return(1);
    }
    /*
     * bracket the current time in scopes 2 & 3
     */
    for(; !wsp2->last_qc_set; wsp2=wsp2->next) { /* scope2 */
	if(gri->time <= wsp2->at->stop_time)
	      break;
    }

    for(; !wsp3->last_qc_set; wsp3=wsp3->next) { /* scope3 */

	if(gri->time >= wsp3->next->at->start_time) {
	    /*
	     * bring in the next entry
	     */
	    if((mm = nab_wsrqc_stuff(stream, buf, wsp3->next, (char *)"scan", 1)) < 2) {
		wsp3->last_qc_set = YES;
		break;
	    }
	    if(gri->time < wsp3->next->at->start_time) {
		/* calculate some parameters that will not change
		 * until the next entry
		 */
		db_off = wsp1->at->dboff
		      + wsp2->at->dboff
			    + wsp3->at->dboff;
		attenuation = wsp1->at->attenuation
		      + wsp2->at->attenuation
			    + wsp3->at->attenuation;
		attenuation *= .001; /* for meters instead of km. */
		log_slope = wsp1->at->log_slope
		      + wsp2->at->log_slope
			    + wsp3->at->log_slope;
		log_intercept = wsp1->at->log_intercept
		      + wsp2->at->log_intercept
			    + wsp3->at->log_intercept;
		
		min_curve = -1.0 * (log_intercept/log_slope);
		max_value = log_intercept + log_slope*(-20.0);
	    }
	}
	if(gri->time < wsp3->next->at->start_time) {
	    break;
	}
    }
    /*
     * correct the power field
     */

    wsrqc_power_cor(gri, name);
    return(0);
}
/* c------------------------------------------------------------------------ */

int 
wsrqc_init (struct generic_radar_info *gri)
{
    /* routine to
     * read in scope1 stuff then
     * read in all the scope2 descriptors then
     * loop to postition near the first scope3 entry
     */
    char *a;
    int ii, jj, kk, mm, nn, mark;
    struct wsrqc_stuff_ptrs *wsp, *last_wsp;
    int32_t fpos, flen, finc;


    if(a=get_tagged_string("WSR_QC_FILE")) {
	if(!(stream = fopen(a, "r"))) {
	    printf("Unable to open wsr qc file: %s\n", a);
	    wsr_correct = NO;
	    return(0);
	}
    }
    else {
	wsr_correct = NO;
	return(1);
    }
    /*
     * nab scope1 qc set
     */
    top_scope1 = wsp1 = wsp = wsr_malloc();
    /*
     * read in the next set of lines up to and including
     * the first line containing a scan "scan"
     */
    if(!(mm = nab_wsrqc_stuff(stream, buf, wsp, (char *)"scan", 1))) {
	/* unable to read in any data
	 */
	wsr_correct = NO;
	return(0);
    }
    look_for = (char *)"scope3";
    /*
     * nab scope2 qc sets
     */
    for(ii=0;; ii++) {
	wsp = wsr_malloc();
	mm = nab_wsrqc_stuff(stream, buf, wsp, (char *)"scan", 1);
	if(!ii) {
	    top_scope2 = wsp;
	}
	else {
	    wsp->last = last_wsp;
	    last_wsp->next = wsp;
	}
	wsp->next = top_scope2;
	top_scope2->last = wsp;
	last_wsp = wsp;

	if(strstr(buf, "scope3")) { /* should be have read in the first
				     * "scan" line of scope3 at this point
				     */
	}
	if(look_for_found) {
	    wsp->last_qc_set = YES;
	    break;
	}
    }
    look_for = NULL;
    /*
     * position to the appropriate scope2 qc set
     */
    for(wsp=top_scope2;  !wsp->last_qc_set;  wsp=wsp->next) {
	if(wsp->at->start_time <= gri->time &&
	   wsp->at->stop_time >= gri->time)
	      break;
    }
    wsp2 = wsp;
    /*
     * build a scope3 queue with 3 entries
     */
    for(ii=0; ii < 3; ii++) {
	wsp = wsr_malloc();
	if(!ii) { top_scope3 = wsp; }
	else {
	    wsp->last = last_wsp;
	    last_wsp->next = wsp;
	}
	top_scope3->last = wsp;
	wsp->next = top_scope3;
	last_wsp = wsp;
    }
    /*
     * nab the first scope3 qc set
     */
    mm = nab_wsrqc_stuff(stream, buf, wsp, (char *)"scan", 1);
    /*
     * set up to jump to 11 other locations in the file
     */
    flen = fseek(stream, 0L, SEEK_END);
    flen = ftell(stream);
    fpos = wsp->file_position;
    finc = (flen -fpos)/12; /* divide scope3 into 12 sections */
    /*
     * try and bracket the time in scope3
     */
    for(ii=0; ii < 11; ii++,wsp=wsp->next) {
	fpos += finc;
	nn = fseek(stream, fpos, SEEK_SET);
	nn = fread(buf, sizeof(char), sizeof(buf), stream);
	/*
	 * locate the next "scan" line
	 */
	for(a=buf,jj=0,kk=-1; a < buf+nn; a++,jj++) {
	    if(*a != '\n')
		  continue;
	    a++; jj++;
	    if(strncmp(a, "scan", 4) == 0) {
		kk = jj;
		break;
	    }
	}
	if(kk == -1) {
	    /* punt! */
	    break;
	}
	mm = fseek(stream, fpos+kk, SEEK_SET);
	current_offset = ftell(stream);
	/*
	 * nab the next qc set
	 */
	last_line = NULL;
	mm = nab_wsrqc_stuff(stream, buf, wsp->next, (char *)"scan", 2);
	/*
	 * see if we have bracket the time
	 */
	if(wsp->next->at->start_time < gri->time)
	      continue;
	else
	      break;
    }
    wsp3 = wsp;
    /*
     * position to reread the appropriate starter qc set
     */
    fpos = fseek(stream, wsp->file_position, SEEK_SET); 
    current_offset = ftell(stream);
    last_line = NULL;
    mm = nab_wsrqc_stuff(stream, buf, wsp, (char *)"scan", 2);

    for(wsp=wsp->next,ii=0; ii < 2; wsp=wsp->next,ii++) {
	/* clear out the other two entries
	 */
	memset(wsp->at, 0, sizeof(struct wsrqc_stuff));
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

struct wsrqc_stuff_ptrs *
wsr_malloc (void)
{
    struct wsrqc_stuff_ptrs *wsp;

    wsp = (struct wsrqc_stuff_ptrs *)
	  malloc(sizeof(struct wsrqc_stuff_ptrs));
    memset(wsp, 0, sizeof(struct wsrqc_stuff_ptrs));
    wsp->at = (struct wsrqc_stuff *)malloc(sizeof(struct wsrqc_stuff));
    memset(wsp->at, 0, sizeof(struct wsrqc_stuff));
    return(wsp);
}
/* c------------------------------------------------------------------------ */

int nab_wsrqc_stuff(FILE *stream, char *buf, struct wsrqc_stuff_ptrs *wsp, char *quit_at, int num_occurs)
{
    /* routine to read in the next set of wsrqc stuff and return a pointer
     * to the last line read in
     */
    int ii, nn, hh, mm, ss, noc=0, mark, lc;
    char *a, *b, *c=buf, *nl;
    struct wsrqc_stuff *ws=wsp->at;
    DD_TIME dts;
    double d;

    if(last_line && (nn=strlen(last_line))) {
	/* move last line to beginning of buffer
	 */
	memcpy(c, last_line, nn);
	c += nn;
    }
    wsp->file_position = current_offset;
    /*
     * read in the next qc set and quit when you've
     * encountered the quit_at string
     */
    for(lc=0;;) {
	current_offset = ftell(stream); /* save the position
					 * of this line */
	last_line = c;
	if(!(a = fgetsx((unsigned char *)last_line, (int)88, stream))) {  //Jul 26, 2011
	    break;
	}
	lc++;
	c += strlen(c);
	if(strstr(last_line, quit_at)) {
	    /* we may be looking for more than one occurance of the quit string
	     */
	    if(++noc >= num_occurs) {
		break;
	    }
	}
    }
    if(!lc) {			/* no more data */
	return(0);
    }
    if(look_for) {
	look_for_found = strstr(buf, look_for);
    }
    else
	  look_for_found = NULL;

    memset(ws, 0, sizeof(struct wsrqc_stuff));
    /*
     * now loop through the input buffer
     */

    for(a=buf; a < last_line; a=nl+1) {
	/* find the next new line and put a null there
	 */
	if(nl = strchr(a, '\n')) *nl = '\0'; else nl = c;
	if(strlen(a) < 5)
	      continue;

	if((b = strstr(a, "scan"))) { /* get start and stop times */
	    b = dd_delimit(b);
	    a = dd_whiteout(b);	/* should be postioned at start of date */
	    /*
	     * nab the start date/time
	     */
	    nn = sscanf(a, "%02d%02d%02d", &dts.year, &dts.month, &dts.day);
	    dts.year = dts.year < 1900 ? dts.year+1900 : dts.year;
	    b = dd_delimit(a);
	    a = dd_whiteout(b);	/* should be postioned at start of the time */
	    nn = sscanf(a, "%02d:%02d:%02d", &hh, &mm, &ss);
	    dts.day_seconds = D_SECS(hh, mm, ss, (int)0);
    	    ws->start_time = d_time_stamp(&dts);
	    /*
	     * see if there is a stop time
	     */
	    b = dd_delimit(a);
	    a = dd_whiteout(b);
	    if(strlen(a)) { /* next date */
		nn = sscanf(a, "%02d%02d%02d"
			    , &dts.year, &dts.month, &dts.day);
		dts.year = dts.year < 1900 ? dts.year+1900 : dts.year;
		b = dd_delimit(a);
		a = dd_whiteout(b);
		nn = sscanf(a, "%02d:%02d:%02d", &hh, &mm, &ss);
		dts.day_seconds = D_SECS(hh, mm, ss, (int)0);
		ws->stop_time = d_time_stamp(&dts);
	    }
	}
	else if(b = strstr(a, "lat")) {
	    sscanf(a, "%*s %lf", &ws->lat);
	    /* "%*s" ignores the string in that position */
	}
	else if(b = strstr(a, "lon")) {
	    sscanf(a, "%*s %lf", &ws->lon);
	}
	else if(b = strstr(a, "horz_beam_w")) {
	    sscanf(a, "%*s %f", &ws->h_beamwidth);
	}
	else if(b = strstr(a, "vert_beam_w")) {
	    sscanf(a, "%*s %f", &ws->v_beamwidth);
	}
	else if(b = strstr(a, "site")) {
	    sscanf(a, "%*s %s", &ws->site);
	}
	else if(b = strstr(a, "name")) {
	    sscanf(a, "%*s %s", &ws->radar_name);
	}
	else if(b = strstr(a, "f_048")) {
	    sscanf(a, "%*s %f", &ws->dbzoff_048);
	}
	else if(b = strstr(a, "f_182")) {
	    sscanf(a, "%*s %f", &ws->dbzoff_182);
	}
	else if(b = strstr(a, "f_080")) {
	    sscanf(a, "%*s %f", &ws->dbzoff_080);
	}
	else if(b = strstr(a, "dboff")) {
	    sscanf(a, "%*s %f", &ws->dboff);
	}
	else if(b = strstr(a, "st_048")) {
	    sscanf(a, "%*s %f", &ws->radconst_048);
	}
	else if(b = strstr(a, "st_182")) {
	    sscanf(a, "%*s %f", &ws->radconst_182);
	}
	else if(b = strstr(a, "st_080")) {
	    sscanf(a, "%*s %f", &ws->radconst_080);
	}
	else if(b = strstr(a, "atten")) {
	    sscanf(a, "%*s %f", &ws->attenuation);
	}
	else if(b = strstr(a, "g_slope")) {
	    sscanf(a, "%*s %f", &ws->log_slope);
	}
	else if(b = strstr(a, "g_inter")) {
	    sscanf(a, "%*s %f", &ws->log_intercept);
	}
	else {
	    printf("Non scanned string: %s\n", a);
	    mark = 0;
	}
    }
    return(lc);
}
/* c------------------------------------------------------------------------ */

int 
wsrqc_power_cor (struct generic_radar_info *gri, char *name)
{
    int ii, nn, mark, fn, ibad;
    float *rc=gri->range_correction, scale, bias, f, rcp_scale;
    float *rr=gri->range_value;
    float dbz_off, rad_const;
    float dbi, dbc, pw_micros, ratt;
    unsigned char *cc;
    short *ss;

    for(fn=0; fn < gri->num_fields_present; fn++) {
	if(strncmp(gri->field_name[fn], name, strlen(name)) == 0)
	      break;
    }
    if(fn == gri->num_fields_present)
	  return(0);

    ibad = gri->missing_data_flag;
    ss = gri->scaled_data[fn];
    cc = (unsigned char *)gri->byte_data[fn];
    scale = gri->dd_scale[fn];
    rcp_scale = 1./scale;
    bias = gri->dd_offset[fn];
    pw_micros = (gri->pulse_width*2./SPEED_OF_LIGHT)*1.e6;

    if(pw_micros < .52) {
	dbz_off = wsp1->at->dbzoff_048 + wsp2->at->dbzoff_048
	      + wsp3->at->dbzoff_048;
	rad_const = wsp1->at->radconst_048 + wsp2->at->radconst_048
	      + wsp3->at->radconst_048;
    }
    else if(pw_micros > 1.80) {
	dbz_off = wsp1->at->dbzoff_182 + wsp2->at->dbzoff_182
	      + wsp3->at->dbzoff_182;
	rad_const = wsp1->at->radconst_182 + wsp2->at->radconst_182
	      + wsp3->at->radconst_182;
    }
    else if(pw_micros > .52) {
	dbz_off = wsp1->at->dbzoff_080 + wsp2->at->dbzoff_080
	      + wsp3->at->dbzoff_080;
	rad_const = wsp1->at->radconst_080 + wsp2->at->radconst_080
	      + wsp3->at->radconst_080;
    }
    else {
	/*  ??? */
	mark = 0;
    }

    /*
     * now loop through the data
     */

    for(nn=gri->num_bins; nn--; ss++,rc++,rr++) {
	if(*rr <= 0)
	      continue;		/* range to this cell is <= 0 */
	ratt = *rr * attenuation;

	if(gri->binary_format == DD_8_BITS ) {
	    if(*cc == ibad)
		  continue;
	    dbi = DD_UNSCALE((float) *cc, rcp_scale, bias) +dbz_off -(*rc)
		  -ratt - rad_const; 
	}
	else {
	    if(*ss == ibad)
		  continue;
	    dbi = DD_UNSCALE(*ss, rcp_scale, bias) +dbz_off -(*rc)
		  -ratt - rad_const;
	}
	dbc = dbi +db_off;
	/*
	 * These data are linear fits to Gerlach tests
	 * Linear fits obtained from CSU
	 * Modified for new correction curve
	 */
	if(dbc < min_curve) {
	    dbc = 0;
	}
	else if(dbc < -20. && dbc >= min_curve) {
	    dbc = log_intercept +log_slope*dbc;
	}
	else if(dbc > -20.) {
	    dbc = max_value;
	}
	else {
	    dbc = 0;
	}
	f = dbi -dbc +(*rc) +ratt +rad_const;
	if(gri->binary_format == DD_8_BITS ) {
	    *cc = (unsigned char)DD_SCALE(f, scale, bias);
	}
	else {
	    *ss = (short) DD_SCALE(f, scale, bias);
	}
    }
    return(1);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */


