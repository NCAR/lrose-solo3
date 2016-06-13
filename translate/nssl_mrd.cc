/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

#define NEW_ALLOC_SCHEME

#include <LittleEndian.hh>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <math.h>
#include <dd_files.h>
#include <dd_math.h>
#include <input_sweepfiles.h>
#include "dd_stats.h"
#include "swp_file_acc.hh"
#include "nssl_mrd.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "dorade_share.hh"
#include "input_limits.hh"
#include "dd_der_flds.hh"
#include "dd_crackers.hh"

static int mrd_num_radars=0;
static struct mrd_production *mrdp_list[MAX_SENSORS];
static int mrd_output=YES;
static float rcp_mrd_vel_res=7.5;
static float mrd_vel_bias=511.;
static float mrd_max_range=2.e22;
static int count=0;
static int contiguous=YES;
static char *mrd_dz_field=(char *)"DBZ DZ";
static char *mrd_ve_field=(char *)"VG VE";
static int options = 0;

static struct dd_input_sweepfiles_v3 *dis;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats;

/* c------------------------------------------------------------------------ */

void 
dd_mrd_conv (void)
{
    static int count=0;
    int ii, rn, flush=NO;
    struct unique_sweepfile_info_v3 *usi;
    struct dd_general_info *dgi;
    
    if((ii=dd_mrd_init()) == END_OF_TIME_SPAN ) {
	return;
    }
    else if(ii < 0)
	  return;

    usi = dis->usi_top;
    /*
     * for each radar, write out the UF file
     */
    for(rn=0; rn++ < dis->num_radars; usi=usi->next) {
	dgi = dd_window_dgi(usi->radar_num);
	dgi->radar_name = usi->radar_name;
	for(;;) {
	    if(!(count++ % 500))
		  dgi_interest_really(dgi, NO, (char *)"", (char *)"", dgi->dds->swib);
	    produce_nssl_mrd(dgi, flush);
	    
	    dgi->new_vol = NO;
	    dgi->new_sweep = NO;
	    if(ddswp_next_ray_v3(usi) == END_OF_TIME_SPAN) {
		break;
	    }
# ifdef obsolete
	    if(usi->swp_count >= usi->num_swps)
		  break;
# endif
	}
    }
    flush = YES;
    produce_nssl_mrd(dgi, flush);

    dd_stats->rec_count = dis->rec_count;
    dd_stats->ray_count = dis->ray_count;
    dd_stats->sweep_count = dis->sweep_count;
    dd_stats->vol_count = dis->vol_count;
    dd_stats->file_count = dis->file_count;
    dd_stats->MB = dis->MB;
}
/* c------------------------------------------------------------------------ */

int 
dd_mrd_init (void)
{
    int ii, ok;
    struct unique_sweepfile_info_v3 *usi;

    /* do all the necessary initialization including
     * reading in the first ray
     */
    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();
    dis = dd_setup_sweepfile_structs_v3(&ok);
    dis->print_swp_info = YES;

    for(ii=0,usi=dis->usi_top; ii++ < dis->num_radars; usi=usi->next) {
	/* now try and find files for each radar by
	 * reading the first record from each file
	 */
	if(ddswp_next_ray_v3(usi) == END_OF_TIME_SPAN) {
	    printf("Problems reading first record of %s/%s\n",
		   usi->directory.c_str(), usi->filename.c_str());
	    ok = NO;
	}
    }
    if(!ok)
	  return(-1);
    return(0);
}
/* c------------------------------------------------------------------------ */

void 
produce_nssl_mrd (struct dd_general_info *dgi, int flush)
{
    short *ss, *tt, *dd;

    struct dds_structs *dds=dgi->dds;
    struct radar_d *radd=dds->radd;
    struct cell_d *celv=dds->celvc;
    struct ray_i *ryib=dds->ryib;
    struct platform_i *asib=dds->asib;
    struct correction_d *cfac=dds->cfac;
    struct radar_angles *ra=dds->ra;
    DD_TIME *dts=dds->dts;

    struct mrd_production *mrdp;
    struct mrd_record *this_record, *that, *last;  //Jul 26, 2011 *this
    struct mrd_head *mrdh;
    struct mrd_head2 *mrdh2;
    char *a, *id, *sfx, *dir, str[256];
    char name[12];
    short bad;
    double d, ns, ew;
    float x, y, r, gs, rcp_scale, bias, maxr;
    int g, i, j, k, l, m, n, mark, ng, bc=0, gc=0, fgg=0;
    register short val;
    int new_vol=NO, new_sweep=NO;


    if(!mrd_output)
	  return;

    count++;
    if(count >= 3) {
	mark = 0;
    }

    if(!mrd_num_radars) {
	/*
	 * first time through
	 */
	mrd_output = NO;

	if(a=get_tagged_string("OUTPUT_FLAGS")) {
	    if(strstr(a,"NSSL_MRD")) {
		mrd_output = YES;
	    }
	}
	if(!mrd_output)
	      return;

	if(a=get_tagged_string("OPTIONS")) {
	    if( strstr( a, "USE_PALT" )) {
		options |= USE_PALT;
	    }
	}
	if(a=get_tagged_string("MRD_DZ_FIELD")) {
	    mrd_dz_field = a;
	}
	if(a=get_tagged_string("MRD_VE_FIELD")) {
	    mrd_ve_field = a;
	}
	if(a=get_tagged_string("MRD_MAX_RANGE")) {
	    d = atof(a);
	    if(d > 0)
		  mrd_max_range = d*1000.; /* meters! */
	}

	for(i=0; i < MAX_SENSORS; i++)
	      mrdp_list[i] = NULL;
    }

    if(mrdp_list[dgi->radar_num] == NULL) {
	/*
	 * create a control struct and record que for this radar
	 */
	mrd_num_radars++;
	mrdp_list[dgi->radar_num] = mrdp = (struct mrd_production *)
	      malloc(sizeof(struct mrd_production));
	mrdp->prev_vol_num = -1;
	mrdp->file_size = mrdp->rec_count = mrdp->vol_count = 0;
	if(!(dir=get_tagged_string("DORADE_DIR"))) {
	}
	slash_path(mrdp->file_name, dir);
	for(i=0; i < 3; i++) {
	    this_record = (struct mrd_record *)malloc(sizeof(struct mrd_record));
	    if(!i) {
		mrdp->rec_que = this_record;
	    }
	    else {
		last->next = this_record;
		this_record->last = last;
	    }
	    this_record->next = mrdp->rec_que;
	    mrdp->rec_que->last = this_record;
	    last = this_record;
	    this_record->mrdh = (struct mrd_head *)malloc(sizeof(struct mrd_head));
	    this_record->mrdh2 = (struct mrd_head2 *)malloc(sizeof(struct mrd_head2));
	}
    }
    mrdp = mrdp_list[dgi->radar_num];
    /*
    if(dgi->vol_num != mrdp->prev_vol_num) {
     */
    if(dgi->new_vol) {
	new_vol = new_sweep = YES;
    }
    /*
    else if(dgi->sweep_num != mrdp->prev_sweep_num) {
     */
    else if(dgi->new_sweep) {
	new_sweep = YES;
    }

    if((flush || new_sweep) && mrdp->rec_count > 0) {
	mrdp->rec_que->mrdh->sweep_num = MRD_SWEEP_MARK;
	/* rewrites the last record with the sweep mark */
	mrdp_write(mrdp,YES);
    }
    if(flush) {
	close(mrdp->fid);
	return;
    }
    mrdp->rec_que = mrdp->rec_que->next;
    this_record = mrdp->rec_que;
    mrdh = this_record->mrdh;
    mrdh2 = this_record->mrdh2;

    mrdp->prev_vol_num = dgi->vol_num;
    mrdp->prev_sweep_num = dgi->sweep_num;
    d_unstamp_time(dts);

    if(new_vol && (mrdp->vol_count == 0 || !contiguous)) {
	if(mrdp->rec_count > 0 ) {
	    close(mrdp->fid);
	}
	mrdp->file_size = mrdp->sweep_num = mrdp->rec_count = 0;
	mrdp->vol_count++;
	str_terminate(name, dgi->radar_name.c_str(), 8);
	dd_file_name("mrd", (int32_t)dgi->time, name, 0
		     , mrdp->file_name +strlen(mrdp->file_name));
	strcat(mrdp->file_name, ".tape");
	printf("Opening MRD file %s", mrdp->file_name);
	if((mrdp->fid = creat(mrdp->file_name, PMODE)) < 0 ) {
	    printf("\nUnable to open %s %d\n", mrdp->file_name
		   , mrdp->fid);
	    exit(1);
	}
	printf(" fid: %d\n", mrdp->fid);
	/*
	 * set up gate lookup tables
	 */
	mrdp->r1 = celv->dist_cells[0];
	mrdp->gs = celv->dist_cells[1] -mrdp->r1;

	if(mrd_max_range < (r = celv->dist_cells[celv->number_cells-1]))
	      r = mrd_max_range;
# ifndef notyet
	mrdp->max_gates = (int)((r - mrdp->r1)/mrdp->gs + 1.5);
	dd_uniform_cell_spacing
	      (celv->dist_cells, celv->number_cells
	       , mrdp->gs, mrdp->dd_gate_lut, mrdp->r1, mrdp->max_gates);
# else
	n = (r - mrdp->r1)/mrdp->gs +.5;

	for(r=mrdp->r1,g=0,i=0; i < n-1; i++,r+=mrdp->gs) {
	    x = fabs((double)(r-celv->dist_cells[g]));
	    y = .5*fabs((double)(celv->dist_cells[g+1] -celv->dist_cells[g]));
	    
	    if(x > y) {		/* this gate is closer to the next cell */
		g++;
	    }
	    mrdp->dd_gate_lut[i] = g;
	}
	mrdp->dd_gate_lut[i] = g;
	mrdp->max_gates = i + 1;
# endif
	if (dgi->radar_name.find("ELDR") != std::string::npos) {
	    id = (char *)"EL"; sfx = (char *)"E";
	}
	else if (dgi->radar_name.find("42") != std::string::npos) {
	    id = (char *)"42"; sfx = (char *)"H";
	}
	else if (dgi->radar_name.find("43") != std::string::npos) {
	    id = (char *)"43"; sfx = (char *)"I";
	}
	else {
	    id = (char *)"UK"; sfx = (char *)"U";
	}
	sprintf(str, "%02d%02d%02d%s ", dts->year % 100
		, dts->month, dts->day, sfx);
	for(that=this_record,i=0; i < 3; i++,that=that->next) {
	    strncpy(that->mrdh->aircraft_id, id, 2);
	    dd_blank_fill(str, 8, that->mrdh->flight_number);
	    dd_blank_fill(dgi->dds->vold->proj_name, 12
			  , that->mrdh->storm_name);
	}
	/*
	 * find the velocity and reflectivity to be used
	 */
	mrdp->vpn = mrd_find_param(dgi, FIND_VEL);
	mrdp->rpn = mrd_find_param(dgi, FIND_REFL);

    }

    mrdh2->year = dts->year;
    mrdh2->month = dts->month;
    mrdh2->day = dts->day;
    mrdh->word_1 = mrdh2->hour = dts->hour;
    mrdh->word_2 = mrdh2->minute = dts->minute;
    mrdh->word_3 = mrdh2->second = dts->second;

    mrdh->raw_rot_ang_x10 = (short)S10(asib->rotation_angle+cfac->rot_angle_corr);

    mrdh->lat_deg = (short) asib->latitude;
    d = (asib->latitude -mrdh->lat_deg)*60.;
    mrdh->lat_min = (short)d;
    d = (d -mrdh->lat_min)*60.;
    mrdh->lat_sec_x10 = (short)S10(d);

    mrdh->lon_deg = (short) asib->longitude;
    d = (asib->longitude -mrdh->lon_deg)*60.;
    mrdh->lon_min = (short)d;
    d = (d -mrdh->lon_min)*60.;
    mrdh->lon_sec_x10 = (short) S10(d);

    d = dd_altitude( dgi );
    mrdh->altitude = options & USE_PALT ? (short)S1000(dd_altitude( dgi )) :
	(short)S1000(asib->altitude_agl);
    mrdh->roll_x10 = (short)S10(asib->roll +cfac->roll_corr);
    mrdh->heading_x10 = (short)S10(asib->heading +cfac->heading_corr);
    mrdh->drift_x10 = (short)S10(asib->drift_angle +cfac->drift_corr);
    mrdh->pitch_x10 = (short)S10(asib->pitch +cfac->pitch_corr);
    d = asib->tilt +cfac->tilt_corr +180.;
    mrdh->raw_tilt_x10 = (short)S10(fmod(d,(double)360.));
    mrdh->nyq_vel_x10 = (short)S10(radd->eff_unamb_vel);
    mrdh->julian_date = ryib->julian_day;

    mrdh->azimuth_samples = dds->parm[mrdp->vpn]->num_samples;
    mrdh->gate_length = (short) mrdp->gs;
    d = sqrt((double)(asib->ew_velocity*asib->ew_velocity
		      +asib->ns_velocity*asib->ns_velocity));
    mrdh->ground_speed_x64 = (short)S64(d);
    mrdh->vert_airspeed_x64 = (short)S64(asib->vert_velocity);
    ew = asib->ew_horiz_wind != -999 ? asib->ew_horiz_wind : 0;
    ns = asib->ns_horiz_wind != -999 ? asib->ns_horiz_wind : 0;
    d = atan2(ns, ew);
    mrdh->wind_dir_x10 = (short)S10(fmod((double)270.-DEGREES(d), (double)360.));
    mrdh->nav_flag = dgi->nav_system;
    mrdh->wind_speed_x10 = (short)S10(sqrt(ew*ew+ns*ns));

    mrdh->noise_threshold = -9990;
    d = (DEGREES(ra->tilt));
    mrdh->corrected_tilt_x10 = (short)S10(fmod(d,(double)360.));
    mrdh->gspd_vel_corr_x10 = (short)(-S10(dd_ac_vel(dgi)));
    if(new_sweep)
	  mrdp->sweep_num++;
    mrdh->sweep_num = mrdp->sweep_num;
    mrdh->num_good_gates = mrdh->max_gates =
	  ng = mrdp->max_gates;
    mrdh->tilt_corr_flag = YES;
    mrdh->altitude_flag = ALTITUDE_RA;

# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dgi->dds->qdat_ptrs[mrdp->vpn];
# else
    ss = (short *)((char *)dds->rdat[mrdp->vpn] +sizeof(struct paramdata_d));
# endif
    bad = dds->parm[mrdp->vpn]->bad_data;

    mrdh->range_delay = (short) celv->dist_cells[0];

    rcp_scale = 1./dds->parm[mrdp->vpn]->parameter_scale;
    bias = dds->parm[mrdp->vpn]->parameter_bias;
    bc = 0;

    for(dd=(short *)this_record->data,i=0; i < ng ; i++) { /* velocities */
	val = *(ss+mrdp->dd_gate_lut[i]);
	if(val == bad) {
	    bc++;		/* yields a consecutive bad count */
	    *dd++ = -1;
	}
	else {
	    bc = 0;		/* zero out consecutive bad count */
	    m = (int)(DD_UNSCALE((float)(-val), rcp_scale, bias)*rcp_mrd_vel_res
		  + mrd_vel_bias+.5);
	    /* mrd has the opposite sign convention for velocities
	     */
	    if(m & (~MASK10)) {	/* m < 0 or m > 1023 */
		if(m < 0)
		      m = 0;
		else
		      m = 1023;
	    }
	    *dd++ = m & MASK10;
	}
    }
    if(bc == ng) {		/* all data has been thresholded out;
				 * put in at least one gate */
	*dd = -1;
	mrdh->num_good_gates = 1;
    }
    else {
	mrdh->num_good_gates -= bc;
    }
    rcp_scale = 1./dds->parm[mrdp->rpn]->parameter_scale;
    bias = dds->parm[mrdp->rpn]->parameter_bias;
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dgi->dds->qdat_ptrs[mrdp->rpn];
# else
    ss = (short *)((char *)dds->rdat[mrdp->rpn] +sizeof(struct paramdata_d));
# endif
    ng = mrdh->num_good_gates;

    for(dd=(short *)this_record->data,i=0; i < ng; i++,dd++) {
	if(*dd != -1) {
	    if((m = (int)(DD_UNSCALE(*(ss+mrdp->dd_gate_lut[i])
			       , rcp_scale, bias) +.5)) > 0) {
		*dd |= (m << 10) & (~MASK10);
	    }
	}
    }
    mrdp_write(mrdp, NO);
}
/* c------------------------------------------------------------------------ */

void 
mrdp_write (struct mrd_production *mrdp, int header_only)
{
    struct mrd_record *this_mrd=mrdp->rec_que;  //Jul 26, 2011 this keyword issue
    int i, n;
    char *aa;
    struct mrd_head *xmrdh;
    static char *swap_buf=NULL, *place_holder;
    static int count=0, trip_count=123;

    count++;
    if(count >= trip_count) {
       i = 0;
    }
    if(hostIsLittleEndian()) {
       if(!swap_buf) {
	  swap_buf = (char *)malloc(8192);
	  memset(swap_buf, 0, 8192);
	  xmrdh = (struct mrd_head *)swap_buf;
	  place_holder = (char *)&xmrdh->place_holder;
       }
//       nssl_crack_head(this_mrd->mrdh, swap_buf, (int)0);  //JUl 26, 2011
//       swack_long(this_mrd->mrdh2, place_holder, 6);  //JUl 26, 2011
       nssl_crack_head((char *)this_mrd->mrdh, swap_buf, (int)0);  //JUl 26, 2011
       swack_long((char *)this_mrd->mrdh2, place_holder, 6);  //JUl 26, 2011
       aa = swap_buf;
    }
    else {
       /* move date to facilitate a single write */
       memcpy((char *)this_mrd->mrdh->place_holder, (char *)this_mrd->mrdh2
	      , 6*sizeof(int32_t));
       aa = (char *)this_mrd->mrdh;
    }
    if((n=gp_write(mrdp->fid, aa, sizeof(struct mrd_head)
		   , FB_IO)) < 1) {
	printf("Trouble on first (nssl/mrd) write  fid: %d stat: %d\n"
	       , mrdp->fid, n);
	exit(1);
    }
    if(header_only) {
	return;
    }
    mrdp->rec_count++;
    mrdp->file_size += n;

    if(hostIsLittleEndian()) {
       swack_short(this_mrd->data, swap_buf, (int)this_mrd->mrdh->num_good_gates);
       aa = swap_buf;
    }
    else {
       aa = this_mrd->data;
    }
    if((n=gp_write(mrdp->fid, aa
		   , this_mrd->mrdh->num_good_gates*sizeof(short), FB_IO)) < 1) {
	printf("Trouble on second (nssl/mrd) write  fid: %d stat: %d\n"
	       , mrdp->fid, n);
	exit(1);
    }
    mrdp->rec_count++;
    mrdp->file_size += n;
    return;
}
/* c------------------------------------------------------------------------ */

int 
mrd_find_param (struct dd_general_info *dgi, int find)
{
    struct dds_structs *dds=dgi->dds;
    char pname[12], string_space[256], *str_ptrs[32];
    int ii, jj, nt;

    if(find == FIND_VEL) {
	strcpy(string_space, mrd_ve_field);
    }
    else if(find == FIND_REFL) {
	strcpy(string_space, mrd_dz_field);
    }
    else {
	return(-1);
    }
    nt = dd_tokens(string_space, str_ptrs);
    
    for(ii=0; ii < dgi->num_parms; ii++) {
	str_terminate(pname, dds->parm[ii]->parameter_name, 8);
	
	for(jj=0; jj < nt; jj++) {
	    if(!strcmp(pname, str_ptrs[jj])) {
		return(ii);
	    }
	}
    }
    return(-1);
}
/* c------------------------------------------------------------------------ */



