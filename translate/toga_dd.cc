/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */
/*
 * This file contains the following routines
 * 
 * 
 * toga_data_types
 * toga_dd_conv
 * toga_gen_luts
 * toga_grab_header
 * toga_ini
 * toga_nab_data
 * toga_nab_info
 * toga_next_ray
 * toga_ok_ray
 * toga_time_stamp
 * toga_update_header
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dd_io_structs.h>
#include <dd_math.h>
#include "generic_radar_info.h"
#include "input_limits.hh"
#include "dd_stats.h"
#include "toga_dd.hh"
#include "dorade_share.hh"
#include "dda_common.hh"
#include "gneric_dd.hh"
#include "dd_io_mgmt.hh"


static char *toga_buf=NULL;
static char *toga_unpacked=NULL;

static struct toga_header *thed=NULL;
static struct toga_rec_header *treh;
static struct toga_ray_header *trah;

static struct generic_radar_info *gri;
static struct toga_useful_items *tui;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats=NULL;

static struct input_read_info *irq;
static int view_char=0, view_char_count=200;
static char preamble[24], postamble[64];
static struct solo_list_mgmt *slm;
static char *pbuf=NULL, **pbuf_ptrs=NULL;
static int pbuf_max=1200;

/* c------------------------------------------------------------------------ */

void 
toga_data_types (void)
{
    int i;

    /*
     * redefine the unpacking scheme
     */
    gri->num_fields_present = 0;


    switch(trah->data_type) {
	
    case TOGA_DOPPLER_DATA:
	tui->gate_width = 3;

	i = gri->num_fields_present++;
	tui->data_type_id[i] = TOGA_SNR;
	tui->relative_offset[i] = 0;
	strncpy(gri->field_name[i], "SNR       ", 8);
	strcpy(gri->field_long_name[i],
	       "Signal to noise ratio");		   
	strncpy(gri->field_units[i], "dB       ", 8 );
	/* get the predefined scaling info
	 */
	gri->dd_scale[i] = tui->toga_data_scale[TOGA_SNR];
	gri->dd_offset[i] = tui->toga_data_offset[TOGA_SNR];
	tui->data_type_id[i] = TOGA_SNR;
	
	i = gri->num_fields_present++;
	tui->data_type_id[i] = TOGA_VELOCITY;
	tui->relative_offset[i] = 0;
	strncpy(gri->field_name[i], "VE        ", 8);
	strcpy(gri->field_long_name[i],
	       "Doppler Velocity");		   
	strncpy(gri->field_units[i], "m/s       ", 8 );
	gri->dd_scale[i] = tui->toga_data_scale[TOGA_VELOCITY];
	gri->dd_offset[i] = tui->toga_data_offset[TOGA_VELOCITY];

	i = gri->num_fields_present++;
	tui->data_type_id[i] = TOGA_SPECTRAL_WIDTH;
	tui->relative_offset[i] = 1;
	strncpy(gri->field_name[i], "SW        ", 8);
	strcpy(gri->field_long_name[i],
	       "Spectral Width");		   
	strncpy(gri->field_units[i], "m/s       ", 8 );
	gri->dd_scale[i] = tui->toga_data_scale[TOGA_SPECTRAL_WIDTH];
	gri->dd_offset[i] = tui->toga_data_offset[TOGA_SPECTRAL_WIDTH];
	
	i = gri->num_fields_present++;
	tui->data_type_id[i] = TOGA_DBZ;
	tui->relative_offset[i] = 1;
	strncpy(gri->field_name[i], "DZ        ", 8);
	strcpy(gri->field_long_name[i],
	       "Reflectivity Factor");		   
	strncpy(gri->field_units[i], "dBZ      ", 8 );
	gri->dd_scale[i] = tui->toga_data_scale[TOGA_DBZ];
	gri->dd_offset[i] = tui->toga_data_offset[TOGA_DBZ];
	
	i = gri->num_fields_present++;
	tui->data_type_id[i] = TOGA_DBZ;
	tui->relative_offset[i] = 2;
	strncpy(gri->field_name[i], "UDBZ_        ", 8);
	strcpy(gri->field_long_name[i],
	       "Uncorrected Reflectivity Factor");		   
	strncpy(gri->field_units[i], "dBZ      ", 8 );
	gri->dd_scale[i] = tui->toga_data_scale[TOGA_DBZ];
	gri->dd_offset[i] = tui->toga_data_offset[TOGA_DBZ];
	break;
	
	
    case TOGA_REFLECTIVITY_DATA:
	tui->gate_width = 1;
	i = gri->num_fields_present++;
	tui->data_type_id[i] = TOGA_Z_;
	tui->relative_offset[i] = 0;
	strncpy(gri->field_name[i], "Z_        ", 8);
	strcpy(gri->field_long_name[i],
	       "Reflectivity");		   
	strncpy(gri->field_units[i], "dBZ      ", 8 );
	gri->dd_scale[i] = tui->toga_data_scale[TOGA_Z_];
	gri->dd_offset[i] = tui->toga_data_offset[TOGA_Z_];
	break;
	
    default:
	break;
    }
}
/* c------------------------------------------------------------------------ */

void 
toga_dd_conv (void)
{
    static int i, n, count=0, nok_count=0;

    /* initialize and read in first ray
     */
    if(!count) {
	toga_ini();
    }
# ifdef notyet
    if(interactive_mode) {
	dd_intset();
	toga_positioning();
    }
    toga_reset();
# endif

    /* loop through the data
     */
    for(;;) {
	count++;

	if(difs->stop_flag &&
	   (difs->abrupt_start_stop || tui->new_sweep)) {
	    printf("\nBreak on stop time!\n");
	    break;
	}

	if(toga_select_ray()) {
	    if(difs->max_beams)
		  if(++tui->ray_count > difs->max_beams ) {
		      printf("\nBreak on ray count!\n");
		      break;
		  }
	    if(tui->sweep_count_flag) {
		tui->sweep_count_flag = NO;
		if(difs->max_sweeps) {
		    if(++tui->sweep_count > difs->max_sweeps ) {
			printf("\nBreak on sweep count!\n");
			break;
		    }
		}
		if(tui->vol_count_flag) {
		    tui->vol_count_flag = NO;
		    if(difs->max_vols) {
			if(++tui->vol_count > difs->max_vols ) {
			    printf("\nBreak on volume count!\n");
			    break;
			}
		    }
		}
	    }
	    if(!difs->catalog_only)
		  toga_nab_data();
	    dd_stuff_ray();	/* pass it off to dorade code */
	}
	tui->new_vol = tui->new_sweep = NO;

	if((n = toga_next_ray()) < 1) {
	    break;
	}
    }
    /* c...mark */
}
/* c------------------------------------------------------------------------ */

void 
toga_gen_luts (void)
{
    /*
     * generate lookup tables for the various fields
     * create a 16-bit lookup table for every field
     * except spectral width
     */
    int i, j, signx, mark;
    short *ss, *lut;
    static float last_nyq = -1.;
    float nyq=gri->nyquist_vel;

    if( nyq == last_nyq ) {
	/* do nothing if no change */
	return;
    }
    last_nyq = nyq;

    /* for SNR
     */
    ss = lut = tui->toga_lut[TOGA_SNR];
    tui->toga_data_scale[TOGA_SNR] = 100.;
    tui->toga_data_offset[TOGA_SNR] = 0;
    signx = ~0 << 15;
    
    for(i=0; i < 65536; i++ ) {
	j = (i >> 10) & 0x1f;
	if( j > 15 )
	      j -= 32;
	*lut++ = (short)S100((float)j);
    }


    /* for velocity
     */
    lut = tui->toga_lut[TOGA_VELOCITY];
    tui->toga_data_scale[TOGA_VELOCITY] = 100.;
    tui->toga_data_offset[TOGA_VELOCITY] = 0;
    signx = ~0 << 15;
    nyq = gri->nyquist_vel;
    
    for(i=0; i < 65536; i++ ) {
	j = i & MASK10;
	if(SIGN10 & j) {
	    j -= 1024;
	}
	*lut++ = (short)S100(((float)j)*RCP512*nyq);
    }

    /* for spectral width
     */
    lut = tui->toga_lut[TOGA_SPECTRAL_WIDTH];
    tui->toga_data_scale[TOGA_SPECTRAL_WIDTH] = 100.;
    tui->toga_data_offset[TOGA_SPECTRAL_WIDTH] = 0;
    *lut++ = EMPTY_FLAG;

    for( i=0; i < 256; i++ )
	 *lut++ = (short) S100(((float)i)*RCP512*2.0*nyq);
    /*
     * for reflectivity factor
     */
    lut = tui->toga_lut[TOGA_DBZ];
    tui->toga_data_scale[TOGA_DBZ] = 100.;
    tui->toga_data_offset[TOGA_DBZ] = 0;
    signx = ~0 << 12;

    for(i=0; i < 65536; i++ ) {
	j = i & MASK12;
	if(SIGN12 & i) {
	    j -= 4096;
	}
# ifdef obsolete
	j = SIGN12 & i ? i | signx : i & MASK12;
# endif
	if( j == -2048 )
	     *lut++ = EMPTY_FLAG;
	else
	     *lut++ = (short) S100(((float)j)*RCP16);
    }

    /*
     * for reflectivity
     */
    lut = tui->toga_lut[TOGA_Z_];
    tui->toga_data_scale[TOGA_Z_] = 10.;
    tui->toga_data_offset[TOGA_Z_] = 0;
    signx = ~0 << 15;

    for(i=0; i < 65536; i++ ) {
	j = i & MASK15;
	if(SIGN15 & i) {
	    j -= 32768;
	}
	*lut++ = (short) S10(((float)j)*RCP16);
    }
}
/* c------------------------------------------------------------------------ */

void 
toga_grab_header_info (void)
{
    if( thed == NULL ) {
	thed = (struct toga_header *)malloc(2048);
    }
    memcpy((char *)thed, toga_buf, SHORTS_TO_BYTES(640)); /* Save header */
    tui->header_count++;
    toga_update_header_info();
}
/* c------------------------------------------------------------------------ */

//toga_ini (int32_t start_time, int32_t stop_time, char *dir, char *toga_file, char *project_name, char *site_name, char *output_flags)
void toga_ini(void)
{
    /* c...mark */
    int i, j, n;
    char *c;
    char *a;

    toga_unpacked = (char *)malloc(BLOCK_SIZE);

    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();
    gri = return_gri_ptr();

    for(j=0; j < SRS_MAX_FIELDS; j++ ) {
	gri->range_bin[j] = gri->range_value;
    }

    tui = (struct toga_useful_items *)malloc(sizeof(struct toga_useful_items));
    memset((char *)tui, 0, sizeof(struct toga_useful_items));
    
    irq = dd_return_ios(0, TOGA_FMT);

    if((a=get_tagged_string("SOURCE_DEV"))) {
	dd_input_read_open(irq, a);
    }
    else {
    }

    tui->min_ray_size=sizeof(struct toga_ray_header)+SIZEOF_16_BITS;
    tui->prev_data_fmt = -1;
    tui->cw_az_corr = -1.;
    tui->ccw_az_corr = 1.;

    treh = (struct toga_rec_header *)toga_buf;
    trah = (struct toga_ray_header *)toga_unpacked;

    gri->missing_data_flag = EMPTY_FLAG;
    gri->source_format = TOGA_FMT;
    gri->h_beamwidth = EMPTY_FLAG;
    gri->v_beamwidth = EMPTY_FLAG;
    gri->rcv_bandwidth = EMPTY_FLAG; /* MHz */

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
	strcpy(gri->radar_name, "TOGA");
    }
    gri->dd_radar_num = dd_assign_radar_num(gri->radar_name);

    /* lookup tables
     */
    for(i=0; i < TOGA_MAX_FIELDS; i++ ) {
	gri->scaled_data[i] = (short *)malloc(TOGA_MAX_GATES*SIZEOF_16_BITS);
	/* lookup tables */
	tui->toga_lut[i] = (short *)malloc(65536*sizeof(short));
    }

    /* loop through the data until a header is encountered */
    while(1) {
	n = toga_next_ray();
	if( thed )
	      break;
    }
}
/* c------------------------------------------------------------------------ */

void 
toga_nab_data (void)
{
    int i, j, k, l, m, gw=tui->gate_width;
    short *dst, *lut;
    unsigned short *w, *x;
    static int count=0, trip=0;

    
    count++;

    for(i=0; i < gri->num_fields_present; i++ ) {
	/* set pointer to first word of first datum
	 */
	w = (unsigned short *)(toga_unpacked+sizeof(struct toga_ray_header));
	/* set pointer to the word containing the first datum
	 */
	x = w + tui->relative_offset[i];
	j = tui->data_type_id[i];
	lut = tui->toga_lut[j];
	dst = gri->scaled_data[i];

	if( tui->data_type_id[i] == TOGA_SPECTRAL_WIDTH ) {
	    for(k=0; k < gri->num_bins; k++, w+=gw, x+=gw ) {
		/* examine the bad flag!
		 */
		if(!(*w & SIGN16)) { /* Bad! */
		    *dst++ = EMPTY_FLAG;
		}
		else {
		    l = (*x >> 12) & 0x000f;
		    m = (*(x+1) >> 8 ) & 0x00f0;
		    *dst++ = *(lut+(l|m));
		}
	    }
	}
	else {
	    for(k=0; k < gri->num_bins; k++, w+=gw, x+=gw ) {
		if(!(*w & SIGN16)) { /* Bad! */
		    *dst++ = EMPTY_FLAG;
		}
		else {
		    *dst++ = *(lut+(*x));
		}
	    }
	}
    }


    if(count > trip ) {
	trip += 250;
# ifdef obsolete
	trip += 1000;
	d = gri->time;
	i = d;
	toga_print_stat_line(count);
# endif
    }
}
/* c------------------------------------------------------------------------ */

void 
toga_nab_info (void)
{
    /*
     * stash all the useful generic info in a struct
     */
    int i, j, k, n;
    static int nab_num=0, trip=0, drn;
    double d;
    float f;

    nab_num++;
    tui->ok_ray = YES;
    gri->time = toga_time_stamp(trah, gri->dts);
    tui->new_sweep = gri->sweep_num != trah->tilt_num || tui->new_vol
	  ? YES : NO;
    gri->sweep_num = trah->tilt_num;

    gri->altitude = thed->ant_altitude_msl;
    gri->latitude = tui->base_latitude
	  + RCP60*.01*trah->latitude_minutes_x100;
    gri->longitude = tui->base_longitude
	  + RCP60*.01*trah->longitude_minutes_x100;

    gri->rotation_angle = BA2F( trah->azimuth );
    gri->azimuth = BA2F( trah->azimuth );
    gri->elevation = BA2F( trah->elevation );
    if(tui->new_sweep) {
	tui->sweep_count_flag = YES;
	tui->sum_delta_az = tui->sum_delta_el = 0;
    }
    else {
	tui->sum_delta_az += angdiff(tui->last_az, gri->azimuth);
	tui->sum_delta_el += angdiff(tui->last_el, gri->elevation);
    }
    tui->last_az = gri->azimuth;
    tui->last_el = gri->elevation;
    gri->fixed = gri->tilt = US10(thed->fixed_angle_x10[trah->tilt_num-1]);

    if(tui->sum_delta_az > 0) {
	gri->azimuth = FMOD360(gri->azimuth +tui->cw_az_corr);
    }
    else if(tui->sum_delta_az < 0) {
	gri->azimuth = FMOD360(gri->azimuth +tui->ccw_az_corr);
    }

    if( gri->scan_mode == DD_SCAN_MODE_PPI ) {
	gri->corrected_rotation_angle = gri->azimuth;
    }
    else {
	gri->corrected_rotation_angle = gri->elevation;
    }
    gri->heading = BA2F( trah->heading );
    gri->pitch = BA2F( trah->pitch );
    gri->roll = BA2F( trah->roll );
    d = trah->ships_speed_x100*.01;
    gri->vns = (float) (d*sin((double)RADIANS(90. - (double) trah->heading)));
    gri->vew = (float) (d*cos((double)RADIANS(90. - (double) trah->heading)));

# ifdef undef
    gri->drift = ms;
    gri->vud = US10(ms);
    gri->ui = US10(ms);
    gri->vi = US10(ms);
    gri->wi = US10(ms);
# endif

    if( trah->data_type != tui->prev_data_fmt ) {
	tui->prev_data_fmt = trah->data_type;
	toga_data_types();
    }

# ifdef obsolete		
    if( nab_num > trip ) {
	trip += 1000;
	toga_print_stat_line(nab_num);
    }
# endif		
}
/* c----------------------------------------------------------------------- */

int 
toga_next_ray (void)
{
    /* c...mark
     * read in the next ray 
     * since rays can cross record boundaries and the data are compressed
     * all this gets rather obtuse
     */
    static int whats_left=0, rec_num=0, ray_num=0, prev_toga_rec=0;
    static int count=0, trip=111;
    static unsigned short *us=NULL;

    int i, n, iter, hold, mark;
    int eof_count=0, rlcount, wcount=0, new_run=YES, unpack_data;

    unsigned short *dst=(unsigned short *)toga_unpacked;
    

    while( whats_left < SIZEOF_16_BITS
	  || us == NULL || *us != TOGA_END_OF_RAY) {

	count++;
	if(count >= trip) {
	    mark = 0;
	}
	if(dd_control_c()) {
	    dd_reset_control_c();
	    return(-1);
	}

	if( whats_left > SIZEOF_16_BITS  && new_run
	   && *us == TOGA_LAST_RECORD ) {
	    whats_left = 0;
# ifdef obsolete
	    printf( "TOGA_LAST_RECORD!\n" );
	    return(-1);
# endif
	}
	/*
	 * keep looping through records and
	 * unpacking data until
	 * an end of ray is encountered
	 */
	if(whats_left < SIZEOF_16_BITS) {

	    dd_logical_read(irq, FORWARD);

	    toga_buf = irq->top->buf;
	    treh = (struct toga_rec_header *)toga_buf;
	    n = irq->top->read_state;
	    
	    if( n < 0 )
		  return(n);

	    else if( n == 0 ) {
		dd_stats->file_count++;
		eof_count++;
		whats_left = 0;
		printf( "EOF number: %d at %.2f MB\n"
		       , dd_stats->file_count
		       , dd_stats->MB);

		if(eof_count > 2)
		      return(n);

		continue;
	    }
	    else {
		dd_stats->MB += BYTES_TO_MB(n);
		dd_stats->rec_count++;
		rec_num++;
		eof_count = 0;
		if(n == SHORTS_TO_BYTES(640)) { /* Header! */
		    toga_grab_header_info();
		    whats_left = 0;
		    tui->vol_count_flag = 
			  tui->new_vol = YES;
		    continue;
		}
		else {
		    whats_left = n -sizeof(struct toga_rec_header);

		    if( prev_toga_rec && prev_toga_rec+1 != treh->rec_num ) {
			/* complain! */
		    }
		    us = (unsigned short *)
			  (toga_buf+sizeof(struct toga_rec_header));
		    prev_toga_rec = treh->rec_num;
		}
	    }
	}
	/* at this point we could be partially
	 * through unpacking a run of data
	 */

	if(new_run) {		/* starting a new run */
	    unpack_data = *us & SIGN16;
	    rlcount = *us & MASK15;
	    wcount += rlcount;			
	    us++;
	    whats_left -= SIZEOF_16_BITS;
	    iter = 0;
	}

	if(unpack_data) {	/* Data! */
	    for(; iter < rlcount && whats_left >= SIZEOF_16_BITS; iter++ ) {
		*dst++ = *us++;
		whats_left -= SIZEOF_16_BITS;
	    }
	}
	else {			/* uncompress zeroes */
	    for(; iter < rlcount; iter++ ) 
		  *dst++ = 0;
	}
	new_run = iter >= rlcount;
    }


    us++;
    whats_left -= SIZEOF_16_BITS;
    
    ray_num++;
    dd_stats->ray_count++;
    toga_nab_info();

    if(wcount < SIZEOF_16_BITS || ray_num >= 47 ) {
	hold = 0;
    }
    return((int)SHORTS_TO_BYTES(wcount));
}
/* c------------------------------------------------------------------------ */

int 
toga_select_ray (void)
{
    int ii, jj, mark;
    int ok;

    ok = tui->ok_ray;

    if(gri->time >= difs->final_stop_time)
	  difs->stop_flag = YES;

    if(difs->num_time_lims) {
	for(ii=0; ii < difs->num_time_lims; ii++ ) {
	    if(gri->time >= difs->times[ii]->lower &&
	       gri->time <= difs->times[ii]->upper)
		  break;
	}
	if(ii == difs->num_time_lims)
	      ok = NO;
    }

    if(difs->num_prf_lims) {
	for(ii=0; ii < difs->num_prf_lims; ii++ ) {
	    if(gri->PRF >= difs->prfs[ii]->lower &&
	       gri->PRF <= difs->prfs[ii]->upper)
		  break;
	}
	if(ii == difs->num_prf_lims)
	      ok = NO;
    }

    if(difs->num_fixed_lims) {
	for(ii=0; ii < difs->num_fixed_lims; ii++ ) {
	    if(gri->fixed >= difs->fxd_angs[ii]->lower &&
	       gri->fixed <= difs->fxd_angs[ii]->upper)
		  break;
	}
	if(ii == difs->num_fixed_lims)
	      ok = NO;
    }

    if(difs->num_az_sectors) {
	for(ii=0; ii < difs->num_az_sectors; ii++) {
	    if(in_sector(gri->azimuth
			 , (float)difs->azs[ii]->lower
			 , (float)difs->azs[ii]->upper)) {
		break;
	    }
	}
	if(ii == difs->num_az_sectors)
	      ok = NO;
    }

    if(difs->num_el_sectors) {
	for(ii=0; ii < difs->num_el_sectors; ii++) {
	    if(in_sector(gri->elevation
			 , (float)difs->els[ii]->lower
			 , (float)difs->els[ii]->upper)) {
		break;
	    }
	}
	if(ii == difs->num_el_sectors)
	      ok = NO;
    }

    if(difs->num_az_xouts) {
	for(ii=0; ii < difs->num_az_xouts; ii++ ) {
	    if(in_sector(gri->azimuth, (float)difs->xout_azs[ii]->lower
			 , (float)difs->xout_azs[ii]->upper)) {
		if(difs->num_el_xouts) {
		    for(jj=0; jj < difs->num_el_xouts; jj++) {
			if(in_sector(gri->elevation
				     , (float)difs->xout_els[jj]->lower
				     , (float)difs->xout_els[jj]->upper)) {
			    ok = NO;
			}
		    }
		}
		else {
		    ok = NO;
		}
	    }
	}
    }

    if(difs->num_modes) {
	if(difs->modes[gri->scan_mode]) {
	    mark = 0;
	}
	else
	      ok = NO;
    }
# ifdef notyet
    if(!difs->radar_selected[gri->dd_radar_num]) 
	  ok = NO;
# endif
    return(ok);
}
/* c------------------------------------------------------------------------ */

int 
toga_ok_ray (void)
{
    int ok;

    ok = tui->ok_ray;

    if((gri->time < tui->start_time))
	  ok = NO;

    return(ok);
}
/* c------------------------------------------------------------------------ */

void 
toga_print_stat_line (int count)
{
    gri->dts->time_stamp = gri->time;
    d_unstamp_time(gri->dts);

    printf(" %5d %3d %s %6.2f %.2f %.2f %s"
	   , count
	   , gri->sweep_num
	   , gri->radar_name
	   , gri->fixed
	   , gri->corrected_rotation_angle
	   , gri->time
	   , dts_print(gri->dts)
	   );
}
/* c------------------------------------------------------------------------ */

double 
toga_time_stamp (struct toga_ray_header *trah, DD_TIME *dts)
{

    double d;

    dts->year = trah->year > 1900 ? trah->year : trah->year +1900;
    dts->month = trah->month;
    dts->day = trah->day;

    dts->day_seconds = trah->hour*3600 + trah->minute*60
	  + (float)trah->seconds_x100*.01
		+ thed->time_zone_offset*60;
    d = d_time_stamp(dts);

    return(d);
 }
/* c------------------------------------------------------------------------ */

void 
toga_update_header_info (void)
{
    int i, ms = EMPTY_FLAG;
    float r;

    gri->vol_num = tui->header_count;
    /* keep distances in meters */
    gri->range_b1 = thed->bin1_range_x40*.025;
    gri->bin_spacing = thed->bin_spacing;
    gri->num_bins = thed->num_bins;
    for(i=0; i < TOGA_MAX_FIELDS; i++) {
	gri->actual_num_bins[i] = thed->num_bins;
    }
    gri->num_samples = thed->num_samples;
    gri->wavelength = thed->wavelength_xe4*.0001; /* meters! */
    gri->PRF = thed->PRF;

    gri->nyquist_vel = gri->wavelength*gri->PRF*.25;
# ifdef obsolete
    gri->pulse_width = .5*thed->pulse_width_xe8*1.e-8
	  *SPEED_OF_LIGHT;
# else
    gri->pulse_width = thed->pulse_width_xe8*1.e-8;
# endif
    gri->polarization = DD_POLAR_ELLIPTICAL;

    switch(thed->scan_mode) {
    case TOGA_PPI:
	gri->scan_mode = DD_SCAN_MODE_PPI;
	break;
    case TOGA_RHI:
	gri->scan_mode = DD_SCAN_MODE_RHI;
	break;
    case TOGA_MAN:
	gri->scan_mode = DD_SCAN_MODE_MAN;
	break;
    default:
	gri->scan_mode = DD_SCAN_MODE_IDL;
	break;
    }

    gri->radar_type = DD_RADAR_TYPE_SHIP;

    gri->peak_power = ms;

    /* generate luts for all known fields
     */
    toga_gen_luts();

    if( thed->coarse_latitude_x100 == 0 ) {
	tui->base_latitude = thed->latitude_degrees;
	tui->base_longitude = thed->longitude_degrees;
    }
    else {
	tui->base_latitude = (int)thed->coarse_latitude_x100*.01 +.001; 
	tui->base_longitude = (int)thed->coarse_longitude_x100*.01 +.001; 
	gri->latitude = thed->coarse_latitude_x100*.01;
	gri->longitude = thed->coarse_longitude_x100*.01;
    }

    gri->num_freq = 1;
    gri->num_ipps = 1;
    gri->freq[0] = SPEED_OF_LIGHT/gri->wavelength*1.e-9; /* GHz */
    gri->ipps[0] =  1./gri->PRF;
    
    gri->sweep_speed = EMPTY_FLAG;

    sprintf(gri->flight_id, "STORM%3d", thed->storm_num );

    /* set up range values */
    for(i=0,r=gri->range_b1; i < gri->num_bins;
	i++, r+=gri->bin_spacing) 
	  gri->range_value[i] = r;
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */
