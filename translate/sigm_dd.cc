/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */
/*
 * This file contains the following routines
 * 
 * 
 * sigmet_data_types
 * sigmet_dd_conv
 * sigmet_gen_luts
 * sigmet_grab_header
 * sigmet_ini
 * sigmet_nab_data
 * sigmet_nab_info
 * sigmet_next_ray
 * sigmet_select_ray
 * sigmet_time_stamp
 * sigmet_update_header
 * 
 * 
 */

#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <dd_math.h>
#include <generic_radar_info.h>
#include "sigh.h"
#include "dorade_share.hh"
#include "input_limits.hh"
#include "dd_stats.h"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "gneric_dd.hh"
#include "sigm_dd.hh"
#include "xwsrqc.hh"

# define TIMEX 724000000

static char *tape_buf=NULL;

static struct ingest_summary *is=NULL;
static struct generic_radar_info *gri;
static struct sigmet_useful_items *sui;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats=NULL;


/* c------------------------------------------------------------------------ */

void 
sigmet_dd_conv (void)
{
    int i, n, count=0, mark;
 
    /* initialize and read in first ray
     */
    sigmet_ini();

    /* loop through the data
     */
    while(1){
	count++;

	if(difs->stop_flag == YES && sui->new_sweep) {
	    printf("Stop time!\n");
	    sigmet_print_stat_line(count);
	    break;
	}
	
	if(sigmet_select_ray()) {

	    sigmet_nab_data();
	    dd_stuff_ray();	/* pass it off to dorade code */
	}
	else if(!(count % 1000)) {
	    sigmet_print_stat_line(count);
	}
	sui->new_vol = NO;
	sui->new_sweep = NO;
	sui->last_time = gri->time;

	if((n = sigmet_next_ray()) < 1) {
	    printf("Break on input status: %d\n", n);
	    break;
	}
    }
    /* c...mark */
}
/* c------------------------------------------------------------------------ */

void 
sigmet_ini (void)
{
    int i, n, mark;
    char *a;
    struct sig_rec_struct *rsq, *last_rsq;

    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();

    is = (struct ingest_summary *)malloc(sizeof(struct ingest_summary));

    gri = return_gri_ptr();

    sui = (struct sigmet_useful_items *)
	  malloc(sizeof(struct sigmet_useful_items));
    memset((char *)sui, 0, sizeof(struct sigmet_useful_items));
    
    for(i=0; i < RAW_REC_QUE_SIZE; i++) {
	rsq = (struct sig_rec_struct *)malloc(sizeof(struct sig_rec_struct));
	memset((char *)rsq, 0, sizeof(struct sig_rec_struct));
	rsq->rec_buf = (char *)malloc(BLOCK_SIZE);
	rsq->rph = (struct raw_prod_bhdr *)rsq->rec_buf;
	rsq->sh = (struct structure_header *)rsq->rec_buf;

	if(!i)
	      sui->top_rsq = rsq;
	else {
	    last_rsq->next = rsq;
	    rsq->last = last_rsq;
	}
	last_rsq = rsq;
	sui->top_rsq->last = rsq;
	rsq->next = sui->top_rsq;
    }
    sui->sigmet_fid = dd_src_fid(&sui->io_type);
    sui->min_ray_size=sizeof(struct ray_header)+SIZEOF_16_BITS;
    sui->header_count = 0;
    sui->sweep_num = 0;
    gri->source_format = SIGMET_FMT;
    gri->missing_data_flag = EMPTY_FLAG;
# ifdef obsolete
    gri->binary_format = DD_8_BITS;
    gri->missing_data_flag = 0;
# endif
    gri->h_beamwidth = EMPTY_FLAG;
    gri->v_beamwidth = EMPTY_FLAG;
    gri->rcv_bandwidth = EMPTY_FLAG; /* MHz */
    gri->compression_scheme = NO_COMPRESSION;

    if(a=get_tagged_string("PROJECT_NAME")) {
	strcpy(gri->project_name, a);
    }
    else {
	strcpy(gri->project_name, "UNKNOWN");
    }
    if(a=get_tagged_string("SITE_NAME")) {
	strcpy(gri->site_name, a);
    }
    else {
	strcpy(gri->site_name, "UNKNOWN");
    }
    if(a=get_tagged_string("RADAR_NAME")) {
	strcpy(gri->radar_name, a);
    }
    else {
	strcpy(gri->radar_name, "SIGMET");
    }
    gri->dd_radar_num = dd_assign_radar_num(gri->radar_name);

    /* lookup tables
     */
    for(i=0; i < SIGMET_MAX_FIELDS; i++ ) {
	gri->scaled_data[i] =
	      (short *)malloc(SIGMET_MAX_GATES*SIZEOF_16_BITS);
	/* lookup tables */
	sui->sigmet_lut[i] = (short *)malloc(65536*sizeof(short));
	sui->sigmet_raw_field[i] =
	      (char *)malloc((SIGMET_MAX_GATES*sizeof(short)
			      +sizeof(struct ray_header)));
	sui->idh[i] = (struct ingest_data_header *)
	      malloc(sizeof(struct ingest_data_header));
    }

    /* read in the first ray */
    n = sigmet_next_ray();
}
/* c------------------------------------------------------------------------ */

void 
sigmet_nab_data (void)
{
    /* unpack and scale the raw data */
    int i, j, n, fld;
    unsigned char *a;
    short *dst, *lut;
    static int count=0, trip=0;
    struct ray_header *rh;

    
    count++;

    for(i=0,fld=0; fld < sui->num_fields; fld++ ) {
	if( sui->data_type_id[fld] != DB_XHDR_P ) {

	    lut = sui->sigmet_lut[fld];	/* get the lookup table */
	    dst = gri->scaled_data[i];
	    rh = (struct ray_header*)sui->sigmet_raw_field[fld];
	    gri->actual_num_bins[i] = n = X2(rh->ibincount);
	    a = (unsigned char *) sui->sigmet_raw_field[fld] +
                   sizeof(struct ray_header);
	    memcpy(gri->byte_data[i]
		   , sui->sigmet_raw_field[fld] +sizeof(struct ray_header)
		   , n);
	    i++;
# ifndef obsolete	    
	    for(j=0; j < n; j++) 
		  *dst++ = *(lut+(*a++));
	    
	    for(; j < gri->num_bins; j++ )
		  *dst++ = EMPTY_FLAG;
# endif
	}
    }
    /* apply corrections
     */
  wsrqc_correct(gri, (char *)"DZ"); 
}
/* c------------------------------------------------------------------------ */

void 
sigmet_nab_info (void)
{
    /*
     * stash all the useful generic info in a struct
     */
    int ii, n, mark;
    time_t t;
    struct ray_header *rh;
    struct extended_header_v1 *eh;
    static int nab_num=0, trip=0;
    float a1, a2;
    double d;
    DD_TIME *dts=gri->dts;

    
    nab_num++;
    if(!sui->field_word_count[0]) {
	sui->ok_ray = NO;
	return;
    }
    sui->ok_ray = YES;
    rh = (struct ray_header*)sui->sigmet_raw_field[0];
    a1 = BA2F(X2(rh->iaz_start));
    a2 = BA2F(X2(rh->iaz_end));
    gri->azimuth = a1 + .5*angdiff(a1,a2) +360.;
    gri->azimuth = fmod(gri->azimuth, 360.);

    a1 = BA2F(X2(rh->iel_start));
    a2 = BA2F(X2(rh->iel_end));
    gri->elevation = a1 + .5*angdiff(a1,a2) +360.;
    gri->elevation = fmod(gri->elevation, 360.);

//    t = gri->time = (double)sui->base_time + .1*X2(rh->itime);
    gri->time = (double)sui->base_time + .1*X2(rh->itime);
    t = (time_t) gri->time;
    gri->rotation_angle = gri->azimuth;

    if( gri->scan_mode == DD_SCAN_MODE_PPI ||
	gri->scan_mode == DD_SCAN_MODE_SUR ) {
	gri->corrected_rotation_angle = gri->azimuth;
    }
    else {
	gri->corrected_rotation_angle = gri->elevation;
    }
    
    if( sui->extended_header_flag ) {
	n = X2(is->igh.ib_xhdr); /* bytes in extended header */
	eh = (struct extended_header_v1 *)
	      (sui->sigmet_raw_field[0]+sizeof(struct ray_header));

	if((d = .001*X4(eh->itimems)) > (1.1*SECONDS_IN_A_DAY)) {
	    mark = 0;
	}
	else if(d > 90. ) {
	    mark = 0;
	}
	else if(!sui->new_vol && (fabs(d+sui->base_time-sui->last_time) >
		(double)SECONDS_IN_A_DAY)) {
	    mark = 0;
	}
	else {
	    mark = 0;
	}
	d = (double)sui->base_time + .001*X4(eh->itimems);

	gri->latitude = UNSCALE_LAT_LON(X4(eh->ilatitude));
	gri->longitude = UNSCALE_LAT_LON(X4(eh->ilongitude));
	gri->altitude = X2(eh->ialtitude);
	gri->heading = BA2F(X2(eh->iheading));
	gri->pitch = BA2F(X2(eh->ipitch));
	gri->roll = BA2F(X2(eh->iroll));
	gri->vns = X2(eh->ivel_north)*.01; /* convert m/s */
	gri->vew = X2(eh->ivel_east)*.01;
	gri->vud = X2(eh->ivel_up)*.01;
	gri->drift = EMPTY_FLAG;
	gri->ui = EMPTY_FLAG;
	gri->vi = EMPTY_FLAG;
	gri->wi = EMPTY_FLAG;
	/* get more detail on the extend header flag! */
	gri->radar_type = DD_RADAR_TYPE_SHIP;
    }
    else {
	gri->radar_type = DD_RADAR_TYPE_GROUND;
    }
    dts->time_stamp = gri->time;
    d_unstamp_time(dts);
    gri->year = dts->year -1900;
    gri->month = dts->month;
    gri->day = dts->day;
    gri->hour = dts->hour;
    gri->minute = dts->minute;
    gri->second = dts->second;
    gri->millisecond = dts->millisecond;
}
/* c------------------------------------------------------------------------ */

int 
sigmet_next_ray (void)
{
    /*  read in the next ray */
    /* c...mark */

    static int whats_left=0, ray_num=0, count=0, recl=0;
    static int product_header=YES, summary_ingest_header=YES
	  , ingest_data_headers=YES;
    static unsigned short *us=NULL;
    static int counttrip=440, rectrip=264, raytrip=1168, eor_count=0;
    static int vol_rec_count, b_offset;
    static struct sig_rec_struct *rsq;
    struct raw_prod_bhdr *rph;

    int i, n, iter, mark, field_count=0, wtotal=0, id, eors;
    int eof_count=0, rlcount, wcount=0, new_run=YES, unpack_data;
    int irec, isweep;

    unsigned short *dst=(unsigned short *)sui->sigmet_raw_field[0], *usx;
    

    if( dd_stats->ray_count >= raytrip ) {
	mark = 0;
    }

    while(1) {
	count++;
	if( count >= counttrip ) {
	    mark = 0;
	}
	if( dd_stats->rec_count >= rectrip ) {
	    mark = 0;
	}
	if( whats_left >= SIZEOF_16_BITS  && new_run
	   && *us == SIGMET_LAST_RECORD ) {
	    whats_left = 0;
	    ingest_data_headers = YES;
# ifdef obsolete
	    printf( "SIGMET_LAST_RECORD!\n" );
# endif
	}
	/*
	 * keep looping through records and
	 * unpacking data until
	 * all data fields have been absorbed
	 * for a ray
	 */
	if( whats_left < SIZEOF_16_BITS) {
	    rsq = sui->top_rsq = sui->top_rsq->next;
	    rph = rsq->rph;
	    tape_buf = rsq->rec_buf;

	    recl = n = sig_read( sui->sigmet_fid, tape_buf, BLOCK_SIZE );
	    dd_stats->MB += BYTES_TO_MB(n);

	    id = irec = X2(rph->irec);
# ifdef obsolete
	    isweep = X2(rph->isweep);
	    iray_ptr = X2(rph->iray_ptr);
	    iray_num = X2(rph->iray_num);
	    iflags = X2(rph->iflags);
# endif
	    if( n < 0 || ( n == 0 && eof_count > 2 ))
		  return(n);
	    else if( n == 0 ) {
		eof_count++;
		dd_stats->file_count++;
		product_header = YES;
		summary_ingest_header = YES;
		ingest_data_headers = YES;
		sui->ignore_rest_of_file = NO;
		whats_left = 0;
		printf( "EOF number: %d at %.2f MB\n"
		       , dd_stats->file_count
		       , dd_stats->MB);
		continue;
	    }
	    else if( n < SIGMET_REC_SIZE ) {
		whats_left = 0;
		continue;
	    }
	    else {
		dd_stats->rec_count++;
		if(sui->ignore_rest_of_file) {
		    whats_left = 0;
		    continue;
		}
		eof_count = 0;
# ifdef obsolete
		if(id == ST_INGEST_SUM_P || id == ST_PRODUCT_HDR_P) {
		    mark = 0;
		}
# endif
		if(product_header && id == ST_PRODUCT_HDR_P) {
		    /* this record contains the product header */
		    product_header = NO;
		    whats_left = 0;
		    continue;
		}
		else if(product_header ) {
		    /* still looking for the product header */
		    printf("Not a product header\n");
		    whats_left = 0;
		    continue;
		}
		else if(summary_ingest_header) {
		    if(id == ST_INGEST_SUM_P) {
			/* record containing summary headers */
			grab_summary_header(tape_buf);
			summary_ingest_header = NO;
			vol_rec_count = 1;
			dd_stats->vol_count++;
		    }
		    else {
			/* not raw data skip to the next file */
			product_header = YES;
			printf("Not a summary header either\n");
			sui->ignore_rest_of_file = YES;
			return(-66);
		    }
		    whats_left = 0;
		    continue;
		}
		else {
		    if((isweep = X2(rph->isweep)) != sui->isweep) {
			ingest_data_headers = YES;
			mark = 0;
		    }
# ifdef obsolete
		    vol_rec_count++;
		    if( irec != vol_rec_count) {
			printf("Volume record counts out of sync!\n");
			mark = 0;
		    }
# endif
		    if( ingest_data_headers ) {
			/* this record begins a new sweep  c...mark
			 * which starts with ingest data headers
			 */
			ingest_data_headers = NO;
			b_offset = ingest_ingest_headers(tape_buf)
			      +sizeof(struct raw_prod_bhdr);
			sui->isweep = isweep;
			ray_num = 0;
			field_count = 0;
			if(sui->ignore_rest_of_file) {
			    whats_left = 0;
			    continue;
			}
			whats_left = n -b_offset;
			us = (unsigned short *)(tape_buf+b_offset);
# ifdef obsolete
			for(; X2(*us) == SIGMET_END_OF_RAY; us++
			    , whats_left -= SIZEOF_16_BITS) {
			    printf("EOR after data header\n");
			}
# endif
		    }
		    else {
			b_offset = sizeof(struct raw_prod_bhdr);
			whats_left = n -b_offset;
			us = (unsigned short *)(tape_buf+b_offset);
		    }
# ifdef obsolete
		    if(eor_count+1 != iray_num) {
			mark = 0;
		    }
# endif
		}
	    }
	}

	if(new_run) {
//	    unpack_data = X2(*us);  //Jul 26, 2011
	    unpack_data = X2(*(twob *)us);  //Jul 26, 2011 unsigned short to twob
	    if(unpack_data == SIGMET_END_OF_RAY) {
		rlcount = 0;
	    }
	    else {
		rlcount = unpack_data & MASK15;
	    }
	    wcount += rlcount;			
	    us++; whats_left -= SIZEOF_16_BITS;
	    iter = 0;
	}

	if(unpack_data & SIGN16) { /* Data! */
	    for(; iter < rlcount && whats_left >= SIZEOF_16_BITS; iter++ ) {
		*dst++ = *us++;
		whats_left -= SIZEOF_16_BITS;
	    }
	}
	else if(rlcount) {
	    for(; iter < rlcount; iter++ ) 
		  *dst++ = 0;
	}
	/* count the number of eor flags
	 */
	eors = 0;
	if(new_run = iter >= rlcount)
//	      for(usx=us; X2(*usx++) == SIGMET_END_OF_RAY; eors++);  //Jul 26, 2011
	      for(usx=us; X2(*(twob *)usx++) == SIGMET_END_OF_RAY; eors++);  //Jul 26, 2011

	if( eors ) {
	    eor_count += eors;
	    us += eors; whats_left -= eors*SIZEOF_16_BITS;
	    sui->field_word_count[field_count] = wcount;
	    wtotal += wcount;
	    if(++field_count >= sui->num_fields || eors > 1 ){
		ray_num++;
		break;
	    }
	    wcount = 0;
	    dst = (unsigned short *)sui->sigmet_raw_field[field_count];
	}
    }


# ifdef obsolete    
    if(ray_num >= sui->rays_in_scan) {
	whats_left = 0;
	ingest_data_headers = YES;
    }
# endif
    dd_stats->ray_count++;
    sigmet_nab_info();

    return(1);
}
/* c------------------------------------------------------------------------ */

int 
sigmet_select_ray (void)
{
    int i, mark;
    int ok;

    ok = sui->ok_ray;

    if((sui->bad_date_time))
	  ok = NO;
    else if(gri->time >= difs->final_stop_time)
	  difs->stop_flag = YES;

    if(difs->num_time_lims) {
	for(i=0; i < difs->num_time_lims; i++ ) {
	    if(gri->time >= difs->times[i]->lower &&
	       gri->time <= difs->times[i]->upper)
		  break;
	}
	if(i == difs->num_time_lims)
	      ok = NO;
    }

    if(difs->num_prf_lims) {
	for(i=0; i < difs->num_prf_lims; i++ ) {
	    if(gri->PRF >= difs->prfs[i]->lower &&
	       gri->PRF <= difs->prfs[i]->upper)
		  break;
	}
	if(i == difs->num_prf_lims)
	      ok = NO;
    }

    if(difs->num_fixed_lims) {
	for(i=0; i < difs->num_fixed_lims; i++ ) {
	    if(gri->fixed >= difs->fxd_angs[i]->lower &&
	       gri->fixed <= difs->fxd_angs[i]->upper)
		  break;
	}
	if(i == difs->num_fixed_lims)
	      ok = NO;
    }

    if(difs->num_modes) {
	if(difs->modes[gri->scan_mode]) {
	    mark = 0;
	}
	else
	      ok = NO;
    }

    return(ok);
}
/* c------------------------------------------------------------------------ */

void 
sigmet_print_stat_line (int count)
{
    time_t i;
    double d;
    DD_TIME *dts=gri->dts;
    
    d_unstamp_time(dts);
    d = gri->time;
    i = (time_t) d;
    printf(" %5d %3d %7.2f %.2f  %s\n"
	   , count
	   , gri->sweep_num
	   , gri->fixed
	   , dts->time_stamp
	   , dts_print(dts)
	   );
}
/* c------------------------------------------------------------------------ */

int 
ingest_ingest_headers (char *tbuf)
{
    /* routine to absorb all the ingest data headers
     * and generate lookup tables and scaling info
     */
    char *c=tbuf;
    struct raw_prod_bhdr *rph=
	  (struct raw_prod_bhdr *)tbuf;
    int i, j, k, mark, isize=0, trgr;
    int fld, jsize=sizeof(struct ray_header);
    int irec, isweep, iray_ptr, iray_num, iflags;
    short *ss;
    struct ingest_data_header *idh;
    DD_TIME *dts=gri->dts;
    /*
     * c...mark
     * X2 and X4 are macros that swap the bytes using
     * sig_swap2 and sig_swap4
     */


    irec = X2(rph->irec);
    isweep = X2(rph->isweep);
    iray_ptr = X2(rph->iray_ptr);
    iray_num = X2(rph->iray_num);
    iflags = X2(rph->iflags);
    if(iray_ptr > 12) {
	d_unstamp_time(dts);
	printf("Ignoring the rest of this file\n ");
       printf("ray: %d rec: %d vol: %d swp: %d %02d%02d%02d%02d%02d%02d %.2f\n"
	       , dd_stats->ray_count
	       , dd_stats->rec_count
	       , dd_stats->vol_count
	       , dd_stats->sweep_count
	       , dts->year-1900, dts->month, dts->day
	       , dts->hour, dts->minute, dts->second
	       , dts->time_stamp
	       );
	sui->ignore_rest_of_file = YES;
	mark = 0;
    }

    for(c+=iray_ptr,j=0; j < sui->num_fields; j++ ) {

	idh = (struct ingest_data_header *)c;
	memcpy((char *)sui->idh[j]
	       , (char *)idh, sizeof(struct ingest_data_header));
	
	if( j == 0 ) {		/* from the first header */
	    gri->day = dts->day = X2(idh->time.iday);
	    gri->month = dts->month = X2(idh->time.imon);
	    dts->year =  X2(idh->time.iyear);
	    gri->year =  dts->year -1900;
	    if(gri->day > 31 || gri->day < 1 ||
	       gri->month > 12 || gri->month < 1 ||
	       gri->year < 80 || X2(idh->time.iyear) > 2020) {
		sui->bad_date_time = YES;
	    }
	    else {
		sui->bad_date_time = NO;
		dts->day_seconds = X4(idh->time.isec)
			    +X2(is->igh.igmt_min)*60.;

		sui->base_time = d_time_stamp(dts);
	    }
	    sui->irtotl = X2(sui->idh[j]->irtotl);
	    sui->rays_in_scan = sui->iwritn = X2(sui->idh[j]->iwritn);

	    if(sui->irtotl > 800 || sui->iwritn > 800) {
		printf("!!! irtotl=%d iwritn=%d\n"
		       , sui->irtotl, sui->iwritn);
		mark = 0;
	    }
	    gri->fixed = gri->tilt = BA2F(X2(sui->idh[j]->iangle));
	}
	isize += sizeof(struct ingest_data_header);
	c += sizeof(struct ingest_data_header);
    }


    trgr = (X2(is->tcf.dsp.itrig)+1);
    sui->extended_header_flag = NO;
    /*
     * Lookup tables and scaleing info
     */
    for(fld=0,k=0; fld < sui->num_fields; fld++) {

	gri->dd_scale[k] = 100.;
	gri->dd_offset[k] = 0;
	sui->data_type_id[fld] = X2(sui->idh[fld]->idata);

	ss = sui->sigmet_lut[fld];
	*ss++ = EMPTY_FLAG;	/* no data */

	switch(sui->data_type_id[fld]) {

	case DB_XHDR_P:
	    /* Extended Headers...no lookup table */
	    sui->extended_header_flag = YES;
	    jsize += sizeof(struct extended_header_v1);
	    break;

	case DB_UDBZ_P:
	    /* UnCorrected reflectivity */
	    strncpy(gri->field_name[k], "UDBZ         ", 8);
	    strcpy(gri->field_long_name[k],
		   "Uncorrected Reflectivity Factor");		   
	    strncpy(gri->field_units[k], "dBZ      ", 8 );

	    for(i=1; i < 256; i++ ) {
		*ss++ = (short)S100((i-64)*.5);
	    }
# ifdef obsolete
	    gri->dd_scale[k] = 1./.5;
	    gri->dd_offset[k] = 64.;
# endif
	    k++;
	    break;

	case DB_CDBZ_P:
	    /* Corrected reflectivity */
	    strncpy(gri->field_name[k], "DZ        ", 8);
	    strcpy(gri->field_long_name[k],
		   "Corrected Reflectivity Factor");		   
	    strncpy(gri->field_units[k], "dBZ      ", 8 );

	    for(i=1; i < 256; i++ ) {
		*ss++ = (short)S100((i-64)*.5);
	    }
# ifdef obsolete
	    gri->dd_scale[k] = 1./.5;
	    gri->dd_offset[k] = 64.;
# endif
	    k++;
	    break;

	case DB_VEL_P:
	    /* Velocity */
	    strncpy(gri->field_name[k], "VE          ", 8);
	    strcpy(gri->field_long_name[k],
		   "Doppler Velocity");		   
	    strncpy(gri->field_units[k], "m/s       ", 8 );

	    for(i=1; i < 256; i++ ) {
		*ss++ = (short)S100((i-128)*.007874*trgr*gri->nyquist_vel);
	    }
# ifdef obsolete
	    gri->dd_scale[k] = 1./(.007874*trgr*gri->nyquist_vel);
	    gri->dd_offset[k] = 128.;
# endif
	    k++;
	    break;

	case DB_WIDTH_P:
	    /* Width */
	    strncpy(gri->field_name[k], "SW             ", 8);
	    strcpy(gri->field_long_name[k],
		   "Spectral Width");		   
	    strncpy(gri->field_units[k], "m/s       ", 8 );

	    for(i=1; i < 256; i++ ) {
		*ss++ = (short)S100(i*.00390625*trgr*gri->nyquist_vel);
	    }
# ifdef obsolete
	    gri->dd_scale[k] = 1./(.00390625*trgr*gri->nyquist_vel);
	    gri->dd_offset[k] = 0;
# endif
	    k++;
	    break;
	    
	case DB_ZDR_P:
	    /* Differential reflectivity */
	    strncpy(gri->field_name[k], "ZDR         ", 8);
	    strcpy(gri->field_long_name[k],
		   "Differential Reflectivity");		   
	    strncpy(gri->field_units[k], "dB         ", 8 );

	    for(i=1; i < 256; i++ ) {
		*ss++ = (short)S100((i-128)*.0625);
	    }
# ifdef obsolete
	    gri->dd_scale[k] = 1./.0625;
	    gri->dd_offset[k] = 128.;
# endif
	    k++;
	    break;

	default:
	    break;
	}

	if(fld) {
	    sui->sigmet_raw_field[fld] = sui->sigmet_raw_field[0] +jsize;
	    jsize += sizeof(struct ray_header) +gri->num_bins*sizeof(short);
	}
    }
    gri->num_fields_present = k;
    gri->sweep_num++;
    dd_stats->sweep_count++;
    sui->new_sweep = YES;
    return(isize);
}
/* c------------------------------------------------------------------------ */

void 
grab_summary_header (char *buf)
{
    int i, j, k, mark;
    static int count=0;
    int32_t li;
    float f, r;
    double d;
    
    count++;

    /* save the ingest summary headers */
    memcpy((char *)is, buf, sizeof(struct ingest_summary));

    gri->vol_num = ++sui->header_count;
    sui->new_vol = YES;

    /* establish the number of fields by
     * getting the data collection mask
     * and counting the bits
     */
    sui->num_fields = 0;

    li = X4(is->tcf.dsp.idata);
    for(k=0; k < 32; k++, li >>= 1 )
	  if( li & 1 )
		sui->num_fields++;
		
    /* from task_dsp_info */
    gri->PRF = X4(is->tcf.dsp.iprf); 
    gri->num_samples = X2(is->tcf.dsp.isamp); 

    gri->pulse_width = X4(is->tcf.dsp.ipw)
	  *1.e-8; /* seconds */
# ifdef obsolete
    gri->pulse_width *= .5*SPEED_OF_LIGHT; /* convert to meters */
# endif
    /* from task_misc_info */
    /*
    gri->polarization  is related to is->tcf.misc.ipolar
     */
    gri->wavelength =
	  X4(is->tcf.misc.ilambda)
		*.0001; /* to meters */
    gri->num_freq = gri->num_ipps = 1;
    gri->freq[0] = SPEED_OF_LIGHT/gri->wavelength*1.e-9; /* GHz */
    gri->ipps[0] = 1./gri->PRF;
    gri->peak_power =
	  X4(is->tcf.misc.ixmt_pwr);
    /* in watts? */

    mark = (X2(is->tcf.dsp.itrig));
    gri->nyquist_vel = gri->wavelength*gri->PRF*.25;
    gri->radar_constant = EMPTY_FLAG;
    gri->noise_power = EMPTY_FLAG;
    gri->ant_gain = EMPTY_FLAG;
    gri->rcvr_gain = EMPTY_FLAG;
    gri->peak_power = EMPTY_FLAG;
    gri->h_beamwidth = EMPTY_FLAG;
    gri->v_beamwidth = EMPTY_FLAG;
    gri->rcv_bandwidth = EMPTY_FLAG;
    gri->track = EMPTY_FLAG;
    gri->sweep_speed = EMPTY_FLAG;
    gri->polarization = EMPTY_FLAG;
    gri->ui = EMPTY_FLAG;
    gri->vi = EMPTY_FLAG;
    gri->wi = EMPTY_FLAG;
    gri->track = EMPTY_FLAG;

    /* from task_range_info */
    r = gri->range_b1 =
	  X4(is->tcf.rng.ibin_first)*.01; /* meters */
    f = X4(is->tcf.rng.ibin_last)*.01; /* meters */

    gri->num_bins = X2(is->tcf.rng.ibin_out_num);
    gri->bin_spacing =
	  X4(is->tcf.rng.ibin_out_step)*.01; /* meters */
# ifdef obsolete
    printf("r1,r2:%.1f %.1f gs:%.1f nb:%d\n"
	   , r, f, gri->bin_spacing, gri->num_bins);
# endif
    for(k=0; k < gri->num_bins; k++, r+=gri->bin_spacing ) {
	gri->range_value[k] = r;
	gri->range_correction[k] = r > 0 ? 20.*log10(r*.001) : 0;
    }
    for(k=0; k < sui->num_fields; k++ )
	  gri->range_bin[k] = gri->range_value;

    /* from task_scan_info */

    switch(k=X2(is->tcf.scan.iscan)) {
    case SCAN_CON_P:
	gri->scan_mode = DD_SCAN_MODE_SUR;
	break;
    case SCAN_RHI_P:
    case SCAN_RHIMAN_P:
	gri->scan_mode = DD_SCAN_MODE_RHI;
	break;
    default:
	gri->scan_mode = DD_SCAN_MODE_PPI;
	break;
    }


    /* from ingest_header */
    gri->latitude = UNSCALE_LAT_LON
	  (X4(is->igh.ilat));
    gri->longitude = UNSCALE_LAT_LON
	  (X4(is->igh.ilon));
    gri->altitude = X2(is->igh.ignd_hgt) + X2(is->igh.irad_hgt);

    strncpy(gri->site_name, is->igh.sitename, 8 );
    gri->site_name[8] = '\0';

    /* 
     * project name
     * flight id
     * 
     * 
     * 
     * 
     * c...mark
     */

}
/* c------------------------------------------------------------------------ */

int 
sig_read (int fid, char *buf, int size)
{
    int n;

    if( sui->io_type == FB_IO ) {
	n = fb_read( fid, buf, size );
    }
    else {
	n = read( fid, buf, size );
    }
    return(n);
}
/* c------------------------------------------------------------------------ */

short 
sig_swap2 (		/* swap integer*2 */
    twob *ov
)
{
    union {
	short newval;
	char nv[2];
    }u;
    u.nv[1] = ov->one; u.nv[0] = ov->two;
    return(u.newval);
}
/* c------------------------------------------------------------------------ */

int32_t 
sig_swap4 (		/* swap integer*4 */
    fourb *ov
)
{
    union {
	int32_t newval;
	char nv[4];
    }u;
    u.nv[3] = ov->one; u.nv[2] = ov->two;
    u.nv[1] = ov->three; u.nv[0] = ov->four;
    return(u.newval);
}
/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */
