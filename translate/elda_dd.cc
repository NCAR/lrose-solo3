#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NEW_ALLOC_SCHEME

/*
 * This file contains the following routines
 * 
 * 
 * eld_dd_conv
 * 
 * cspd2celv
 * dde_desc_fwd_sync
 * dor_print_flib
 * dor_print_lidr
 * 
 * 
 * eld_dump_raw_header
 * eld_dump_this_ray
 * eld_ini
 * eld_init_sweep
 * eld_interest
 * eld_inventory
 * eld_is_new_sweep
 * eld_lidar_avg
 * eld_lut
 * eld_next_ray
 * eld_positioning
 * eld_print_cspd
 * eld_print_frad
 * eld_print_ldat
 * eld_print_stat_line
 * eld_radar_rename
 * eld_reset
 * eld_return_eui
 * eld_select_ray
 * eld_stuff_data
 * eld_stuff_ray
 * 
 * 
 */

#include <LittleEndian.hh>
#include <sys/time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <TimeSeries.h>
#include <dd_math.h>
#include <time_dependent_fixes.h>
#include "dd_io_mgmt.hh"
#include "dd_stats.h"
#include "input_limits.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "dorade_share.hh"
#include "elda_dd.hh"
#include "swp_file_acc.hh"
#include "gpro_data.hh"
#include "dd_crackers.hh"
#include "dd_catalog.hh"
#include "ddin.hh"
#include "by_products.hh"
#include "dd_swp_files.hh"
#include "dorade_tape.hh"

# define UPPER(c) ((c) >= 'A' && (c) <= 'Z')

static struct volume_d *xvold;
static struct cell_spacing_d *xcspd;

static struct lidar_d *lidr, *xlidr;
static struct lidar_parameter_data *ldat, *xldat;
static struct field_lidar_i *flib;
static struct eldora_unique_info *eui=NULL;
static struct generic_descriptor *gd;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats=NULL;
static char desc_ids[512];
static RTIME D950502=0;
static RTIME D20030501=0;

static struct input_read_info *irq;
static int source_vol_count=0;
static int view_char=0, view_char_count=200;
static char preamble[24], postamble[64];
static struct io_packet vol_dp;
static struct solo_list_mgmt *slm;
static double start_test1, end_vortex_tests;
static double start_vortex_fix_1, end_vortex_fix_1;
static double time_for_nimbus;
static int lidar_data=NO;
static char *current_file_name=NULL;
static char *acmrg = NULL;


/* c------------------------------------------------------------------------ */

double 
eld_fixed_angle (struct dd_general_info *dgi)
{
    double d_rot=0;


    switch(dgi->dds->radd->scan_mode) {

    case DD_SCAN_MODE_AIR:
	d_rot = dd_tilt_angle(dgi);
	break;
    case DD_SCAN_MODE_RHI:
	d_rot = dd_azimuth_angle(dgi);
	break;
    case DD_SCAN_MODE_TAR:
    case DD_SCAN_MODE_VER:
	d_rot = dd_rotation_angle(dgi);
	break;
    default:
	d_rot = dd_tilt_angle(dgi);
	break;
    }
    return(d_rot);
}
/* c------------------------------------------------------------------------ */

void 
eld_dd_conv (int interactive)
{
    int i, n, mark, rn;
    int32_t tn;
    char str[256];
    static int count=0, count_rejects=0, count_break=6000;  //Jul 26, 2011
    struct individual_radar_info *iri;
    FILE *err_stream;

# ifdef obsolete
    sprintf(str, "/scr/steam/oye/err.%d.txt", time_now());
    err_stream = fopen(str, "w");
# endif    

    if(!count) {
	eldx_ini();
    }

    if(interactive) {
	dd_intset();
	eld_positioning();
    }
    eld_reset();

    /* loop through the data
     */
    while(1){
	if(++count >= count_break) {
	    mark = 0;
	}

# ifdef obsolete
	if(count > 6100) {
	    fprintf(err_stream, "%7d %.3f\n", count, eui->dgi->time
		    , eui->dgi->radar_num
		    , eui->dgi->in_swp_fid, eui->dgi->sweep_fid);
	    fflush(err_stream);
	}
# endif

	if(difs->stop_flag &&
	   (difs->abrupt_start_stop || eui->dgi->new_sweep))
	      break;

	if(eui->num_rays_avgd) {
	    eld_lidar_avg(eui->dgi);
	}

	if(eld_select_ray(eui->dgi)) {

	    iri = eui->iri[eui->dgi->radar_num];
	    if(difs->max_beams)
		  if(++iri->ray_count > difs->max_beams)
			break;

	    if(iri->sweep_count_flag) {
		eui->dgi->new_sweep = YES;
		iri->sweep_count_flag = NO;
		++iri->sweep_count;
		if(difs->max_sweeps)
		      if(iri->sweep_count > difs->max_sweeps)
			    break;
		if(iri->vol_count_flag) {
		    eui->dgi->new_vol = YES;
		    iri->vol_count_flag = NO;
		    ++iri->vol_count;
		    if(difs->max_vols)
			  if(iri->vol_count > difs->max_vols)
				break;
		}
	    }
	    /* pass it off to dorade code
	     */
	    eld_stuff_ray(eui->dgi, eui->current_time);
	}
	else {
	    if(!(++count_rejects % 2000)) {
		printf("Not ");
		eld_print_stat_line(count, eui->dgi);
	    }
	}

	if((n = eld_next_ray()) < 1)
	      break;
    }
}
/* c------------------------------------------------------------------------ */

void 
cspd2celv (struct dds_structs *dds, double spacing)
{
    /* convert range gate info to dorade format
       * "spacing" is used to indicate spacing that is non integer
     */
    int i, j, k, ns, nc=0;
    float dist, cs, corr=dds->cfac->range_delay_corr;

    dist = dds->cspd->distToFirst;

    for(i=k=0; i < dds->cspd->num_segments; i++ ) {
	ns = dds->cspd->num_cells[i];
	cs = spacing ? spacing : dds->cspd->spacing[i];
	nc += ns;
	for(j=0; j < ns; j++,k++,dist+=cs) {
	    dds->celv->dist_cells[k] = dist;
	}
    }
    dds->celv->number_cells = nc;
    dds->celv->cell_des_len = 12 + nc * sizeof(float);

    /* corrected cell vector */
    memcpy((char *)dds->celvc, (char *)dds->celv, sizeof(struct cell_d)); 
    memcpy((char *)dds->celvc, (char *)dds->celv
	   , dds->celv->cell_des_len );

    for(i=0; i < dds->celv->number_cells; i++){
	dds->celvc->dist_cells[i] += corr;
    }
}
/* c------------------------------------------------------------------------ */

int 
dde_desc_fwd_sync (char *aa, int ii, int jj)
{
    /* search for the next descriptor
     * from ii up to jj of aa
     */
    int cuc;
    static char dname[8]={0,0,0,0,0,0,0,0};

# ifdef obsolete
    if(hostIsLittleEndian()) {
# endif
	for(cuc=0; ii < jj; ii++) {
	    if(UPPER((int)(*(aa+ii)))) {
		if(++cuc == 4) {
		    strncpy(dname, aa+ii-3, 4);
		    if(strstr(desc_ids, dname)) { return(ii-3); }
		    --cuc;
		}
	    }
	    else { cuc=0; }
	}
# ifdef obsolete
    }
    else {
	for(; ii < jj;) {
	    /* find the next null byte counting consecutive UC
	     * characters along the way
	     */
	    for(cuc=0; ii < jj && *(aa+ii); ii++)
		if(UPPER((int)*(aa+ii)))
		    cuc++;
		else
		    cuc=0;

	    if(ii >= jj) break;
	    if(cuc > 3) {
		/* is this a legitimate descriptor? */
		if(strstr(desc_ids, aa+ii-4))
		    return(ii-4);
	    }
	    ii++;
	}
    }
# endif
    return(-1);
}
/* c------------------------------------------------------------------------ */

void 
dor_print_flib (struct field_lidar_i *flib, struct solo_list_mgmt *slm)
{
    int ii;
    char str[64];
    char *aa, bstr[128];

    aa = bstr;

    /* routine to print the contents of the parameter descriptor */

    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of the field lidar descriptor  len: %d"
            , sizeof(struct parameter_d));
    solo_add_list_entry(slm, aa);

    sprintf(aa, " field_lidar_info[4]   %s", flib->field_lidar_info);   
    solo_add_list_entry(slm, aa);
    sprintf(aa, " field_lidar_info_len  %d", flib->field_lidar_info_len);  
    solo_add_list_entry(slm, aa);
    sprintf(aa, " data_sys_id           %d", flib->data_sys_id);           
    solo_add_list_entry(slm, aa);

    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " transmit_beam_div[%d] %f"
		, ii, flib->transmit_beam_div[ii]); 
	solo_add_list_entry(slm, aa);
    }
    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " xmit_power[%d]        %f"
		, ii, flib->xmit_power[ii]);        
	solo_add_list_entry(slm, aa);
    }
    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " receiver_fov[%d]      %f"
		, ii, flib->receiver_fov[ii]);      
	solo_add_list_entry(slm, aa);
    }
    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " receiver_type[%d]     %d"
		, ii , flib->receiver_type[ii]);     
	solo_add_list_entry(slm, aa);
    }
    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " r_noise_floor[%d]     %f"
		, ii, flib->r_noise_floor[ii]);     
	solo_add_list_entry(slm, aa);
    }
    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " receiver_spec_bw[%d]  %f"
		, ii, flib->receiver_spec_bw[ii]);  
	solo_add_list_entry(slm, aa);
    }

    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " receiver_elec_bw[%d]  %f"
		, ii, flib->receiver_elec_bw[ii]);  
	solo_add_list_entry(slm, aa);
    }
    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " calibration[%d]       %f"
		, ii, flib->calibration[ii]);       
	solo_add_list_entry(slm, aa);
    }
    sprintf(aa, " range_delay           %d", flib->range_delay);           
    solo_add_list_entry(slm, aa);

    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " peak_power_multi[%d]  %f"
		, ii, flib->peak_power_multi[ii]);  
	solo_add_list_entry(slm, aa);
    }
    sprintf(aa, " encoder_mirror_up     %f", flib->encoder_mirror_up);     
    solo_add_list_entry(slm, aa);
    sprintf(aa, " pitch_mirror_up       %f", flib->pitch_mirror_up);       
    solo_add_list_entry(slm, aa);
    sprintf(aa, " max_digitizer_count   %d", flib->max_digitizer_count);   
    solo_add_list_entry(slm, aa);
    sprintf(aa, " max_digitizer_volt    %f", flib->max_digitizer_volt);    
    solo_add_list_entry(slm, aa);
    sprintf(aa, " digitizer_rate        %f", flib->digitizer_rate);        
    solo_add_list_entry(slm, aa);
    sprintf(aa, " total_num_samples     %d", flib->total_num_samples);     
    solo_add_list_entry(slm, aa);
    sprintf(aa, " samples_per_cell      %d", flib->samples_per_cell);      
    solo_add_list_entry(slm, aa);
    sprintf(aa, " cells_per_ray         %d", flib->cells_per_ray);         
    solo_add_list_entry(slm, aa);
    sprintf(aa, " pmt_temp              %f", flib->pmt_temp);              
    solo_add_list_entry(slm, aa);
    sprintf(aa, " pmt_gain              %f", flib->pmt_gain);              
    solo_add_list_entry(slm, aa);
    sprintf(aa, " apd_temp              %f", flib->apd_temp);              
    solo_add_list_entry(slm, aa);
    sprintf(aa, " apd_gain              %f", flib->apd_gain);              
    solo_add_list_entry(slm, aa);
    sprintf(aa, " transect              %d", flib->transect);              
    solo_add_list_entry(slm, aa);

    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " derived_names[%d][12] %s"
		, ii, str_terminate(str, flib->derived_names[ii], 12)); 
	solo_add_list_entry(slm, aa);
    }
    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " derived_units[%d][8]  %s"
		, ii, str_terminate(str, flib->derived_units[ii], 8));  
	solo_add_list_entry(slm, aa);
    }
    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " temp_names[%d][12]    %s"
		, ii, str_terminate(str, flib->temp_names[ii], 12));    
	solo_add_list_entry(slm, aa);
    }

    return;
}
/* c------------------------------------------------------------------------ */

void 
dor_print_lidr (struct lidar_d *lidr, struct solo_list_mgmt *slm)
{
    int ii;
    char str[64];
    char *aa, bstr[128];

    aa = bstr;
    /* routine to print the contents of the descriptor */

    solo_add_list_entry(slm, " ");

    sprintf(aa, "Contents of the lidar descriptor  len: %d"
	    , sizeof(struct lidar_d));

    solo_add_list_entry(slm, aa);
    sprintf(aa, " lidar_des[4]      %s", lidr->lidar_des);     
    solo_add_list_entry(slm, aa);
    sprintf(aa, " lidar_des_length  %d", lidr->lidar_des_length); 
    solo_add_list_entry(slm, aa);
    sprintf(aa, " lidar_name[8]     %s"
	    , str_terminate(str, lidr->lidar_name, sizeof(lidr->lidar_name))); 
    solo_add_list_entry(slm, aa);
    sprintf(aa, " lidar_const       %f", lidr->lidar_const);      
    solo_add_list_entry(slm, aa);
    sprintf(aa, " pulse_energy      %f", lidr->pulse_energy);     
    solo_add_list_entry(slm, aa);
    sprintf(aa, " peak_power        %f", lidr->peak_power);       
    solo_add_list_entry(slm, aa);
    sprintf(aa, " pulsewidth        %f", lidr->pulsewidth);       
    solo_add_list_entry(slm, aa);
    sprintf(aa, " aperature_size    %f", lidr->aperature_size);   
    solo_add_list_entry(slm, aa);
    sprintf(aa, " field_of_view     %f", lidr->field_of_view);    
    solo_add_list_entry(slm, aa);
    sprintf(aa, " aperature_eff     %f", lidr->aperature_eff);    
    solo_add_list_entry(slm, aa);
    sprintf(aa, " beam_divergence   %f", lidr->beam_divergence);  
    solo_add_list_entry(slm, aa);
    sprintf(aa, " lidar_type        %d", lidr->lidar_type);       
    solo_add_list_entry(slm, aa);
    sprintf(aa, " scan_mode         %d", lidr->scan_mode);        
    solo_add_list_entry(slm, aa);
    sprintf(aa, " req_rotat_vel     %f", lidr->req_rotat_vel);    
    solo_add_list_entry(slm, aa);
    sprintf(aa, " scan_mode_pram0   %f", lidr->scan_mode_pram0);  
    solo_add_list_entry(slm, aa);
    sprintf(aa, " scan_mode_pram1   %f", lidr->scan_mode_pram1);  
    solo_add_list_entry(slm, aa);
    sprintf(aa, " num_parameter_des %d", lidr->num_parameter_des);
    solo_add_list_entry(slm, aa);
    sprintf(aa, " total_number_des  %d", lidr->total_number_des); 
    solo_add_list_entry(slm, aa);
    sprintf(aa, " data_compress     %d", lidr->data_compress);    
    solo_add_list_entry(slm, aa);
    sprintf(aa, " data_reduction    %d", lidr->data_reduction);   
    solo_add_list_entry(slm, aa);
    sprintf(aa, " data_red_parm0    %f", lidr->data_red_parm0);   
    solo_add_list_entry(slm, aa);
    sprintf(aa, " data_red_parm1    %f", lidr->data_red_parm1);   
    solo_add_list_entry(slm, aa);
    sprintf(aa, " lidar_longitude   %f", lidr->lidar_longitude);  
    solo_add_list_entry(slm, aa);
    sprintf(aa, " lidar_latitude    %f", lidr->lidar_latitude);   
    solo_add_list_entry(slm, aa);
    sprintf(aa, " lidar_altitude    %f", lidr->lidar_altitude);   
    solo_add_list_entry(slm, aa);
    sprintf(aa, " eff_unamb_vel     %f", lidr->eff_unamb_vel);    
    solo_add_list_entry(slm, aa);
    sprintf(aa, " eff_unamb_range   %f", lidr->eff_unamb_range);  
    solo_add_list_entry(slm, aa);
    sprintf(aa, " num_wvlen_trans   %d", lidr->num_wvlen_trans);  
    solo_add_list_entry(slm, aa);
    sprintf(aa, " prf               %f", lidr->prf);              
    solo_add_list_entry(slm, aa);

    for(ii=0; ii < 10; ii++) {
	sprintf(aa, " wavelength[%d]    %f", ii, lidr->wavelength[ii]);   
	solo_add_list_entry(slm, aa);
    }
    return;
}
/* c------------------------------------------------------------------------ */

void 
eld_dump_raw_header (struct dd_general_info *dgi, struct input_read_info *irq, struct io_packet *vol_dp, struct solo_list_mgmt *slm)
{
    int bytes_left, limit, max, mark;
    struct generic_descriptor xgd;
    char *at, *xat;
    char *aa, bstr[128];
    static char * xbuf = NULL;
    struct cell_d * celv;
	struct generic_descriptor *gd;

    if( hostIsLittleEndian() && !xbuf) {
       xbuf = (char *)malloc( 16384 ); 
       celv = (struct cell_d *)xbuf;
    }
    aa = bstr;

    solo_reset_list_entries(slm);
    at = vol_dp->at;
    bytes_left = vol_dp->bytes_left;
    limit = vol_dp->bytes_left-vol_dp->len;
    limit = 0;

    /* see if raw buffer is still there
     */
    if(!vol_dp->ib || vol_dp->seq_num != vol_dp->ib->seq_num) {
	printf("\nBuffer containing this info no longer available!\n");
	return;
    }
    max = dgi->source_num_parms * dgi->dds->celv->number_cells * sizeof(short)
	   + sizeof(struct field_parameter_data);


    /* loop through this buffer
     */
    for(; bytes_left > sizeof(struct generic_descriptor);) {
	if( hostIsLittleEndian() ) {
	   gd = &xgd;
	   memcpy(gd->name_struct, at, 4);
	   swack4(at+4, (char *)&gd->sizeof_struct);
	}
	else {
	   gd = (struct generic_descriptor *)at;
	}	
	xat = hostIsLittleEndian() ? xbuf : at;

	if(!strncmp(at, "PARM", 4)) {
	   if( hostIsLittleEndian() ) {
	      ddin_crack_parm(at, xat, (int)0);
	   }
	   dor_print_parm((parameter_d *)xat, slm);
	}
	else if(!strncmp(at, "RADD", 4)) {
	   if( hostIsLittleEndian() ) {
	      ddin_crack_radd(at, xat, (int)0);
	   }
	    dor_print_radd((radar_d *)xat, slm);
	}
	else if(!strncmp(at, "VOLD", 4)) {
	   if( hostIsLittleEndian() ) {
	      ddin_crack_vold(at, xat, (int)0);
	   }
	    dor_print_vold((volume_d *)xat, slm);
	}
	else if(!strncmp(at, "CELV", 4)) {
	   if( hostIsLittleEndian() ) {
	      ddin_crack_celv(at, (char *)celv, (int)0);
	      swack_long(at+12, (char *)&celv->dist_cells[0],
			 (int)celv->number_cells);
	   }
	   dor_print_celv((cell_d *)xat, slm);
	}
	else if(!strncmp(at, "CSPD", 4)) {
	   if( hostIsLittleEndian() ) {
	      eld_crack_cspd(at, xat, (int)0);
	   }
	   eld_print_cspd((cell_spacing_d *)xat, slm);
	}
	else if(!strncmp(at, "FLIB", 4)) {
	   if( hostIsLittleEndian() ) {
	      mark = 0;
	   }
	   dor_print_flib((field_lidar_i *)xat, slm);
	}
	else if(!strncmp(at, "LIDR", 4)) {
	   if( hostIsLittleEndian() ) {
	      xat = xbuf;
	      eld_crack_lidr(at, xat, (int)0);
	   }
	   dor_print_lidr((lidar_d *)xat, slm);
	}
# ifdef obsolete
	if(gd->sizeof_struct < sizeof(struct generic_descriptor) ||
	   gd->sizeof_struct > max)
	      break;
# else
# endif
	at += gd->sizeof_struct;
	bytes_left -= gd->sizeof_struct;
    }
    slm_print_list(slm);
}
/* c------------------------------------------------------------------------ */

void 
eld_dump_this_ray (struct dd_general_info *dgi, struct solo_list_mgmt *slm)
{
    int ii, gg, np=dgi->source_num_parms, sizeof_data;
    char *b;
    char ts[12];
    short *flds[7];
    float unscale[7], bias[7];
    char *aa, bstr[128];

    aa = bstr;

    solo_reset_list_entries(slm);
    dor_print_ryib(dgi->dds->ryib, slm);
    dor_print_asib(dgi->dds->asib, slm);
    if(eui->data_type & LIDAR_DATA) {
	eld_print_ldat(dgi->dds->ldat, slm);
    }
    else {
	eld_print_frad(dgi->dds->frad, slm);
    }


    dgi->clip_gate = dgi->dds->celv->number_cells-1;

    for(ii=0; ii < np; ii++) {
	sizeof_data = eld_stuff_data(dgi, ii);
    }

    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of data fields");
    solo_add_list_entry(slm, aa);

    sprintf(aa, "name ");
    if(np > 7) np = 7;

    for(ii=0; ii < np; ii++) {
	sprintf(aa+strlen(aa), "%10s"
		, str_terminate(ts, dgi->dds->parm[ii]->parameter_name, 8));
# ifdef NEW_ALLOC_SCHEME
	flds[ii] = (short *)dgi->dds->qdat_ptrs[ii];
# else
	flds[ii] = (short *)((char *)dgi->dds->rdat[ii] +
			     sizeof(struct paramdata_d));
# endif
	bias[ii] = dgi->dds->parm[ii]->parameter_bias;
	unscale[ii] = dgi->dds->parm[ii]->parameter_scale ?
	      1./dgi->dds->parm[ii]->parameter_scale : 1.;
    }
    solo_add_list_entry(slm, aa);

    for(gg=0; gg < dgi->dds->celv->number_cells; gg++) {
	sprintf(aa, "%4d)", gg);

	for(ii=0; ii < np; ii++) {
	    sprintf(aa+strlen(aa), "%10.2f"
		    , DD_UNSCALE(*(flds[ii]+gg), unscale[ii], bias[ii]));
	}
	solo_add_list_entry(slm, aa);
    }
    slm_print_list(slm);
}
/* c------------------------------------------------------------------------ */

void 
eldx_ini (void)
{
    int i, jj, kk, n, nt, arg_num=0;
    char *a, *aa, *bb, *cc, *dd;
    char string_space[2048], *str_ptrs[256], str[16];
    double d, d1, d2;
    struct time_dependent_fixes *fx;
    float f;
    DD_TIME dts;



    /* eld_initialize!
     */
    dd_min_rays_per_sweep();	/* trigger min rays per sweep */
    dd_enable_cat_comments();
    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();
    /*
     * descriptor ids for searching later
     */
    strcpy(desc_ids, DD_ID_LIST);
    strcat(desc_ids, ",NDDS,SITU,ISIT,TIME,INDF,MINI,LDAT,LIDR,FLIB,RAWD");
    dd_clear_dts( &dts );
    dts.year = 1995;
    dts.month = 5;
    dts.day = 2;
    D950502 = d_time_stamp( &dts );
    dd_clear_dts( &dts );
    dts.year = 2003;
    dts.month = 5;
    dts.day = 1;
    D20030501 = d_time_stamp( &dts );
    
    

    /*
     * set up useful structures and control parameters
     */
    if(hostIsLittleEndian()) {
       xldat = (struct lidar_parameter_data *)
	 malloc(sizeof(struct lidar_parameter_data));
       memset(xldat, 0, sizeof(struct lidar_parameter_data));
       xlidr = (struct lidar_d *)malloc(sizeof(struct lidar_d));
       memset(xlidr, 0, sizeof(struct lidar_d));
       xvold = (struct volume_d *)malloc(sizeof(struct volume_d));
       memset(xvold, 0, sizeof(struct volume_d));
       xcspd = (struct cell_spacing_d *)malloc(sizeof(struct cell_spacing_d));
       memset(xcspd, 0, sizeof(struct cell_spacing_d));
    }
    eui = (struct eldora_unique_info *)
	  malloc(sizeof(struct eldora_unique_info));
    memset((char *)eui, 0, sizeof(struct eldora_unique_info));
    eui->funky_sweep_trip_angle = -360;
    eui->funky_sweep_trip_delta = 360.;
    eui->fx = dd_time_dependent_fixes(&eui->fx_count);

    for(i=0; i < MAX_SENSORS; i++){
	eui->iri[i] = (struct individual_radar_info *)
	      malloc(sizeof(struct individual_radar_info));
	memset((char *)eui->iri[i], 0
	       , sizeof(struct individual_radar_info));
	eui->iri[i]->rotang_list[0] = -360.;
	eui->iri[i]->radar_num = -1;
    }
    memset(&vol_dp, 0, sizeof(struct io_packet));

    flib = ( struct field_lidar_i*)malloc(sizeof(struct field_lidar_i));
    memset(flib, 0, sizeof(struct field_lidar_i));

    if(a=get_tagged_string("SWEEP_TRIP_ANGLE")) {
	eui->funky_sweep_trip_angle = atof(a);
	eui->options |= ELD_FUNKY_SCANS;
	printf( "Sweep trip angle: %.2f\n", eui->funky_sweep_trip_angle);
    }
    if(a=get_tagged_string("SWEEP_TRIP_DELTA")) {
	eui->funky_sweep_trip_delta = atof(a);
	eui->options |= ELD_FUNKY_SCANS;
	printf( "Sweep trip delta: %.2f\n", eui->funky_sweep_trip_delta);
    }
    if(a=get_tagged_string("BEAM_SKIP")) {
	eui->beam_skip = atoi(a);
	printf( "Beam skip: %d\n", eui->beam_skip);
    }
    if(a=get_tagged_string("JULIAN_DAY_BIAS")) {
	eui->julian_day_bias = atoi(a);
	eui->time_correction = -(eui->julian_day_bias*SECONDS_IN_A_DAY);
	printf( "Julian_day_bias: %d\n", eui->julian_day_bias);
    }
    if(a=get_tagged_string("TIME_CORRECTION")) {
	eui->time_correction += atof(a);
	printf( "Time correction: %.3f seconds\n", eui->time_correction);
    }
    if(a=get_tagged_string("YEAR_OF_DATA")) {
	if((jj = atoi(a)) > 0) {
	    eui->fix_year = jj > 1900 ? jj : jj + 1900;
	    printf( "Year of data: %d\n", eui->fix_year);
	}
    }
    if(a=get_tagged_string("LIDAR_SWEEP_TIME_LIMIT")) {
	if((d = atof(a)) > 0) {
	    eui->lidar_sweep_time_limit = d;
	    printf( "Lidar_sweep_time_limit: %.3f seconds\n"
		   , eui->lidar_sweep_time_limit);
	}
	eui->fake_sweep_nums = YES;
    }
    if((a=get_tagged_string("OUTPUT_FLAGS"))) {
       if (strstr (a, "TIME_S")) {
	  eui->options |= ELD_TIME_SERIES;
       }
    }
    if((a=get_tagged_string("OPTIONS"))) {
	if(strstr(a, "COARE_FIX")) {
	    eui->options |= TOGACOARE_FIXES;
	}
	if(strstr(a, "AC_COMPARE")) {
	    eui->options |= ELD_AC_COMPARE;
	}
	if(strstr(a, "DESC_SEARCH")) {
	    eui->options |= ELD_DESC_SEARCH;
	}
	if(strstr(a, "ROT_ANG_S")) { /* ROT_ANG_SECTORS */
	    eui->options |= ROT_ANG_SECTORS;
	}
    }

    if((a=get_tagged_string("LIDAR_AVERAGING"))) {
	strcpy(string_space, a);
	nt = dd_tokens(string_space, str_ptrs);

	for(jj=0; nt > 2 && jj < nt; jj += 3) {
	    bb = str_ptrs[jj]; cc = str_ptrs[jj+2];
	    if(strstr(bb, "NUM_SHOT")) {
		if(sscanf(cc, "%d", &n) == 1) {
		    if(n > 0) {
			eui->num_rays_avgd = n;
			printf("Num lidar shots averaged: %d\n", n);
		    }
		}
	    }
	    else if(strstr(bb, "NUM_CELL")) {
		if(sscanf(cc, "%d", &n) == 1) {
		    if(n > 0) {
			eui->num_cells_avgd = n;
			printf("Num lidar cells averaged: %d\n", n);
		    }
		}
	    }
	}
	eui->fake_sweep_nums = YES;
    }
    eui->datum_size[DD_8_BITS] = 1;
    eui->datum_size[DD_16_BITS] = 2;
    eui->datum_size[DD_24_BITS] = 3;
    eui->datum_size[DD_32_BIT_FP] = 4;
    eui->datum_size[DD_16_BIT_FP] = 2;

    strcpy(eui->site_name, "ELECTRA ");

    /* time limits for flights where the velocity sign is reversed
     * or whatever else need fixing
     */
    bb = (char *)"01/01/95:00:00"; dd_crack_datime(bb, strlen(bb), &dts);
    time_for_nimbus = start_vortex_fix_1 = d_time_stamp(&dts);

    bb = (char *)"04/01/95:00:00"; dd_crack_datime(bb, strlen(bb), &dts);
    end_vortex_tests = d_time_stamp(&dts);

    bb = (char *)"04/22/95:00:00"; dd_crack_datime(bb, strlen(bb), &dts);
    end_vortex_fix_1 = d_time_stamp(&dts);

    irq = dd_return_ios(2, ELDORA_FMT);

    if((a=get_tagged_string("ELDORA_VOLUME_HEADER")) ||
       (a=get_tagged_string("VOLUME_HEADER"))) {
	/* nab a volume header from another file
	 */
	dd_input_read_open(irq, a);
	n = eld_next_ray();
	dd_input_read_close(irq);
    }

    if((aa=get_tagged_string("TAPE_DIR"))) {
      slm = solo_malloc_list_mgmt(256);
      nt = generic_sweepfiles( aa, slm, (char *)"", (char *)".tape", 0 );
      if (nt < 1) {
	nt = generic_sweepfiles( aa, slm, (char *)"eldora", (char *)"", 0 );
      }
      if ( nt < 1 ) {		/* Josh's CDs */
	  nt = generic_sweepfiles( aa, slm, (char *)"", (char *)"z", 0 );
      }
      bb = NULL;
      if (nt > 0) {
	bb = dd_est_src_dev_names_list((char *)"ELD", slm, aa );
      }
    }
    else if((aa=get_tagged_string("SOURCE_DEV"))) {
        bb = dd_establish_source_dev_names((char *)"ELD", aa);
    }
    if( aa && bb ) {
      current_file_name = dd_next_source_dev_name((char *)"ELD");
      dd_input_read_open(irq, current_file_name );
    }
    else {
	printf("SOURCE_DEV not defined!\n");
	exit(1);
    }
    /*
     * fill up the io buffers
     */
    for(jj=0; jj < irq->que_count; jj++) {
	dd_logical_read(irq, FORWARD);
    }
    dd_skip_recs(irq, BACKWARD, irq->que_count);
    /*
     * loop through the data until a header is encountered
     */
    if((n = eld_next_ray()) < 1) {
	printf("Could not read in the first ray...state=%d\n", n );
	exit(1);
    }
}
/* c------------------------------------------------------------------------ */


void eld_init_sweep(struct dd_general_info *dgi,  time_t current_time,  int new_vol)
{
    short tdate[6];
    int ii, mark;
    struct dds_structs *dds=dgi->dds;
    struct super_SWIB *sswb=dds->sswb;
    struct sweepinfo_d *swib=dds->swib;
    struct ray_i *ryib=dds->ryib;
    struct platform_i *asib=dds->asib;
    struct prev_swps *pss, *this_swps; //Jul 26, 2011 *this issue
    struct individual_radar_info *iri=eui->iri[dgi->radar_num];
    DD_TIME *dts=dgi->dds->dts;
    
    mark = 0;

# ifdef obsolete
    /* first see if we alread have some rays for this sweep number
     */
    for(this_swps=dgi->swp_que->last,ii=0; ii < MAX_PREV_SWPS; ii++) {
	if(this_swps->source_sweep_num == ryib->sweep_num) {
	    break;
	}	      
	this_swps = this_swps->last;
    }
# endif

    dd_flush(dgi->radar_num);

    if(iri->bad_ray_count > 0) {
# ifdef obsolete
	printf("Bad ray count for previous sweep: %d\n"
	       , iri->bad_ray_count);
# endif
	iri->bad_ray_count = 0;
    }
    d_unstamp_time(dts);
    dgi->sweep_count++;

    pss = dgi->swp_que = dgi->swp_que->next;
    if(eui->fake_sweep_nums) {
	dgi->source_sweep_num =
	      pss->source_sweep_num = iri->sweep_count;
	dgi->source_vol_num = 
	      pss->source_vol_num = iri->vol_count;
    }
    else {
	pss->source_sweep_num =  ryib->sweep_num;
	pss->source_vol_num = dgi->vol_num;
    }
    pss->segment_num = dts->millisecond;
    pss->listed = NO;
    pss->new_vol = dgi->new_vol;
    pss->start_time = dgi->time;
    pss->sweep_time_stamp = dgi->sweep_time_stamp = (int32_t) dgi->time;
    pss->num_rays = 0;
    pss->sweep_file_size = sizeof(struct sweepinfo_d);
    pss->volume_time_stamp = 
	  sswb->volume_time_stamp = dgi->volume_time_stamp;

    /* super sweep info block */
    sswb->last_used = todays_date(tdate);
    sswb->start_time = (int32_t) dgi->time;
    for(ii=0; ii < MAX_PARMS; ii++ ) {
	/* get the names into the rdats */
	if(dgi->dds->field_present[ii]) {
# ifdef NEW_ALLOC_SCHEME
	    strncpy(dds->qdat[ii]->pdata_name
		    , dds->parm[ii]->parameter_name, 8);
# else
	    strncpy(dds->rdat[ii]->pdata_name
		    , dds->parm[ii]->parameter_name, 8);
# endif
	}
    }
    dd_tape(dgi, NO);

    /* sweep info block
     */
    dd_blank_fill(dds->radd->radar_name, 8, swib->radar_name);
    swib->fixed_angle = eld_fixed_angle(dgi);
    swib->fixed_angle = dds->asib->tilt;
    swib->filter_flag = NO_FILTERING;
    strncpy((char *)swib->radar_name, (char *)dds->radd->radar_name, 8);
}
/* c------------------------------------------------------------------------ */

void 
eld_interest (struct dd_general_info *dgi, int verbosity, char *preamble, char *postamble)
{
    /* print interesting info from the gri struct such as:
     * time
     * radar
     * fixed angle
     * prf
     * sweep num
     * azimuth
     * elevation
     * aircraft positioning info if this is aircraft data
     * lat/lon/alt
     * fields present
     * field data
     */
    int ii;
    char str[256];

    printf("%s ", preamble);
    printf("%s ", dts_print(d_unstamp_time(dgi->dds->dts)));
    printf("%s ", dgi->radar_name.c_str());
    printf("t:%5.1f ", dgi->dds->asib->tilt);
    printf("rt:%6.1f ", dgi->dds->asib->rotation_angle);
    printf("rl:%5.1f ", dgi->dds->asib->roll);
    printf("h:%6.1f ", dgi->dds->asib->heading);
    printf("al:%6.3f ", dgi->dds->asib->altitude_msl);
    printf("swp: %2d ", dgi->dds->ryib->sweep_num);
    printf("%s ", postamble);
    printf("\n");
    if(verbosity) {
	printf("p:%5.1f ", dgi->dds->asib->pitch);
	printf("d:%5.1f ", dgi->dds->asib->drift_angle);
	printf("la:%9.4f ", dgi->dds->asib->latitude);
	printf("lo:%9.4f ", dgi->dds->asib->longitude);
	printf("ag:%6.3f ", dgi->dds->asib->altitude_agl);
	printf("\n");
	printf("ve:%6.1f ", dgi->dds->asib->ew_velocity);
	printf("vn:%6.1f ", dgi->dds->asib->ns_velocity);
	printf("vv:%6.1f ", dgi->dds->asib->vert_velocity);
	printf("we:%6.1f ", dgi->dds->asib->ew_horiz_wind);
	printf("wn:%6.1f ", dgi->dds->asib->ns_horiz_wind);
	printf("wv:%6.1f ", dgi->dds->asib->vert_wind);
	printf("\n");
	printf("Num parameters: %d  ", dgi->source_num_parms);
	for(ii=0; ii < dgi->source_num_parms; ii++) {
	    printf("%s ", str_terminate
		   (str, dgi->dds->parm[ii]->parameter_name, 8));
	}
	printf("\n");
    }
}
/* c------------------------------------------------------------------------ */

int 
eld_inventory (void)
{
    /* the purpose of this routine is to facilitate a more detailed
     * examination of the data
     */
    int ii=0, max=gri_max_lines();
    int nn, ival;
    float val;
    char str[256];


    for(;;) {
	for(ii=0; ii < max; ii++) {
	    if((nn=eld_next_ray()) < 1)
		  break;
	    eui->dgi->new_sweep = NO;
	    eui->dgi->new_vol = NO;
	    sprintf(preamble, "%2d", irq->top->b_num);
	    eld_interest(eui->dgi, 0, preamble, postamble);
	}
	if(nn < 1)
	      break;
	printf("Hit <ret> to continue: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || ival) {
	    break;
	}
    }
    return(ival);
}
/* c------------------------------------------------------------------------ */

int eld_is_new_sweep (struct dd_general_info *dgi)
{
    int ii, n_new=NO, rn=eui->radar_num;  //Jul 26, 2011 int new issue
    float ra, rb;
    struct dds_structs *dds=dgi->dds;
    struct individual_radar_info *iri;


    iri = eui->iri[rn];

    if(ELD_FUNKY_SCANS SET_IN eui->options) {
	ii = DEC_NDX(iri->rotang_ndx, ROTANG_LIST_SIZE);
	ra = iri->rotang_list[ii];
	rb = iri->rotang_list[iri->rotang_ndx];

	if(eui->funky_sweep_trip_delta < 360.) {
	    if(fabs(angdiff(ra, rb)) > eui->funky_sweep_trip_delta) {
		n_new = YES;
	    }
	}
	if(eui->funky_sweep_trip_angle >= 0.) {
	    if(ii = in_sector(eui->funky_sweep_trip_angle, ra, rb)) {
		n_new = YES;
	    }
	}
    }
    else if(eui->lidar_sweep_time_limit) {
	if(eui->dgi->time > iri->sweep_reference_time) {
	    iri->sweep_reference_time = eui->dgi->time +
		  eui->lidar_sweep_time_limit;
	    iri->sweep_count_flag = YES;
	    n_new = YES;
	}
    }
    else if(dgi->source_sweep_num != dds->ryib->sweep_num) {
	n_new = YES;
    }

    if(n_new) {
	dgi->new_sweep = YES;
	dd_stats->sweep_count++;
	iri->sweep_count_flag = YES;
    }
    dgi->source_sweep_num = dds->ryib->sweep_num;
    return(n_new);
}
/* c------------------------------------------------------------------------ */

void eld_lidar_avg (DGI_PTR dgi)
{
    /* assume for now we will be averaging the two fields GREEN and INFRARED
     */
    struct individual_radar_info *iri=eui->iri[dgi->radar_num];
    int gg, ii, jj, kk, ll, mm, nn, mark;
    int nca = eui->num_cells_avgd, nra = eui->num_rays_avgd;
    int loop, pn, ndx, sizeof_datum, nc, frg, lrg, bad, doit;
    int32_t *sums, *lp, *lpzz;
    double rcp_num;
    struct running_avg_que *raq;
    struct que_val *qv;
    char *aa, *name, *deep6=NULL;
    unsigned short *us, *zz;
    struct dds_structs *dds=dgi->dds;
    struct cell_d *celv=dds->celv;
    struct field_parameter_data *frad=dds->frad;
    struct parameter_d *parm;

    
    if(!eui->num_rays_avgd)
	  return;

    nc = celv->number_cells;

    if(eui->max_cells < nc) {
	/* initialize!
	 */
	if(iri->green_sums) free(iri->green_sums);
	if(iri->red_sums) free(iri->red_sums);
	nn = eui->max_cells = nc;
	nn *= sizeof(int32_t);

	lp = iri->green_sums = (int32_t *)malloc(nn);
	memset(lp, 0, nn);
	lp = iri->red_sums = (int32_t *)malloc(nn);
    	memset(lp, 0, nn);

        eui->rcp_rays_avgd = 1./eui->num_rays_avgd;

	if(eui->num_cells_avgd) {
	    iri->green_raq = se_return_ravgs(eui->num_cells_avgd);
	    iri->red_raq = se_return_ravgs(eui->num_cells_avgd);
	}
    }

    if(++iri->ray_avg_count < nra) {
	doit = NO;
	eui->ignore_this_ray = YES;
    }
    else {
	iri->ray_avg_count = 0;
	doit = YES;
    }
    if(iri->ray_avg_count == eui->num_rays_avgd/2) {
	iri->avg_time_ctr = dgi->time;
    }
    kk = iri->num_parms;

    for(ii=0; ii < kk; ii++) dgi->dds->field_present[ii] = YES;


    for(loop=0; loop < 2; loop++) {
	switch(loop) {
    	case 0:
	    pn = dd_find_field(dgi, (char *)"GREEN");
	    sums = iri->green_sums;
	    raq = iri->green_raq;
	    break;
	case 1:
	    pn = dd_find_field(dgi, (char *)"INFRARED");
	    sums = iri->red_sums;
	    raq = iri->red_raq;
	    break;
	}
	
	sizeof_datum = iri->sizeof_fparm[pn];
	frg = frad->first_rec_gate > 0 ? frad->first_rec_gate : 0;
	lrg = frad->last_rec_gate > 0 ? frad->last_rec_gate+1 : nc;
	if(lrg > nc) {
	    lrg = nc;
	}
	
	us = (unsigned short *)(dds->raw_data +iri->fparm_offset[pn]);
	zz = us + kk * nc;
	lp = sums;
	lpzz = lp + nc;
	
	if(nca) {		/* cell averaging */
	    /* load up running average queue
	     */
	    qv = raq->at;
	    raq->sum = 0;
	    for(ii=0; ii < nca; ii++) {
		qv->val = *(us + ii * kk);
		raq->sum += qv->val;
		qv = qv->next;
	    }
	    raq->at = qv;
	    rcp_num = raq->rcp_num_vals;
	    /*
	     * leave the first two cells alone!
	     */
	    (*lp++) += *us; us += kk;
	    (*lp++) += *us; us += kk;
	    ll = kk * (nca -2);

	    nn = lrg - nca;
	    for(; nn-- ; us += kk) {
		(*lp++) += (int32_t) (rcp_num * raq->sum);
		qv = raq->at;
		raq->sum -= qv->val;
		qv->val = *(us + ll);
		raq->sum += qv->val;
		raq->at = qv->next;
	    }
	    for(nn=nca-2; nn-- ;) {
		(*lp++) += (int32_t)(rcp_num * raq->sum);
	    }
	}
	else {			/* do not average cells */
	    mark = 0;
	    for(nn=lrg; nn-- ; us += kk) {
		if(lp >= lpzz || us >= zz) {
		    mark = 0;
		}
		*lp++ += *us;
	    }
	}

	if(doit) {
	    bad = dgi->dds->parm[pn]->bad_data;
	    rcp_num = eui->rcp_rays_avgd;
	    lp = sums;
	    us = (unsigned short *)dds->qdat_ptrs[pn];

	    for(gg=0; gg < lrg; gg++) {
		*us++ = (unsigned short)(rcp_num * (*lp++));
	    }
	    for(; gg++ < nc; *us++ = bad);
	    memset(sums, 0, eui->max_cells * sizeof(int32_t));
	}
    }
}
/* c------------------------------------------------------------------------ */

void 
eld_lut (DGI_PTR dgi, int pn, short *dst)
{
    /* create a translation lookup table for this parameter
     * data is rescaled to conform to the DORADE document and
     * to be compatable with UF format
     */
    int i, j, mm, nn, mask16=0xffff, aft, dual_vel;
    short *ss=dst, extend_sign, bit11;
    float scale=100, rcp_scale;
    float f, f_err, bias, *fptr, freq, *pptr, prt;
    struct dds_structs *dds=dgi->dds;
    struct parameter_d *parm=dgi->dds->parm[pn];
    char *a;
    char *aa, name[16];
    double ts=dgi->time, nyqv;
    
    bias = parm->parameter_bias;
    rcp_scale = 1./parm->parameter_scale;
    if (dgi->radar_name.find("TA") == std::string::npos)
      aft = 0;
    else
      aft = 1;

    if(eui->options & TOGACOARE_FIXES &&
       dds->field_id_num[pn] == NCP_ID_NUM) {
	/* counts on tape go from 0 to 100 */
	for(i=0; i < 256; i++){
	    *ss++ = i;
	}
    }
    else if(eui->options & TOGACOARE_FIXES &&
	    dds->field_id_num[pn] == SW_ID_NUM) {
	/* counts on tape go from 0 to 100 */
	rcp_scale = .01*dds->radd->eff_unamb_vel;
	rcp_scale = 1./(4.4902*1.77); /* (1./7.95) */
	for(i=0; i < 256; i++){
	    f = ELD_UNSCALE((float)i, rcp_scale, bias);
	    j = (int)DD_SCALE(f, scale, (float)0);
	    *ss++ = j & mask16;
	}
    }
    else if(eui->options & TOGACOARE_FIXES &&
	    dds->field_id_num[pn] == VR_ID_NUM && aft) {
	for(i=0; i < 65536; i++){
	    f = ELD_UNSCALE((float)i, rcp_scale, bias);
	    f *= TOGACOARE_VEL_ERR;
	    j = (int)DD_SCALE(f, scale, (float)0);
	    *ss++ = j & mask16;
	}
    }
    else if(eui->options & TOGACOARE_FIXES &&
	    dds->field_id_num[pn] == DBZ_ID_NUM && aft) {
	f_err = TG_AFT_REFL_ERR;
	if(a=get_tagged_string("AFT_REFL_CORR")) {
	    f_err = atof(a);
	}
	for(i=0; i < 65536; i++){
	    f = ELD_UNSCALE((float)i, rcp_scale, bias);
	    f += f_err;
	    j = (int)DD_SCALE(f, scale, (float)0);
	    *ss++ = j & mask16;
	}
    }
    else if(eui->options & TOGACOARE_FIXES &&
	    dds->field_id_num[pn] == DBZ_ID_NUM) {
	f_err = TG_FORE_REFL_ERR;
	if(a=get_tagged_string("FORE_REFL_CORR")) {
	    f_err = atof(a);
	}
	for(i=0; i < 65536; i++){
	    f = ELD_UNSCALE((float)i, rcp_scale, bias);
	    f += f_err;
	    j = (int)DD_SCALE(f, scale, (float)0);
	    *ss++ = j & mask16;
	}
    }
    else if(eui->data_type & LIDAR_DATA) {
	/* c...mark */
	scale = eui->data_type & NAILS_DATA ? 1.: parm->parameter_scale;
	str_terminate(name, parm->parameter_name, 8);

	if((!strcmp("I", name) || !strcmp("Q", name))) {
	    /* nails  12-bit data */
	    extend_sign = 0xf000;
	    bit11 = 0x0800;
	    scale = 1.;
	    for(i=0; i < 65536; i++){
# ifdef obsolete		
		*ss++ = i & bit11 ? i | extend_sign : i;
# else
		*ss++ = (short)DD_SCALE((float)i, scale, (float)0);
# endif
	    }
	}
	else {
	    for(i=0; i < 65536; i++) {
		*ss++ = (short)DD_SCALE((float)i, scale, (float)0);
	    }
	}
    }
    else {
	str_terminate(name, parm->parameter_name, 8);

	if(ts > start_vortex_fix_1 && ts < end_vortex_tests &&
	   strstr("VR VS VL", name)) {
	    for(i=0; i < 65536; i++){
		f = - ELD_UNSCALE((float)i, rcp_scale, bias);
		/* sign of the velocity was reversed during this time span
		 */
		j = (int)DD_SCALE(f, scale, (float)0);
		*ss++ = j & mask16;
	    }
	}
	else if(ts > start_vortex_fix_1 && ts < end_vortex_fix_1 &&
		strstr("DBZ", name) &&
		dgi->dds->radd->num_ipps_trans == 2) {
	    for(i=0; i < 65536; i++){
		f = ELD_UNSCALE((float)i, rcp_scale, bias) -3.0;
		/* subtracting 3 to fix dBz calculation in dual PRT mode
		 */
		j = (int)DD_SCALE(f, scale, (float)0);
		*ss++ = j & mask16;
	    }
	}
	else {
	    for(i=0; i < 65536; i++){
		f = ELD_UNSCALE((float)i, rcp_scale, bias);
		j = (int)DD_SCALE(f, scale, (float)0);
		*ss++ = j & mask16;
	    }
	}
    }
    j = parm->bad_data & mask16;
    *(dst+j) = DELETE_FLAG;
    parm->parameter_scale = scale;
    parm->parameter_bias = 0;
    parm->bad_data = DELETE_FLAG;
}
/* c------------------------------------------------------------------------ */
# define PMODE 0666

int eld_next_ray (void)
{
    static float cell_spacing = 0.;
    static int minRecSize = sizeof(struct volume_d)
	  + sizeof(struct radar_d) +sizeof(struct parameter_d);
    static int count=0, eld_bytes_left=0, eof_count=0, rlen=0, ts_count=0;
    static int ts_fid = -1, tape_count = 0, bad_desc_count = 0, n_selected=0;
    static char *eld_next_block=NULL, select_radars[32], *srptrs[8];
    static int source_vol_num=-1, counttrip=84, ray_trip=1322;
    static struct volume_d *vold=NULL;
    static struct indep_freq *indf;
    static struct dd_general_info *dgi=NULL;
    static struct field_radar_i *frib = 0;
    static struct time_series *tmsr;
    static struct waveform_d *wave=0;

    struct generic_descriptor xgd;
    int ii, jj, kk, nc, nn, ndx, pn, rn, ryib_flag=NO, mark, trip_inc=1800;
    int gdsos, ncopy, really_break_out=NO, new_vol=NO, sz, ok;
    int fore, aft, ta_fore, zeros_count=0, num_parameter_des=0;
    int vol_radd_count=0, radd_count=0, param_count=0, total_param_count=0;
    int gotta_cspd = NO, new_cell_vec, err_count = 0;
    int ignore_cfacs = NO;
    char *aa, *at, *b, radar_name[12],dname[16], *dst;
    char str[256], frib_fname[80], *cfac_id;
    char *deep6=0;
    time_t t;
    short tdate[6];
    double d;
    float gs=0, f;
    static FILE *strm;

    struct dd_general_info *dgii;
    struct field_parameter_data *frad;
    struct lidar_parameter_data *ldat=NULL;
    static struct radar_d *radd;
    struct lidar_d *lidr=NULL;
    struct parameter_d *parm;
    struct dds_structs *dds;
    struct ray_i *ryib;
    struct cell_spacing_d *cspd;
    struct platform_i *asib;
    struct cfac_wrap *cfw;
    static struct individual_radar_info *iri;
    DD_TIME *dts, dtx;
    struct io_packet *dp;
    struct time_dependent_fixes *fx, *fx_kill;



    eui->ignore_this_ray = NO;
    tmsr = 0;			/* time series pointer/flag */

    while(1) {
	/* absorb the next ray as well as all intervening headers
	 */
	count++;
	if(count >= counttrip) {
	    mark = 0;
	}
	if(dd_control_c()) {
	    dd_reset_control_c();
	    return(-1);
	}
	if(dd_stats->ray_count >= ray_trip) {
	    mark = 0;
	}
	while(irq->top->bytes_left <= sizeof(struct generic_descriptor)) {
	    if(dd_control_c()) {
		dd_reset_control_c();
		return(-1);
	    }
	    if(ryib_flag) {
		really_break_out = YES;
		break;
	    }
	    /*
	     * read in the next record
	     */
	    dd_stats->rec_count++;
	    dd_logical_read(irq, FORWARD);
	    str_terminate(dname, irq->top->buf, 4);
	    
	    if( (nn = irq->top->read_state ) < 0) {
	       irq->top->bytes_left = 0;
	       ryib_flag = NO;
	       if( ++err_count < 4 ) {
		  printf("Last read: %d errno: %d\n"
			 , irq->top->read_state, err_count);
		  continue;
	       }
	       dd_input_read_close(irq);
	       if (tape_count++ > 18) {
		 mark = 0;
	       }
	       if(aa = dd_next_source_dev_name((char *)"ELD")) {
		  current_file_name = aa;
		  if((ii = dd_input_read_open(irq, aa)) <= 0) {
		     return(nn);
		  }
		  err_count = 0; eof_count = 0;
		  continue;
	       }
	       return( nn );
	    }
	    else if(irq->top->eof_flag) {
		dd_stats->file_count++;
		dtx.time_stamp = eui->prior_time;
		sprintf(str, "EOF at %s %.2f MB file: %d rec: %d ray: %d"
			, dts_print(d_unstamp_time(&dtx))
			, dd_stats->MB
			, dd_stats->file_count, dd_stats->rec_count
			, dd_stats->ray_count
			);
		printf("%s\n", str);
		dd_append_cat_comment(str);
		if( ryib_flag )	/* assume last ray in the file */
		      return(1);

		else if( ++eof_count >= 4 ) {
		  dd_input_read_close(irq);
		  if(aa = dd_next_source_dev_name((char *)"ELD")) {
		      current_file_name = aa;
		      if((ii = dd_input_read_open(irq, aa)) <= 0) {
			 return(0);
		      }
		      eof_count = 0; err_count = 0;
		      continue;
		   }
		   return(0);
		}
		continue;
	    }
	    else if(strlen(dname) < 4 || !strstr("VOLD RYIB COMM", dname)) {
		/* a record should only start with one of these descriptors
		 */
		d_unstamp_time(dgi->dds->dts);
		sprintf(str,
			"%s First descriptor is unrecognizeable"
			, dts_print(dgi->dds->dts));
		dd_append_cat_comment(str);

		printf("%s\n", str);
		ezascdmp(irq->top->buf, 0, 200);
		ezhxdmp(irq->top->buf, 0, 200);

		err_count = 0; eof_count = 0;
		ndx = dde_desc_fwd_sync
		      (irq->top->buf, sizeof(struct generic_descriptor)
		       , irq->top->sizeof_read);
		if(ndx > 0) {
		    irq->top->at = irq->top->buf +ndx;
		    irq->top->bytes_left -= ndx;
		}
		else {
		    irq->top->bytes_left = 0;
		    continue;
		}
	    }
	    else if(irq->top->sizeof_read < minRecSize) {
		sprintf(str, "Record size: %d is too small!"
			, irq->top->sizeof_read);
		dd_append_cat_comment(str);

		printf("!%s\n", str);
		err_count = 0; eof_count = 0;
	    }
	    else {
		err_count = 0; eof_count = 0;
		if(irq->io_type == BINARY_IO)
		      dorade_reset_offset(irq);
		dd_stats->MB += BYTES_TO_MB(irq->top->sizeof_read);
		break;
	    }
	}
	if(really_break_out)
	      break;

	eld_next_block = at = irq->top->at;
	eld_bytes_left = irq->top->bytes_left;
	gd = (struct generic_descriptor *)eld_next_block;
	if (!strncmp(eld_next_block, "RAWD", 4)) {
	  mark = 0;
	}

	if(hostIsLittleEndian()) {
	   gd = &xgd;
	   memcpy(gd->name_struct, eld_next_block, 4);
	   swack4(eld_next_block+4, (char *)&gd->sizeof_struct);
	}
	gdsos = gd->sizeof_struct;

	if(gd->sizeof_struct < sizeof(struct generic_descriptor)) {
	    gd->sizeof_struct = 0;
	    irq->top->bytes_left = 0;
	    param_count = 0;
	    if(++zeros_count > 7) {
		printf("\n**** Consecutive false reads > 7  %d ****\n\n"
		       , rlen);
		return(-1);
	    }
	}
	else if(gd->sizeof_struct > eld_bytes_left) {
	    
	    param_count = 0;
	    ryib_flag = NO;
	    if (++bad_desc_count < 7) {
	       str_terminate(dname, eld_next_block, 4);
	       sprintf(str
		       , "*** Bad desc. size: %d for %s  bytes left: %d ***"
		       , gd->sizeof_struct
		       , str_terminate(dname, eld_next_block, 4), eld_bytes_left);
	       dd_append_cat_comment(str);
	       printf("%s\n", str);
	    }
	    frad = (struct field_parameter_data *)eld_next_block;
	    gd->sizeof_struct = 0;
	    eld_bytes_left = irq->top->bytes_left = 0;
	    continue;
	}

	else if(ryib_flag && (strncmp(eld_next_block,"FRAD",4) == 0 ||
			      strncmp(eld_next_block,"LDAT",4) == 0)) {
	    /* Beginning of FRAD or LDAT specific code
	     */
	   if(*eld_next_block == 'L') {
	      if(hostIsLittleEndian()) {
		 eld_crack_ldat(eld_next_block, (char *)xldat, (int)0);
		 ldat = xldat;
	      }
	      else
		  { ldat = (struct lidar_parameter_data *)eld_next_block; }
	   }
	   else { ldat = NULL; }

	    if(ldat) {
		sz = sizeof(struct lidar_parameter_data);
		str_terminate(radar_name, ldat->lidar_name
			      , sizeof(ldat->lidar_name));
		if(!strcmp("Nails", radar_name)) {
		    strncpy(ldat->lidar_name, "NAILS     ", 8);
		    strcpy(radar_name, "NAILS");
		}
	    }
	    else {
		sz = sizeof(struct field_parameter_data);
		frad = (struct field_parameter_data *)eld_next_block;

		if(eui->options & TOGACOARE_FIXES) {
		    fore = !strncmp(frad->radar_name, "FORE", 4);

		    ta_fore = fabs((double)asib->tilt -OPTIMUM_FORE_TILT) <
			  ELD_TILT_TOLERANCE ? YES : NO;
		    eui->ignore_this_ray = fore && ta_fore ? YES : NO;
		    
		    if(ta_fore && !fore ) {
			strncpy(frad->radar_name, "FORE    ", 8);
		    }
		    else if(!ta_fore && fore) {
			strncpy(frad->radar_name, "AFT     ", 8);
		    }
		    if(!strncmp(frad->radar_name, "AFT", 3)) {
			asib->rotation_angle += .88;
			asib->tilt -= 1.;
		    }
		    asib->ns_horiz_wind = -asib->ns_horiz_wind;
		    asib->ew_horiz_wind = -asib->ew_horiz_wind;
		}
		eld_radar_rename(frad->radar_name);
		str_terminate(radar_name, frad->radar_name
			      , sizeof(frad->radar_name));
	    }		
	    eui->dgi = dgi = dd_get_structs
		  (dd_assign_radar_num(radar_name));
	    dds = dgi->dds;
	    rn = eui->radar_num = dgi->radar_num;
	    iri = eui->iri[rn];
	    iri->radar_num = rn;
	    iri->ray_count++;
	    ncopy = gdsos;
	    if( ldat ) {
		memcpy( dgi->dds->ldat, ldat
			, sizeof( struct lidar_parameter_data ) );
	    }
	    if(eui->options & ELD_DESC_SEARCH) {
		/*
		 * do a little checking; some data have bad descriptor lengths
		 */
		ndx = dde_desc_fwd_sync
		      (eld_next_block, sizeof(struct generic_descriptor)
		       , eld_bytes_left);
		if(ndx > 0 && ndx != gd->sizeof_struct) {
		   sprintf(str, "Descriptor length error: %s says %d is %d"
			   , gd->name_struct, gd->sizeof_struct, ndx);
		   dd_append_cat_comment(str);
		   printf("%s\n", str);
		   mark = 0;
		}
		if(ndx < 0) {
		    gd->sizeof_struct = ncopy = eld_bytes_left;
		    irq->top->bytes_left = gdsos = eld_bytes_left = 0;
		}
		else {
		    gd->sizeof_struct = ncopy = gdsos = ndx;
		    b = eld_next_block +ndx;
		    if(ndx % 4) {
			printf("\nLost long word alignment\n");
			mark = *deep6;
		    }
		}
	    }
	    if(!eui->ignore_this_ray) {
		if(!dds->frad) {
		    if(!(dds->frad = (struct field_parameter_data *)
			  malloc(MAX_REC_SIZE))) {
			printf("Unable to malloc FRAD\n");
			exit(1);
		    }
		    memset(dds->frad, 0, MAX_REC_SIZE);
		    dds->raw_data = (char *)dds->frad +
			  sizeof(struct field_parameter_data);
		}
		if(ldat) {
		   if(hostIsLittleEndian()) {
		      eld_crack_ldat(eld_next_block, (char *)dds->ldat, (int)0);
		      swack_short(eld_next_block + sz, dds->raw_data
				  , (gd->sizeof_struct -sz)/sizeof(short));
		   }
		   else {
		      memcpy(dds->ldat, eld_next_block, sz);
		      memcpy(dds->raw_data, eld_next_block + sz
			   , gd->sizeof_struct -sz);
		   }
		    frad = dds->frad;
		    frad->field_param_data_len =
			  sizeof(struct field_parameter_data)
				+ gd->sizeof_struct -sz;
		    frad->first_rec_gate = ldat->first_rec_gate;
		    frad->last_rec_gate = ldat->last_rec_gate;
		}
		else {
		   if(hostIsLittleEndian()) {
		      eld_crack_frad(eld_next_block, (char *)dds->frad, (int)0);
		      swack_short(eld_next_block + sz, dds->raw_data
				  , (gd->sizeof_struct -sz)/sizeof(short));
		   }
		   else {
		      memcpy((char *)dds->frad, eld_next_block
			     , ncopy);
		   }
		   /*
		   dds->frad->first_rec_gate = 
		     dds->frad->last_rec_gate = 0;
		    */
		}
	    }
	    /*
	     * now nab the ryib and asib for this radar
	     */
	   if(hostIsLittleEndian()) {
	      ddin_crack_ryib((char*)ryib, (char*)dds->ryib, (int)0);
	   }
	   else {
	      memcpy((char *)dds->ryib, (char *)ryib
		     , sizeof(struct ray_i));
	   }
	    dds->ryib->ray_info_length = sizeof(struct ray_i);
	    dts = dds->dts;
	    dorade_time_stamp(dds->vold, dds->ryib, dts );
	    if(eui->time_correction) {
		dts->time_stamp += eui->time_correction;
		d_unstamp_time(dts);
		dds->vold->year = dts->year;
		dds->ryib->julian_day = dts->julian_day;
		dds->ryib->hour = dts->hour;
		dds->ryib->minute = dts->minute;
		dds->ryib->second = dts->second;
		dds->ryib->millisecond = dts->millisecond;
	    }
	    eui->prior_time = dgi->time = dts->time_stamp;
	    eui->current_time = (time_t) eui->prior_time;

	   if(hostIsLittleEndian()) {
	      ddin_crack_asib((char *)asib, (char *)dds->asib, (int)0);
	   }
	   else {
	      memcpy((char *)dds->asib, (char *)asib
		     , sizeof(struct platform_i));
	   }
	    dds->asib->platform_info_length = sizeof(struct platform_i);

	    iri->rotang_ndx = INC_NDX(iri->rotang_ndx, ROTANG_LIST_SIZE);
	    iri->rotang_list[iri->rotang_ndx] = asib->rotation_angle;

	    if(iri->new_radar_desc) {
		iri->new_radar_desc = NO;
		for(pn=0; pn < dgi->source_num_parms; pn++) {
		    eld_lut(dgi, pn, iri->xlate_lut[pn]);
		    /* you have to do this here because you need the time!
		     */
		}
	    }
	    /* Begin lidar specific code
	     */
	    if(eui->data_type & LIDAR_DATA) {
		if(eui->data_type & NAILS_DATA) {
		}
		else {		/* SABL data */
		    if(dds->asib->tilt == -999.) {
			dds->asib->tilt = 0;
		    }
		    switch(dds->lidr->scan_mode) {

		    case 10:	/* vert. pt. up (zenith) */
			dds->asib->rotation_angle = 0.;
			dds->ryib->azimuth = 0;
			break;
		    case 11:	/* vert. pt. down (nadir) */
			dds->asib->rotation_angle = 180.;
			dds->ryib->azimuth = 0;
			break;
		    case 12:	/* horz. right */
			dds->asib->rotation_angle = 90.;
			dds->ryib->azimuth = 90;
			break;
		    case 13:	/* horz. left */
			dds->asib->rotation_angle = 270.;
			dds->ryib->azimuth = 270;
			break;
		    case DD_SCAN_MODE_VER:
		    default:
			break;
		    }
		}
		new_cell_vec = NO;

		if(eui->fx_count) { /* apply fixes */
		   d_unstamp_time(dts);
		    fx_kill = NULL;
		    for(fx=eui->fx; fx; fx = fx->next) {
			if(dgi->time < fx->start_time)
			      continue;

			if(dgi->time < fx->stop_time) {

			    switch(fx->typeof_fix) {
				
			    case FIX_POINTING_ANGLE:
				dds->asib->rotation_angle =
				      fx->pointing_angle;
				break;
			    case FIX_CELL_SPACING:
				if(iri->cell_spacing != fx->cell_spacing) {
				    new_cell_vec = YES;
				    iri->cell_spacing = fx->cell_spacing;
# ifdef obsolete
				    fx->use_it = NO;
				    fx_kill = fx;
# endif
				}
				break;
			    default:
				break;
			    } /* end switch */
			} 
			else {
			    fx->use_it = NO;
			    fx_kill = fx;
			}
		    }
		    if(fx_kill) { /* there is at least one kill */
			eui->fx = dd_clean_tdfs(&eui->fx_count);
		    }
		}
		/* we also must check to see if the number of cells changed
		 * because the new cspd may have been omitted
		 */
		if((nc = ldat->last_rec_gate +1) != dds->celv->number_cells) {
		    /* fake a new cspd
		     */
		    dds->cspd->num_cells[0] = nc;
		    sprintf(str, "Unannounced cell count change from %d to %d"
			    , dds->celv->number_cells, nc);
		    dd_append_cat_comment(str);
		    printf("%s\n", str);
		    new_cell_vec = YES;
		}
		if(new_cell_vec) {
		    iri->sweep_count_flag = iri->vol_count_flag = YES;
		    cspd2celv(dds, iri->cell_spacing);
		    dd_set_uniform_cells(dgi->dds);
		    gotta_cspd = YES;
		    if(dgi->num_parms) {
			/* make sure there is enough space for this data */
			for(jj=0; jj < dgi->num_parms; jj++) {
			    dd_alloc_data_field(dgi, jj);
			}
		    }
		}
		if(iri->vol_count_flag) {
		    iri->sweep_reference_time = eui->dgi->time +
			  eui->lidar_sweep_time_limit;
		}
		if(eui->data_type & NAILS_DATA) {
		}
		else {		/* SABL data */
		    d = dds->asib->rotation_angle;
		    if(dd_isnanf(d)) {
			d = 180.;
		    }
		    else {
			d = CART_ANGLE(d);
		    }
		    dds->ryib->elevation = d;
		}
	    } /* end of lidar specific code */

	} /* end of FRAD or LDAT specific code */
	/* c...mark */

	else if(ryib_flag && strncmp(eld_next_block,"INDF",4)==0 ) {
	   indf = (struct indep_freq *)eld_next_block;
	    if(eui->options & ELD_DESC_SEARCH) {
		ndx = dde_desc_fwd_sync
		      (eld_next_block, sizeof(struct generic_descriptor)
		       , eld_bytes_left);
		if(ndx > 0 && ndx != gd->sizeof_struct) {
		   sprintf(str, "Descriptor length error: %s says %d is %d"
			   , gd->name_struct, gd->sizeof_struct, ndx);
		   dd_append_cat_comment(str);
		   printf("%s\n", str);
		   gdsos = ndx;
		}
		if(ndx < 0) {
		    irq->top->bytes_left = gdsos = eld_bytes_left = 0;
		}
		else if(ndx % 4) {
		    mark = *deep6; /* have lost alignment */
		}
	    }
	}
	else if(strncmp(eld_next_block,"RAWD",4)==0 ) {

	    if(eui->options & ELD_DESC_SEARCH) {
		ndx = dde_desc_fwd_sync
		      (eld_next_block, sizeof(struct generic_descriptor)
		       , eld_bytes_left);
		if(ndx > 0 && ndx != gd->sizeof_struct) {
		   sprintf(str, "Descriptor length error: %s says %d is %d"
			   , gd->name_struct, gd->sizeof_struct, ndx);
		   dd_append_cat_comment(str);
		   printf("%s\n", str);
		   gdsos = ndx;
		}
		if(ndx < 0) {
		    irq->top->bytes_left = gdsos = eld_bytes_left = 0;
		}
		else if(ndx % 4) {
		    mark = *deep6; /* have lost alignment */
		}
	    }
	}
	else if(strncmp(eld_next_block,"FRIB",4)==0 ) {
	   dds = dgi->dds;
	   frib = (struct field_radar_i *)eld_next_block;
	   if (wave) {
	      if (!dds->wave)
		{ dds->wave = (struct waveform_d *)malloc (sizeof (*wave)); }
	      memcpy (dds->wave, wave, sizeof (*wave));
	   }
	   if (!dds->frib) {
	      dds->frib = (struct field_radar_i *)malloc (sizeof (*frib));
	   }
	   if(hostIsLittleEndian()) {
	     ddin_crack_frib(eld_next_block, (char *)dds->frib, (int)0);
	   }
	   else {
	     memcpy (dds->frib, frib, sizeof (*frib));
	   }
	   dds->frib->field_radar_info_len = sizeof (*frib);
	   str_terminate( frib_fname, frib->file_name
			 , strlen(frib->file_name)); 
	   strcpy (frib->file_name, frib_fname);

	   strcpy( str, "FRIB.file_name:" );
	   strcat( str, frib_fname );
	   dd_append_cat_comment(str);
	   printf("%s\n", str);

	   ignore_cfacs = NO;
	   if (cfw = ddswp_nab_cfacs (radar_name)) { /* external cfac files */
	      cfac_id = const_cast<char *>((frib) ? frib->file_name : "default");

	      for(; cfw ; cfw = cfw->next ) {
		 if( strstr (cfw->frib_file_name, cfac_id)) {
		    memcpy( dgi->dds->cfac, cfw->cfac, sizeof( *cfw->cfac ));
		    ignore_cfacs = YES;
		    break;
		 }
	      }
	   }
	   if(eui->options & ELD_DESC_SEARCH) {
	      ndx = dde_desc_fwd_sync
		(eld_next_block, sizeof(struct generic_descriptor)
		 , eld_bytes_left);
	      if(ndx > 0 && ndx != gd->sizeof_struct) {
		 sprintf(str, "Descriptor length error: %s says %d is %d"
			 , gd->name_struct, gd->sizeof_struct, ndx);
		 dd_append_cat_comment(str);
		 printf("%s\n", str);
		 gdsos = ndx;
	      }
	      if(ndx < 0) {
		 irq->top->bytes_left = gdsos = eld_bytes_left = 0;
	      }
	      else if(ndx % 4) {
		 mark = *deep6;
	      }
	   }
	}
	else if(strncmp(eld_next_block,"WAVE",4)==0 ) {

          /* radd->num_freq_trans * 2 * wave->repeat_seq_dwel *
	   * wave->num_chips[0] = number of floating pt values
	   * in the time series descriptor ("TIME")
	   * frib->time_series_gate (68) is the gate number
	   */
	   if (!wave) {
	      wave = (struct waveform_d *)malloc (sizeof (*wave));
	   }
	   if(hostIsLittleEndian()) {
# ifdef notyet
	     ddin_crack_wave(eld_next_block, wave, (int)0);
# endif
	     memcpy (wave, eld_next_block, sizeof (*wave));
	   }
	   else {
	     memcpy (wave, eld_next_block, sizeof (*wave));
	   }
	}
	else if(ryib_flag && strncmp(eld_next_block,"TIME",4)==0 ) {
	    tmsr = ( struct time_series *)eld_next_block;
	    if(eui->options & ELD_DESC_SEARCH) {
		ndx = dde_desc_fwd_sync
		      (eld_next_block, sizeof(struct generic_descriptor)
		       , eld_bytes_left);
		if(ndx > 0 && ndx != gd->sizeof_struct) {
		   sprintf(str, "Descriptor length error: %s says %d is %d"
			   , gd->name_struct, gd->sizeof_struct, ndx);
		   dd_append_cat_comment(str);
		   printf("%s\n", str);
		   gdsos = ndx;
		}
		if(ndx < 0) {
		    irq->top->bytes_left = gdsos = eld_bytes_left = 0;
		}
		else if(ndx % 4) {
		    mark = *deep6;
		}
	    }
	    else if (dgi->time > D20030501) {
	      irq->top->bytes_left = gdsos = eld_bytes_left = 0;
	    }
	}
	else if(strncmp(eld_next_block,"RYIB",4)==0 && ryib_flag ) {
	    break;
	}
	else if(strncmp(eld_next_block,"RYIB",4)==0 ) {
	    ryib = (struct ray_i *)eld_next_block;
	    ryib_flag = YES;
	    vol_radd_count = 0;
	    dp = dd_return_next_packet(irq);
	}
	else if(strncmp(eld_next_block,"ASIB",4)==0) {
	    asib = (struct platform_i *)eld_next_block;
	}

	else if(strncmp(eld_next_block,"PARM",4)==0 ) {
	    /* assume dds defined by the previous
	     * radd block encounter
	     */
	    param_count++;
	    pn = dgi->num_parms++;
	    dgi->source_num_parms = dgi->num_parms;
	    if(hostIsLittleEndian()) {
	       ddin_crack_parm(eld_next_block, (char *)dds->parm[pn], (int)0);
	    }
	    else {
	       memcpy((char *)dds->parm[pn]
		      , eld_next_block, sizeof(struct parameter_d));
	    }
	    dds->parm[pn]->parameter_des_length =
		  sizeof(struct parameter_d);
	    if(eui->data_type & NAILS_DATA) {
		b = dds->parm[pn]->parameter_name;
		*(b+1) = *b;
	    }
	    dds->field_id_num[pn] =
		  dd_return_id_num(dds->parm[pn]->parameter_name);
	    dgi->parm_type[pn] =
		  dds->field_id_num[pn] != DBZ_ID_NUM ? VELOCITY : OTHER;
	    dds->field_present[pn] = YES;
	    iri->num_parms = dgi->num_parms;
	    iri->sizeof_fparm[pn] =
		  eui->datum_size[dds->parm[pn]->binary_format];
	    iri->sizeof_gate += iri->sizeof_fparm[pn];
	    if( pn > 0 )
		  iri->fparm_offset[pn] =
			iri->fparm_offset[pn-1] +iri->sizeof_fparm[pn-1];
	    if(!iri->xlate_lut[pn])
		  iri->xlate_lut[pn] = (short *)malloc(65536*sizeof(short));
	    if(gotta_cspd) {
		/* already have a cspd so we can allocate space
		 * for this field  */
		dd_alloc_data_field(dgi, pn);
	    }
	    if(strncmp(iri->parameter_name[pn]
		       , dds->parm[pn]->parameter_name, 8)) {
		/* the mix of parameters has changed! */
		iri->sweep_count_flag = 
		      iri->vol_count_flag = YES;
	    }
	    strncpy(iri->parameter_name[pn]
		       , dds->parm[pn]->parameter_name, 8);

	    if(eui->num_rays_avgd)
		  dds->parm[pn]->num_samples = eui->num_rays_avgd;
	}

	else if(strncmp(eld_next_block,"RADD",4)==0  ||
		strncmp(eld_next_block,"LIDR",4) == 0) {

	   lidr = NULL;

	    if(strncmp(eld_next_block,"LIDR",4) == 0) {
	       lidr = (struct lidar_d *)eld_next_block;
	       str_terminate(radar_name, lidr->lidar_name, 8);
	       eui->data_type |= LIDAR_DATA;
		if(strncmp(radar_name, "NAILS", 5) == 0) {
		    eui->data_type |= NAILS_DATA;
		}
	    }
	    else {
	       radd = (struct radar_d *)eld_next_block;
	       str_terminate(radar_name, radd->radar_name, 8);
	       aft = strncmp(radd->radar_name, "AFT", 3) == 0 ? YES : NO;
	       eld_radar_rename( radd->radar_name );
	       str_terminate(radar_name, radd->radar_name, 8);
	    }
	    radd_count++;
	    param_count = 0;


	    dgi = dd_get_structs(dd_assign_radar_num(radar_name));
	    dgi->source_fmt = ELDORA_FMT;
	    dds = dgi->dds;
	    rn = dgi->radar_num;
	    iri = eui->iri[rn];
	    if(lidr) {
	       if(hostIsLittleEndian()) {
		  eld_crack_lidr(eld_next_block, (char *)dds->lidr, (int)0);
	       }
	       else {
		  memcpy((char *)dds->lidr, eld_next_block
			 , sizeof(struct lidar_d));
	       }
	       lidr = dds->lidr;
		radd = dgi->dds->radd;
		strncpy(radd->radar_name, lidr->lidar_name, 8);
		radd->peak_power = lidr->peak_power;

		if(eui->data_type & NAILS_DATA) {
		    radd->radar_type = DD_RADAR_TYPE_LIDAR_FIXED;
		}
		else {
		    radd->radar_type = DD_RADAR_TYPE_LIDAR_MOVING;
		}

		switch(lidr->scan_mode) {
		case DD_SCAN_MODE_VER:
		case 10:	/* vert. pt. up (zenith) */
		    radd->scan_mode = DD_SCAN_MODE_TAR;
		    break;
		case 11:	/* vert. pt. down (nadir) */
		    radd->scan_mode = DD_SCAN_MODE_TAR;
		    break;
		case 12:	/* horz. right */
		    radd->scan_mode = DD_SCAN_MODE_TAR;
		    break;
		case 13:	/* horz. left */
		    radd->scan_mode = DD_SCAN_MODE_TAR;
		    break;
		default:
		    radd->scan_mode = lidr->scan_mode;
		    break;
		}

		radd->req_rotat_vel = lidr->req_rotat_vel;
		radd->scan_mode_pram0 = lidr->scan_mode_pram0;
		radd->scan_mode_pram1 = lidr->scan_mode_pram1;
		radd->num_parameter_des = lidr->num_parameter_des;
		radd->total_num_des = lidr->total_number_des;
		radd->data_compress = lidr->data_compress;
		radd->data_reduction = lidr->data_reduction;
		radd->data_red_parm0 = lidr->data_red_parm0;
		radd->data_red_parm1 = lidr->data_red_parm1;
		radd->radar_longitude = lidr->lidar_longitude;
		radd->radar_latitude = lidr->lidar_latitude;
		radd->radar_altitude = lidr->lidar_altitude;
		radd->eff_unamb_vel = lidr->eff_unamb_vel;
		radd->eff_unamb_range = lidr->eff_unamb_range;
		radd->num_freq_trans = lidr->num_wvlen_trans;
	    }
	    else {
	       if(hostIsLittleEndian()) {
		  ddin_crack_radd(eld_next_block, (char *)dds->radd, (int)0);
	       }
	       else {
		  memcpy((char *)dds->radd, eld_next_block
			 , sizeof(struct radar_d));
	       }
	       radd = dds->radd;
	       radd->scan_mode = DD_SCAN_MODE_AIR;
	       radd->data_compress = NO_COMPRESSION;
	       /* klooges!
		*/
	       if(eui->options & TOGACOARE_FIXES && aft) {
		  radd->eff_unamb_vel *= TOGACOARE_VEL_ERR;
	       }
	       /* end klooges!
		*/
	    }
	    memcpy((char *)dds->vold
		  , (char *)vold, sizeof(struct volume_d));

	    total_param_count += radd->num_parameter_des;
	    dds->vold->number_sensor_des = 1;
	    dd_radar_selected(radar_name, dgi->radar_num, difs);

	    if(dgi->source_vol_num != source_vol_count) {
		dgi->source_vol_num = source_vol_count;
		iri->sweep_count_flag = 
		      iri->vol_count_flag =
			    dgi->new_vol = dgi->new_sweep = YES;
	    }
	    else if(eui->fake_sweep_nums) {
		iri->sweep_count_flag = 
		      iri->vol_count_flag = dgi->new_vol =
			    dgi->new_sweep = YES;
	    }
	    if(iri->num_parameter_des != radd->num_parameter_des) {
		iri->sweep_count_flag = 
		      iri->vol_count_flag =
			    dgi->new_vol = dgi->new_sweep = YES;
	    }

	    iri->num_parameter_des = radd->num_parameter_des;
	    dgi->num_parms = num_parameter_des = 0;
	    gotta_cspd = NO;
	    for(ii=0; ii < MAX_PARMS; dds->field_present[ii++]=NO);
	    iri->sizeof_gate = 0;
	    iri->fparm_offset[0] = 0;
	    iri->new_radar_desc = YES;
	    if(strstr(radar_name, "TA" )) {
		eui->aft_radar_num = dgi->radar_num;
	    }
	    else {
		eui->fore_radar_num = dgi->radar_num;
	    }
	}

	else if(strncmp(eld_next_block,"CFAC",4)==0 ) {
	   ignore_cfacs = NO;

	   if (cfw = ddswp_nab_cfacs (radar_name)) { /* external cfac files */
	      cfac_id = const_cast<char *>((frib) ? frib->file_name : "default");

	      for(; cfw ; cfw = cfw->next ) {
		 if( strstr (cfw->frib_file_name, cfac_id)) {
		    memcpy( dgi->dds->cfac, cfw->cfac, sizeof( *cfw->cfac ));
		    ignore_cfacs = YES;
		    break;
		 }
	      }
	   }
	   if(!ignore_cfacs) {
	      if(hostIsLittleEndian()) {
		 ddin_crack_cfac(eld_next_block, (char *)dds->cfac, (int)0);
	      }
	      else {
		 memcpy((char *)dds->cfac, eld_next_block
			, sizeof(struct correction_d));
	      }
	   }
	}
	else if(strncmp(eld_next_block,"CSPD",4)==0 ) {
	   if(hostIsLittleEndian()) {
	      eld_crack_cspd(eld_next_block, (char *)xcspd, (int)0);
	      cspd = xcspd;
	   }
	   else {
	      cspd = (struct cell_spacing_d *)eld_next_block;
	   }
	   if(memcmp(cspd, dds->cspd, sizeof(struct cell_spacing_d))) {
	      /* number of cells and/or cell spacing has changed
	       */
	      iri->sweep_count_flag = iri->vol_count_flag = YES;
	      
	      sprintf(str, "New CSPD nc: %d   cs: %d  "
		      , cspd->num_cells[0], cspd->spacing[0]);
	      sprintf(str+strlen(str), "Old CSPD nc: %d   cs: %d  "
		      , dds->cspd->num_cells[0], dds->cspd->spacing[0]);
	      dd_append_cat_comment(str);
	      printf("%s\n", str);
	   }
	   memcpy((char *)dds->cspd, cspd, sizeof(struct cell_spacing_d));

	    if(lidr && !strncmp(lidr->lidar_name, "NAILS", 5)) {
		if(!dds->cspd->num_segments) {
		    dds->cspd->num_segments = 1;
		    iri->cell_spacing = dds->cspd->spacing[0] = 15;
		    dds->cspd->num_cells[0] = 670;
		}
	    }
	    else if(lidr && !strncmp(lidr->lidar_name, "SABL", 4)) {
# ifdef obsolete
		switch((int)dds->cspd->spacing[0]) {
		case 4:
		    iri->cell_spacing = 5;
		    break;
		case 7:
		    iri->cell_spacing = 7.5;
		    break;
		default:
		    iri->cell_spacing = dds->cspd->spacing[0];
		    break;
		}
# endif
	    }
	    cspd2celv(dds, iri->cell_spacing);
	    dd_set_uniform_cells(dgi->dds);
	    gotta_cspd = YES;
	    if(dgi->num_parms) {
		/* we read in parameter descriptors but not allocated space
		 * for the data */
		for(jj=0; jj < dgi->num_parms; jj++) {
		    dd_alloc_data_field(dgi, jj);
		}
	    }
	}
	else if(strncmp(eld_next_block,"FLIB",4)==0) {
	    kk = sizeof(struct field_lidar_i);
	    memcpy(flib, eld_next_block, kk);
	    if(flib->digitizer_rate > 0) {
		iri->cell_spacing = METERS_PER_MICROSECOND
		      /flib->digitizer_rate;
	    }
	    if(flib->digitizer_rate != iri->digitizer_rate) {
		sprintf(str, "Changed digitizer rate...new: %.1f  old: %.1f"
			, flib->digitizer_rate, iri->digitizer_rate);
		dd_append_cat_comment(str);
		printf("%s\n", str);
	    }
	    iri->digitizer_rate = flib->digitizer_rate;
	}
	else if(strncmp(eld_next_block,"VOLD",4)==0) {
	    if(ryib_flag) {
		/* assume this is a volume header before the eof */
		irq->top->bytes_left = eld_bytes_left = 0;
		return(1);
	    }
	    total_param_count = 0;
	    param_count = -1;
	    dd_gen_packet_info(irq, &vol_dp); /* mark this spot! */
	    eui->volume_header_found = YES;

	    if(hostIsLittleEndian()) {
	       ddin_crack_vold(eld_next_block, (char *)xvold, (int)0);
	       vold = xvold;
	    }
	    else {
	       vold = (struct volume_d *)eld_next_block;
	    }
	    /* flag that we are reading a header */
	    vol_radd_count = vold->number_sensor_des;
	    dd_stats->vol_count++;
	    if(vold->volume_num != source_vol_num) {
		source_vol_num = vold->volume_num;
		source_vol_count++;
		new_vol = YES;
	    }
	    strncpy(vold->gen_facility, "NCAR/ATD ", 8 );
	    todays_date( tdate );
	    vold->gen_year = tdate[0] > 1900 ? tdate[0] : tdate[0]+1900;
	    vold->gen_month = tdate[1];
	    vold->gen_day = tdate[2];
	    if(eui->options & TOGACOARE_FIXES) {
		/* always make the year 93 */
		vold->year = 1993;
	    }
	    else if( eui->fix_year)
	      { vold->year = eui->fix_year; }
	    else if(vold->year < 1900)
	      { vold->year += 1900; }
	}
	else {
	    mark = 0;
	}
	irq->top->at += gdsos;
	irq->top->bytes_left -= gdsos;

	if( vol_radd_count && radd_count >= vol_radd_count &&
	   param_count >= total_param_count) {
	    /* we've got all we need from the header block */
	    vol_dp.len = vol_dp.bytes_left - irq->top->bytes_left;
	    irq->top->bytes_left = eld_bytes_left = 0;
	}
    }
    dp->len = dp->bytes_left - irq->top->bytes_left;
    dd_stats->ray_count++;
 
   if(eui->options & ELD_TIME_SERIES && tmsr) {
       if (!ts_count++) {
	   slash_path(str, get_tagged_string("DORADE_DIR"));
	   dd_file_name("tmsr", (int32_t)dgi->time, dgi->radar_name.c_str(), 0,
			str+strlen(str));
	  strcat (str, ".xml");
	  /*
	  strcat (str, "time_series.xml");
	   */
	  strm = fopen (str, "w+");
	  if (aa = get_tagged_string("SELECT_RADARS")) {
	     strcpy (select_radars, aa);
	     n_selected = dd_tokens (select_radars, srptrs);
	  }
       }
       if (strm) {
	  /* c...mark */
	  ok = YES;
	  if(difs->num_time_lims) {
	     for(ii=0; ii < difs->num_time_lims; ii++ ) {
		if(dgi->time >= difs->times[ii]->lower &&
		   dgi->time <= difs->times[ii]->upper)
		  break;
	     }
	     if(ii == difs->num_time_lims)
	       { ok = NO; }
	  }
	  if(difs->num_az_sectors) {
	     for(ii=0; ii < difs->num_az_sectors; ii++) {
		if(in_sector((float)dd_rotation_angle(dgi)
			     , (float)difs->azs[ii]->lower
			     , (float)difs->azs[ii]->upper)) {
		   break;
		}
	     }
	     if (ii == difs->num_az_sectors)
	       { ok = NO; }
	  }
	  if (n_selected) {
	     for (ii=0; ii < n_selected; ii++) {
	       if (dgi->radar_name.find(srptrs[ii]) != std::string::npos)
		  { break; }
	     }
	     if (ii == n_selected)
	       { ok = NO; }
	  }
	  if (ok)
	    { ts_xml_out (dgi, strm, tmsr, wave); }
       }
    }
    if( gotta_cspd ) {
      cspd2celv(dds, iri->cell_spacing);
      dd_set_uniform_cells(dgi->dds);
    }
    if(!eui->ignore_this_ray) {
	dgi->beam_count++;
	eld_is_new_sweep(dgi);
    }
    return(1);
}
/* c------------------------------------------------------------------------ */
int xml_set_version (char *buf, int *bsize)
{
   char *aa = buf + *bsize;
   sprintf (aa, "<?xml version =\"1.0\"?>\n");
   *bsize += strlen (aa);
}
/* c------------------------------------------------------------------------ */
int xml_open_element (char *buf, int *bsize, char *name)
{
   char *aa = buf + *bsize;
   sprintf (aa, "<%s", name);
   *bsize += strlen (aa);
}
/* c------------------------------------------------------------------------ */
int xml_close_element (char *buf, int *bsize, char *name)
{
   char *aa = buf + *bsize;
   sprintf (aa, "</%s>\n", name);
   *bsize += strlen (aa);
   return strlen (aa);
}
/* c------------------------------------------------------------------------ */
void xml_append_attribute (char *buf, int *bsize,
			   const char *name, const char *value)
{
   char *aa = buf + *bsize;
   sprintf (aa, "\n\t%s=\"%s\"", name, value);
   *bsize += strlen (aa);
}
/* c------------------------------------------------------------------------ */
int xml_append_comment (char *buf, int *bsize, char *str)
{
   char *aa = buf + *bsize;
   sprintf (aa, "<!-- %s -->\n", str);
   *bsize += strlen (aa);
}
/* c------------------------------------------------------------------------ */
int xml_end_attributes (char *buf, int *bsize, int empty_element)
{
   char *aa = buf + *bsize;
   sprintf (aa, "%s\n", (empty_element) ? "/>" : ">");
   *bsize += strlen (aa);
   return strlen (aa);
}
/* c------------------------------------------------------------------------ */
# ifdef obsolete
# endif

int ts_xml_out (struct dd_general_info *dgi, FILE *strm
		, struct time_series *tmsr, struct waveform_d *wave)
{
  struct dds_structs *dds=dgi->dds;
  DD_TIME *dts=dgi->dds->dts;
  static int count = 0, bsize=0, mxbsize=0, esize;
  static char *buf = 0, *str;
  static RTIME rtime;
  char *aa, ele[32], name[32];
  static char *ns = (char *)"acft:";
  static char acts[32];
  static char iqs[32];
  static char apos[32];
  static char ardr[32];
  int empty_element = NO;
  int nipps=dds->radd->num_ipps_trans, nfreqs=dds->radd->num_freq_trans;
  int ii, jj, kk, mark, ipp, freq, gg, nsamp;
  double ipp_vals[16], freq_vals[16];
  int offs = 16, len, loop;
  float ff, *fvals;
  short i2, gate_dist[16];
  fpos_t fpos;
  int32_t loffs;
  char *f2 = (char *)"%.2f", *f4=(char *)"%.4f", *f6=(char *)"%.6f";
  RTIME dtime;


  if (!buf) {
    mxbsize = 64 * 1024;
    buf = (char *)malloc (mxbsize);
    str = (char *)malloc (512);
    ns = (char *)"";
    strcpy (acts, ns);
    strcat (acts, "TimeSeries");
    strcpy (iqs, ns);
    strcat (iqs, "IQs");
    strcpy (apos, ns);
    strcat (apos, "Antenna_position");
    strcpy (ardr, ns);
    strcat (ardr, "Radar");
  }
  nsamp = dds->parm[0]->num_samples;


  if (!count++) {
    /* put out the one time only info */
    xml_set_version (buf, &bsize);
    xml_open_element (buf, &bsize, acts);
    rtime = dts->day_seconds;
    d_unstamp_time (dts);

    sprintf (str, "%4d%02d%02d", dts->year, dts->month, dts->day);
    xml_append_attribute (buf, &bsize, (char *)"date", str);

    sprintf (str, "%02d%02d%02d", dts->hour, dts->minute, dts->second);
    xml_append_attribute (buf, &bsize, (char *)"time", str);

    sprintf (str, "%d", nipps);
    xml_append_attribute (buf, &bsize, (char *)"interpulse_period_count", str);

    sprintf (str, "%d", nfreqs);
    xml_append_attribute (buf, &bsize, (char *)"frequency_count", str);
    xml_append_attribute (buf, &bsize, (char *)"gate_count", (char *)"1");

    sprintf (str, "%d", nsamp);
    xml_append_attribute (buf, &bsize, (char *)"num_samples", str);
    xml_end_attributes (buf, &bsize, empty_element=NO);



    strcpy (name, ns); strcat (name, "Naming_Conventions");
    xml_open_element (buf, &bsize, name);
    xml_append_attribute (buf, &bsize, (char *)"I", (char *)"real part of the autocorrelation function");
    xml_append_attribute (buf, &bsize, (char *)"Q", (char *)"imaginary part");
    xml_append_attribute (buf, &bsize, (char *)"Pnorm", (char *)"sum(I^2+Q^2)/num_samples");
    xml_end_attributes (buf, &bsize, empty_element = YES);

    strcpy (name, ns); strcat (name, "Velocity_calculation");
    xml_open_element (buf, &bsize, name);
    xml_append_attribute (buf, &bsize, (char *)"V", (char *)"3.0/(4.0*PI*freq*20.0)*2000/ipp*atan(Q/I)");
    xml_end_attributes (buf, &bsize, empty_element = YES);

    strcpy (name, ns); strcat (name, "Reflectivity_calculation");
    xml_open_element (buf, &bsize, name);
    xml_append_attribute (buf, &bsize, (char *)"dBZ", (char *)"10.0*log10(Pnorm)-conversion_gain-receiver_gain-radar_const+20.0*log10(range_km)");
    xml_end_attributes (buf, &bsize, empty_element = YES);
    xml_end_attributes (buf, &bsize, empty_element = NO);
  }

  xml_open_element (buf, &bsize, ardr);
  xml_append_attribute (buf, &bsize, (char *)"name", dgi->radar_name.c_str());
  
  sprintf (str, "%d", dgi->radar_num);
  xml_append_attribute (buf, &bsize, (char *)"radar_number", str);
  
  sprintf (str, f4, dds->radd->radar_const);
  xml_append_attribute (buf, &bsize, (char *)"radar_const", str);
  
  ff = dds->frib->conversion_gain;
  sprintf (str, f4, ff);
  xml_append_attribute (buf, &bsize, (char *)"conversion_gain", str);
  
  ff = dds->frib->x_band_gain;
  sprintf (str, f4, ff);
  xml_append_attribute (buf, &bsize, (char *)"x_band_gain", str);
  
  dtime = dts->day_seconds -rtime;
  sprintf (str, f4, dts->day_seconds -rtime);
  xml_append_attribute (buf, &bsize, (char *)"et_seconds", str);

  xml_end_attributes (buf, &bsize, empty_element = NO);


  strcpy (name, ns); strcat (name, "AntennaPosition");
  xml_open_element (buf, &bsize, name);
  
  sprintf (str, f2, dd_rotation_angle(dgi));
  xml_append_attribute (buf, &bsize, (char *)"rotation_angle_deg", str);
  
  sprintf (str, f2, dds->asib->tilt);
  xml_append_attribute (buf, &bsize, (char *)"tilt_deg", str);
  
  dd_radar_angles (dds->asib, dds->cfac, dds->ra, dgi);
  sprintf (str, f4, DEGREES(dds->ra->elevation));
  xml_append_attribute (buf, &bsize, (char *)"true_elevation_deg", str);
  
  sprintf (str, f4, dds->asib->altitude_msl);
  xml_append_attribute (buf, &bsize, (char *)"altitude_km_msl", str);
  
  sprintf (str, f4, dds->asib->altitude_agl);
  xml_append_attribute (buf, &bsize, (char *)"altitude_km_agl", str);
  
  sprintf (str, f2, dds->asib->heading);
  xml_append_attribute (buf, &bsize, (char *)"heading", str);
  
  sprintf (str, f2, dds->asib->roll);
  xml_append_attribute (buf, &bsize, (char *)"roll", str);
  
  sprintf (str, f2, dds->asib->pitch);
  xml_append_attribute (buf, &bsize, (char *)"pitch", str);
  
  sprintf (str, f2, dds->asib->drift_angle);
  xml_append_attribute (buf, &bsize, (char *)"drift", str);
  
  xml_end_attributes (buf, &bsize, empty_element = YES);
  
  dd_return_interpulse_periods( dgi, 0, ipp_vals );
  dd_return_frequencies( dgi, 0, freq_vals ) ;
  len = nsamp * 2 * sizeof (float);
  offs = 8;			/* data starts after descripter length */
  if (hostIsLittleEndian()) {
     swack_short ((char *)wave->gate_dist1, (char *)gate_dist, 5*2);
  }
  else {
     memcpy (gate_dist, wave->gate_dist1, 5*2*sizeof(short));
  }

  for (ipp=0; ipp < nipps; ipp++) {
     for (freq=0; freq < nfreqs; freq++) {
	
	xml_open_element (buf, &bsize, iqs);
	
	sprintf (str, f6, ipp_vals[ipp]*.001);
	xml_append_attribute (buf, &bsize, (char *)"ipp", str);
	
	sprintf (str, "%.6e", freq_vals[freq]*1.e9);
	xml_append_attribute (buf, &bsize, (char *)"freq", str);
	
	gg = dds->frib->time_series_gate;
	sprintf (str, "%d", gg);
	xml_append_attribute (buf, &bsize, (char *)"gate", str);
	ff = gate_dist[freq*2 +1]; /* "90 60 90 60 ..." */

	sprintf (str, f4, 2.5 * gg * ff *.001);
	xml_append_attribute (buf, &bsize, (char *)"range_km", str);
	
	ff = dds->frib->receiver_gain[freq]; 
	sprintf (str, f4, ff);
	xml_append_attribute (buf, &bsize, (char *)"receiver_gain", str);
			      
	sprintf (str, f4, dts->day_seconds -rtime);
	xml_append_attribute (buf, &bsize, (char *)"time_offset", str);
	
	xml_end_attributes (buf, &bsize, empty_element = NO);

	fvals = (float *)((char *)tmsr +offs);
	strcpy (name, ns); strcat (name, "I");


	for (kk=0; kk < 2; kk++) { /* do the I and Q values */
	   aa = str;
	   for (ii=0,jj=kk; ii < nsamp; ii++, jj+=2) {
	      if (hostIsLittleEndian()) 	/* these are big endian values */
		{ swack4 ((char *)&fvals[jj], (char *)&ff); }
	      else
		{ ff = fvals[jj]; }
	      sprintf (aa, "%.2f,", ff);
	      aa += strlen (aa);
	   }
	   *(aa-1) = '\0';	/* knock off the last comma */
	   xml_open_element (buf, &bsize, name);
	   xml_append_attribute (buf, &bsize, (char *)"values", str);
	   xml_end_attributes (buf, &bsize, empty_element = YES);
	   strcpy (name, ns); strcat (name, "Q");
	}
	esize = xml_close_element (buf, &bsize, iqs);
	offs += len;
     }
  }
  xml_close_element (buf, &bsize, ardr);
  loffs = (int32_t)xml_close_element (buf, &bsize, acts);
  fwrite ((void *)buf, sizeof(char), bsize, strm);
  /*
   * close the main encircling element and then
   * back over it in case it's not the last ray
   */
  if (fseek (strm, -loffs, SEEK_CUR)) {
     mark = 0;
  }
  bsize = 0;
}
/* c------------------------------------------------------------------------ */
/* c------------------------------------------------------------------------ */

void 
eld_positioning (void)
{
    int ii, jj, kk, mm, mark, direction;
    int nn, ival;
    float val;
    char str[256];
    DD_TIME dts;
    static double skip_secs=0;
    double d, dtarget;
    struct io_packet *dp, *dp0;


    slm = solo_malloc_list_mgmt(87);

    postamble[0] = '\0';
    preamble[0] = '\n';
    eld_interest(eui->dgi, 1, preamble, postamble);


 menu2:
    printf("\n\
-2 = Exit\n\
-1 = Begin processing\n\
 0 = Skip rays\n\
 1 = Inventory\n\
 2 = Skip files \n\
 3 = Rewind\n\
 4 = 16-bit integers\n\
 5 = hex display\n\
 6 = dump as characters\n\
 7 = Skip records\n\
 8 = Forward time skip\n\
 9 = Set input limits\n\
10 = Display raw header contents\n\
11 = Display ray contents\n\
Option = "
	   );
    nn = getreply(str, sizeof(str));
    if( cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 12 ) {
	if(ival == -2) exit(1);
	printf( "\nIllegal Option!\n" );
	goto menu2;
    }

    if(ival == -1) {
	solo_unmalloc_list_mgmt(slm);
	return;
    }
    else if(ival < 0)
	  exit(0);

    eui->dgi->new_sweep = NO;
    eui->dgi->new_vol = NO;

    if(ival == 0) {
	printf("Type number of rays to skip:");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > 65536) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	if(ival > 0) {
	    for(ii=0; ii < ival; ii++) {
		if((jj=eld_next_ray()) < 0)
		      break;
	    }
	}
	else if(ival < 0) {
	    /* figure out the number of rays we can really back up
	     */
	    nn = -ival > irq->packet_count ? irq->packet_count-1 : -ival;
	    dp0 = dp = dd_return_current_packet(irq);
	    for(ii=0; ii < nn; ii++, dp=dp->last) {
		/* see if the sequence num for this packet
		 * agrees with the sequence num for the input buf
		 * if so, the data has not been stomped on
		 */
		if(!dp->ib || dp->seq_num != dp->ib->seq_num)
		      break;
	    }
	    if(ii) {
		dp = dp->next;	/* loop pushed us one too far */
		/*
		 * figure out how many buffers to skip back
		 */
		kk = ((dp0->ib->b_num - dp->ib->b_num + irq->que_count)
		      % irq->que_count) +1;
		dd_skip_recs(irq, BACKWARD, kk);
		irq->top->at = dp->at;
		irq->top->bytes_left = dp->bytes_left;
	    }
	    if(ii < -ival) {
		/* did not achive goal */
		printf("Unable to back up %d rays. Backed up %d rays\n"
		       , -ival, ii);
		printf("Use record positioning to back up further\n");
	    }
	}
	eld_next_ray();
	eld_interest(eui->dgi, 1, preamble, postamble);
    }
    else if(ival == 1) {
	if((nn = eld_inventory()) == -2)
	      exit(0);
    }
    else if(ival == 2) {
	printf("Type skip: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > 65536) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	if(ival) {
	    direction = ival >= 0 ? FORWARD : BACKWARD;
	    dd_skip_files(irq, direction, ival >= 0 ? ival : -ival);
	}
	eld_next_ray();
	eld_interest(eui->dgi, 1, preamble, postamble);
    }
    else if(ival == 3) {
	dd_rewind_dev(irq);
	eld_next_ray();
	eld_interest(eui->dgi, 1, preamble, postamble);
    }
    else if(ival == 4) {
	if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
	    printf("\n");
	    ctypeu16((unsigned char*)irq->top->buf, view_char, view_char_count);
	}
    }
    else if(ival == 5) {
	if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
	    printf("\n");
	    ezhxdmp(irq->top->buf, view_char, view_char_count);
	}
    }
    else if(ival == 6) {
	if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
	    printf("\n");
	    ezascdmp(irq->top->buf, view_char, view_char_count);
	}
    }
    else if(ival == 7) {
	printf("Type record skip # or hit <return> to read next rec: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > 65536) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	direction = ival >= 0 ? FORWARD : BACKWARD;
	dd_skip_recs(irq, direction, ival > 0 ? ival : -ival);
	eld_next_ray();
	nn = irq->top->sizeof_read;
	printf("\n Read %d bytes\n", nn);
	eld_interest(eui->dgi, 1, preamble, postamble);
    }
    else if(ival == 8) {
	dtarget = 0;
	dd_clear_dts(&dts);

	printf("Type target time (e.g. 14:22:55 or +2h, +66m, +222s): ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) == 1 && !ival) {
	    /* assume they have hit a <return>
	     */
	    if(skip_secs > 0)
		 dtarget = eui->dgi->time + skip_secs; 
	}
	else if(skip_secs = dd_relative_time(str)) {
	    dtarget = eui->dgi->time + skip_secs;
	}
	else if(kk = dd_crack_datime(str, nn, &dts)) {
	    if(!dts.year) dts.year = eui->dgi->dds->dts->year;
	    if(!dts.month) dts.month = eui->dgi->dds->dts->month;
	    if(!dts.day) dts.day = eui->dgi->dds->dts->day;
	    dtarget = d_time_stamp(&dts);
	}
	if(!dtarget) {
	    printf( "\nCannot derive a time from %s!\n", str);
	    goto menu2;
	}
	printf("Skipping ahead %.3f secs\n", dtarget -eui->dgi->time);
	/* loop until the target time
	 */
	for(mm=1;; mm++) {
	    if((nn = eld_next_ray()) <= 0 || eui->dgi->time >= dtarget ||
	       dd_control_c()) {
		break;
	    }
	    if(!(mm % 1000))
		  eld_interest(eui->dgi, 1, preamble, postamble);
	    mark = 0;
	}
	if(nn)
	      eld_interest(eui->dgi, 1, preamble, postamble);
    }
    else if(ival == 9) {
	gri_nab_input_filters(eui->dgi->time, difs, 1);
    }
    else if(ival == 10) {
	eld_dump_raw_header(eui->dgi, irq, &vol_dp, slm);
    }
    else if(ival == 11) {
	eld_dump_this_ray(eui->dgi, slm);
    }
    else if(ival == 12) {
    }
    preamble[0] = '\0';

    goto menu2;
    
}
/* c------------------------------------------------------------------------ */

void 
eld_print_cspd (struct cell_spacing_d *cspd, struct solo_list_mgmt *slm)
{
    int ii;
    char *aa, bstr[128];

    aa = bstr;


    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of the cell_spacing_d descriptor  len: %d"
            , sizeof(struct cell_spacing_d));

    solo_add_list_entry(slm, aa);
    sprintf(aa, "cell_spacing_des     %s"      , cspd->cell_spacing_des);
    solo_add_list_entry(slm, aa);
    sprintf(aa, "cell_spacing_des_len %d"      , cspd->cell_spacing_des_len);
    solo_add_list_entry(slm, aa);
    sprintf(aa, "num_segments         %d"      , cspd->num_segments);
    solo_add_list_entry(slm, aa);
    sprintf(aa, "distToFirst          %d"      , cspd->distToFirst);
    solo_add_list_entry(slm, aa);

    for(ii=0; ii < 6; ii++) {
        sprintf(aa, "%3d) spacing: %4d   num_cells: %4d"
                , ii,  cspd->spacing[ii], cspd->num_cells[ii]);
        solo_add_list_entry(slm, aa);
    }
    return;
}
/* c------------------------------------------------------------------------ */

void 
eld_print_frad (struct field_parameter_data *frad, struct solo_list_mgmt *slm)
{
    int ii, jj, kk;
    short *ss;
    char *c, str[16];
    char *aa, bstr[128];

    aa = bstr;


    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of the field parameter descriptor  len: %d"
	    , sizeof(struct field_parameter_data));

    solo_add_list_entry(slm, aa);
    sprintf(aa, "field_param_data        %s", frad->field_param_data    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "field_param_data_len    %d", frad->field_param_data_len);
    solo_add_list_entry(slm, aa);
    sprintf(aa, "data_sys_status         %d", frad->data_sys_status     );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "radar_name              %s", frad->radar_name          );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "test_pulse_level        %f", frad->test_pulse_level    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "test_pulse_dist         %f", frad->test_pulse_dist     );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "test_pulse_width        %f", frad->test_pulse_width    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "test_pulse_freq         %f", frad->test_pulse_freq     );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "test_pulse_atten        %d", frad->test_pulse_atten    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "test_pulse_fnum         %d", frad->test_pulse_fnum     );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "noise_power             %f", frad->noise_power         );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ray_count               %d", frad->ray_count           );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "first_rec_gate          %d", frad->first_rec_gate      );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "last_rec_gate           %d", frad->last_rec_gate       );
    solo_add_list_entry(slm, aa);

    return;
}
/* c------------------------------------------------------------------------ */

void 
eld_print_ldat (struct lidar_parameter_data *ldat, struct solo_list_mgmt *slm)
{
    int ii;
    char str[64];
    char *aa, bstr[128];

    aa = bstr;


    /* routine to print the contents of the parameter descriptor */

    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of the lidar parameter data descriptor  len: %d"
            , sizeof(struct lidar_parameter_data));
    solo_add_list_entry(slm, aa);

    sprintf(aa, "lidar_param_data[4]  %d", ldat->lidar_param_data); 
    solo_add_list_entry(slm, aa);
    sprintf(aa, "lidar_param_data_len %d", ldat->lidar_param_data_len);
    solo_add_list_entry(slm, aa);
    sprintf(aa, "data_sys_status      %d", ldat->data_sys_status);     
    solo_add_list_entry(slm, aa);
    sprintf(aa, "lidar_name[8]        %s"
	    , str_terminate(str, ldat->lidar_name, sizeof(ldat->lidar_name)));  
    solo_add_list_entry(slm, aa);

    for(ii=0; ii < 10; ii++) {
	sprintf(aa, "pulse_energy[%d]     %f", ii, ldat->pulse_energy[ii]);    
	solo_add_list_entry(slm, aa);
    }

    sprintf(aa, "ray_count            %d", ldat->ray_count);           
    solo_add_list_entry(slm, aa);

    for(ii=0; ii < 10; ii++) {
	sprintf(aa, "derived_param[%d]    %f", ii, ldat->derived_param[ii]);   
	solo_add_list_entry(slm, aa);
    }

    sprintf(aa, "event                %d", ldat->event);               
    solo_add_list_entry(slm, aa);
    sprintf(aa, "pmt_voltage          %d", ldat->pmt_voltage);         
    solo_add_list_entry(slm, aa);
    sprintf(aa, "apd_voltage          %d", ldat->apd_voltage);         
    solo_add_list_entry(slm, aa);
    sprintf(aa, "flashlmp_voltage     %d", ldat->flashlmp_voltage);    
    solo_add_list_entry(slm, aa);

    for(ii=0; ii < 10; ii++) {
	sprintf(aa, "temperatures[%d]     %d", ii, ldat->temperatures[ii]);    
	solo_add_list_entry(slm, aa);
    }

    sprintf(aa, "first_rec_gate       %d", ldat->first_rec_gate);      
    solo_add_list_entry(slm, aa);
    sprintf(aa, "last_rec_gate        %d", ldat->last_rec_gate);       
    solo_add_list_entry(slm, aa);

    return;
}
/* c------------------------------------------------------------------------ */

void eld_print_stat_line (int count, struct dd_general_info *dgi)
{
    DD_TIME *dts=dgi->dds->dts;

    printf("%5d %3d  %s  %.3f %s f:%2d r:%4d %.2f MB\n",
	   count,
	   dgi->dds->ryib->sweep_num,
	   dgi->radar_name.c_str(),
	   dts->time_stamp,
	   dts_print(d_unstamp_time(dts)),
	   dd_stats->file_count,
	   dd_stats->rec_count,
	   dd_stats->MB);
}
/* c------------------------------------------------------------------------ */

void 
eld_radar_rename (char *name)
{
    /* spiff up the radar name */

    if( strncmp(name,"FORE",4)==0 ) {
	/* if the name is just FORE */
	strncpy(name,"TF-ELDR  ", 8);
    }
    else if( strncmp(name,"AFT",3)==0 ) {
	strncpy(name,"TA-ELDR  ", 8);
    }
}
/* c------------------------------------------------------------------------ */

void eld_reset (void)
{
    /* reset various parameters so the program will startup right
     */
    int ii;
    struct individual_radar_info *iri;
    struct dd_general_info *dgi;

    difs->stop_flag = NO;

    for(ii=0; ii < MAX_SENSORS; ii++){ /* for all the radars so far */
	if(eui->iri[ii]->radar_num < 0)
	      continue;
	iri = eui->iri[ii];
	iri->rotang_list[iri->rotang_ndx] = -360.;
	iri->ray_count = iri->sweep_count = iri->vol_count = 0;
	iri->vol_count_flag = iri->sweep_count_flag = YES;
	dgi = dd_get_structs(iri->radar_num);
	dgi->source_sweep_num = -1;
	dgi->sweep_count = 0;
	dgi->new_sweep = dgi->new_vol = YES;
    }
}
/* c------------------------------------------------------------------------ */

struct eldora_unique_info *
eld_return_eui (void)
{
    return(eui);
}
/* c------------------------------------------------------------------------ */

int 
eld_select_ray (struct dd_general_info *dgi)
{
    int i, ii, ok=YES, mark, rn=eui->radar_num;
    double d;



    if( dgi->new_sweep ) {
	/* increment the sweep count for this radar
	 * and see if it should be used
	 */
	if(difs->sweep_skip) {
	    eui->use_this_sweep[rn] = 
		  dgi->sweep_count % difs->sweep_skip ? NO : YES;
	}
	else {
	    eui->use_this_sweep[rn] = YES;
	    mark = 0;
	}
    }

    if(difs->beam_skip) {
	if(eui->iri[rn]->ray_count % difs->beam_skip){
	    mark = 0;
	    return(NO);
	}
    }

    if(dgi->time >= difs->final_stop_time)
	  difs->stop_flag = YES;

    if(eui->ignore_this_ray)
	  return(NO);

    if(!eui->use_this_sweep[rn])
	  return(NO);

    if(!difs->radar_selected[dgi->radar_num]) 
	  return(NO);

    if(difs->num_time_lims) {
	for(i=0; i < difs->num_time_lims; i++ ) {
	    if(dgi->time >= difs->times[i]->lower &&
	       dgi->time <= difs->times[i]->upper)
		  break;
	}
	if(i == difs->num_time_lims)
	      return(NO);
    }
    if(difs->num_az_sectors) {
       d = eui->options & ROT_ANG_SECTORS ? dd_rotation_angle(dgi) :
	 dd_azimuth_angle(dgi);
	for(ii=0; ii < difs->num_az_sectors; ii++) {
	    if(in_sector((float)d
			 , (float)difs->azs[ii]->lower
			 , (float)difs->azs[ii]->upper)) {
		break;
	    }
	}
	if(ii == difs->num_az_sectors)
	      ok = NO;
    }
    
    return(ok);
}
/* c------------------------------------------------------------------------ */

int 
eld_stuff_data (struct dd_general_info *dgi, int parm_num)
{
    char *dst;
    int i, k, mm, n, nb, ng=0, sizeof_datum, nc, frg, lrg, clen;
    int pn = parm_num, dlen;
    struct dds_structs *dds=dgi->dds;
    char *d, bbad;
    short *ss, sbad, *lut;
    unsigned short *s;
    int32_t lbad;
    struct cell_d *celv=dds->celv;
    struct field_parameter_data *frad=dds->frad;
    struct parameter_d *parm=dgi->dds->parm[parm_num];
    struct individual_radar_info *iri=eui->iri[dgi->radar_num];

    if(difs->catalog_only)
	  return( celv->number_cells*iri->sizeof_fparm[parm_num]);

    dgi->dds->field_present[pn] = YES;
# ifdef NEW_ALLOC_SCHEME
    dst = (char *)dds->qdat_ptrs[pn];
# else
    dst = (char *)dds->rdat[pn] +sizeof(struct paramdata_d);
# endif
    sizeof_datum = iri->sizeof_fparm[parm_num];
    nb = frad->field_param_data_len - sizeof(struct field_parameter_data);
    ng = nc = celv->number_cells;
    frg = frad->first_rec_gate > 0 ? frad->first_rec_gate : 0;
    lrg = frad->last_rec_gate > 0 ? frad->last_rec_gate : nc;
    if (lrg <= frg)
      { frg = 0; lrg = nc; }

    lut = iri->xlate_lut[parm_num];
    d = dds->raw_data +iri->fparm_offset[parm_num];

    switch(sizeof_datum) {
    case 1:			/* byte */
	bbad = parm->bad_data;
	break;
    case 2:			/* 2 bytes */
	sbad=parm->bad_data;
	for(ss=(short *)dst,i=0; i < frg; i++)
	      *ss++ = sbad;
	k = iri->num_parms;	/* assumes all parms are 2 bytes */
	clen = k*sizeof(short);
	ng = nb/clen;
	if(lrg > ng) {
	    lrg = ng;
	}
	d += frg*k*sizeof(short);
	n = lrg-frg;

	for(s=(unsigned short *)d; n--; s+=k)
	      *ss++ = *(lut+ *s);

	for(; lrg++ < nc; *ss++ = sbad);
	break;
    case 4:			/* 4 bytes */
	lbad = parm->bad_data;
	break;
    default:
	break;
    }
    mm = LONGS_TO_BYTES(BYTES_TO_LONGS(celv->number_cells * sizeof_datum));

# ifdef NEW_ALLOC_SCHEME
    dlen = *dds->qdat[pn]->pdata_desc == 'R'
	  ? sizeof(struct paramdata_d) : sizeof(struct qparamdata_d);
    dds->qdat[pn]->pdata_length = dlen + mm;
# else
    dlen = sizeof(struct paramdata_d);
    dds->rdat[pn]->pdata_length = dlen + mm;
# endif
    return( celv->number_cells*sizeof_datum );
}
/* c------------------------------------------------------------------------ */

void eld_stuff_ray( struct dd_general_info *dgi, time_t current_time)
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
    struct dds_structs *dds=dgi->dds;
    int i, jj, nn, pn, new_vol=NO;
    int sizeof_ray=0, sizeof_data, mark;
    char *aa, *b, str[256];
    static int count=0, zit=123;
    double d;
    DD_TIME *dts;

    struct ray_i *ryib=dds->ryib;
    struct platform_i *asib=dds->asib;
    struct prev_rays *prs;
    struct individual_radar_info *iri=eui->iri[dgi->radar_num];


    count++;
    if(count >= zit) {
	mark=0;			/* c...mark */
    }
    dgi->ray_quality = 0;
    prs = dgi->ray_que = dgi->ray_que->next;
    prs->source_sweep_num = eui->fake_sweep_nums ?
	  iri->sweep_count : ryib->sweep_num;
    dgi->beam_count++;

    if(dgi->new_sweep) {
	if(eui->options & ELD_AC_COMPARE) {
	    sprintf(str, "%.4f %.4f %.3f %.2f %.2f %.2f %.2f %s"
		    , asib->longitude, asib->latitude, asib->altitude_msl
		    , asib->heading, asib->roll, asib->pitch, asib->drift_angle
		    , dts_print(d_unstamp_time(dgi->dds->dts)));
	    dd_append_cat_comment(str);
	}
    }
    /* merge ac data
     */
    if(dgi->time > time_for_nimbus) {
	acmrg = eld_nimbus_fix_asib(dgi->dds->dts, asib, eui->options
				    , dgi->radar_num);
    }
    else {
	eld_gpro_fix_asib(dgi->dds->dts, asib, eui->options); 
    }
    /* build a struct containing all the corrected angles
     */
    dd_radar_angles( asib, dds->cfac, dds->ra, dgi );

    ryib->azimuth = DEGREES(dds->ra->azimuth);
    ryib->elevation = DEGREES(dds->ra->elevation);

    if(difs->altitude_truncations) {
	dgi->clip_gate = dd_clip_gate
	      (dgi, dds->ra->elevation, asib->altitude_msl
	       , difs->altitude_limits->lower
	       , difs->altitude_limits->upper);
    }
    else
	  dgi->clip_gate = dds->celv->number_cells-1;

    prs->clip_gate = dgi->clip_gate;
    /*
     * stuff in the data
     */
    if(!eui->num_rays_avgd) {
	for(pn=0; pn < iri->num_parms; pn++ ) {
	    sizeof_data =
		  eld_stuff_data(dgi, pn);
	}
    }
    else {
	dts = dgi->dds->dts;
	dts->time_stamp = dgi->time = iri->avg_time_ctr;
	d_unstamp_time(dts);
	ryib->julian_day = dts->julian_day;
	ryib->hour = dts->hour;
	ryib->minute = dts->minute;
	ryib->second = dts->second;
	ryib->millisecond = dts->millisecond;
    }
    prs->heading = asib->heading;
    prs->dH = angdiff(prs->last->heading, prs->heading);
    prs->pitch = asib->pitch;
    prs->dP = prs->pitch -prs->last->pitch;
    prs->time = dgi->time;
    prs->rotation_angle = dd_rotation_angle(dgi);
    if(dgi->new_vol) {
	if (acmrg) {
	  nn = sizeof (dds->vold->proj_name);
	  aa = dds->vold->proj_name;
	  str_terminate (str, aa, nn);
	  if (strstr (str, acmrg))
	    { jj = 0; }
	  else if (strlen (str) + strlen (acmrg) < nn ) 
	    { strcat (str, acmrg); }
	  else {
	    jj = nn-strlen (acmrg)-1;
	    strcat (str+jj, acmrg);
	  }
	  strcpy (aa, str);
	}
	dd_dump_headers(dgi);
    }
    if( dgi->new_sweep ) {
	eld_init_sweep(dgi, current_time, new_vol);
    }
    dgi->swp_que->end_time = dgi->time;
    dgi->swp_que->num_rays++;

    ddin_clip_data( dgi );
    by_products(dgi, current_time);
    dd_dump_ray(dgi);

    dgi->new_sweep = NO;
    dgi->new_vol = NO;
    eui->time = dgi->time;
    dgi->prev_scan_mode = dgi->dds->radd->scan_mode;
    dgi->prev_vol_num = dgi->dds->vold->volume_num;

    if(dgi->ray_quality & UNACCEPTABLE)
	  iri->bad_ray_count++;
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */



