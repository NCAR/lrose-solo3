/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */
/*
 * This file contains the following routines
 * 
 * eld_gpro_fix_asib
 * gpro_assemble_vals
 * gpro_decode
 * gpro_header
 * gpro_mount_raw
 * gpro_next_rec
 * gpro_position_err
 * gpro_sync
 * 
 * 
 * 
 * 
 * 
 */

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <fcntl.h>
#include <dd_math.h>
#include <dd_defines.h>
#include "dorade_share.hh"
#include "eldh.h"
#include "gpro_data.hh"
#include "ddb_common.hh"
#include "dd_catalog.hh"

# ifdef NETCDF
# include <netcdf.h>
# endif

# define	ndx_time_offset 1
# define    ndx_HGME 2
# define    ndx_PITCH 3
# define  	ndx_ROLL 4
# define 	ndx_THDG 5
# define 	ndx_VEWC 6
# define 	ndx_VNSC 7
# define 	ndx_VSPD 8
# define 	ndx_ATTACK 9
# define 	ndx_LATC 10
# define 	ndx_LONC 11
# define 	ndx_PALT 12
# define 	ndx_UIC 13
# define 	ndx_VIC 14
# define 	ndx_WIC 15



static char *Project=NULL;
static char *Prdate=NULL;
static char *Prtime=NULL;
static char *Begsnp=NULL;
static char *Endsnp=NULL;
static char *Logbit=NULL;
static char *Datlog=NULL;
static char *Datsiz=NULL;
static char *Vars=NULL;		/* Variables pointer */
static char *Endhd=NULL;

static char *Titles=NULL;	/* Titles pointer */
static char *Units=NULL;	/* Units pointer */
static char *Conkeys=NULL;	/* Conkeys pointer */
static char *tbuf;		/* input tape record buffer */
static char *hbuf;		/* header record buffer */
static char *hd_vbls[MAX_VARS];	/* points the first char of vbl name in header */

static int Proj_yr=0, Proj_mon=0, Proj_day=0;
static int In_lrec_size, In_prec_size, In_lrec_count, Bytes_in=0, In_count=0;
static int ltvi_count=0, gpro_fid = -1, io_type=BINARY_IO;
static int ads_merge=YES;
static int netcdf_merge=YES;
static struct letvar_info *ltvi_top=NULL, *ltvi_list[MAX_VBLS];
static struct ads_raw_data_que *top_rdq=NULL;
static struct ads_data *adsd;
static struct ins_errors *ins_err;
static DD_TIME dtsg;
static char *next_raw_tape=NULL;

static char *Months[] = { (char *)"000", (char *)"JAN", (char *)"FEB",
			  (char *)"MAR", (char *)"APR", (char *)"MAY",
			  (char *)"JUN", (char *)"JUL", (char *)"AUG",
			  (char *)"SEP", (char *)"OCT", (char *)"NOV",
			  (char *)"DEC", (char *)"000" };

static struct nc_ac_info *nai=NULL;
static char *ac_var_tags[MAX_NC_NDX];
static char ac_var_names[MAX_NC_NDX][16];

/* c----------------------------------------------------------------------- */

char *
eld_nimbus_fix_asib (DD_TIME *dts, struct platform_i *asib, int options, int radar_num)

{
    /*
     * Hide the guts of the main routine if NETCDF  not defined
     */

# ifdef NETCDF

//    static int catch=999999, count=0, gap_count=0, unSync=0;
    static int static_catch=999999, count=0, gap_count=0, unSync=0;
    int ii, mark, ndx;
    DD_TIME dtx;
    char *aa, mess[256];
    double max_delta = 1.0001;
    void *vp;
    struct ac_vals *acv;


    if(!netcdf_merge)
	  return (0);

    if(!nai) {
	if(!ac_nc_init()) {
	    netcdf_merge = NO;
	    return (0);
	}
    }
    /* the mission is to try to bracket the time in "dts" and correct
     * the relevant parameters in the platform descriptor
     * 
     * considerations include knowing when to shift to the next file
     * dealing with radar data before there is any nimbus data
     * dealing with a gap in the nimbus data
     */
    for(;;) {
	count++;
	if(nai->next_time > dts->time_stamp) {
	    /* see if we can bracket this time
	     */
	    ndx = -1;
	    acv = nai->ac_var_ptrs[ndx_time_offset]->vals;
	    for(ii=0; ii < AC_NC_VALS_RING; ii++, acv = acv->last) {
		if(acv->delta && acv->delta <= max_delta &&
		   acv->last->time <= dts->time_stamp &&
		   dts->time_stamp < acv->time) {
		    ndx = acv->que_ndx;;
		    if(ii > 0) {
			mark = 0;
		    }
		    break;
		}
	    }
	    if(ndx >= 0) {
		unSync = 0;
		nai->last_radar_time = dts->time_stamp;
		nai->last_radar_num = radar_num;
		break;
	    }
	    if(nai->next_time > dts->time_stamp) {
		if(++unSync == 1) {
		    aa = mess;
		    dtx.time_stamp = nai->next_time;
		    sprintf(aa, "Lost sync with AC data at %s "
			    , dts_print(d_unstamp_time(&dtx)));
		    sprintf(aa+strlen(aa), " radar time: %s  rn: %d"
			    , dts_print(d_unstamp_time(dts)), radar_num);
		    dd_append_cat_comment(mess);
		    printf("%s\n", mess);
		    
		    dtx.time_stamp = nai->reference_time;
		    sprintf(aa, "Previous AC time: %s  "
			    , dts_print(d_unstamp_time(&dtx)));
		    dtx.time_stamp = nai->last_radar_time;
		    sprintf(aa+strlen(aa), "previous radar time: %s  rn: %d"
			    , dts_print(d_unstamp_time(&dtx))
			    , nai->last_radar_num);
		    dd_append_cat_comment(mess);
		    printf("%s\n", mess);
		}
		return (0);
	    }
	}
	if(nai->count >= nai->max_count) {
	    if(!ac_nc_init()) {
		netcdf_merge = NO;
//		return;
		return (0);
	    }
	}
	ac_nc_nab_this_time_step();

	if(!((nai->count-1) % 180)) {
	    dtx.time_stamp = nai->next_time;
	    printf("aircraft time: %s\n", dts_print(d_unstamp_time(&dtx)));
	}
    }
    /* update the asib
     */
    ac_nc_apply_fixes(dts, asib, options, ndx);
    return ((char *)"_MRG");

# endif
    /*
     * ends NETCDF wrapper for the guts of the main routine
     */
}

/*
 * hide all the support routines if NETCDF not defined
 */


# ifdef NETCDF

/* c----------------------------------------------------------------------- */

void 
ac_nc_nab_this_time_step (void)
{
    int ii, ival;
    float fval;
    double d, dval, ref;
    struct nc_var_info *ncvi=nai->vars_top;
    struct ac_vals *acv;
    static int dims[] = {0, 0, 0, 0};
    const int32_t *coords;

    dims[0] = nai->count;
    coords = (const int32_t *)dims;

    for(;  ncvi; ncvi = ncvi->next) {
	acv = ncvi->vals = ncvi->vals->next;
	ii = ncvarget1(nai->nc_fid, ncvi->var_id, coords, acv->vptr);
	switch(ncvi->data_type) {

	case NC_SHORT:
	    acv->dval = *((short *)acv->vptr);
	    break;
	case NC_LONG:
	    acv->dval = *((int32_t *)acv->vptr);
	    break;
	case NC_FLOAT:
	    acv->dval = *((float *)acv->vptr);
	    break;
	case NC_DOUBLE:
	    break;
	default:
	    break;
	}
	acv->delta = nai->count ? acv->dval - acv->last->dval : 0;
    }
    nai->count++;
    nai->reference_time = nai->next_time;
    nai->next_time = nai->base_time +
	  nai->ac_var_ptrs[ndx_time_offset]->vals->dval +
		nai->time_correction;
    nai->delta_time = nai->next_time - nai->reference_time;

    nai->ac_var_ptrs[ndx_time_offset]->vals->time = 
	  nai->ac_var_ptrs[ndx_time_offset]->vals->dval +
		nai->base_time + nai->time_correction;
}
/* c----------------------------------------------------------------------- */

void 
ac_nc_apply_fixes (DD_TIME *dts, struct platform_i *asib, int options, int ndx)
{
    double frac, d, x, y, attack, track, residue;
    struct ac_vals *acv;

    acv = nai->ac_var_ptrs[ndx_time_offset]->vals_ptrs[ndx];

    residue = 1. - (dts->time_stamp - (int32_t)dts->time_stamp);

    if(nai->ac_var_ptrs[ndx_HGME]) {
	asib->altitude_agl =
	      M_TO_KM(nai->ac_var_ptrs[ndx_HGME]->vals_ptrs[ndx]->dval);
    }
    /*
     * values are interpolated to account for distance traveled
     * within the second;
     */
    if(nai->ac_var_ptrs[ndx_PITCH]) {
	d = nai->ac_var_ptrs[ndx_PITCH]->vals_ptrs[ndx]->delta;
	asib->pitch = nai->ac_var_ptrs[ndx_PITCH]->vals_ptrs[ndx]->dval
	      - (residue * d);
    }
    if(nai->ac_var_ptrs[ndx_ROLL]) {
	d = nai->ac_var_ptrs[ndx_ROLL]->vals_ptrs[ndx]->delta;
	asib->roll = nai->ac_var_ptrs[ndx_ROLL]->vals_ptrs[ndx]->dval
	      - (residue * d);
    }
    if(nai->ac_var_ptrs[ndx_THDG]) {
	d = nai->ac_var_ptrs[ndx_THDG]->vals_ptrs[ndx]->delta;
	asib->heading = nai->ac_var_ptrs[ndx_THDG]->vals_ptrs[ndx]->dval
	      - (residue * d);
    }
    if(nai->ac_var_ptrs[ndx_VEWC]) {
	d = nai->ac_var_ptrs[ndx_VEWC]->vals_ptrs[ndx]->delta;
	x = asib->ew_velocity =
	      nai->ac_var_ptrs[ndx_VEWC]->vals_ptrs[ndx]->dval
		    - (residue * d);
    }
    if(nai->ac_var_ptrs[ndx_VNSC]) {
	d = nai->ac_var_ptrs[ndx_VNSC]->vals_ptrs[ndx]->delta;
	y = asib->ns_velocity =
	      nai->ac_var_ptrs[ndx_VNSC]->vals_ptrs[ndx]->dval
		    - (residue * d);
    }
    if(nai->ac_var_ptrs[ndx_VSPD]) {
	asib->vert_velocity = nai->ac_var_ptrs[ndx_VSPD]->vals_ptrs[ndx]->dval;
    }
    if(nai->ac_var_ptrs[ndx_VEWC] &&
       nai->ac_var_ptrs[ndx_VNSC]) {
	track = atan2(y, x);
	track = FMOD360(CART_ANGLE(DEGREES(track)));
	asib->drift_angle = track - asib->heading;
    }
    if(nai->ac_var_ptrs[ndx_LATC]) {
	d = nai->ac_var_ptrs[ndx_LATC]->vals_ptrs[ndx]->delta;
	asib->latitude = nai->ac_var_ptrs[ndx_LATC]->vals_ptrs[ndx]->dval
	      - (residue * d);
    }
    if(nai->ac_var_ptrs[ndx_LONC]) {
	d = nai->ac_var_ptrs[ndx_LONC]->vals_ptrs[ndx]->delta;
	asib->longitude = nai->ac_var_ptrs[ndx_LONC]->vals_ptrs[ndx]->dval
	      - (residue * d);
    }
    if(nai->ac_var_ptrs[ndx_PALT]) {
	asib->altitude_msl = M_TO_KM
	      (nai->ac_var_ptrs[ndx_PALT]->vals_ptrs[ndx]->dval);
    }
    if(nai->ac_var_ptrs[ndx_UIC]) {
	asib->ew_horiz_wind = nai->ac_var_ptrs[ndx_UIC]->vals_ptrs[ndx]->dval;
    }
    if(nai->ac_var_ptrs[ndx_VIC]) {
	asib->ns_horiz_wind = nai->ac_var_ptrs[ndx_VIC]->vals_ptrs[ndx]->dval;
    }
    if(nai->ac_var_ptrs[ndx_WIC]) {
	asib->vert_wind = nai->ac_var_ptrs[ndx_WIC]->vals_ptrs[ndx]->dval;
    }
    return;
}
/* c------------------------------------------------------------------------ */

int 
ac_nc_find_tag (char *tag)
{
    int jj;

    for(jj=0; jj < MAX_NC_NDX; jj++) {
	if(ac_var_tags[jj] && strcmp(ac_var_tags[jj], tag) == 0) {
	    return(jj);
	}
    }
    return(-1);
}
/* c------------------------------------------------------------------------ */

int 
ac_nc_init (void)
{
    struct nc_var_info *ncvi, *ncvi_last;
    struct ac_vals *acv, *last_acv;
    void *vptr;
    int ii, jj, kk, nn, ndx, nt;
    int32_t ll;
    float f;
    char *aa, *bb, *cc, *tag, name[256];
    char var_name[16], string_space[256], *str_ptrs[66];
    static int dims[] = {0, 0, 0, 0}, ndims, natts;
    const int32_t *coords;
    nc_type data_type;


    if(!nai) {			/* first time through
				 */
        if(!(aa=get_tagged_string("AC_NETCDF_FILES"))) {
	    return(NO);
	}
	for(jj=0; jj < MAX_NC_NDX; jj++) {
	    ac_var_names[jj][0] = '\0';
	    ac_var_tags[jj] = NULL;
	}
	/* start out with a default set of variable names and indices
	 */
	ndx = ndx_time_offset;
	ac_var_tags[ndx] = (char *)"time_offset";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

	ndx = ndx_HGME;
	ac_var_tags[ndx] = (char *)"HGME";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

	ndx = ndx_PITCH;
	ac_var_tags[ndx] = (char *)"PITCH";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

	ndx = ndx_ROLL;
	ac_var_tags[ndx] = (char *)"ROLL";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

	ndx = ndx_THDG;
	ac_var_tags[ndx] = (char *)"THDG";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

	ndx = ndx_VEWC;
	ac_var_tags[ndx] = (char *)"VEWC";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

	ndx = ndx_VNSC;
	ac_var_tags[ndx] = (char *)"VNSC";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

	ndx = ndx_VSPD;
	ac_var_tags[ndx] = (char *)"VSPD";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

# ifdef obsolete
	ndx = ndx_ATTACK;
	ac_var_tags[ndx] = "ATTACK";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);
# endif
	ndx = ndx_LATC;
	ac_var_tags[ndx] = (char *)"LATC";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

	ndx = ndx_LONC;
	ac_var_tags[ndx] = (char *)"LONC";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

	ndx = ndx_PALT;
	ac_var_tags[ndx] = (char *)"PALT";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

	ndx = ndx_UIC;
	ac_var_tags[ndx] = (char *)"UIC";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

	ndx = ndx_VIC;
	ac_var_tags[ndx] = (char *)"VIC";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);

	ndx = ndx_WIC;
	ac_var_tags[ndx] = (char *)"WIC";
	strcpy(ac_var_names[ndx], ac_var_tags[ndx]);
	/*
	 * see if we have any name changes
	 */
	if(aa=get_tagged_string("AC_NETCDF_ALIASES")) {
	    strcpy(string_space, aa);
	    ii = 0;
	    if(nt = dd_tokens(string_space, str_ptrs)) {
		if(strstr(str_ptrs[0], "ONLY")) {
		    for(jj=0; jj < MAX_NC_NDX; jj++) {
			if(jj != ndx_time_offset)
			      *ac_var_names[jj] = '\0';
		    }
		    ii = 1;
		}
	    }
	    /*
	     * process the ordered triplets of tag, seperater
	     * and replacement name
	     */
	    for(; ii+2 < nt; ii += 3) {
		/* tag name is first
		 */
		tag = str_ptrs[ii];
		if((jj = ac_nc_find_tag(tag)) >= 0) {
		    /* tags match, replace with substitute name
		     */
		    strcpy(ac_var_names[jj], str_ptrs[ii+2]);
		}
# ifdef obsolete
		for(jj=0; jj < MAX_NC_NDX; jj++) {
		    if(!strlen(ac_var_names[jj])) {
			continue;
		    }
		    if(!strcmp(ac_var_tags[jj], tag)) {
			/* tags match, replace with substitute name
			 */
			strcpy(ac_var_names[jj], str_ptrs[ii+2]);
		    }
		}
# endif
	    }
	}
	/* establish the control struct
	 */
	nai = (struct nc_ac_info *)malloc(sizeof(struct nc_ac_info));
	memset(nai, 0, sizeof(struct nc_ac_info));
	
	for(ii=0; ii < MAX_NC_NDX; ii++) {
	    nai->ac_var_ptrs[ii] = NULL;
	    if(!strlen(ac_var_names[ii]) || !strcmp("NOT", ac_var_names[ii])) {
		continue;
	    }
	    /* setup a struct for this variable
	     */
	    ncvi = ( struct nc_var_info*)malloc(sizeof(struct nc_var_info));
	    memset(ncvi, 0, sizeof(struct nc_var_info));
	    if(!nai->vars_top) {
		nai->vars_top = ncvi;
	    }
	    else {
		ncvi->last = ncvi_last;
		ncvi->last->next = ncvi;
	    
	    }
	    nai->vars_top->last = ncvi;
	    ncvi_last = ncvi;
	    ncvi->var_ndx = ii;
	    nai->ac_var_ptrs[ii] = ncvi;
	}
	/* set up the file names list
	 */
	if(aa=get_tagged_string("AC_NETCDF_FILES")) {
	    strcpy(nai->file_names, aa);
	    nai->file_count = dd_tokens(nai->file_names, nai->file_names_ptrs);
	    if(!nai->file_count) {
		return(NO);
	    }
	}
	if(aa=get_tagged_string("AC_TIME_CORRECTION")) {
	    if((ii = sscanf(aa, "%f", &f)) == 1) {
		nai->time_correction = f;
		printf("ac_netCDF time correction: %.3f seconds!\n"
		       , nai->time_correction);
	    }
	}
    }


    /*
     * for every new netCDF file
     */

    if(nai->file_ndx >= nai->file_count) {
	printf("\n**** Exhaused the netCDF file names list\n");
	exit(1); /* return(NO); */
    }
    aa = nai->file_names_ptrs[nai->file_ndx++];
    if(bb = strrchr(aa, '/')) {
	/* save the directory name
	 */
	cc = nai->file_directory;
	for(++bb; aa < bb; *cc++ = *aa++); *cc = '\0';
    }
    strcpy(name, nai->file_directory);
    strcat(name, aa);
    printf("Opening ac file: %s\n", name);
    if((nai->nc_fid = ncopen(name, NC_NOWRITE)) < 0) {
	return(NO);
    }
    coords = (const int32_t *)dims;
    ii = ncvarid(nai->nc_fid, "base_time");
    ii = ncvarget1(nai->nc_fid, ii, coords, (void *)&jj);
    nai->base_time = jj;

    ii = ncdimid(nai->nc_fid, "Time");
    kk = ncdiminq(nai->nc_fid, ii, (char *)0, &ll); 
    nai->max_count = ll;
    nai->count = 0;

    /*
     * setup to receive the actual values
     */
    ncvi = nai->vars_top;
    for(; ncvi; ncvi = ncvi->next) {
	if(!ncvi->vals) {
	    /* construct a ring of ac parameter values
	     */
	    for(ii=0; ii < AC_NC_VALS_RING; ii++) {
		acv = (struct ac_vals *)malloc(sizeof(struct ac_vals));
		memset(acv, 0, sizeof(struct ac_vals));
		if(!ii) {
		    ncvi->vals = acv;
		}
		else {
		    acv->last = ncvi->vals->last;
		    acv->last->next = acv;
		}
		ncvi->vals->last = acv;
		acv->next = ncvi->vals;
		acv->que_ndx = ii;
		ncvi->vals_ptrs[ii] = acv;
	    }
	}
	/* find this variable in the netCDF file
	 */
	ncvi->var_id = ncvarid(nai->nc_fid, ac_var_names[ncvi->var_ndx]);
	ii = ncvarinq(nai->nc_fid, ncvi->var_id
		      , ncvi->var_name, &data_type
		      , &ncvi->ndims, ncvi->dims, &ncvi->natts);
	ncvi->data_type = data_type;
	acv = ncvi->vals;
	
	for(ii=0; ii < AC_NC_VALS_RING; ii++) {
	    switch(data_type) {
	    case NC_SHORT:
		acv->vptr = (void *)&acv->sval;
		break;
	    case NC_LONG:
		acv->vptr = (void *)&acv->lval;
		break;
	    case NC_FLOAT:
		acv->vptr = (void *)&acv->fval;
		break;
	    case NC_DOUBLE:
		acv->vptr = (void *)&acv->dval;
		break;
	    default:
		break;
	    }
	    acv = acv->next;
	}
    }
    return(YES);
}

# endif
/*
 * end of ifdef NETCDF surrounding all the support routines
 */

/* c----------------------------------------------------------------------- */
/* c...mark */

# ifdef notyet

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

    fn = "/scr/stout/oye/rf32a";
    if(a=getenv("ADS_TAPES")) {
	fn = a;
    }
    next_raw_tape = fn;
    gpro_fid = gpro_mount_raw();
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
	doit = adsd->itime >= 184600;
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
"gpro %06d %.2f %5.1f %5.1f %6.2f %.4f %.4f %4.0f %4.0f %5.1f %4.0f %4.0f %5.1f %5.1f\n"
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
			 );
# endif
	      }
    }
}
# endif
/* c------------------------------------------------------------------------ */

void 
eld_gpro_fix_asib (DD_TIME *dts, struct platform_i *asib, int options)
{
    static int count=0, trip_count=500;
    struct ads_raw_data_que *rdq;
    float delta;
    int i, status, mark;
    double palt_corr, d;

    if(!ads_merge)
	  return;

    if(!(count++ % trip_count)) {
	mark = 0;
    }
    if(!hbuf) {
	adsd = (struct ads_data *)malloc(sizeof(struct ads_data));
	hbuf = (char *)malloc(BLOCK_SIZE);
	tbuf = (char *)malloc(BLOCK_SIZE);
	dtsg.time_stamp = dts->time_stamp;
	d_unstamp_time(&dtsg);

	if(!(next_raw_tape=get_tagged_string("GPRO_TAPES"))) {
	    printf("No Genpro tapes list\n");
	    ads_merge = NO;
	    return;
	}
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

    asib->longitude = adsd->alon - ins_err->alon_err;
    asib->latitude = adsd->alat - ins_err->alat_err;
    asib->altitude_msl = adsd->palt*.001; /* make km. */
    asib->altitude_agl = adsd->hgme*.001; /* make km. */
    if(options & TOGACOARE_FIXES) {
	/* pressure altitude correction for TOGA-COARE */
	palt_corr = 0.062*asib->altitude_msl -0.0232;
	d = asib->altitude_msl -asib->altitude_agl;
	if( fabs(d) > 2.0*fabs(palt_corr))
	      asib->altitude_agl = asib->altitude_msl +palt_corr;
	asib->altitude_msl = asib->altitude_agl;
    }
    asib->ew_velocity = adsd->vew;
    asib->ns_velocity = adsd->vns;
    asib->vert_velocity = adsd->ivspd;
    asib->heading = adsd->thi;
    asib->roll = adsd->roll;
    asib->pitch = adsd->pitch;
    asib->drift_angle = adsd->drift;
    asib->ew_horiz_wind = adsd->ui;
    asib->ns_horiz_wind = adsd->vi;
    asib->vert_wind = adsd->wi;
}
/* c------------------------------------------------------------------------ */

void 
gpro_assemble_vals (struct ads_raw_data_que *rdq, struct ads_data *adsd, double pct)
{
    int hr, min, sec;
    double d, track;
    /*
     * assemble the requested values for the sample
     * this involves reading the records that will bracket the time
     * usually an interpolation between the two records that bracket the time
     */
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
    adsd->palt = gpro_decode(ltvi_list[GPRO_ID_PALT]
			      , pct, rdq->at[GPRO_ID_PALT]);
    adsd->hgme = gpro_decode(ltvi_list[GPRO_ID_HGME]
			      , pct, rdq->at[GPRO_ID_HGME]);
# ifdef obsolete
    adsd->alat2 = gpro_decode(ltvi_list[GPRO_ID_ALAT2]
			      , pct, rdq->at[GPRO_ID_ALAT2]);
    adsd->alon2 = gpro_decode(ltvi_list[GPRO_ID_ALON2]
			      , pct, rdq->at[GPRO_ID_ALON2]);
    adsd->galt = gpro_decode(ltvi_list[GPRO_ID_GALT]
			      , pct, rdq->at[GPRO_ID_GALT]);
    adsd->gvns = gpro_decode(ltvi_list[GPRO_ID_GVNS]
			      , pct, rdq->at[GPRO_ID_GVNS]);
    adsd->gvew = gpro_decode(ltvi_list[GPRO_ID_GVEW]
			      , pct, rdq->at[GPRO_ID_GVEW]);
# endif
    adsd->glat = gpro_decode(ltvi_list[GPRO_ID_GLAT]
			      , pct, rdq->at[GPRO_ID_GLAT]);
    adsd->glon = gpro_decode(ltvi_list[GPRO_ID_GLON]
			      , pct, rdq->at[GPRO_ID_GLON]);
    adsd->vns = gpro_decode(ltvi_list[GPRO_ID_VNS]
			      , pct, rdq->at[GPRO_ID_VNS]);
    adsd->vew = gpro_decode(ltvi_list[GPRO_ID_VEW]
			      , pct, rdq->at[GPRO_ID_VEW]);
    if(adsd->vns) {
	d = atan2((double)adsd->vns, (double)adsd->vew);
	track = fmod(((double)450. -DEGREES(d)), 360.);
	adsd->drift = track -adsd->thi;
	if(adsd->drift < -180.)
	      adsd->drift += 360.;
	else if(adsd->drift > 180.)
	      adsd->drift -= 360.;
    }
    else {
	adsd->drift = 0.;
    }

    adsd->ivspd = gpro_decode(ltvi_list[GPRO_ID_IVSPD]
			      , pct, rdq->at[GPRO_ID_IVSPD]);
    adsd->ui = gpro_decode(ltvi_list[GPRO_ID_UI]
			      , pct, rdq->at[GPRO_ID_UI]);
    adsd->vi = gpro_decode(ltvi_list[GPRO_ID_VI]
			      , pct, rdq->at[GPRO_ID_VI]);
    adsd->wi = gpro_decode(ltvi_list[GPRO_ID_WI]
			      , pct, rdq->at[GPRO_ID_WI]);
    adsd->ptime = (int)gpro_decode(ltvi_list[GPRO_ID_PTIME]
			      , pct, rdq->at[GPRO_ID_PTIME]);
    hr = (int)gpro_decode(ltvi_list[GPRO_ID_HR]
			      , pct, rdq->at[GPRO_ID_HR]);
    min = (int)gpro_decode(ltvi_list[GPRO_ID_MIN]
			      , pct, rdq->at[GPRO_ID_MIN]);
    sec = (int)gpro_decode(ltvi_list[GPRO_ID_SEC]
			      , pct, rdq->at[GPRO_ID_SEC]);
    adsd->ptime = (int)gpro_decode(ltvi_list[GPRO_ID_PTIME]
			      , pct, rdq->at[GPRO_ID_PTIME]);
    dtsg.julian_day=0;
    dtsg.day_seconds = adsd->ptime;
    adsd->time = rdq->time = d_time_stamp(&dtsg);

    adsd->itime = hr*10000 +min*100 +sec;

    return;
}
/* c------------------------------------------------------------------------ */

void 
setup_ltvi (void)
{

    char *a;
    int i, j, m, n, count=0, its=GOOD, first_time, mark;
    double d;
    
    struct letvar_info *ltvi, *ltvi_last;
    struct ads_raw_data_que *rdq, *last_rdq;
    struct running_avg *ravg, *lastravg;
    char str[16];
    char *vlist[MAX_VBLS];
    int gpro_ids[MAX_VBLS];

    dtsg.day_seconds = dtsg.julian_day = 0;
    dtsg.year = Proj_yr+1900;
    dtsg.month = Proj_mon;
    dtsg.day = Proj_day;
    d = d_time_stamp(&dtsg);

    i = 0;
    gpro_ids[i] = GPRO_ID_HR;
    vlist[i] = (char *)"HR";
    i++;
    gpro_ids[i] = GPRO_ID_MIN;
    vlist[i] = (char *)"MIN";
    i++;
    gpro_ids[i] = GPRO_ID_SEC;
    vlist[i] = (char *)"SEC";
    i++;
    gpro_ids[i] = GPRO_ID_PITCH;
    vlist[i] = (char *)"PITCH";
    i++;
    gpro_ids[i] = GPRO_ID_ROLL;
    vlist[i] = (char *)"ROLL";
    i++;
    gpro_ids[i] = GPRO_ID_THI;
    vlist[i] = (char *)"THI";
    i++;
    gpro_ids[i] = GPRO_ID_ALAT;
    vlist[i] = (char *)"ALAT";
    i++;
    gpro_ids[i] = GPRO_ID_ALON;
    vlist[i] = (char *)"ALON";
    i++;
    gpro_ids[i] = GPRO_ID_GSTAT;
    vlist[i] = (char *)"GSTAT";
    i++;
    gpro_ids[i] = GPRO_ID_GLAT;
    vlist[i] = (char *)"GLAT";
    i++;
    gpro_ids[i] = GPRO_ID_GLON;
    vlist[i] = (char *)"GLON";
    i++;
# ifdef obsolete
    gpro_ids[i] = GPRO_ID_GMOD1;
    vlist[i] = "GMOD1";
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
    gpro_ids[i] = GPRO_ID_GVEW;
    vlist[i] = "GVEW";
    i++;
    gpro_ids[i] = GPRO_ID_GVNS;
    vlist[i] = "GVNS";
    i++;
    gpro_ids[i] = GPRO_ID_SSRD;
    vlist[i] = "SSRD";
    i++;
# endif
    gpro_ids[i] = GPRO_ID_VEW;
    vlist[i] = (char *)"VEW";
    i++;
    gpro_ids[i] = GPRO_ID_VNS;
    vlist[i] = (char *)"VNS";
    i++;
    gpro_ids[i] = GPRO_ID_UI;
    vlist[i] = (char *)"UI";
    i++;
    gpro_ids[i] = GPRO_ID_VI;
    vlist[i] = (char *)"VI";
    i++;
    gpro_ids[i] = GPRO_ID_PALT;
    vlist[i] = (char *)"PALT";
    i++;
    gpro_ids[i] = GPRO_ID_HGME;
    vlist[i] = (char *)"HGME";
    i++;
    gpro_ids[i] = GPRO_ID_IVSPD;
    vlist[i] = (char *)"IVSPD";
    i++;
    gpro_ids[i] = GPRO_ID_WI;
    vlist[i] = (char *)"WI";
    i++;
    gpro_ids[i] = GPRO_ID_PTIME;
    vlist[i] = (char *)"PTIME";
    i++;


    ltvi_count = i;

    if(!ltvi_top) {
	/* initialize structures */
	ltvi = ltvi_top = (struct letvar_info *)malloc
	      (ltvi_count*sizeof(struct letvar_info));
	memset((char *)ltvi_top, 0, ltvi_count*sizeof(struct letvar_info));
	for(i=0; i < ltvi_count; i++,ltvi++) {
	    if(i) {
		ltvi_last->next = ltvi;
		ltvi->last = ltvi_last;
	    }
	    ltvi_last = ltvi;
	    ltvi->next = ltvi_top;
	    ltvi_top->last = ltvi;
	}
	/* set up the lat/lon error structures
	 */
	ins_err = (struct ins_errors *)malloc(sizeof(struct ins_errors));
	memset((char *)ins_err, 0, sizeof(struct ins_errors));
	
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
    count = 0;

    for(ltvi=ltvi_top,i=0; i < ltvi_count; i++,ltvi++) {
	strcpy(ltvi->name, vlist[i]);
	/* pad out the name with blanks
	 */
	for(n=strlen(ltvi->name); n < VAR_LEN; ltvi->name[n++] = ' ');
	ltvi->name[n] = '\0';

	if((j = is_gp_var( ltvi->name))) {
	    ltvi->letvar = Units +j*LINE_LEN;

	    a = gp_arg_ptr(ltvi->letvar, FSTBIT);
	    m = len_arg( a, ltvi->letvar+LINE_LEN );
	    strncpy(str,a,m);
	    str[m] = '\0';
	    ltvi->offset = atoi(a)/8;
		  
	    a = gp_arg_ptr(ltvi->letvar, SAMPLE);
	    m = len_arg( a, ltvi->letvar+LINE_LEN );
	    strncpy(str,a,m);
	    str[m] = '\0';
	    ltvi->sample = atoi(a);
		  
	    /* be able to reference this struct by variable id num */
	    ltvi->id_num = gpro_ids[i];
	    ltvi_list[gpro_ids[i]] = ltvi;
	}
	else {
	    its = BAD;
	    printf( "Could not locate variable \"%s\" in input data\n"
		   , ltvi->name);
	}
    }
    if(its == BAD)
	  exit(1);

    first_time = top_rdq == NULL;
    /*
     * set up the raw data que
     */
    for(j=0; j < MAX_RDQ; j++) {
	if(first_time) {
	    rdq = (struct ads_raw_data_que *)
		  malloc(sizeof(struct ads_raw_data_que));
	    memset((char *)rdq, 0, sizeof(struct ads_raw_data_que));
	    if(!j) {
		top_rdq = rdq;
	    }
	    else {
		last_rdq->next = rdq;
		rdq->last = last_rdq;
	    }
	    rdq->raw_data_buf = (char *)malloc(BLOCK_SIZE);
	    top_rdq->last = rdq;
	    rdq->next = top_rdq;
	    last_rdq = rdq;
	}
	else if(!j)
	      rdq = top_rdq;
	else
	      rdq = rdq->next;

	for(i=0,ltvi=ltvi_top; i < ltvi_count; i++,ltvi=ltvi->next) {
	    rdq->at[ltvi->id_num] = (int32_t *)(rdq->raw_data_buf +ltvi->offset);
	}
    }
}
/* c------------------------------------------------------------------------ */

int 
gpro_mount_raw (void)
{
    int n;
    char *a, *b, *c, str[256];

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
    gpro_fid = n;
    gp_header(hbuf);
# ifdef obsolete
    if(!ltvi_top)
# endif
	  setup_ltvi();
    return(n);
}
/* c------------------------------------------------------------------------ */

void 
gp_header (char *buf)
{
    char *a, *s=buf;
    char tmp[88];
    int i, m, n, count=0;
    /*
     * load in the header and establish pointers to critical info
     * 
     */

    Project = Prdate = Prtime = Begsnp = Endsnp = Logbit = Datlog = NULL;
    Datsiz = Vars = Endhd = Titles = Units = Conkeys = NULL;     

    for(;;) {
	count++;
	n = gp_get_line( s );

	if( n <= 0 || count > 5555 ) {
	    printf( "gp_header--break! n,count=%d,%d\n", n, count );
	    break;
	}

	if( !Endhd && strncmp( s, " ENDHD", strlen(" ENDHD")) == 0 ) {
	    Endhd = s;
	    break;
	}
	if( !Project && strncmp( s, "/PROJECT", strlen("/PROJECT")) == 0 ) {
	    Project = s;
	    strncpy( tmp, s, LINE_LEN );
	    tmp[LINE_LEN-2] = '\0';
	    printf( "## %s\n", tmp );
	}

	if( !Prdate && strncmp( s, "/PRDATE", strlen("/PRDATE")) == 0 ) {
	    Prdate = s;
	    strncpy( tmp, s, LINE_LEN );
	    tmp[LINE_LEN-2] = '\0';
	    printf( "## %s\n", tmp );
	    gp_date( Prdate, &Proj_yr, &Proj_mon, &Proj_day );
	}
	if( !Prtime && strncmp( s, "/PRTIME", strlen("/PRTIME")) == 0 ) {
	    Prtime = s;
	}
	if( !Begsnp && strncmp( s, "/BEGSNP", strlen("/BEGSNP")) == 0 ) {
	    Begsnp = s;
	    strncpy( tmp, s, LINE_LEN );
	    tmp[LINE_LEN-2] = '\0';
	    printf( "## %s\n", tmp );
	}
	if( !Endsnp && strncmp( s, "/ENDSNP", strlen("/ENDSNP")) == 0 ) {
	    Endsnp = s;
	    strncpy( tmp, s, LINE_LEN );
	    tmp[LINE_LEN-2] = '\0';
	    printf( "## %s\n", tmp );
	}
	if( !Logbit && strncmp( s, " LOGBIT", strlen(" LOGBIT")) == 0 ) {
	    Logbit = s;
	    a = gp_arg_ptr(Logbit, 1);
	    m = len_arg( a, Logbit+LINE_LEN );
	    *(a+m) = '\0';
	    In_lrec_size = atoi(a)/8;
	}
	if( !Datlog && strncmp( s, " DATLOG", strlen(" DATLOG")) == 0 ) {
	    Datlog = s;
	    a = gp_arg_ptr(Datlog, 1);
	    m = len_arg( a, Datlog+LINE_LEN );
	    *(a+m) = '\0';
	    In_lrec_count = atoi(a);
	}
	if( !Datsiz && strncmp( s, " DATSIZ", strlen(" DATSIZ")) == 0 ) {
	    Datsiz = s;
	    a = gp_arg_ptr(Datsiz, 1);
	    m = len_arg( a, Datsiz+LINE_LEN );
	    *(a+m) = '\0';
	    In_prec_size = atoi(a)/8;
	}
	if( !Vars && strncmp( s, "/VARIABLES", strlen("/VARIABLES")) == 0 ) {
	    Vars = s;
	}
	if( !Titles && strncmp( s, " ORDVAR = TITLE"
			     , strlen(" ORDVAR = TITLE")) == 0 ) {
	    Titles = s;
	}
	if( !Units && strncmp( s, " ORDVAR = UNITS"
				, strlen(" ORDVAR = UNITS")) == 0 ) {
	    Units = s;
	}
	if( !Conkeys && strncmp( s, " ORDVAR = CONKEY"
				, strlen(" ORDVAR = CONKEY")) == 0 ) {
	    Conkeys = s;
	}
	s += n;
    }
    /* round out the reads to a multiple of 10 */
    for(; count++ % 10;) {
	n = gp_get_line( s );
	s += n;
    }
    Bytes_in = (count-1)*LINE_LEN;

    printf( "Read %d characters from header\n", Bytes_in);
    /*
     * set up pointers to and count the variable names and descriptions
     */
    for(In_count=0,a=Titles +LINE_LEN; a < Units; a+=LINE_LEN ) {
	if( hd_vbls[In_count] = gp_arg_ptr( a, TNAME ) ) {
	    In_count++;
	}
    }
}
 /* c---------------------------------------------------------------------- */

int 
is_gp_var (char *name)
{
    /* is this variable in the input */
    int i, k;
    char *a, *b;

    for( i=0; i < In_count; i++ ) {
	a = name;
	b = hd_vbls[i];
	for( k=0; k < VAR_LEN; k++, a++, b++ )
	      if( *a != *b )
		    break;
	if( k == VAR_LEN )
	      return(i+1);
    }
    return(0);
}
/* c------------------------------------------------------------------------ */

int 
len_arg (char *a, char *b)
{
    /*
     * determine the length of the argument beginning at "a"
     *
     * argument is terminated by a second double quote, whitespace
     * or a comma
     */
    int i, dq='"';

    for(i=0; a < b; a++,i++) {
	if( *a == dq ) {	/* skip to next dq */
	    for( i++,a++; a < b; a++, i++ )
		  if( *a == dq ) 
			return(++i);
	}
	else if( *a == ' ' || *a == '\t' || *a == ',' )
	      break;
    }
    return(i);
}
/* c------------------------------------------------------------------------ */

int 
gp_get_line (			/* CRAY version */
    char *s
)
{
    int i, n, rlen=LINE_LEN;

    n = read(gpro_fid, s, rlen);
    return(n);
}
/* c------------------------------------------------------------------------ */

void 
gp_date (char *s, int *year, int *mon, int *day)
{
    /*
     * unpack the genpro date
     */
    char *t;
    int i;

    for(; *s++ != '"'; );	/* first double quote */
    for(i=0,t=s; *t++ != '"'; i++ ); /* second double quote*/
    *(s+i) = '\0';
    *day = atoi(s);

    for(s=t; *s++ != '"'; );
    for(t=s; *t++ != '"'; );
    for( *mon=1; *mon < 13; (*mon)++ )
	  if( strncmp( Months[*mon], s, 3 ) == 0 )
		break;

    for(s=t; *s++ != '"'; );
    for(i=0,t=s; *t++ != '"'; i++ );
    *(s+i) = '\0';
    *year = atoi(s);
}
/* c------------------------------------------------------------------------ */

float 
gp_time (char *s)
{
    /*
     * unpack the genpro time
     */
    char *c;
    int i;
    float x;

    for(; *s++ != '('; );	/* first paren */

    for(i=0,c=s; *c++ != ','; i++ ); /* first comma */
    *(s+i) = '\0';
    x = (float)atoi(s)*60;

    for(i=0,s=c; *c++ != ','; i++ );
    *(s+i) = '\0';
    x = (x +(float)atoi(s))*60.;

    for(i=0,s=c; *c++ != ')'; i++ ); /* last paren */
    *(s+i) = '\0';
    return(x+(float)atoi(s));	/* return total seconds */
}
/* c------------------------------------------------------------------------ */

char *
gp_arg_ptr (char *s, int nth)
{
    /*
     * return a pointer to the nth argument
     */
    int i;
    char *a=s, *b=s+LINE_LEN, d[1];
    /*
     * d is the argument delimeter
     */
    for(*d='=',i=0; i < nth; i++, *d=',' ) {
	for(; a < b; a++ ) {
	    if( *a == '"' )	/* move to the next '"' */
		  for( ++a; a < b && *a != '"'; a++ );
	    if( *a == *d )
		  break;
	}
	if( a == b )
	      return(NULL);
	a++;
    }
    /* skip to first nonblank character */
    for(; a < b && (*a == ' ' || *a == '\t'); a++ );
    if( a == b )
	  return(NULL);
    else
	  return( a );
}
/* c------------------------------------------------------------------------ */

double 
gpro_decode (struct letvar_info *this_info, double pct_samp, int32_t *at)
{
    int sample=(int)(this_info->sample*pct_samp);
    double d;
    
    d = GP_UNSCALE(*(at+sample));
    return(d);
}
/* c------------------------------------------------------------------------ */

char *
gpro_next_rec (struct ads_raw_data_que *rdq, int *status)
{
    /* This routine returns a pointer to the start of then next
     * logical record of ads data
     */
    int i, m, n=0, mark;
    static int lrec_num=0, count=0, trip=10;
    double d;

    count++;
    *status = n =
	  gp_read(gpro_fid, rdq->raw_data_buf, In_lrec_size, io_type);

    if(n < In_lrec_size) {
	printf("GPRO end of file at: %d  status: %d\n", count, n);
	return(NULL);
    }
    if(++lrec_num == In_lrec_count) {
	lrec_num = 0;
	if((m = In_prec_size -In_lrec_count*In_lrec_size) > 0) {
	    /* we have to move over the extra fill bytes */
            mark = lseek(gpro_fid, m, SEEK_CUR);
	}
    }
    gpro_assemble_vals(rdq, adsd, 0);
    gpro_position_err(adsd);
    return(rdq->raw_data_buf);
}
/* c------------------------------------------------------------------------ */

void 
gpro_position_err (struct ads_data *adsd)
{
    /* keep the average error in lat/lon by
     * keeping a running average of n seconds of non-sever roll data
     */

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
    struct ads_raw_data_que *this_que;  //Jul 26, 2011 *this issue
    static int count=0, trip=100, no_count=0, keep_trying=YES, sync_count=0;
    int where=AFTER, status, mark;
    DD_TIME dts;
    char str[32];
    
    if(keep_trying) {
	for(;;) {
	    if(!(count % trip)) {
		mark = 0;
	    }
	    /* try to keep reading in records until we have
	     * bracketed the requested time
	     */
	    for(this_que=top_rdq;;) { /* loop through the que */
		if(time >= this_que->time && time < this_que->time +1.) {
		    /* we have bracketed the time requested! */
		    if(!(sync_count++ % 1500)) {
			dts.time_stamp = time;
			d_unstamp_time(&dts);
			printf("  ADS at %.2f sync'd with %.2f  %s\n"
			       , this_que->time, time, dts_print(&dts));
		    }
		    return(this_que);
		}
		else if(this_que->time < time) {
		    where = BEFORE;
		}
		else {
		    where = AFTER;
		}
		if((this_que=this_que->last) == top_rdq) {
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
	printf("  No ADS sync for time: %s\n", dts_print(&dts));
    }
    return(NULL);
}
/* c----------------------------------------------------------------------- */

struct ads_raw_data_que *
gpro_initialize (struct ads_data **adsd_at)
{

    if(!hbuf) {
	*adsd_at = adsd = (struct ads_data *)malloc(sizeof(struct ads_data));
	hbuf = (char *)malloc(BLOCK_SIZE);
	tbuf = (char *)malloc(BLOCK_SIZE);

	if(!(next_raw_tape=get_tagged_string("GPRO_TAPES"))) {
	    printf("No Genpro tapes list\n");
	    ads_merge = NO;
	    return(NULL);
	}
	/* mount the first raw tape */
	if((gpro_fid = gpro_mount_raw()) < 0) {
	    printf("Unable to mount first raw tape: %s %d\n"
		   , next_raw_tape, gpro_fid);
	    exit(1);
	}
    }
    return(top_rdq);
}
/* c------------------------------------------------------------------------ */

/* c----------------------------------------------------------------------- */

