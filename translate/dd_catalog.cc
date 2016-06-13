#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

#include <dd_math.h>
#include "dd_stats.h"
#include <sys/time.h>
#include <piraq.hh>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <math.h>
#include <stdio.h>
#include "dd_catalog.hh"
#include "dda_common.hh"
#include "dorade_share.hh"
#include "ddb_common.hh"
#include <fcntl.h>

# define READ_AND_WRITE_ACCESS 2
# define    DDCAT_NORMAL_WRITE 0x0001
# define     DDCAT_NUVOL_WRITE 0x0002
# define   DDCAT_NUVOL_REWRITE 0x0004
# define           DDCAT_FLUSH 0x0008

# define MAX_ROTANG_DELTA 4.
# define MAX_RANGE_SEGMENTS 64
# define MAX_TEXT_SIZE 4*BLOCK_SIZE
# define MAX_SWEEP_SPACE 8192
# define MAX_ARG_COUNT 16
# define MAX_ARG_SIZE 80
# define CAT_FLUSH_POINT BLOCK_SIZE
# define FLUSH_SWEEP_COUNT 5
# define FLUSH YES

# define VOLUME "VOLUME"
# define SENSOR "SENSOR"
# define PARAMETER "PARAMETER"
# define SCAN "SCAN"
# define RANGE_INFO "RANGE_INFO"

char *Unix_months[] = { (char *)"JAN", (char *)"FEB", (char *)"MAR",
			(char *)"APR", (char *)"MAY", (char *)"JUN",
			(char *)"JUL", (char *)"AUG", (char *)"SEP",
			(char *)"OCT", (char *)"NOV", (char *)"DEC" };

char *Stypes[] = { (char *)"CAL", (char *)"PPI", (char *)"COP", (char *)"RHI",
		   (char *)"VER", (char *)"TAR", (char *)"MAN", (char *)"IDL",
		   (char *)"SUR", (char *)"AIR", (char *)"???" };
/* for each possible radar
 * you need to check for a new volume
 * or a new sweep and keep interesting information
 * from the beginning and end of each sweep
 *
 */

static struct comment_mgmt *cmt=NULL;
static int time_interval=0;		/* every n seconds */
static int flush_sweep_count=FLUSH_SWEEP_COUNT;
static int Catalog_flag=YES;
static struct dd_stats *dd_stats=NULL;

static int Cat_radar_count=0;
static struct catalog_info *ci[MAX_SENSORS];
static char *source_tape_id=NULL;

/* c------------------------------------------------------------------------ */

void 
dd_append_cat_comment (char *comm_str)
{
    /* it is assumed each call represents a line of text in the catalog
     */
    int nc=strlen(comm_str);

    if(!nc || !cmt)
	  return;

    if(cmt->sizeof_comments +nc +4 >= cmt->sizeof_buf) {
	cmt->sizeof_buf = cmt->sizeof_comments +nc +4;
	cmt->buf = (char *)realloc(cmt->buf, cmt->sizeof_buf);
	cmt->at = cmt->buf +cmt->sizeof_comments;
    }
    strcat(cmt->at, "! ");
    strcat(cmt->at, comm_str);
    strcat(cmt->at, "\n");
    cmt->sizeof_comments += strlen(cmt->at);
    cmt->at += strlen(cmt->at);
}
/* c------------------------------------------------------------------------ */

void 
dd_enable_cat_comments (void)
{
    if(cmt)
	  return;

    cmt = (struct comment_mgmt *)malloc(sizeof(struct comment_mgmt));
    memset(cmt, 0, sizeof(struct comment_mgmt));
    cmt->sizeof_buf = 512;
    cmt->buf = cmt->at = (char *)malloc(cmt->sizeof_buf);
    memset(cmt->at, 0, cmt->sizeof_buf);
    
}
/* c------------------------------------------------------------------------ */

void dd_catalog(struct dd_general_info *dgi, time_t ignored_time, int flush)
{
    int rn=dgi->radar_num;
    static int count=0;
    int i, j, new_vol, new_swp, mark;
    double d;
    char *a, *b;
    float rotang;

    time_t current_time=(time_t) dgi->time;
    struct dds_structs *dds=dgi->dds;
    struct radar_d *radd=dds->radd;
    struct sweepinfo_d *swib=dds->swib;
    struct platform_i *asib=dds->asib;
    struct catalog_info *cii;


    count++;

    if(!Cat_radar_count) {
	for(i=0; i < MAX_SENSORS; i++) {
	    ci[i] = NULL;
	}
	dd_enable_cat_comments();
	dd_stats = dd_return_stats_ptr();

	if(a = get_tagged_string( "OUTPUT_FLAGS" )) {
	    if(b=strstr(a, "NO_CATALOG")) {
		Catalog_flag = NO;
	    }
	}
	if(a=get_tagged_string("CATALOG_SWEEP_SUMMARY")) {
	    if((j = atoi(a)) >= 0 )
		  time_interval = j;
	}
	if(a=get_tagged_string("CATALOG_FLUSH_COUNT")) {
	    if((j = atoi(a)) > 0 )
		  flush_sweep_count = j;
	}
	if(a=get_tagged_string("SOURCE_TAPE_ID")) {
	    source_tape_id = a;
	}
    }

    /* get structure for this radar
     */
    if(ci[rn] == NULL) {
	/* create space for and initialize info for this radar
	 */
	cii = ci[rn] =
	      (struct catalog_info *)malloc(sizeof(struct catalog_info));
	memset((char *)cii, 0, sizeof(struct catalog_info));
	cii->volumes_fid = -1;
	Cat_radar_count++;
	cii->vol_txt_ptr = (char *)malloc(BLOCK_SIZE/2);
	cii->swp_txt_ptr = (char *)malloc(BLOCK_SIZE);
    }
    else {
	cii = ci[rn];
    }

    if( flush == YES ) {

	if(cii->vol_txt_size <= 0)
	      return;
	cat_volume(dgi, cii, cii->stop_time, flush);
	cii->prev_MB_count = dd_stats->MB;
	cat_sweep(dgi, cii, cii->stop_time, flush);
	ddcat_close( cii->volumes_fid );
	cii->volumes_fid = -1;
	return;
    }

    if(cii->volumes_fid == -1) { /* initialization or reinitialization
				  */
	a = cii->vol_txt_ptr;
	b = cii->swp_txt_ptr;
	memset((char *)cii, 0, sizeof(struct catalog_info));
	cii->vol_txt_ptr = a;
	cii->swp_txt_ptr = b;
	memset(cii->vol_txt_ptr, 0, BLOCK_SIZE/2);
	memset(cii->swp_txt_ptr, 0, BLOCK_SIZE);
	cii->vol_num = -1;
	cii->radar_num = dgi->radar_num;
	strcpy(cii->volumes_file_name, dgi->directory_name.c_str());
	dd_file_name("cat", current_time, dd_radar_name(dds), 0
		     , cii->volumes_file_name
		     +strlen(cii->volumes_file_name));
	cii->volumes_fid = ddcat_open( cii->volumes_file_name );
    }

    /*
     * process this ray
     */

    new_vol = new_swp = dgi->vol_num != cii->vol_num;
    if(!new_vol && dgi->sweep_num != cii->sweep_num) {
	i = (   radd->radar_type == DD_RADAR_TYPE_GROUND
	     || radd->radar_type == DD_RADAR_TYPE_SHIP
	     || radd->radar_type == DD_RADAR_TYPE_AIR_LF);
	j = current_time > cii->trip_time;
	new_swp = i || j ? YES : NO;
    }
    if( new_vol && cii->vol_txt_size > 0 ) {
	cat_volume(dgi, cii, cii->stop_time, FLUSH);
    }
    if( new_swp && cii->swp_txt_size > 0 ) {
	cat_sweep(dgi, cii, cii->stop_time, FLUSH);
    }
    if(new_vol) {
	/* Initialize for a new volume
	 */
	cii->vol_num = dgi->vol_num;
	cat_volume(dgi, cii, current_time, NO);
	cii->trip_time = current_time +time_interval;
    }

    if(new_swp) {
	/* Initialize for a new sweep
	 * but don't do it for every sweep if aircraft data
	 */
	cii->sweep_num = dgi->sweep_num;
	cii->num_rays = 0;
	cat_sweep(dgi, cii, current_time, NO);

	if(current_time > cii->trip_time) {
	    cii->trip_time = current_time +time_interval;
	}
    }

    cii->stop_time = current_time;
    cii->num_rays++;
    cii->rays_per_volume++;
    cii->f_fixed_angle = swib->fixed_angle;
    cii->f_swp_stop_angle = swib->stop_angle;
    cii->f_stop_latitude = dd_latitude(dgi);
    cii->f_stop_longitude = dd_longitude(dgi);
    cii->f_stop_altitude = dd_altitude(dgi);
    cii->f_stop_EW_wind = asib->ew_horiz_wind;
    cii->f_stop_NS_wind = asib->ns_horiz_wind;
    cii->f_ew_velocity += asib->ew_velocity;
    cii->f_ns_velocity += asib->ns_velocity;
    d = dd_heading(dgi);
    cii->d_cos_heading += cos((double)RADIANS(90.-d));
    cii->d_sin_heading += sin((double)RADIANS(90.-d));
    d = dd_drift(dgi);
    cii->d_cos_drift += cos((double)RADIANS(d));
    cii->d_sin_drift += sin((double)RADIANS(d));
    d = dd_pitch(dgi);
    cii->d_cos_pitch += cos((double)RADIANS(d));
    cii->d_sin_pitch += sin((double)RADIANS(d));
    d = dd_roll(dgi);
    cii->d_cos_roll += cos((double)RADIANS(d));
    cii->d_sin_roll += sin((double)RADIANS(d));

    rotang = dgi->dds->radd->scan_mode == DD_SCAN_MODE_RHI ?
      dd_elevation_angle(dgi)
	: dd_rotation_angle(dgi);
    /* this is done so that upward (counter clockwise) rotation for RHIs
     * appears positive
     */
    d = angdiff(cii->f_last_rotation_angle, rotang );

    if(fabs(d) < MAX_ROTANG_DELTA) {
	cii->avg_rota_count++;
	cii->f_delta_angles += d;
    }
    cii->f_last_rotation_angle = rotang;
    cii->prev_MB_count = dd_stats->MB;
    cii->prev_rec_count = dd_stats->rec_count;
    cii->num_good_cells += dgi->num_good_cells;
    dgi->num_good_cells = 0;
    cii->tot_cells += dgi->dds->celv->number_cells;
    return;
}
/* c------------------------------------------------------------------------ */

void cat_volume(struct dd_general_info *dgi, 
                struct catalog_info *cii, 
                time_t current_time, 
                int finish)
{
    int i, n;
    double d;
    char arglist[MAX_ARG_COUNT][MAX_ARG_SIZE], *arg;
    char *arg_ptr[MAX_ARG_COUNT];
    char str[256], *arg1=arglist[0];
    float f, gs, gate_spacing[MAX_RANGE_SEGMENTS];
    int num_gates[MAX_RANGE_SEGMENTS];
    short date_time[6];
    time_t t;

    struct dds_structs *dds=dgi->dds;
    struct volume_d *vold=dds->vold;
    struct radar_d *radd=dds->radd;
    struct cell_d *celv=dds->celv;
    char *cat=cii->vol_txt_ptr;
    HEADERV *dwel;

    
    if( finish == YES ) {
	str[0] = '\0';
	cat_cat_att( (char *)"Stop_date", 1, cat_date(cii->stop_time, arglist[0]), str );
	cat_cat_att( (char *)"Stop_time", 1, cat_time(cii->stop_time, arglist[0]), str );
	strncpy( cii->vol_stop_time, str, strlen(str));
	ddcat_write(cii, cii->vol_txt_ptr, cii->vol_txt_size
		     , DDCAT_NUVOL_REWRITE);
	cii->vol_txt_size = 0;
	return;
    }

    cii->vol_rec_mark = cii->prev_rec_count;

    *cii->vol_txt_ptr = 0;
    cat_cat_att( (char *)VOLUME, 0, arglist[0], cat );

    sprintf(arg1, "%d", vold->volume_num );
    cat_cat_att( (char *)"Number", 1, arglist[0], cat );

    if(source_tape_id) {
	cat_cat_att( (char *)"Source_tape_id", 1, source_tape_id, cat );
    }

    sprintf(arg1, "%.2f", (float)cii->prev_MB_count);
    cat_cat_att( (char *)"MB_to_Vol", 1, arglist[0], cat );

    sprintf(arg1, "%d", current_time );
    cat_cat_att( (char *)"Unix_time_stamp", 1, arglist[0], cat );

    cat_cat_att( (char *)"Start_date", 1, cat_date(current_time, arglist[0]), cat );
    cat_cat_att( (char *)"Start_time", 1, cat_time(current_time, arglist[0]), cat );

    cii->vol_stop_time = (cat += strlen(cat));
    cat_cat_att( (char *)"Stop_date", 1, cat_date(current_time, arglist[0]), cat );
    cat_cat_att( (char *)"Stop_time", 1, cat_time(current_time, arglist[0]), cat );

    t = todays_date(date_time);
    cat_cat_att( (char *)"Production_date", 1, cat_date(t, arglist[0]), cat );
    cat_cat_att( (char *)"Production_time", 1, cat_time(t, arglist[0]), cat );
    

    c_deblank(vold->proj_name, 20, arglist[0]);
    cat_cat_att( (char *)"Project", 1, arglist[0], cat );

    arg = (char *)"/RTF/ATD/NCAR/UCAR/NSF";
    cat_cat_att( (char *)"Facility", 1, arg, cat );

    arg = (char *)"GMT";
    cat_cat_att( (char *)"Time_zone", 1, arg, cat );

    if( dgi->source_fmt == APIRAQ_FMT ) {
      dwel = (HEADERV *)dgi->gpptr2;
      sprintf(arg1, "%d", dwel->clutterfilter );
      cat_cat_att( (char *)"Piraq_clutter_filter", 1, arglist[0], cat );
    }

    c_deblank(vold->flight_num, 8, arglist[0]);
    cat_cat_att( (char *)"Flight/IOP_number", 1, arglist[0], cat );

    /* Begin sensor specific attributes
     */
    cat_cat_att( (char *)SENSOR, 0, arglist[0], cat );

    c_deblank(dds->radd->radar_name, 8, arglist[0]);
    cat_cat_att( (char *)"Name", 1, arglist[0], cat );

    cii->radar_type = radd->radar_type;

    switch (radd->radar_type) {
	
    case DD_RADAR_TYPE_GROUND:
	arg = (char *)"Ground_based";
	break;
    case DD_RADAR_TYPE_AIR_FORE:
	arg = (char *)"Airborne_forward";
	break;
    case DD_RADAR_TYPE_AIR_AFT:
	arg = (char *)"Airborne_aft";
	break;
    case DD_RADAR_TYPE_AIR_TAIL:
	arg = (char *)"Airborne_tail";
	break;
    case DD_RADAR_TYPE_AIR_LF:
	arg = (char *)"Airborne_lower_fuselage";
	break;
    case DD_RADAR_TYPE_SHIP:
	arg = (char *)"Shipboard";
	break;
    case DD_RADAR_TYPE_AIR_NOSE:
	arg = (char *)"Airborne_nose";
	break;
    case DD_RADAR_TYPE_SATELLITE:
	arg = (char *)"Satellite";
	break;
    default:
	arg = (char *)"Airborne_forward";
	break;
    }
    cat_cat_att( (char *)"Platform_configuration", 1, arg, cat );

    cat += strlen(cat);
    arg = Stypes[radd->scan_mode];
    cat_cat_att( (char *)"Scan_mode", 1, arg, cat );

    sprintf(arg1, "%.3e", radd->radar_const);
    cat_cat_att( (char *)"Radar_constant", 1, arglist[0], cat );

    sprintf(arg1, "%.3e", radd->peak_power);
    cat_cat_att( (char *)"Peak_power", 1, arglist[0], cat );

    sprintf(arg1, "%.3e", radd->noise_power);
    cat_cat_att( (char *)"Noise_power", 1, arglist[0], cat );

    sprintf(arg1, "%.3e", radd->receiver_gain);
    cat_cat_att( (char *)"Receiver_gain", 1, arglist[0], cat );

    sprintf(arg1, "%.3e", radd->antenna_gain);
    cat_cat_att( (char *)"Antenna_gain", 1, arglist[0], cat );

    sprintf(arg1, "%.3e", radd->system_gain);
    cat_cat_att( (char *)"System_gain", 1, arglist[0], cat );

    sprintf(arg1, "%.2f", radd->horz_beam_width);
    cat_cat_att( (char *)"Horz_beam_width_(deg)", 1, arglist[0], cat );

    sprintf(arg1, "%.2f", radd->vert_beam_width);
    cat_cat_att( (char *)"Vert_beam_width_(deg)", 1, arglist[0], cat );

    sprintf(arg1, "%.2f", radd->req_rotat_vel);
    cat_cat_att( (char *)"Rotational_velocity_(deg/sec)", 1, arglist[0], cat );

    cat += strlen(cat);

    switch (radd->data_compress) {
    case NO_COMPRESSION:
	arg = (char *)"No_compression";
	break;
    case HRD_COMPRESSION:
	arg = (char *)"Hrd_compression";
	break;
    default:
	arg = (char *)"Unknown";
	break;
    }
    cat_cat_att((char *)"Data_compression", 1, arg, cat );

    switch (radd->data_reduction) {
    case NO_DATA_REDUCTION:
	arg_ptr[0] = (char *)"None";
	n = 1;
	break;
    case TWO_ANGLES:
	arg_ptr[0] = (char *)"Between_angles";
	n = 3;
	break;
    case TWO_CIRCLES:
	arg_ptr[0] = (char *)"Between_concentric_circles";
	n = 3;
	break;
    case TWO_ALTITUDES:
	arg_ptr[0] = (char *)"Above/below altitudes";
	n = 3;
	break;
    default:
	arg_ptr[0] = (char *)"Unknown";
	n = 1;
	break;
    }
    if( n > 1 ) {
	sprintf( arglist[1], "%.1f", radd->data_red_parm0);
	sprintf( arglist[2], "%.1f", radd->data_red_parm1);
	arg_ptr[1] = arglist[1];
	arg_ptr[2] = arglist[2];
	cat_cat_att_args( (char *)"Data_reduction", n, arg_ptr, cat );
    }
    else {
	cat_cat_att((char *)"Data_reduction", 1, arg_ptr[0], cat );
    }

    /* Summarize the range/gate spacing info
     */

    cat_cat_att( (char *)RANGE_INFO, 0, arglist[0], cat );
    gate_spacing[0] = gs = celv->dist_cells[1] - celv->dist_cells[0];
    num_gates[0] = 2;

    /* itemize the range segments
     */
    for(i=1,n=0; i < celv->number_cells-1; i++ ) {
	d = celv->dist_cells[i+1] - celv->dist_cells[i];
	if( fabs(d-gs) > .9 ) {
	    gs = d;
	    n++;
	    if(n+1 >= MAX_RANGE_SEGMENTS) {
		break;
	    }
	    gate_spacing[n] = gs;
	    num_gates[n] = 0;
	}
	num_gates[n]++;
    }
    n++;
    sprintf(arg1, "%d", n );
    cat_cat_att( (char *)"Num_segments", 1, arglist[0], cat );

    sprintf(arg1, "%.1f", celv->dist_cells[0]
	    +dds->cfac->range_delay_corr);
    cat_cat_att( (char *)"Range_gate1_center", 1, arglist[0], cat );

    sprintf(arg1, "%.1f", celv->dist_cells[celv->number_cells-1]
	    +dds->cfac->range_delay_corr);
    cat_cat_att( (char *)"Max_range", 1, arglist[0], cat );

    arg_ptr[0] = arglist[0];
    arg_ptr[1] = arglist[1];
    
    for(i=0; i < n; i++ ) {
	sprintf( arglist[0], "%d", num_gates[i]);
	sprintf( arglist[1], "%.1f", gate_spacing[i]);
	cat_cat_att_args( (char *)"Segment_pair", 2, arg_ptr, cat );
    }
    cat_cat_att(reverse_string( (char *)RANGE_INFO, arglist[0]), 0, arglist[0], cat );


    /* create a list of the parameters present in this volume
     */
    if (dgi->source_field_mnemonics != "") {
	cat_cat_att("Raw_fields_present", 1, dgi->source_field_mnemonics, cat );
    }
    sprintf(arg1, "%d", dgi->num_parms );
    cat_cat_att( (char *)"Param_count", 1, arglist[0], cat );

    for(i=0; i < dgi->num_parms; i++ ) {
	cat_cat_att( (char *)PARAMETER, 0, arglist[0], cat );

	c_deblank( dds->parm[i]->parameter_name, 8, arglist[0]);
	cat_cat_att( (char *)"Name", 1, arglist[0], cat );

	c_deblank( dds->parm[i]->param_description, 39, arglist[0]);
	cat_cat_att( (char *)"Description", 1, arglist[0], cat );

	c_deblank( dds->parm[i]->param_units, 8, arglist[0]);
	cat_cat_att( (char *)"Units", 1, arglist[0], cat );

	sprintf(arg1, "%.3f", dds->parm[i]->recvr_bandwidth);
	cat_cat_att( (char *)"Receiver_bandwidth_(mhz)", 1, arglist[0], cat );

	switch (dds->parm[i]->polarization) {

	case DD_POLAR_HORIZONTAL:
	    arg = (char *)"Horizontal";
	    break;
	case DD_POLAR_VERTICAL:
	    arg = (char *)"Vertical";
	    break;
	case DD_POLAR_CIRCULAR_RIGHT:
	    arg = (char *)"Circular_right";
	    break;
	case DD_POLAR_ELLIPTICAL:
	    arg = (char *)"Ellipitical";
	    break;
	default:
	    arg = (char *)"Unknown";
	    break;
	}
	cat_cat_att( (char *)"Polarization", 1, arg, cat );

	sprintf(arg1, "%d", dds->parm[i]->num_samples );
	cat_cat_att( (char *)"Num_samples", 1, arglist[0], cat );

	switch (dds->parm[i]->binary_format) {

	case DD_8_BITS:
	    arg = (char *)"8_bit_integers";
	    break;
	case DD_16_BITS:
	    arg = (char *)"16_bit_integers";
	    break;
	case DD_24_BITS:
	    arg = (char *)"24_bit_integers";
	    break;
	case DD_32_BIT_FP:
	    arg = (char *)"32_bit_floating_point";
	    break;
	case DD_16_BIT_FP:
	    arg = (char *)"16_bit_floating_point";
	    break;
	default:
	    arg = (char *)"Unknown";
	    break;
	}
	cat_cat_att( (char *)"Binary_format", 1, arg, cat );

	sprintf(arg1, "%.3e", dds->parm[i]->parameter_scale);
	cat_cat_att( (char *)"Param_scale", 1, arglist[0], cat );

	sprintf(arg1, "%.3e", dds->parm[i]->parameter_bias);
	cat_cat_att( (char *)"Param_bias", 1, arglist[0], cat );

	cat_cat_att(reverse_string( (char *)PARAMETER, arglist[0]), 0, arglist[0], cat );
	cat += strlen(cat);
    }

    /* List the frequencies and PRTs
     */
    for(i=0; i < radd->num_freq_trans; i ++ ) {
	switch (i) {

	case 0:
	    f = radd->freq1;
	    break;
	case 1:
	    f = radd->freq2;
	    break;
	case 2:
	    f = radd->freq3;
	    break;
	case 3:
	    f = radd->freq4;
	    break;
	case 4:
	    f = radd->freq5;
	    break;
	}
	sprintf( arglist[i], "%.3e", f );
	arg_ptr[i] = arglist[i];
    }
    cat_cat_att_args( (char *)"Frequencies", radd->num_freq_trans, arg_ptr, cat );

    for(i=0; i < radd->num_ipps_trans; i ++ ) {
	switch (i) {

	case 0:
	    f = radd->interpulse_per1;
	    break;
	case 1:
	    f = radd->interpulse_per2;
	    break;
	case 2:
	    f = radd->interpulse_per3;
	    break;
	case 3:
	    f = radd->interpulse_per4;
	    break;
	case 4:
	    f = radd->interpulse_per5;
	    break;
	}
    	sprintf( arglist[i], "%.3e", f );
	arg_ptr[i] = arglist[i];
    }
    cat_cat_att_args( (char *)"Interpulse_periods", radd->num_ipps_trans
		, arg_ptr, cat );

    cat += strlen(cat);
    sprintf(arg1, "%.4f", dd_longitude(dgi));
    cat_cat_att( (char *)"Radar_longitude", 1, arglist[0], cat );

    sprintf(arg1, "%.4f", dd_latitude(dgi));
    cat_cat_att( (char *)"Radar_latitude", 1, arglist[0], cat );

    sprintf(arg1, "%.4f", dd_altitude(dgi));
    cat_cat_att( (char *)"Radar_altitude", 1, arglist[0], cat );

    sprintf(arg1, "%.2f", radd->eff_unamb_vel );
    cat_cat_att( (char *)"Unambiguous_velocity_(m/s)", 1, arglist[0], cat );

    sprintf(arg1, "%.3f", radd->eff_unamb_range);
    cat_cat_att( (char *)"Unambiguous_range_(km)", 1, arglist[0], cat );


    cat_cat_att(reverse_string((char *)SENSOR,arglist[0]), 0, arglist[0], cat );

    /* End sensor specific attributes
     */

    cat_cat_att(reverse_string((char *)VOLUME,arglist[0]), 0, arglist[0], cat );
    cat += strlen(cat);

    cii->vol_txt_size = strlen(cii->vol_txt_ptr);
    ddcat_write(cii, cii->vol_txt_ptr, cii->vol_txt_size
		     , DDCAT_NUVOL_WRITE);
    cii->rays_per_volume = 0;
}
/* c------------------------------------------------------------------------ */

void cat_sweep(struct dd_general_info *dgi,
			   struct catalog_info *cii, 
			   time_t current_time, 
			   int finish) 
{
    int i, n, nc, llen, max_llen=77;
    float e, f;
    double d;
    char arglist[MAX_ARG_COUNT][MAX_ARG_SIZE], *aa;
    char *arg1 = arglist[0];

    struct dds_structs *dds=dgi->dds;
    struct sweepinfo_d *swib=dds->swib;
    struct platform_i *asib=dds->asib;
    char *mark_char;
    char *cat;


    if( finish == YES ) {
	mark_char = cat = cii->swp_txt_ptr +cii->swp_txt_size;

	sprintf(arg1, "%.1f", cii->f_fixed_angle );
	cat_cat_att( (char *)"Fixed_angle", 1, arglist[0], cat );

	cat_cat_att( (char *)"Stop_date", 1, cat_date(cii->stop_time, arglist[0]), cat );
	cat_cat_att( (char *)"Stop_time", 1, cat_time(cii->stop_time, arglist[0]), cat );

	sprintf(arg1, "%.1f", cii->f_swp_stop_angle );
	cat_cat_att( (char *)"Stop_angle", 1, arglist[0], cat );

	f = cii->avg_rota_count > 1
	      ? cii->f_delta_angles/(float)(cii->avg_rota_count-1) : 0;
	sprintf(arg1, "%.2f", f );
	cat_cat_att( (char *)"Average_delta_rotation", 1, arglist[0], cat );

	f = cii->tot_cells > 0 ? (float)cii->num_good_cells/cii->tot_cells : 0;
	sprintf(arg1, "%.2f", f*100.);
	cat_cat_att( (char *)"Percent_good_cells", 1, arglist[0], cat );

	sprintf(arg1, "%d", cii->rays_per_volume );
	cat_cat_att( (char *)"Num_rays", 1, arglist[0], cat );

	sprintf(arg1, "%.2f", (float)cii->prev_MB_count);
	cat_cat_att( (char *)"MB_sofar", 1, arglist[0], cat );

	sprintf(arg1, "%d", cii->prev_rec_count-cii->vol_rec_mark);
	cat_cat_att( (char *)"Recs_sofar", 1, arglist[0], cat );

	if( cii->radar_type != DD_RADAR_TYPE_GROUND ) {
	    cat += strlen(cat);

	    sprintf(arg1, "%.3f", cii->f_stop_latitude );
	    cat_cat_att( (char *)"Stop_latitude", 1, arglist[0], cat );

	    sprintf(arg1, "%.3f", cii->f_stop_longitude );
	    cat_cat_att( (char *)"Stop_longitude", 1, arglist[0], cat );

	    sprintf(arg1, "%.3f", cii->f_stop_altitude);
	    cat_cat_att( (char *)"Stop_altitude_(km)", 1, arglist[0], cat );

	    sprintf(arg1, "%.1f", cii->f_stop_EW_wind );
	    cat_cat_att( (char *)"Stop_EW_wind_(m/s)", 1, arglist[0], cat );

	    sprintf(arg1, "%.1f", cii->f_stop_NS_wind );
	    cat_cat_att( (char *)"Stop_NS_wind_(m/s)", 1, arglist[0], cat );

	    if(cii->num_rays > 0) {
		
		e = cii->f_ns_velocity/(float)cii->num_rays;
		f = cii->f_ew_velocity/(float)cii->num_rays;
		sprintf(arg1, "%.0f", sqrt((double)(e*e+f*f)));
		cat_cat_att( (char *)"Average_ground_speed_(m/s)", 1, arglist[0], cat );
	    }
	    d = 360.+90. -DEGREES(atan2(cii->d_sin_heading
			      , cii->d_cos_heading));
	    sprintf(arg1, "%.0f", fmod(d,(double)360.));
	    cat_cat_att( (char *)"Average_heading_(deg)", 1, arglist[0], cat );
	    
	    d = DEGREES(atan2(cii->d_sin_drift, cii->d_cos_drift));
	    sprintf(arg1, "%.1f", d );
	    cat_cat_att( (char *)"Average_drift_(deg)", 1, arglist[0], cat );

	    d = DEGREES(atan2(cii->d_sin_pitch, cii->d_cos_pitch));
	    sprintf(arg1, "%.1f", d );
	    cat_cat_att( (char *)"Average_pitch_(deg)", 1, arglist[0], cat );

	    d = DEGREES(atan2(cii->d_sin_roll, cii->d_cos_roll));
	    sprintf(arg1, "%.1f", d);
	    cat_cat_att( (char *)"Average_roll_(deg)", 1, arglist[0], cat );
	}

	if(cmt->sizeof_comments) { /* dump out the comments */
	    n = strlen(mark_char);
	    cii->swp_txt_size += n;
	    mark_char = cat += strlen(cat);

	    strcat(cat, cmt->buf);
	    *cmt->buf = '\0';
	    cmt->sizeof_comments = 0;
	    cmt->at = cmt->buf;
	}
	cat_cat_att(reverse_string((char *)SCAN,arglist[0]), 0, arglist[0], cat );
	n = strlen(mark_char);
	cii->swp_txt_size += n;
	mark_char = cat += strlen(cat);

	ddcat_write(cii, cii->swp_txt_ptr, cii->swp_txt_size
		     , DDCAT_NORMAL_WRITE);
	return;
    }


    mark_char = cat = cii->swp_txt_ptr;
    cii->swp_txt_size = 0;
    *cii->swp_txt_ptr = 0;
    cat_cat_att((char *)SCAN, 0, arglist[0], cat );

    sprintf(arg1, "%d", swib->sweep_num );
    cat_cat_att( (char *)"Number", 1, arglist[0], cat );

    cat_cat_att( (char *)"Start_date", 1, cat_date(current_time, arglist[0]), cat );
    cat_cat_att( (char *)"Start_time", 1, cat_time(current_time, arglist[0]), cat );
# ifdef obsolete
    sprintf(arg1, "%.1f", swib->fixed_angle );
    cat_cat_att( "Fixed_angle", 1, arglist[0], cat );
# endif
    sprintf(arg1, "%.1f", swib->start_angle );
    cat_cat_att( (char *)"Start_angle", 1, arglist[0], cat );

    sprintf(arg1, "%.2f", dds->radd->eff_unamb_vel );
    cat_cat_att( (char *)"Unambiguous_velocity_(m/s)", 1, arglist[0], cat );

    if( cii->radar_type != DD_RADAR_TYPE_GROUND ) {
	sprintf(arg1, "%.3f", dd_latitude(dgi));
	cat_cat_att( (char *)"Start_latitude", 1, arglist[0], cat );

	sprintf(arg1, "%.3f", dd_longitude(dgi));
	cat_cat_att( (char *)"Start_longitude", 1, arglist[0], cat );

	sprintf(arg1, "%.3f", dd_altitude(dgi));
	cat_cat_att( (char *)"Start_altitude_(km)", 1, arglist[0], cat );

	sprintf(arg1, "%.3f", dd_altitude_agl(dgi));
	cat_cat_att( (char *)"Start_altitude_agl(km)", 1, arglist[0], cat );

	sprintf(arg1, "%.1f", dd_heading(dgi));
	cat_cat_att( (char *)"Start_heading_(deg)", 1, arglist[0], cat );

	e = asib->ew_velocity;
	f = asib->ns_velocity;
	sprintf(arg1, "%.0f", sqrt((double)(e*e+f*f)));
	cat_cat_att( (char *)"Start_ground_speed_(m/s)", 1, arglist[0], cat );

	sprintf(arg1, "%.1f", dd_roll(dgi) );
	cat_cat_att( (char *)"Start_roll", 1, arglist[0], cat );

	sprintf(arg1, "%.1f", dd_pitch(dgi) );
	cat_cat_att( (char *)"Start_pitch", 1, arglist[0], cat );

	sprintf(arg1, "%.1f", dd_drift(dgi) );
	cat_cat_att( (char *)"Start_drift", 1, arglist[0], cat );

	sprintf(arg1, "%.1f", asib->ew_horiz_wind );
	cat_cat_att( (char *)"EW_horiz_wind_(m/s)", 1, arglist[0], cat );

	sprintf(arg1, "%.1f", asib->ns_horiz_wind );
	cat_cat_att( (char *)"NS_horiz_wind_(m/s)", 1, arglist[0], cat );
    }
    cat += strlen(cat);
    cii->swp_txt_size = strlen(cii->swp_txt_ptr);
    cii->f_ew_velocity = cii->f_ns_velocity = 0;
    cii->d_cos_heading = cii->d_sin_heading = 0;
    cii->d_cos_drift = cii->d_sin_drift = 0;
    cii->d_cos_pitch = cii->d_sin_pitch = 0;
    cii->d_cos_roll = cii->d_sin_roll = 0;
    cii->f_last_rotation_angle = asib->rotation_angle;
    cii->f_delta_angles = 0;
    cii->avg_rota_count = cii->num_rays = 0;
    cii->flush_sweep_count++;
    cii->num_good_cells = cii->tot_cells = 0;
}
/* c------------------------------------------------------------------------ */

char *cat_time(time_t time, char *dst)
{
    DD_TIME dts;

    dts.time_stamp = time;
    d_unstamp_time(&dts);
    sprintf( dst, "%02d:%02d:%02d", dts.hour, dts.minute, dts.second);
    
    return(dst);
}
/* c------------------------------------------------------------------------ */

char *cat_date(time_t time, char *dst)
{
    DD_TIME dts;

    dts.time_stamp = time;
    d_unstamp_time(&dts);

    sprintf( dst, "%02d-%s-%d"
	    , dts.day
	    , Unix_months[dts.month-1]
	    , dts.year );

    return(dst);
}
/* c------------------------------------------------------------------------ */

void cat_cat_att (char *name, int num_args, char *arg, char *cat)
{
    strcat(cat,name );
    if( num_args == 1 ) {
	strcat(cat, ":" );
	strcat(cat, arg);
    }
    strcat(cat, "\n");
}

void cat_cat_att(const std::string &name, int num_args,
		 const std::string &arg, char *cat)
{
  strcat(cat, name.c_str());
  if (num_args == 1)
  {
    strcat(cat, ":");
    strcat(cat, arg.c_str());
  }
  strcat(cat, "\n");
}

void cat_cat_att(const std::string &name, int num_args,
		 const std::string &arg, std::string &cat)
{
  cat += name;
  if (num_args == 1)
  {
    cat += ":";
    cat += arg;
  }
  cat += "\n";
}

/* c------------------------------------------------------------------------ */

void 
cat_cat_att_args (char *name, int num_args, char *arglist[], char *cat)
{
    int i;

    strcat(cat,name );
    if( num_args > 0 )
	  strcat(cat, ":" );
    for(i=0; i < num_args; i++) {
	strcat(cat, arglist[i]);
	if( i < num_args-1 )
	      strcat(cat, "," );
    }
    strcat(cat, "\n");
}
/* c------------------------------------------------------------------------ */

char *
reverse_string (char *str, char *rev)
{
    int n=strlen(str);
    char *a=str+n, *b=rev;

    for(; n-- > 0; )
	  *b++ = *(--a);
    *b = '\0';
    return(rev);
}
/* c------------------------------------------------------------------------ */

char *
c_deblank (char *a, int n, char *clean)
{
    /* remove leading and trailing blanks from the
     * n character string
     */
    char *b;

    for(;*a == ' ' && n > 0; a++,n--); /* leading blanks */
    for(b=a+n; *(--b) == ' ' && n > 0; n--); /* trailing blanks */
    
    for(b=clean; n-- > 0;) {
	*b++ = *a++;
    }
    *b= '\0';
    return(clean);
}
/* c------------------------------------------------------------------------ */

void 
ddcat_close (int fid)
{
    if( !Catalog_flag  )
	  return;
    close( fid );
    return;
}
/* c------------------------------------------------------------------------ */
# define PMODE 0666

int 
ddcat_open (char *file_name)
{
    int i = -99;

    if( Catalog_flag )
	  i = creat( file_name, PMODE );
    printf( " file %s:%d \n", file_name, i );
    return(i);
}
/* c------------------------------------------------------------------------ */

void 
ddcat_write (struct catalog_info *cii, char *buf, int size, int func)
{
    int i, j, *blowit=0, mark, save_place, fid=cii->volumes_fid;

    if( !Catalog_flag )
	  return;

    if(func & DDCAT_NUVOL_REWRITE) {
	save_place = lseek( fid, 0L, SEEK_CUR);
	i = lseek(fid, cii->lseek_vol_offset, SEEK_SET);
    }
    else if(func & DDCAT_NUVOL_WRITE) {
	cii->lseek_vol_offset = lseek( fid, 0L, SEEK_CUR);
    }

    if((i = write( fid, buf, size )) < size ) {
	printf( "Problem in ddcat_write--err=%d\n", i );
	exit(1);
    }

    if(func & DDCAT_NUVOL_REWRITE) {
	i = lseek(fid, save_place, SEEK_SET);
    }

    if(func & DDCAT_FLUSH) {
	save_place = lseek( fid, 0L, SEEK_CUR);
	j = close(fid);
	fid = open(cii->volumes_file_name, READ_AND_WRITE_ACCESS);
	j = lseek(fid, 0L, SEEK_END);
	printf("Close & reopen %s : %d\n", cii->volumes_file_name, fid);
	j = lseek( fid, 0L, SEEK_CUR);
	mark = 0;
    }
    return;
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */
