/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NEW_ALLOC_SCHEME

/*
 * This file contains the following routines
 * 
 * dd_celv_update
 * dd_init_sweep
 * dd_parms_update
 * dd_radd_update
 * dd_stuff_ray
 * dd_vold_update
 * return_gri_ptr
 * 
 */

#include <dd_math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "input_limits.hh"
#include "dd_stats.h"
#include "gneric_dd.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "dorade_share.hh"
#include "gpro_data.hh"
#include "by_products.hh"
#include "dd_swp_files.hh"
#include "dorade_tape.hh"

static struct generic_radar_info *gri;
static struct dd_stats *dd_stats=NULL;
static struct dd_input_filters *difs=NULL;
static int ray_num=0;

/* c------------------------------------------------------------------------ */

void 
dd_celv_update (struct dds_structs *dds)
{
    int ii, nn;
    struct cell_d *celv=dds->celv;
    float corr=dds->cfac->range_delay_corr;

    /* update bin spacing info */

    celv->number_cells = gri->num_bins;
    memcpy((char *)celv->dist_cells, (char *)gri->range_value
	   , gri->num_bins*sizeof(float));
    nn = celv->number_cells > MAXCVGATES
	? 12 + celv->number_cells * sizeof(float) : sizeof(struct cell_d);
    celv->cell_des_len = nn;
    memcpy((char *)dds->celvc, (char *)dds->celv, nn);

    for(ii=0; ii < dds->celv->number_cells; ii++){
	dds->celvc->dist_cells[ii] += corr;
    }
    dd_set_uniform_cells(dds);
}
/* c------------------------------------------------------------------------ */

void 
dd_init_sweep (struct dd_general_info *dgi, int new_vol)
{
    short date[6];
    int i;
    struct dds_structs *dds=dgi->dds;
    struct super_SWIB *sswb=dds->sswb;
    struct sweepinfo_d *swib=dds->swib;
    struct prev_swps *pss;
    DD_TIME *dts=dgi->dds->dts;
    FLOAT_PTR freq, ipps;
    
    dgi->ignore_this_sweep = gri->ignore_this_sweep;
    gri->ignore_this_sweep = NO;

    dd_flush(dgi->radar_num);

    d_unstamp_time(dts);
    pss = dgi->swp_que = dgi->swp_que->next;
    pss->source_sweep_num = gri->sweep_num;
    pss->source_vol_num = gri->vol_num;
    pss->num_rays = 0;
    pss->new_vol = new_vol;
    pss->segment_num = dts->millisecond;
    pss->listed = NO;
//    pss->sweep_time_stamp = 
//	  dgi->sweep_time_stamp = pss->start_time = dgi->time;
	pss->start_time = dgi->time;
    pss->sweep_time_stamp = dgi->sweep_time_stamp = (int32_t) dgi->time;
    pss->sweep_file_size = sizeof(struct sweepinfo_d);
    pss->volume_time_stamp = 
	  sswb->volume_time_stamp = dgi->volume_time_stamp;

    dgi->sweep_count++;
    dgi->new_sweep = YES;

    /* super sweep info block */
    sswb->last_used = todays_date(date);
    sswb->start_time = (int32_t)gri->time;
    sswb->stop_time = EMPTY_FLAG;

    dd_tape(dgi, NO);
    dgi->ignore_this_sweep = NO;

    /* sweep info block */

    dd_blank_fill(gri->radar_name, 8, swib->radar_name);
    swib->filter_flag = NO_FILTERING;
    swib->fixed_angle = gri->fixed;

    dd_parms_update(dgi);

    dds->radd->eff_unamb_vel = gri->nyquist_vel;
    if(gri->PRF > 0)
	  dds->radd->eff_unamb_range =
		(.5*SPEED_OF_LIGHT/(float)gri->PRF)*.001;
    else
	  dds->radd->eff_unamb_range = EMPTY_FLAG;

    dds->radd->scan_mode = gri->scan_mode;
    dds->radd->num_freq_trans = gri->num_freq;
    for(freq= &dds->radd->freq1,i=0; i < gri->num_freq; i++)
	  *freq++ = gri->freq[i];

    dds->radd->num_ipps_trans = gri->num_ipps;
    for(ipps= &dds->radd->interpulse_per1,i=0; i < gri->num_ipps; i++)
	  *ipps++ = gri->ipps[i];
    dgi->source_field_mnemonics = gri->source_field_mnemonics;
}
/* c------------------------------------------------------------------------ */

void 
dd_parms_update (struct dd_general_info *dgi)
{
    int ii, jj, ncopy;

    /* parameters */
    struct dds_structs *dds=dgi->dds;
    dgi->num_parms = gri->num_fields_present;
    for(ii=0; ii < MAX_PARMS; dds->field_present[ii++]=NO);

    for(ii=0; ii < gri->num_fields_present; ii++ ) {
	dds->field_present[ii] = YES;
	jj = dds->field_id_num[ii] = gri->field_id_num[ii];
	dgi->parm_type[ii] =  !(jj == DBZ_ID_NUM || jj == DZ_ID_NUM)
	      ? VELOCITY : OTHER;
	str_terminate(dds->parm[ii]->parameter_name, gri->field_name[ii], 8);
	strncpy(dds->parm[ii]->param_description
		, gri->field_long_name[ii], 40 );
	dds->parm[ii]->param_description[39] = '\0';
	strncpy( dds->parm[ii]->param_units, gri->field_units[ii], 8 );
	dds->parm[ii]->interpulse_time = 1; /* one frequency */
	dds->parm[ii]->xmitted_freq = 1; /* one frequency */

	switch (gri->source_format) {
	case TOGA_FMT:
	    break;
	case HRD_FMT:
	    break;
	case UF_FMT:
	    break;
	}
	dds->parm[ii]->recvr_bandwidth = gri->rcv_bandwidth;
	dds->parm[ii]->pulse_width = (short)(.5 * gri->pulse_width * SPEED_OF_LIGHT);
	dds->parm[ii]->num_samples = gri->num_samples;
	dds->parm[ii]->polarization = gri->polarization; 
	strncpy( dds->parm[ii]->threshold_field, "NONE     ", 8 );

	dds->parm[ii]->parameter_scale = gri->dd_scale[ii];
	dds->parm[ii]->parameter_bias = gri->dd_offset[ii];
	dds->parm[ii]->bad_data = gri->missing_data_flag;
	if( gri->source_format == NETCDF_FMT ) {
	  dds->parm[ii]->bad_data = gri->int_missing_data_flag[ii];
	}
	dds->parm[ii]->binary_format = gri->binary_format;
# ifdef NEW_ALLOC_SCHEME
	str_terminate(dds->qdat[ii]->pdata_name, gri->field_name[ii], 8);
	ncopy = *dds->qdat[ii]->pdata_desc == 'R'
	      ? sizeof(struct paramdata_d) : sizeof(struct qparamdata_d);
	dd_alloc_data_field(dgi, ii);
	dds->qdat[ii]->pdata_length = ncopy + dds->sizeof_qdat[ii];
# else
	str_terminate(dds->rdat[ii]->pdata_name, gri->field_name[ii], 8);
	if(gri->binary_format == DD_8_BITS) {
	    /* get everything properly aligned */
	    dds->rdat[ii]->pdata_length = sizeof(struct paramdata_d)
		  + LONGS_TO_BYTES(BYTES_TO_LONGS(dds->celv->number_cells));
	}
	else {			/* assume 16_bit data */
	    dds->rdat[ii]->pdata_length = sizeof(struct paramdata_d)
		  +LONGS_TO_BYTES(SHORTS_TO_LONGS(dds->celv->number_cells));
	}
# endif
    }
}
/* c------------------------------------------------------------------------ */

void 
dd_radd_update (struct dds_structs *dds)
{
    int i;
    struct radar_d *radd=dds->radd;
    FLOAT_PTR freq, ipps;


    dd_blank_fill(gri->radar_name, 8, radd->radar_name);
    radd->radar_const = gri->radar_constant;
    radd->peak_power = gri->peak_power;
    radd->noise_power = gri->noise_power;
    radd->receiver_gain = gri->rcvr_gain;
    radd->antenna_gain = gri->ant_gain;
    radd->system_gain = gri->rcvr_gain;
    radd->horz_beam_width = gri->h_beamwidth;
    radd->vert_beam_width = gri->v_beamwidth;
    radd->radar_type = gri->radar_type;
    radd->req_rotat_vel = gri->sweep_speed;
    radd->scan_mode = gri->scan_mode;
    radd->scan_mode_pram0 = EMPTY_FLAG;
    radd->scan_mode_pram1 = EMPTY_FLAG;
    radd->num_parameter_des = gri->num_fields_present;
    radd->total_num_des = 0;
    radd->data_compress = gri->compression_scheme;
    radd->data_reduction = 0;
    radd->data_red_parm0 = EMPTY_FLAG;
    radd->data_red_parm1 = EMPTY_FLAG;
    radd->radar_longitude = gri->longitude;
    radd->radar_latitude = gri->latitude;
    radd->radar_altitude = gri->altitude*.001;
    radd->eff_unamb_vel = gri->nyquist_vel;
    if(gri->PRF)
	  radd->eff_unamb_range = (.5*SPEED_OF_LIGHT/(float)gri->PRF)*.001;
    else
	  radd->eff_unamb_range = EMPTY_FLAG;

    radd->num_freq_trans = gri->num_freq;
    for(freq= &radd->freq1,i=0; i < gri->num_freq; i++)
	  *freq++ = gri->freq[i];

    radd->num_ipps_trans = gri->num_ipps;
    for(ipps= &radd->interpulse_per1,i=0; i < gri->num_ipps; i++)
	  *ipps++ = gri->ipps[i];

    switch (gri->source_format) {
    case SIGMET_FMT:
	break;
    case UF_FMT:
	break;
    case HRD_FMT:
	break;
    }
}
/* c------------------------------------------------------------------------ */

void 
dd_stuff_ray (void)
{
    /*
     * this routine assumes that there is a ray of data to be
     * dealt with
     *
     * if this is the first ray of a volume, dump out volume headers
     * if this is the first ray of a sweep,
     *    close out the previous sweep file and
     *    open a new one with the appropriate sweep and parameter
     *    headers
     *
     */
    static float prev_fixed = 0;
    int ii, nn, new_vol=NO, mark;
    int ibad, anb, nc, rsize, new_sweep=NO;
    char *aa;
    short *ss;
    static int count=0, tripcount=13;
    struct dds_structs *dds;
    struct dd_general_info *dgi;
    struct radar_d *radd;
    struct super_SWIB *sswb;
    struct sweepinfo_d *swib;
    struct ray_i *ryib;
    struct platform_i *asib;
    struct prev_rays *prs;
    float elev=0;
    DD_TIME *dts;


    if(!count++) {		/* first time through */
	difs = dd_return_difs_ptr();
	dd_stats = dd_return_stats_ptr();
    }
    ray_num++;

//    dgi = dd_get_structs(gri->dd_radar_num, gri->radar_name); // Jun 29, 2011
    dgi = dd_get_structs(gri->dd_radar_num);   // Jun 29, 2011

    dds = dgi->dds;
    radd = dds->radd;
    sswb = dds->sswb;
    swib = dds->swib;
    ryib = dds->ryib;
    asib = dds->asib;
    dts = dgi->dds->dts;
    dgi->time = dts->time_stamp = gri->time;
    d_unstamp_time(dts);

    if( count >= tripcount ) {
	mark = 0;
    }


    if( gri->vol_num != dgi->source_vol_num ) {
	dd_vold_update(dgi);
	dgi->source_vol_num = gri->vol_num;
	new_vol = YES;
	dgi->source_fmt = gri->source_format;
    }
    if( new_vol || gri->sweep_num != dgi->source_sweep_num ) {
	dd_vold_update(dgi);
	dd_init_sweep(dgi,new_vol);
	dgi->source_sweep_num = gri->sweep_num;
	new_sweep = YES;
	new_vol = NO;
    }
    /* update ray info block */
    ryib->julian_day = dts->julian_day;
    ryib->hour = dts->hour;
    ryib->minute = dts->minute;
    ryib->second = dts->second;
    ryib->millisecond = dts->millisecond;

    ryib->azimuth = gri->azimuth;
    ryib->elevation = gri->elevation;
    /* nab the fixed angle each time in case it's an average
     */
    if(gri->source_format != FOF_FMT && !new_sweep)
	{ swib->fixed_angle = gri->fixed; }
    ryib->peak_power = gri->peak_power;
    ryib->true_scan_rate = gri->sweep_speed;
    ryib->ray_status = (gri->transition) ? IN_TRANSITION : NORMAL;
    /*
     * NORMAL=0,IN_TRANSITION=1,BAD=2,QUESTIONABLE=3
     *
     * update platform info block
     */
    asib->longitude = gri->longitude;
    asib->latitude = gri->latitude;
    asib->altitude_msl = gri->altitude*.001;
    asib->altitude_agl = gri->altitude_agl == EMPTY_FLAG
	  ? asib->altitude_msl : gri->altitude_agl*.001;
    asib->ew_velocity = gri->vew;
    asib->ns_velocity = gri->vns;
    asib->vert_velocity = gri->vud;
    asib->heading = gri->heading;
    asib->roll = gri->roll;
    asib->pitch = gri->pitch;
    asib->drift_angle = gri->drift;
# ifdef obsolete
    asib->rotation_angle = gri->corrected_rotation_angle;
# else
    asib->rotation_angle = gri->rotation_angle;
# endif
    asib->tilt = gri->tilt;
    asib->ew_horiz_wind = gri->ui;
    asib->ns_horiz_wind = gri->vi;
    asib->vert_wind = gri->wi;
    asib->heading_change = EMPTY_FLAG;
    asib->pitch_change = EMPTY_FLAG;

    prs = dgi->ray_que = dgi->ray_que->next;

    prs->source_sweep_num = gri->sweep_num;
//    sswb->stop_time = dgi->swp_que->end_time = prs->time = dgi->time;
    dgi->swp_que->end_time = prs->time = dgi->time;
    sswb->stop_time = (int32_t) dgi->time;
    prs->heading = asib->heading;
    prs->dH = angdiff(prs->last->heading, prs->heading);
    prs->pitch = asib->pitch;
    prs->dP = prs->pitch -prs->last->pitch;
    prs->rotation_angle =
	  fmod((double)(asib->rotation_angle +dds->cfac->rot_angle_corr),360.);
    dgi->swp_que->num_rays++;
    dgi->beam_count++;
    nc = dds->celv->number_cells;

    if( dgi->dds->radd->scan_mode == DD_SCAN_MODE_AIR ||
       dgi->dds->radd->radar_type == DD_RADAR_TYPE_AIR_LF ||
	dgi->dds->radd->radar_type == DD_RADAR_TYPE_AIR_NOSE )
      {
	 dd_radar_angles( asib, dds->cfac, dds->ra, dgi );
      }
    if(difs->altitude_truncations) {
	dgi->clip_gate = dd_clip_gate
	      (dgi, elev, asib->altitude_msl
	       , difs->altitude_limits->lower
	       , difs->altitude_limits->upper);
    }
    else
	  dgi->clip_gate = nc-1;

    prs->clip_gate = dgi->clip_gate;
    /*
     * stuff in the data  c...mark
     */
    for(ii=0; ii < gri->num_fields_present; ii++ ) {
	dds->field_present[ii] = YES;
# ifdef NEW_ALLOC_SCHEME
	aa = dds->qdat_ptrs[ii];
# else
	aa = (char *)dds->rdat[ii] +sizeof(struct paramdata_d);
# endif
	anb = gri->actual_num_bins[ii];
	ibad = dds->parm[ii]->bad_data;
	if(anb > dgi->clip_gate +1)
	      anb = dgi->clip_gate +1;

	if(gri->binary_format == DD_8_BITS) {
	    memcpy(aa, gri->byte_data[ii], anb);
	    for(; anb < nc; anb++ )
		  *(aa+anb) = ibad;
	}
	else {			/* assume 16_bit data */
	    memcpy(aa, gri->scaled_data[ii]
		   , SHORTS_TO_BYTES(anb));
	    for(ss = (short *)aa; anb < nc; anb++ )
		  *(ss+anb) = ibad;
	}
    }
    dgi->gpptr1 = gri->gpptr1;
    dgi->gpptr2 = gri->gpptr2;
    dgi->gpptr3 = gri->gpptr3;
    dgi->gpptr4 = gri->gpptr4;
    dgi->gpptr5 = gri->gpptr5;
    dgi->gpptr6 = gri->gpptr6;
    dgi->gpptr7 = gri->gpptr7;

    switch (gri->source_format) {
    case APIRAQ_FMT:
    case PIRAQX_FMT:
      if (!dds->xstf) {
	/* extra stuff */
 	nn = sizeof (XTRA_STUFF) + gri->sizeof_gpptr4;
	dds->xstf = (XTRA_STUFF *)malloc (nn);
	memset(dds->xstf, 0, nn);

	strncpy (dds->xstf->name_struct, "XSTF", 4);
	dds->xstf->sizeof_struct = nn;
	dds->xstf->one = 1;
	dds->xstf->source_format = PIRAQX_FMT;
	dds->xstf->offset_to_first_item = sizeof (XTRA_STUFF);
      }
      aa = (char *)dds->xstf +dds->xstf->offset_to_first_item;
      memcpy (aa, gri->gpptr4, gri->sizeof_gpptr4);
      dds->xstf->transition_flag = (gri->transition) ? IN_TRANSITION : NORMAL;
      break;

    case WSR_88D_FMT:
      break;

    default:
      break;
    };

    by_products(dgi,(time_t)gri->time);

    prev_fixed = gri->fixed;
    dd_dump_ray(dgi);
    dgi->new_vol = NO;
    dgi->new_sweep = NO;
    dgi->prev_scan_mode = dgi->dds->radd->scan_mode;
    dgi->prev_vol_num = dgi->dds->vold->volume_num;
}
/* c------------------------------------------------------------------------ */

void 
dd_vold_update (struct dd_general_info *dgi)
{
    struct dds_structs *dds=dgi->dds;
    struct volume_d *vold=dds->vold;
    struct super_SWIB *sswb=dds->sswb;
    struct sweepinfo_d *swib=dds->swib;
    DD_TIME *dts=dgi->dds->dts;


    vold->year = dts->year;
    vold->month = dts->month;
    vold->day = dts->day;
    vold->data_set_hour = dts->hour;
    vold->data_set_minute = dts->minute;
    vold->data_set_second = dts->second;

    strcpy( vold->proj_name, gri->project_name );
    /* append the site name
    dd_site_name( vold->proj_name, gri->site_name );
     */
    strncpy( vold->flight_num, gri->flight_id, 8 );
    dgi->nav_system = gri->nav_system;

    dd_radd_update(dds);
    dd_celv_update(dds);
    dd_dump_headers(dgi);
}
/* c------------------------------------------------------------------------ */

struct generic_radar_info *
return_gri_ptr (void)
{
    int j;
    char *c;
    
    if(!gri) {
	gri = (struct generic_radar_info *)
	      malloc(sizeof(struct generic_radar_info));
	memset((char *)gri, 0, sizeof(struct generic_radar_info));
	gri->dts = (struct d_time_struct *)malloc(sizeof(struct d_time_struct));
	memset((char *)gri->dts, 0, sizeof(struct d_time_struct));
	gri->binary_format = DD_16_BITS;
	gri->altitude_agl = EMPTY_FLAG;
	for(j=0; j < SRS_MAX_FIELDS; j++ ) {
	    gri->range_bin[j] = gri->range_value;
# ifdef obsolete
	    c = gri->rdat_ptr[j] = (char *)malloc
		  (sizeof(struct paramdata_d) +SRS_MAX_GATES*sizeof(short));
	    strncpy(c, "RDAT", 4);
	    gri->scaled_data[j] = (short *)(c +sizeof(struct paramdata_d));
# else
	    if(gri->scaled_data[j] = (short *)
	       malloc(SRS_MAX_GATES*sizeof(short)))
		  memset(gri->scaled_data[j], 0, SRS_MAX_GATES*sizeof(short));
	    else {
		printf("Unable to allocate gri\n");
		exit(1);
	    }
# endif
	    gri->byte_data[j] = (unsigned char *)gri->scaled_data[j];
	}
    }
    return(gri);
}
/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */


