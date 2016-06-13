#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NEW_ALLOC_SCHEME

/*
 * This file contains the following routines
 * 
 * dd_derived_fields
 * 
 * dd_ac_vel
 * dd_BB_unfold
 * dd_denotch
 * dd_despeckle
 * 
 * dd_fix_vortex_vels
 * dd_gp_threshold
 * dd_legal_pname
 * dd_lidar_db
 * dd_new_param
 * dd_nix_air_motion
 * dd_pct_stats
 * dd_thr_parms
 * 
 * 
 * dder_lidar_opts
 * 
 * 
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <time_dependent_fixes.h>
#include <dd_math.h>
#include "dd_der_flds.hh"
#include "dorade_share.hh"
#include "dda_common.hh"
#include "ddb_common.hh"

# ifndef A_SPECKLE
# define            A_SPECKLE 3
# define      FIRST_GOOD_GATE 2
# define    NCP_THRESHOLD_VAL 0.333
# define    VUF_MIN_BAD_COUNT 3
# define    VUF_NON_SHEAR_RUN 3
# define        VUF_NOTCH_MAX 6.3
# define      VUF_NOTCH_SHEAR 11.1
# define            VUFQ_SIZE 4
# endif

# define	THRESHOLD_LOW 1
# define       THRESHOLD_HIGH 2
# define     THRESHOLD_INSIDE 3
# define    THRESHOLD_OUTSIDE 4

# define  DONT_CENTER_ON_ZERO 0
# define       CENTER_ON_ZERO 1
# define         USE_PREV_RAY 2
# define         USE_THIS_RAY 3

# ifndef UPPER
# define UPPER(c) ((c) >= 'A' && (c) <= 'Z')
# define LOWER(c) ((c) >= 'a' && (c) <= 'z')
# endif
# define     NCP_HI_THRESHOLD 999.0

struct der_pair {
    struct der_pair * next;
    char src_name[16];
    char dst_name[16];
    int num_ders;
};



static struct time_dependent_fixes *tdf=NULL;
static int fx_count = 0, rdb_ndx=-1, gdb_ndx=-1;
static float bigL=29.8;		/* distance between INS and antenna */
static float level_1=.25;
static float rota=31., rotb=34.;
static float f_max_wind=999.;
static int first=YES, do_it=YES, Include_dH_dP=NO;
static int dont_use_wind_info=NO;
static int min_bad_count=VUF_MIN_BAD_COUNT;
static short *sscr=NULL;
static int a_speckle=A_SPECKLE;
static int fgg=0;
static float param_bias = 0;
static float ncp_threshold=NCP_THRESHOLD_VAL;
static float ncp_hi_threshold=NCP_HI_THRESHOLD;
static float sw_threshold=5.0;
static float pwr_threshold=-327.68;
static float snr_threshold=7.;
static float rhv_threshold=0.9;
static float f_notch_max = VUF_NOTCH_MAX;
static float f_notch_shear = VUF_NOTCH_SHEAR;
static int sidelobe_ring=0;
static int hail_test = YES;
static int hail_hits = 0;
static int32_t D950502;
static int32_t D970401;
static DD_TIME dts;


static struct derived_fields *derfs;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats=NULL;
static struct ve_que_val *vqv;

struct fix_vortex_vels {
    double rcp_half_nyqL;
    double d_round;
    int vConsts[16];
    int level_bias;
};

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

void 
dd_derived_fields (DGI_PTR dgi)
{
    DDS_PTR dds=dgi->dds;
    int dn, fns, fnd, ii, n, nt, mark, kill_count;
    char *a, *aa, *b, *z, *sname, *dname;
    struct radar_angles *ra=dds->ra;
    struct platform_i *asib=dds->asib;
    struct prev_rays *prs=dgi->ray_que;
    double d, vert;
    double d2, dH=0, dP=0, dt, cosP, sinP, cosH, sinH;
    double cosPHI, cosLAM, sinPHI, sinLAM;
    float ac_vel;
    struct ve_que_val *this_val, *last;  //Jul 26, 2011 *this
    struct time_dependent_fixes *fx;
    struct lidar_parameters *lps;

    struct der_pair * dplist;

    static float f_fold_shear=20., f_vals[2];
    static int qsize=3, count=0, trip=200, num_dst=0;
    static char *srs[12], *dst[12];
    static char ncp_name[12], sw_name[12], pwr_name[12], snr_name[12];
    static char string_space[256];
    static char *str_ptrs[32];
    /* c...mark */

    count++;
    if(count >= trip) {
	mark = 0;
    }

    if(!do_it)
	  return;

    if(first) {
	first = NO;
	do_it = NO;
	if(dd_catalog_only_flag()) {
	    return;
	}
	derfs = (struct derived_fields *)malloc(sizeof(struct derived_fields));
	memset(derfs, 0, sizeof(struct derived_fields));
	dd_clear_dts( &dts );
	dts.year = 1995;
	dts.month = 5;
	dts.day = 2;
	D950502 = (int32_t)d_time_stamp( &dts );
	dd_clear_dts( &dts );
	dts.year = 1997;
	dts.month = 4;
	dts.day = 1;
	D970401 = (int32_t)d_time_stamp( &dts );
	


	if(a = get_tagged_string("DERIVED_FIELDS")) {
	    if( setup_der_alts( a ) < 1 )
		{ return; }

# ifdef obsolete
	    str_terminate(string_space, a, strlen(a));
	    nt = dd_tokens(string_space, str_ptrs);

	    for(ii=0; ii < nt; ii += 3) {
		b = str_ptrs[ii+1];
		srs[num_dst] = str_ptrs[ii]; /* source name! */
		dst[num_dst] = str_ptrs[ii+2]; /* derived field name */

		if(!strncmp(dst[num_dst], "RDB", 3) ||
		   !strncmp(dst[num_dst], "GDB", 3)) {

		    lps = derfs->lps[num_dst] = (struct lidar_parameters *)
			  malloc(sizeof(struct lidar_parameters));
		    memset(lps, 0, sizeof(struct lidar_parameters));
		    lps->baseline = 0;
		    lps->counts_per_db = 100.;

		    if(!strncmp(dst[num_dst], "RDB", 3)) {
			rdb_ndx = num_dst;
			lps->end_range = KM_TO_M(3.);
		    }
		    if(!strncmp(dst[num_dst], "GDB", 3)) {
			gdb_ndx = num_dst;
			lps->end_range = KM_TO_M(5.);
		    }
		}
		num_dst++;
	    }
	    if(!num_dst)
		  return;
# endif
	}
	else
	      return;

	do_it = YES;
	strcpy(ncp_name, "NC");
	strcpy(sw_name, "SW");
	strcpy(pwr_name, "DZ");
	strcpy(snr_name, "SNR");

	if(tdf = dd_time_dependent_fixes(&fx_count)) {
	    for(fx = tdf; fx ; fx = fx->next) {
		/*
		 * remove non-applicable fixes
		 */
		switch(fx->typeof_fix) {
		    
		case RED_BASELINE:
		case RED_COUNTS_PER_DB:
		case RED_END_RANGE:
		    if(!derfs->RDB_lps) fx->use_it = NO;
		    break;

		case GREEN_BASELINE:
		case GREEN_COUNTS_PER_DB:
		case GREEN_END_RANGE:
		    if(!derfs->GDB_lps) fx->use_it = NO;
		    break;
		default:
		    break;
		}
	    }
	    tdf = dd_clean_tdfs(&fx_count);
	}

	if((a=get_tagged_string("LIDAR_OPTIONS"))) {
	    dder_lidar_opts(derfs, a, dst, num_dst);
	}
	if((a=get_tagged_string("UNFOLD_QUE_SIZE"))) {
	    if((ii=atoi(a)) >= 0)
		  qsize = ii;
	}
	printf("Que size: %d\n", qsize);
	if((a=get_tagged_string("MAX_WIND"))) {
	    f_max_wind = atof(a);
	}
	printf("Max wind: %.2f\n", f_max_wind);
	if((a=get_tagged_string("NO_WIND_INFO"))) {
	   dont_use_wind_info = strstr(a, "PREV") /* "USE_PREV_RAY" */
	     ? USE_PREV_RAY : USE_THIS_RAY;
	   aa = const_cast<char *>(dont_use_wind_info == USE_PREV_RAY
	     ? "Using first good gate from previous ray."
	       : "Using first good gate in ray.");  //Jul 26, 2011
	    printf("Not using wind information to initialize\n%s\n");
	}
	if((a=get_tagged_string("INCLUDE_DH_DP"))) {
	    Include_dH_dP = YES;
	    printf("Include dH/dt and dP/dt\n");
	}
	if(a=get_tagged_string("NCP_THRESHOLD_VAL")) {
	    f_vals[0] = ncp_threshold;
	    f_vals[1] = ncp_hi_threshold;
	    nt = dd_thr_parms(a, ncp_name, f_vals);
	    ncp_threshold = f_vals[0];
	    ncp_hi_threshold = f_vals[1];
	}
	printf( "NCP threshold field: %s  vals: %f <-> %f\n"
	       , ncp_name, ncp_threshold, ncp_hi_threshold);

	if(a=get_tagged_string("BIAS")) {
	    d = atof( a );
	    if( d )
	      param_bias = d;
	}
	printf( "SW threshold field: %s  val: %f\n"
	       , sw_name, sw_threshold);

	if(a=get_tagged_string("SW_THRESHOLD_VAL")) {
	    dd_thr_parms(a, sw_name, &sw_threshold);
	}
	printf( "SW threshold field: %s  val: %f\n"
	       , sw_name, sw_threshold);

	if(a=get_tagged_string("PWR_THRESHOLD_VAL")) {
	    dd_thr_parms(a, pwr_name, &pwr_threshold);
	}
	printf( "Power threshold field: %s  val: %f\n"
	       , pwr_name, pwr_threshold);

	if(a=get_tagged_string("SNR_THRESHOLD_VAL")) {
	    dd_thr_parms(a, snr_name, &snr_threshold);
	}
	printf( "SNR threshold field: %s  val: %f\n"
	       , snr_name, snr_threshold);

	if((a=get_tagged_string("A_SPECKLE"))) {
	    if((ii=atoi(a)) > 0)
		  a_speckle = ii;
	}
	printf("A speckle is %d cells or less\n", a_speckle);

	if((a=get_tagged_string("SIDELOBE_RING"))) {
	    if((ii=atoi(a)) > 0)
		 sidelobe_ring = ii;
	    printf("Sidelobe surface ring gate count: %d\n"
		   , sidelobe_ring);
	}
	if((a=get_tagged_string("FIRST_GOOD_GATE"))) {
	    if((ii=atoi(a)) > 0)
		  fgg = ii;
	}
	printf("First good gate is %d\n", fgg);
	if((a=get_tagged_string("MIN_BAD_COUNT"))) {
	    if((ii=atoi(a)) >= 0)
		  min_bad_count = ii;
	}
	printf("Min bad count: %d\n", min_bad_count);
	if((a=get_tagged_string("MAX_NOTCH_VELOCITY"))) {
	    f_notch_max = atof(a);
	}
	printf( "Notch max: %f\n", f_notch_max);
	if((a=get_tagged_string("NOTCH_SHEAR"))) {
	    f_notch_shear = atof(a);
	}
	printf( "Notch shear: %f\n", f_notch_shear);

	/* establis a scratch field */
	sscr = (short *)malloc(MAXCVGATES*sizeof(short));

	/* set up the que for averaging */
	for(ii=0; ii < qsize; ii++) {
	    this_val = (struct ve_que_val *)malloc(sizeof(struct ve_que_val));
	    if(!ii)
		  vqv = this_val;
	    else {
		this_val->last = last;
		last->next = this_val;
	    }
	    last = this_val;
	    this_val->next = vqv;
	    vqv->last = this_val;
	}
	    
	difs = dd_return_difs_ptr();
	dd_stats = dd_return_stats_ptr();
    }
    /* end of initialization
     */


    ac_vel = dd_ac_vel(dgi);
# ifdef obsolete
    vert =  asib->vert_velocity != -999 ? asib->vert_velocity : 0;
    d = sqrt((double)(SQ(asib->ew_velocity) + SQ(asib->ns_velocity)));
    d += dds->cfac->ew_gndspd_corr; /* klooge to correct ground speed */
    ac_vel = d*sin(ra->tilt) + vert*sin(ra->elevation);
# endif
    if((Include_dH_dP) &&
       !dgi->new_sweep && dds->swib->num_rays > 3) {
	dH = RADIANS(prs->dH +prs->last->dH +prs->last->last->dH);
	dP = RADIANS(prs->dP +prs->last->dP +prs->last->last->dP);
	dt = prs->time - prs->last->last->last->time;
	cosP = cos((double)RADIANS(asib->pitch));
	sinP = sin((double)RADIANS(asib->pitch));
	cosH = cos((double)RADIANS(asib->heading));
	sinH = sin((double)RADIANS(asib->heading));
	cosLAM = cos((double)(PIOVR2-ra->azimuth));
	sinLAM = sin((double)(PIOVR2-ra->azimuth));
	cosPHI = cos((double)ra->elevation);
	sinPHI = sin((double)ra->elevation);

	d2 = bigL*
	      ((1.+cosP)*(cosPHI*cosLAM*cosH -cosPHI*sinLAM*sinH)*
	       (RADIANS(asib->heading_change))
	       -(sinP*(cosPHI*cosLAM*sinH +cosPHI*sinLAM*cosH)
		 -sinPHI*cosP)*(dP/dt));
	ac_vel -= d2;
    }
    dgi->ray_que->vel_corr = ac_vel;


    if(fx_count) {
	kill_count = 0;
	for(fx = tdf; fx ; fx = fx->next) {
	    if(dgi->time < fx->start_time)
		  continue;
	    if(dgi->time < fx->stop_time) {

		switch(fx->typeof_fix) {
		    
		case RED_BASELINE:
		    derfs->RDB_lps->baseline = fx->baseline;
		    fx->use_it = NO;
		    kill_count++;
		    break;
		case GREEN_BASELINE:
		    derfs->GDB_lps->baseline = fx->baseline;
		    fx->use_it = NO;
		    kill_count++;
		    break;
		case RED_COUNTS_PER_DB:
		    derfs->RDB_lps->counts_per_db = fx->counts_per_db;
		    fx->use_it = NO;
		    kill_count++;
		    break;
		case GREEN_COUNTS_PER_DB:
		    derfs->GDB_lps->counts_per_db = fx->counts_per_db;
		    fx->use_it = NO;
		    kill_count++;
		    break;
		case RED_END_RANGE:
		    derfs->RDB_lps->end_range = fx->end_range;
		    fx->use_it = NO;
		    kill_count++;
		    break;
		case GREEN_END_RANGE:
		    derfs->GDB_lps->end_range = fx->end_range;
		    fx->use_it = NO;
		    kill_count++;
		    break;

		default:
		    break;
		} /* end switch */
		
	    } 
	    else {		/* beyond the time span */
		fx->use_it = NO;
		kill_count++;
	    }
	}
	if(kill_count) {
	    tdf = dd_clean_tdfs(&fx_count);
	}
    }
	

    dgi->num_parms = 0;
    for(ii=0; ii < MAX_PARMS; ii++) {
       if(dgi->dds->field_present[ii]) { dgi->num_parms++;  }
    }
    /* which sequence of derived fields? */

    if( derfs->alt_ders[DUAL_PRF_ALT] && dd_find_field(dgi, (char *)"VS") >= 0 &&
	dgi->dds->radd->num_ipps_trans == 2 )
	{ dplist = derfs->alt_ders[DUAL_PRF_ALT]; }
    else
	{ dplist = derfs->alt_ders[DEFAULT_ALT]; }

    /*
     * loop through the derived fields
     */

    for(; dplist ; dplist = dplist->next ) {

	sname = dplist->src_name;
	dname = dplist->dst_name;

	fnd = fns = -1;
	a = b = NULL;
	/* find the source field and destination fields
	 */
	if(!strcmp(dname, "ZAP")) {
	    if((fns = dd_find_field(dgi, sname)) >= 0) {
		dgi->dds->field_present[fns] = NO;
		--dgi->num_parms;
	    }
	}
	else if(!strcmp(dname, "BIAS")) {
	    if((fns = dd_find_field(dgi, sname)) >= 0) {
	      dd_add_bias(dgi, fns, param_bias);
	    }
	}
	else if((fnd = dd_new_param(dgi, sname, dname)) < 0) {
	    /*
	    printf("Unable to form relationship between fields: %s\n"
		   , str);
	    do_it = NO;
	    exit(1);
	     */
	    return;
	}
	else if(!strcmp(dname, "VH")) {
	    /* generate velocity field with air motion removed
	     * but don't force the Nyquist interval to be
	     * centered at zero
	     */
	    dd_nix_air_motion(dgi, fnd, ac_vel, DONT_CENTER_ON_ZERO);
	}
	else if(!strcmp(dname, "VG")) {
	    /* generate velocity field with air motion removed
	     */
	    dd_nix_air_motion(dgi, fnd, ac_vel, CENTER_ON_ZERO);
	}
	else if(!strcmp(dname, "VQ") || !strcmp(dname, "VQB1")
		|| !strcmp(dname, "VQC1")) {
	    /* fix vortex and fastex velocities dual prt
	     */
	    dd_fix_vortex_vels(dgi, fnd, dname);
	}
	else if(!strcmp(dname, "VRB1") || !strcmp(dname, "VRC1")) {
	    /* fix vortex and fastex velocities single prt
	     */
	    dd_single_prf_vortex_vels(dgi, fnd, dname);
	}
	else if(!strcmp(dname, "VU")) {
	    /* generate unfolded velocity field with air motion removed
	     */
	    dd_BB_unfold(dgi, fnd, ac_vel, qsize, f_fold_shear, vqv);
	}
	else if(!strcmp(dname, "VD")) {
	    /* threshold on ncp and
	     * remove false zeroes from the data
	     */
	    dd_gp_threshold(dgi, fnd, ncp_name, ncp_threshold, 0.0
			    , (int)THRESHOLD_LOW);
	    dd_despeckle(dgi, fnd, a_speckle);
	    dd_denotch(dgi, fnd, f_notch_max, f_notch_shear);
	}
	else if(!strcmp(dname, "VT")) {
	    /* just threshold on ncp
	     */
	    dd_gp_threshold(dgi, fnd, ncp_name, ncp_threshold
			    , ncp_hi_threshold, (int)THRESHOLD_OUTSIDE);
	    dd_despeckle(dgi, fnd, a_speckle);
	}
	else if(!strcmp(dname, "VS") || !strcmp(dname, "VW")) {
	    /* threshold on spectral width
	     */
	    dd_gp_threshold(dgi, fnd, sw_name, 0.0, sw_threshold
			    , (int)THRESHOLD_HIGH);
	    dd_despeckle(dgi, fnd, a_speckle);
	}
	else if(!strcmp(dname, "VP")) {
	    /* threshold on power
	     */
	    dd_gp_threshold(dgi, fnd, pwr_name, pwr_threshold, 0.0
			    , (int)THRESHOLD_LOW);
	    dd_despeckle(dgi, fnd, a_speckle);
	}
	else if(!strcmp(dname, "VN")) {
	    /* threshold on SNR
	     */
	    dd_gp_threshold(dgi, fnd, snr_name, snr_threshold, 0.0
			    , (int)THRESHOLD_LOW);
	    dd_despeckle(dgi, fnd, a_speckle);
	}
	else if(!strcmp(dname, "DP")) {
	    /* threshold on power
	     */
	    dd_gp_threshold(dgi, fnd, pwr_name, pwr_threshold, 0.0
			    , (int)THRESHOLD_LOW);
	    dd_despeckle(dgi, fnd, a_speckle);
	}
	else if(!strcmp(dname, "DS")) {
	    /* threshold on SNR
	     */
	    dd_gp_threshold(dgi, fnd, snr_name, snr_threshold, 0.0
			    , (int)THRESHOLD_LOW);
	    dd_despeckle(dgi, fnd, a_speckle);
	}
	else if(!strcmp(dname, "DT")) {
	    /* threshold on power
	     */
	    dd_gp_threshold(dgi, fnd, pwr_name, 0.0, pwr_threshold
			    , (int)THRESHOLD_HIGH);
	    dd_despeckle(dgi, fnd, a_speckle);
	}
	else if(!strcmp(dname, "GDB")) {
	    dd_lidar_db(dgi, fnd, derfs->GDB_lps);
	}
	else if(!strcmp(dname, "RDB")) {
	    dd_lidar_db(dgi, fnd, derfs->RDB_lps);
	}
	else if(!strcmp(dname, "KDP")) {
	    get_kdp(dgi, fnd);
	}
	else if(!strcmp(dname, "PZDR")) {
	    zdr_prime(dgi, fnd);
	}
	else if(!strcmp(dname, "TLDR")) {
# ifdef obsolete
	    thr_ldr(dgi, fnd, pwr_threshold);
# endif
	    dd_gp_threshold(dgi, fnd, pwr_name, pwr_threshold, 0.0
			    , (int)THRESHOLD_LOW);
	}
	else {
	    /* this becomes a copy which has already been done by
	     * "dd_new_param()"
	     */
	    mark = 0;
	}
    }
}
/* c------------------------------------------------------------------------ */

int zdr_prime (DGI_PTR dgi, int fn)
{
    struct dds_structs *dds=dgi->dds;
    char * ldrv = (char *)"LVDR";
    char * ldr = (char *)"LDR";
    int fn_ldrv, fn_ldr;
    short *ss, *ssEnd, *ssLDRV, *ssLDR;
    double scale, bias;
    int nc, scaled_thr, bad;

    ss = (short *)dds->qdat_ptrs[fn];
    nc = dgi->clip_gate +1;
    ssEnd = ss + nc;

    if((fn_ldrv = dd_find_field(dgi, ldrv)) < 0) {
	return(-1);
    }
    if((fn_ldr = dd_find_field(dgi, ldr)) < 0) {
	return(-1);
    }
    ssLDRV = (short *)dds->qdat_ptrs[fn_ldrv];
    ssLDR = (short *)dds->qdat_ptrs[fn_ldr];

    for(; ss < ssEnd; ss++, ssLDRV++, ssLDR++ ) {
	*ss = (*ssLDRV) - (*ssLDR);
    }
}
/* c------------------------------------------------------------------------ */

int thr_ldr (DGI_PTR dgi, int fn, double threshold)
{
    struct dds_structs *dds=dgi->dds;
    char * dx = (char *)"DX";
    char * ldr = (char *)"LDR";
    int fn_dx, fn_ldr;
    short *ss, *ssEnd, *ssDX, *ssLDR;
    double scale, bias;
    int nc, scaled_thr, bad;

    ss = (short *)dds->qdat_ptrs[fn];
    nc = dgi->clip_gate +1;
    ssEnd = ss + nc;

    if((fn_dx = dd_find_field(dgi, dx)) < 0) {
	return(-1);
    }
    if((fn_ldr = dd_find_field(dgi, ldr)) < 0) {
	return(-1);
    }
    ssDX = (short *)dds->qdat_ptrs[fn_dx];
    ssLDR = (short *)dds->qdat_ptrs[fn_ldr];
    bad = dds->parm[fn]->bad_data;
    scale = dds->parm[fn_dx]->parameter_scale;
    bias = dds->parm[fn_dx]->parameter_bias;
    scaled_thr = (int)DD_SCALE( threshold, scale, bias );

    for(; ss < ssEnd; ss++, ssDX++, ssLDR++ ) {
	if( *ssDX < scaled_thr )
	    { *ss = bad; }
    }
}
/* c------------------------------------------------------------------------ */

static struct lidar_parameters *
malloc_lidar_parm (void)
{
    struct lidar_parameters * this_lidar =  //Jul 26, 2011 this keyword issue
	(struct lidar_parameters *)malloc(sizeof(struct lidar_parameters));
    memset(this_lidar, 0, sizeof(struct lidar_parameters));
    /* default setup is for RDB */
    this_lidar->baseline = 0;
    this_lidar->counts_per_db = 100.;
    this_lidar->end_range = KM_TO_M(3.);
    return( this_lidar );
}
/* c------------------------------------------------------------------------ */

static struct der_pair *
malloc_der_pair (void)
{
    /* source name and destination name derived pairs
     */
    struct der_pair * this_der =  //Jul 26, 2011 this keyword issue
	(struct der_pair *)malloc(sizeof(struct der_pair));
    memset(this_der, 0, sizeof(struct der_pair));
    return( this_der );
}
/* c------------------------------------------------------------------------ */

static int 
setup_der_pairs (char *list, struct der_pair **der_list)
{
    int ii, nt, nn = 0;
    char *aa, *bb, *cc, *dd;
    char string_space[512], *str_ptrs[64];
    struct der_pair * this_der;  //Jul 26, 2011 this keyword issue

    strcpy(string_space, list);
    nt = dd_tokens(string_space, str_ptrs);

    for(ii=0; ii < nt; ii += 3) {
	if( ii + 2 >= nt )
	    { break; }
	if(!ii)
	    { this_der = malloc_der_pair(); *der_list = this_der; }
	else
	    { this_der->next = malloc_der_pair(); this_der = this_der->next; }

	strcpy( this_der->src_name, str_ptrs[ii]);
	strcpy( this_der->dst_name, str_ptrs[ ii + 2 ]);

	if( !strncmp( this_der->dst_name, "RDB", 3 )) {
	    derfs->RDB_lps = malloc_lidar_parm();
	}
	if( !strncmp( this_der->dst_name, "GDB", 3 )) {
	    derfs->GDB_lps = malloc_lidar_parm();
	    derfs->GDB_lps->end_range = KM_TO_M(5.);
	}
	nn++;
    }
    return( nn );
}
/* c------------------------------------------------------------------------ */

int setup_der_alts (char *list)
{
    int ii, nn, total = 0, nt;
    char *aa, *bb, *cc, *dd;
    char * alts[MAX_ALTS];
    char  string_space[512], *str_ptrs[64];
    char  alt_name[32];

    if( strchr( list, '(') ) {	/* alternate lists in parentheses */
	strcpy(string_space, list);
	aa = string_space;
	for(; *aa; aa++)
	    { if( *aa == '\n' )
		{ *aa = ' '; }}
	nt = dd_tokenz(string_space, str_ptrs, "()" );

	/* this should create a set of ordered pairs
	 * the first token should contain an identifier such as DEFAULT
	 * or DUAL_PRF and the second token contains the usual derived
	 * fields sequence
	 */

	for( ii = 0; ii < nt; ii += 2 ) {
	    str_terminate( alt_name, str_ptrs[ii], strlen(str_ptrs[ii]) );
	    nn = 0;
	    /* the lists supported sofar
	     */
	    if( !strncmp(alt_name, "DEFAULT", 3 )) {
		nn = setup_der_pairs
		    ( str_ptrs[ii+1], &derfs->alt_ders[DEFAULT_ALT] );
	    }
	    else if( !strncmp(alt_name, "DUAL_PRF", 4 )) {
		nn = setup_der_pairs
		    ( str_ptrs[ii+1], &derfs->alt_ders[DUAL_PRF_ALT] );
	    }
	    total += nn;
	}
    }

    else {
	nn = setup_der_pairs( list, &derfs->alt_ders[DEFAULT_ALT] );
	total += nn;
    }

    return( total );
}
/* c------------------------------------------------------------------------ */

double 
dd_ac_vel (DGI_PTR dgi)
{
    DDS_PTR dds=dgi->dds;
    struct radar_angles *ra=dds->ra;
    struct platform_i *asib=dds->asib;

    double vert, d, ac_vel;

    vert =  asib->vert_velocity != -999 ? asib->vert_velocity : 0;
    d = sqrt((double)(SQ(asib->ew_velocity) + SQ(asib->ns_velocity)));
    d += dds->cfac->ew_gndspd_corr; /* klooge to correct ground speed */
    ac_vel = d*sin(ra->tilt) + vert*sin(ra->elevation);
    return(ac_vel);
}
/* c------------------------------------------------------------------------ */

void 
dd_BB_unfold (DGI_PTR dgi, int fn, double ac_vel, int qsize, double f_fold_shear, struct ve_que_val *vqv)
{
    DDS_PTR dds=dgi->dds;
    struct radar_angles *ra=dds->ra;
    struct platform_i *asib=dds->asib;
    float nyqv=dds->radd->eff_unamb_vel;
    float scale=dds->parm[fn]->parameter_scale;
    float bias=dds->parm[fn]->parameter_bias;
    int bad=dds->parm[fn]->bad_data;
    int nc=dgi->clip_gate+1;

    struct ve_que_val *this_val;  //Jul 26, 2011 this keyword issue
    int ii, jj;
    int g, nb, fold_count, sum, mark, scaled_max_wind;
    int gg, first_cell=YES;
    short scaled_nyqv, scaled_nyqi, scaled_ac_vel;
    short *ss, *ssx, *zz;
    short v0, v4, vx;
    float r, rot, ss_ring, alt, tan_elev;
    double folds, rcp_nyqi, rcp_qsize=1./(float)qsize, u, v, w, elev;
    double insitu_wind;
    double d;
    static int count=0, trip=123;
    static short last_good_v0[MAX_SENSORS];


    count++;
    rot = fmod(DEGREES(ra->rotation_angle)+360,(double)360.);
    scaled_max_wind = (int)DD_SCALE(f_max_wind, scale, bias);
    scaled_nyqv = (short)DD_SCALE(nyqv, scale, bias);
    scaled_nyqi = 2*scaled_nyqv;
    rcp_nyqi=1./((float)scaled_nyqi);
    v0 = bad;
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn] + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
    u = asib->ew_horiz_wind;
    v = asib->ns_horiz_wind;
    w = asib->vert_wind != -999 ? asib->vert_wind : 0;

    if(sidelobe_ring > 0) {
	alt = (asib->altitude_agl)*1000. +dds->cfac->pressure_alt_corr*1000.;
	elev = PIOVR2-ra->tilt;
	tan_elev = tan(elev);
	ss_ring = fabs(-(alt)/sin(elev))*
	      (1.+alt/(2.*dd_earthr(dd_latitude(dgi))*SQ(tan_elev)));
	dd_range_gate(dgi, ss_ring, &gg, &r); /* returns a gate number in gg */
	ii = (gg-1) -sidelobe_ring/2;
	for(jj=ii+sidelobe_ring; ii >= 0 && ii < jj; ii++) {
	    *(ss+ii) = bad;
	}
    }

    if(dont_use_wind_info) {
       if(dds->swib->num_rays == 1 || dont_use_wind_info == USE_THIS_RAY) {
	  /* if first ray in sweep  or not initializing from prev ray */
	  v0 = last_good_v0[dgi->radar_num] = bad;
       }
       else {
	  v0 = last_good_v0[dgi->radar_num];
       }
       if(v0 == bad) {	    /* find first good gate? */
	  for(ssx=ss; ssx < zz && *ssx == bad ; ssx++);
	  if(ssx == zz) return;
	  v0 = *ssx;
       }
       /* assumes the data has already been corrected for air motion */
       scaled_ac_vel = 0;
    }
    else {
	insitu_wind =
	      cos(ra->elevation) * (u*sin(ra->azimuth) + v*cos(ra->azimuth))
		    + w*sin(ra->elevation);
	v0 = (short)DD_SCALE(insitu_wind, scale, bias);
	scaled_ac_vel = (short)DD_SCALE(ac_vel, scale, bias);
    }

    for(sum=0,this_val=vqv,ii=0; ii < qsize; ii++) { /* load up the que */
	    this_val->vel = v0;
	    sum += v0;
	    this_val = this_val->next;
    }

    if(count > trip) {
	mark = 0;
    }
    if(rot >= rota && rot <= rotb) {
	mark = 0;
    }

    /* do the unfolding
     */
    for(gg=g=0; ss < zz; g++,gg++) {
	/* skip over bad flagged data */
	for(nb=0; ss < zz && *ss == bad; ss++,nb++,g++);
	if(ss == zz) break;

	vx = *ss;		/* the velocity */
	v4 = (short)(rcp_qsize*sum);	/* the average */
	folds = ((v4 -vx) -scaled_ac_vel) * rcp_nyqi;
	folds = folds < 0 ? folds -0.5 : folds +0.5;
	fold_count = (int)folds;
	vx += fold_count*scaled_nyqi +scaled_ac_vel;

	if(IABS(vx) > scaled_max_wind) {
	    vx = vx < 0 ? vx+scaled_nyqi : vx-scaled_nyqi;
	}
	if(first_cell) {
	    first_cell = NO;
	    last_good_v0[dgi->radar_num] = vx;
	}
	/* update the average
	 */
	sum -= this_val->vel;
	this_val->vel = vx;
	sum += vx;
	this_val = this_val->next;

	*ss++ = vx;		/* replace the old velocity with the new */
    }
    return;
}
/* c------------------------------------------------------------------------ */

void 
dd_denotch (DGI_PTR dgi, int fn, double f_notch_max, double f_notch_shear)
{
    DDS_PTR dds=dgi->dds;
    int bad=dds->parm[fn]->bad_data;
    float nyqv=dds->radd->eff_unamb_vel;
    float scale=dds->parm[fn]->parameter_scale;
    float bias=dds->parm[fn]->parameter_bias;

    int nc=dgi->clip_gate+1;
    int gg, ii, mark, nbad;
    int notch_max, notch_shear, scaled_nyqv, diff, u_diff;
    short *anchor, *good, *rr, *ss, *tt, *zz;

    notch_max = (int)DD_SCALE(f_notch_max, scale, bias);
    notch_shear = (int)DD_SCALE(f_notch_shear, scale, bias);
    scaled_nyqv = (int)DD_SCALE(nyqv, scale, bias);
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn]
			 + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;

    /*
     * loop through the data removing notches (false zeroes)
     * this is mostly for TOGA-COARE velocity!
     */
    for(gg=0; ss < zz;) {
	if(*ss == bad) {gg++; ss++; continue;}
	anchor = ss;		/* good value */
	/*
	 * to find the next notch, locate next shear point
	 */
	for(;;) {		
	    good = ss++; gg++;
	    /* skip over bad flagged data */
	    for(nbad=0; ss < zz && *ss == bad; ss++,gg++,nbad++);
	    if(ss == zz || nbad > min_bad_count) break;
	    diff = *good -*ss;
	    u_diff = IABS(diff);
	    if(u_diff > notch_shear) {
		if(u_diff > notch_shear && IABS(*ss) < notch_max) {
		    /* we appear to be in a notch
		     * find the end of the notch
		     */
		    for(tt=ss; tt < zz; tt++,gg++) {
			for(nbad=0; tt < zz && *tt == bad; tt++,gg++,nbad++);
			/* terminate on too many bad flags or
			 * a non-notch value
			 */
			if(nbad > min_bad_count) break;
			if(IABS(*tt) > notch_max) break;
		    }
		    for(; ss < tt; ss++) {
			if(*ss != bad)
			      *ss += *ss < 0 ? scaled_nyqv : -scaled_nyqv;
		    }
		    break;
		}
		else if(IABS(*good) < notch_max && IABS(*ss) > notch_max) {
		    /* we appear to be at the end of a previously
		     * undetected notch
		     */
		    for(rr=good; rr >= anchor; rr--) {
			if(*rr != bad) {
			    diff = *ss - *rr;
			    if(IABS(diff) > notch_shear &&
			       IABS(*rr) < notch_max)
				  *rr += *rr < 0 ? scaled_nyqv : -scaled_nyqv;
			    else
				  break;
			}
		    }
		    break;
		}
	    }
	    mark = 0;
	}
	mark = 0;
    }
}
/* c------------------------------------------------------------------------ */

int 
dd_despeckle (DGI_PTR dgi, int fn, int a_speckle)
{
    /*
     * remove speckles
     * if the number of consecutive good cells surrounded by bad flags
     * is less than or equal to the speckle value, they will be zapped
     */
    DDS_PTR dds=dgi->dds;
    int bad=dds->parm[fn]->bad_data;

    int nc=dgi->clip_gate+1;
    int nn, mark;
    short *anchor, *ss, *zz;

    if(a_speckle <= 0)
	  return(0);

# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn]
		   + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
    for(; ss < zz;) {
	if(*ss == bad) {ss++; continue;}
	/* at a good cell...count the number of following good cells
	 */
	for(anchor=ss++,nn=1; ss < zz  && *ss != bad ; ss++,nn++);
	if(nn <= a_speckle)
	      for(; anchor < ss;) { /* zap em! */
		  *anchor++ = bad;
	      }
	/* at this point we are sitting on a bad flag or at the end */
    }
    return(0);
}
/* c------------------------------------------------------------------------ */
#define	MAXNUM_GATES	2048	/* Maximum number of range gates */
#define	PRODS_ELEMENTS	10	/* Number of variables in dual-pol prods array */
#define	PRODS_VEL	0	/* Offset of variable "radial velocity (m/s)" */
#define	PRODS_POW	1	/*	power (dBm) */
#define	PRODS_NCP	2	/*	normalized coherent power (0-1) */
#define	PRODS_WIDTH	3	/*	spectrum width (m/s) */
#define	PRODS_REF	4	/*	reflectivity horizontal pol (dBZ) */
#define	PRODS_CREF	5	/*	coherent reflectivity (dBZ) */
#define	PRODS_ZDR	6	/*	differential reflectivity (dB) */
#define	PRODS_PDP	7	/*	differential phase shift (deg) */
#define	PRODS_RHV	8	/*	correlation coeeficient */
#define	PRODS_KDP	9	/*	gradient of differential phase (deg/km) */

#define	POWTHRES	-100	/* Min power threshold (replaces SNR threshold) */
#define THRES		3.	/* Threshold for fluct in phidp;may need to be reset for S pol */
#define THRCRR		0.9	/* rhohv threshold */
#define	MAXPDPSLP	50	/* Max tolerated KdP: 50 deg/km */
#define	MGOOD		5	/* Number of good data to enter cell */
#define	MBAD		3	/* Number of bad data to exit cell */
#define	FLIP_70		70.	/* Thresholds for phase flipping problem */
#define	FLIP_250	250.
#define	FLIP_140	140.
#define	HAIL_TEST_Z	45	/* Minimum reflectivity to test for hail signal */
#define	HAIL_RHOHV	.8	/* Minimum rho-hv for hail signature */
#define	HAIL_SD_RHOHV	.08	/* Maximum sigma on rho-hv for hail sig */
#define FIR3ORDER	20	/* Adaptive filter order and gain */
#define FIR3GAIN	1.044222

#define	OFSX		90	/* Offset used in internal variable */
#define	OFSZ		200	/* Offset used in internal variable */

#define	MAXGATES_ACCUM	2000	/* Max # range gates for accumulation */
#define	MIN_NCP_ACCUM	0.50	/* #### To be fixed! Minimum NCP required to trust the presence of "signal" */
#define	ZR_A		300	/* Z-R relationship: Z = 300 R ^ 1.4*/
#define ZR_B		1.4
#define MIN_KDP_ACCUM	0.1	/* Minimum KdP to trust KdP rainfall */
#define	KDP_A		40.56	/* KdP-R relationship:  R = 40.5 * KdP ^ .85 */
#define	KDP_B		0.866

/* c------------------------------------------------------------------------ */

int get_kdp (DGI_PTR dgi, int fn)
{
   DDS_PTR dds=dgi->dds;
   struct cell_d *celv=dgi->dds->celvc;
   int nc=dgi->clip_gate;
   int fnx, mark;
   short *ss;
   float scale, rcp_scale, bias;
   double d;
   static int ray_count = 0;

   /* c...fred */

	int i,ikdpst,j,jj,il,kf,ij,ibegin,iend,mc,kbegin,n,mloop,irl ;
	int firsttime,count,icount,slpcount,loop,length ;
	float avrg1,avrg2,delavg,POWslevel,pdpslp,flip,rmc,rend,d1,d2,d3 ;
	float zhmn,ymn,sd,ext,acc,delt,POW,rij,aa,delta,tmp ;
	float x[MAXNUM_GATES+OFSX+31], y[MAXNUM_GATES+OFSX+31] ;
	float z[MAXNUM_GATES+OFSZ+31] ;
	float *pptr, range_step ;
	float range[MAXNUM_GATES+OFSX+31], ref[MAXNUM_GATES+OFSX+31] ;
	float zdr[MAXNUM_GATES+OFSX+31], phidp[MAXNUM_GATES+OFSX+31] ;
	float rhv[MAXNUM_GATES+OFSX+31], pow[MAXNUM_GATES+OFSX+31] ;
	float work_kdp[MAXNUM_GATES+OFSX+31], nad[MAXNUM_GATES+OFSX+31], nfl[MAXNUM_GATES+OFSX+31] ;
	float xp[101],yp[101],a[6],b[6] ;
	float init0 ;
	float fir3coef[FIR3ORDER+1] =
	     { 1.625807356e-2, 2.230852545e-2, 2.896372364e-2, 3.595993808e-2,
	       4.298744446e-2, 4.971005447e-2, 5.578764970e-2, 6.089991897e-2,
	       6.476934523e-2, 6.718151185e-2, 6.80010000e-2, 6.718151185e-2,
	       6.476934523e-2, 6.089991897e-2, 5.578764970e-2, 4.971005447e-2,
	       4.298744446e-2, 3.595993808e-2, 2.896372364e-2, 2.230852545e-2,
	       1.625807356e-2 } ;

	static int trip_ray = -11;
	int kk;
	float ff, *from, *to, *phiz, *phiz0, *phid, *phid0, *phif, *phif0;
	FILE * ffid;
	char str[256];

	if(!ray_count++) {
	    if(trip_ray > 0) {
		phiz0 = (float *)malloc(1600 * sizeof(float));
		phid0 = (float *)malloc(1600 * sizeof(float));
		phif0 = (float *)malloc(1600 * sizeof(float));
	    }
	}

	if(ray_count == trip_ray) {
	    mark = 0;
	}


/* Set some initial values */

	avrg2=0. ;		/* initial value of averaged phidp */

	firsttime = 1 ;
	count = 0 ;

# ifdef obsolete
	length = dwell->header.gates ;
# else
	length = dgi->clip_gate +1;
# endif
	ibegin    = length ; /* length of prods array from i=1,number of gates */
	kbegin    = length ;
	iend      = length ;
	loop      = 1 ;

/*cc  x(i) --  storage array
cccc  z(i) --  raw phidp array modified for good/bad data segments
cccc  y(i) --  filtered profile of z(i) array from which Kdp is calculated.
*/
/* Clear arrays */

	for( i = -5 ; i <= length ; i++ ) {
	    z[i+OFSZ] = 0 ;
	    x[i+OFSX] = 0 ;
	    work_kdp[i+OFSX] = 0 ;	/* preset Kdpadap */
	}

/* Move data from prods array in other arrays.  Done because lots of
   code addressing NEGATIVE range gates follows. */

# ifdef obsolete
	range_step = 149900 * dwell->header.rcvr_pulsewidth ;
# else
	range_step = (celv->dist_cells[1] - celv->dist_cells[0]) * .001;
# endif
	for( i = -OFSX ; i <= 0 ; i++) {
	    range[i+OFSX] = (i-1) * range_step ;
	    pow[i+OFSX] = -999 ;
	    ref[i+OFSX] = -99 ;
	    zdr[i+OFSX] = 0 ;
	    rhv[i+OFSX] = 0 ;
	}
	for( i = length+1 ; i <= length+30 ; i++ )
	{
	    range[i+OFSX] = (i-1) * range_step ;
	    pow[i+OFSX] = -999 ;
	    ref[i+OFSX] = -99 ;
	    zdr[i+OFSX] = 0 ;
	    rhv[i+OFSX] = 0 ;
	}

# ifdef obsolete
	pptr = prods ;
	for( i = 1 ; i <= length ; i++, pptr += PRODS_ELEMENTS ) {
	    range[i+OFSX] = (i-1) * range_step ;
	    pow[i+OFSX] = *(pptr+PRODS_POW) ;
	    ref[i+OFSX] = *(pptr+PRODS_REF) ;
	    zdr[i+OFSX] = *(pptr+PRODS_ZDR) ;
	    phidp[i+OFSX] = *(pptr+PRODS_PDP) ;
	    rhv[i+OFSX] = *(pptr+PRODS_RHV) ;
	}
# else
   if((fnx = dd_find_field(dgi, (char *)"DM")) < 0) {
      return(-1);
   }
   scale = dds->parm[fnx]->parameter_scale;
   rcp_scale = 1./scale;
   bias = dds->parm[fnx]->parameter_bias;
   ss = (short *)dds->qdat_ptrs[fnx];

   for( i = 1 ; i <= length ; i++) {
      range[i+OFSX] = (i-1) * range_step ;
      pow[i+OFSX] = DD_UNSCALE(*ss++, rcp_scale, bias);
   }      

   if((fnx = dd_find_field(dgi, (char *)"DZ")) < 0) {
      return(-1);
   }
   scale = dds->parm[fnx]->parameter_scale;
   rcp_scale = 1./scale;
   bias = dds->parm[fnx]->parameter_bias;
   ss = (short *)dds->qdat_ptrs[fnx];

   for( i = 1 ; i <= length ; i++) {
      ref[i+OFSX] = DD_UNSCALE(*ss++, rcp_scale, bias);
   }      

   if((fnx = dd_find_field(dgi, (char *)"ZDR")) < 0) {
      return(-1);
   }
   scale = dds->parm[fnx]->parameter_scale;
   rcp_scale = 1./scale;
   bias = dds->parm[fnx]->parameter_bias;
   ss = (short *)dds->qdat_ptrs[fnx];

   for( i = 1 ; i <= length ; i++) {
      zdr[i+OFSX] = DD_UNSCALE(*ss++, rcp_scale, bias);
   }      

   if((fnx = dd_find_field(dgi, (char *)"PHI")) < 0) {
      return(-1);
   }
   scale = dds->parm[fnx]->parameter_scale;
   rcp_scale = 1./scale;
   bias = dds->parm[fnx]->parameter_bias;
   ss = (short *)dds->qdat_ptrs[fnx];

   for( i = 1 ; i <= length ; i++) {
      phidp[i+OFSX] = DD_UNSCALE(*ss++, rcp_scale, bias);
   }      

   if((fnx = dd_find_field(dgi, (char *)"RHOHV")) < 0) {
      return(-1);
   }
   scale = dds->parm[fnx]->parameter_scale;
   rcp_scale = 1./scale;
   bias = dds->parm[fnx]->parameter_bias;
   ss = (short *)dds->qdat_ptrs[fnx];

   for( i = 1 ; i <= length ; i++) {
      rhv[i+OFSX] = DD_UNSCALE(*ss++, rcp_scale, bias);
   }      
# endif


/* --------- FIND THE start AND stop BINs FOR FILTERING --------------- */

	for( i = 1 ; i <= length ; i++ ) {

	   pdpslp = ( phidp[i+OFSX+1] - phidp[i+OFSX-1] ) / ( 2*range_step ) ;

	    POW = pow[i+OFSX] ;
	    if ( POW < POWTHRES )
		POWslevel = 999 ;
	    else if ( ( i >= 2 ) && ( i <= length - 1 ) )
		POWslevel = POWTHRES ;

	    switch (loop) {
		case 1:  goto L100 ;
		case 2:  goto L200 ;
	    }

/*  Find begin of GOOD data loop */
L100:	    z[i+OFSZ] = avrg2 ;
	    if ( ( rhv[i+OFSX] >= THRCRR ) && ( POW > POWslevel ) &&
		( fabs(pdpslp) < MAXPDPSLP ) ) {
		count++ ;
		if ( count == MGOOD ) {
		    for ( il = 0 ; il < MGOOD ; il++ ) {
			z[i+OFSZ-il] = phidp[i+OFSX-il] ;
		    }
		    if ( firsttime == 1 ) {
			ibegin = i-MGOOD+1 ;  /* begin of the 1st encountered cell */
			init0 = .25 * ( z[ibegin+OFSZ] + z[ibegin+OFSZ+1] + z[ibegin+OFSZ+2] + z[ibegin+OFSZ+3] ) ;
			firsttime = 0 ;
			flip = 0. ;
		    }
		    else {
			mc = i-MGOOD+1 ;  /* begin of the successive encountered cells */
			avrg1 = .25 * ( z[mc+OFSZ+1] + z[mc+OFSZ+2] + z[mc+OFSZ+3] + z[mc+OFSZ+4] ) ;
			delavg = avrg1-avrg2 ;

/* Check and fix flipping problem */
			if ( fabs(delavg) > FLIP_70 ) {
			    if ( delavg > FLIP_250 )
				flip = -360 ;
			    else if ( delavg < -FLIP_250 )
				flip = 360 ;
			    else if ( delavg > FLIP_140 )
				flip = -180 ;
			    else if ( delavg < -FLIP_140 )
				flip = 180 ;
			    else if ( delavg > FLIP_70 )
				flip = - 90 ;
			    else if ( delavg < -FLIP_70 )
				flip = 90 ;
			    for ( kf = 0 ; kf < MGOOD ; kf++ )
				z[i+OFSZ-kf] += flip ;
			    avrg1 += flip ;
			}

/* Tied the bad data according to the monotonous increasing property of Phidp */
			else {
			    flip = 0 ;
			    if ( avrg1 < init0 )
				avrg1 = init0 ;
			    if ( ( avrg2 > avrg1 ) || ( avrg2 < init0 ) )
				if ( avrg2 > avrg1 )
				    avrg2 = avrg1 ;
				else
				    avrg2 = init0 ;
			}

/* Link the bad gap by a slope line instead of a flat line. Li Liu, 9/29/95 */
			rmc = range[mc+OFSX] ;
			rend = range[iend+OFSX] ;
			d1 = rmc - rend ;
			d2 = (avrg1-avrg2) / d1 ;
			d3 = (rend*avrg1 - rmc*avrg2) / d1 ;
			for ( ij = iend+1 ; ij <= mc ; ij++ ) {
			    rij = range[ij+OFSX] ;
			    z[ij+OFSZ] = rij*d2 - d3 ;
			}
		    }	/* end if firsttime */

		    loop = 2 ;
		    count = 0 ;
		    iend = length ;
		}  /* end if (count == MGOOD) */
	    } /* end if good data test */
	    else
		count = 0 ;
	    goto L1000 ;

/*  ************* Find END of GOOD DATA loop **************** */

L200:	    z[i+OFSZ] = phidp[i+OFSX] + flip ;

	    if ( i == length ) {
		iend = length ;
		goto L203 ;
	    }
	    if ( ( rhv[i+OFSX] < THRCRR ) || ( POW < POWslevel ) ||
		 ( pdpslp > MAXPDPSLP-5 ) || ( pdpslp < -MAXPDPSLP + 10 ) ) {
		count++ ;
		if ( count == MBAD ) {  /* Insert test mean&sd of Rhv to preserve hail signal. */
		    zhmn = 0 ;
		    ymn = 0 ;
		    for ( jj = 0 ; jj < MBAD ; jj++ ) {
			zhmn += ref[i+OFSX-jj] ;
			ymn += rhv[i+OFSX-jj] ;
		    }
		    zhmn /= MBAD ;

		    if (hail_test && zhmn > HAIL_TEST_Z ) {
			ymn /= MBAD ;	/* Compute mean & sd like msr would have done */
			for ( jj = 0, sd = 0 ; jj < MBAD ; jj++ ) {
			    tmp = ( rhv[i+OFSX-jj] - ymn ) ;
			    sd += tmp * tmp ;
			}
			sd /= MBAD ;
			if ( ( ymn >= HAIL_RHOHV ) && ( sd <= HAIL_SD_RHOHV ) ) {
			    count = 0 ;
			    hail_hits++;
			    if(!(hail_hits % 50)) {
				/*
				printf( "Hail Hits: %d \n", hail_hits );
				 */
			    }
			    goto L1000 ;
			}
		    }
		    iend = i - MBAD ;


L203:		    avrg2 = .25 * ( z[iend+OFSZ] + z[iend+OFSZ-1] + z[iend+OFSZ-2] + z[iend+OFSZ-3] ) ;
		    if ( iend == length )  goto L1000 ;
		    z[i+OFSZ] = avrg2 ;
		    z[i-1+OFSZ] = avrg2 ;
		    loop = 1 ;
		    count = 0 ;
		}
	    }
	    else
		count = 0 ;
L1000: ;
	}
/* ---------------- END of FINDING start AND stop BINs ------------------- */

	if ( ibegin == length )	{	/* NO good data in whole ray. RETURN. */
# ifdef obsolete
	    pptr = prods + PRODS_KDP ;
	    for ( i = 1 ; i <= length ; i++, pptr += PRODS_ELEMENTS )
		*pptr = 0 ;
# else
# endif
	    return( 0 ) ;
	}
	if ( kbegin == length )
	    kbegin = ibegin ;
	ext = avrg2 ;
	n = length ;
	for ( i = 1 ; i <= 89+kbegin ; i++ )
	    z[kbegin-i+OFSZ] = init0 ;	/* Set the initial conditions */
	for ( i = 1 ; i <= n-iend+30 ; i++ )
	    z[iend+i+OFSZ] = ext ;	/* Extend data record */
	for ( i = -90 ; i <= length+30 ; i++ )
	    x[i+OFSX] = z[i+OFSZ] ;	/* Adjust raw data array */


	if(ray_count == trip_ray) {
	    from = &z[OFSZ];
	    to = phiz0;
	    
	    for(kk = length; kk--; *to++ = *from++);
	}


/* *************** Phidp Glitch Correction ******************* */

	for ( i = 1 ; i <= length-1 ; i++ ) {
	    aa = ( z[i+OFSZ+1] - z[i+OFSZ] ) / range_step ;
	    if ( fabs(aa) > 30 )
		z[i+OFSZ] = z[i+OFSZ-1] ;
	}
	irl = 0 ;

	if(ray_count == trip_ray) {
	    from = &z[OFSZ];
	    to = phid0;
	    
	    for(kk = length; kk--; *to++ = *from++);
	}



/* ------------- MAIN LOOP of Phidp Adaptive Filtering --------------------
		 Hard wired for 2 successive applications of adaptive filter */

	for ( mloop = 1 ; mloop <= 2 ; mloop++ ) {
/*  TIE DOWN THE INITIAL and  EXTENDING DATA RECORD */
	    for ( i = irl ; i <= kbegin + 89 ; i++ )
		z[kbegin-i+OFSZ] = init0 ;
	    for ( i = 1 ; i <= n-iend+30 ; i++ )
		z[iend+i+OFSZ] = ext ;

/*  FIR3 FILTER SECTION  ( change back from FIR1        12/8/92. ) */
	    for ( i = -5 ; i <= n+5 ; i++ ) {
		acc = 0 ;
		for ( j = 0 ; j <= FIR3ORDER ; j++ )
		    acc += fir3coef[j] * z[i+OFSZ-FIR3ORDER/2+j] ;
		y[i+OFSX] = acc * FIR3GAIN ;
	    }
/* END of FIR3 FILTERING */

	    for ( i = 1 ; i <= n ; i++ ) {
		delt = fabs( x[i+OFSX] - y[i+OFSX] ) ;
		if ( delt >= THRES )
		    z[i+OFSZ] = y[i+OFSX] ;
		else
		    z[i+OFSZ] = x[i+OFSX] ;
	    }
	}

	if(ray_count == trip_ray) {
	    from = &y[OFSX];
	    to = phif0;
	    
	    for(kk = length; kk--; *to++ = *from++);

	    sprintf(str, "/scr/hotlips/oye/spol/Ray%dxlt.txt", ray_count);
	    ffid = fopen(str, "w");
	    phiz = phiz0;
	    phid = phid0;
	    phif = phif0;

	    for(kk = 0; kk < length; kk++, phiz++, phid++, phif++) {
		sprintf(str, "%10.2f %10.2f %10.2f %10.2f \n"
			, M_TO_KM(celv->dist_cells[kk])
			, *phiz, *phid, *phif
			);
		fputs(str, ffid);
	    }
	    fclose(ffid);
	    ff = THRCRR;
	    ff = MAXPDPSLP;
	    ff = POWTHRES;

	}

/* Calculate Kdp via finite differences */
	delta = 4. * range_step ;/* two way value */

# ifdef obsolete
	pptr = prods + PRODS_KDP ;
	for ( i = 1 ; i <= n ; i++, pptr += PRODS_ELEMENTS )
	   *pptr = (y[i+1+OFSX] - y[i-1+OFSX]) / (delta) ;
# else
        scale = dds->parm[fn]->parameter_scale;
        bias = dds->parm[fn]->parameter_bias;
        ss = (short *)dds->qdat_ptrs[fn];
	for ( i = 1 ; i <= n ; i++) {
	   d = (y[i+1+OFSX] - y[i-1+OFSX]) / (delta) ;
	   *ss++ = (short)DD_SCALE(d, scale, bias);
	}
# endif
	return ( n ) ;

}
/* c------------------------------------------------------------------------ */

int 
dd_single_prf_vortex_vels (DGI_PTR dgi, int fn, char *dname)
{
    int ii, kk, nc, scaled_av_nyq, vr;
    int fn_vr;
    float f_err = dgi->time < D950502 ? 1.0106326 : 1.0212653;
    float *fptr, d, wvl, prf, av_nyq, scale, bias, *ipptr;
    short *ss, *zz, *tt;
    struct radar_d *radd=dgi->dds->radd;
    struct parameter_d *parm;

    if((fn_vr = dd_find_field(dgi, (char *)"VR")) < 0) {
	return(-1);
    }
    tt = (short *)dgi->dds->qdat_ptrs[fn_vr];
    ss = (short *)dgi->dds->qdat_ptrs[fn];
    nc = dgi->clip_gate +1;
    zz = ss + nc;

    if (dgi->radar_name.find("TA") != std::string::npos &&
	dgi->time < D970401) {
       /* aft antenna! */
	parm = dgi->dds->parm[fn_vr];
	scale = dgi->dds->parm[fn_vr]->parameter_scale;
	bias = dgi->dds->parm[fn_vr]->parameter_bias;
	fptr = &radd->freq1;

	for(d=0, ii=0, kk=1; ii < 5; ii++, kk<<=1) {
	    if(kk & parm->xmitted_freq) {
		d += *(fptr +ii);
	    }
	}
	/* frequency is in Ghz */
	wvl = SPEED_OF_LIGHT/((d/(double)(radd->num_freq_trans)) * 1.e9);
	/*
	 * get the PRFs
	 */
	for(ii=0, kk=1; ii < 5; ii++, kk<<=1) {
	    if(kk & parm->interpulse_time) {
		break;
	    }
	}
	ipptr = &radd->interpulse_per1;
	prf = 1./(*(ipptr + ii) *.001);

	av_nyq = .25 * wvl * prf;
	scaled_av_nyq = (int)DD_SCALE( av_nyq, scale, bias );

	for(; ss < zz; *ss++, *tt++ ) {
	    vr = (int)((float)(*tt) * f_err + .5);
	    *ss = ( vr > scaled_av_nyq ) ? scaled_av_nyq : vr;
	}
    }
    /* otherwise the field has already been copied */
    return(0);
}
/* c------------------------------------------------------------------------ */

int 
dd_fix_vortex_vels (DGI_PTR dgi, int fn, char *dname)
{
    static struct fix_vortex_vels *fvvs[MAX_SENSORS];
    static int radar_count=0;

    int ii, kk, mark, nc, bad, vL, vS, levels;
    double d, wvl, freq, prfL, prfS, ratioXY, rcp_half_nyqL, d_round ;
    double av_nyqL, av_nyqS;
    static int scaled_av_nyqL, scaled_av_nyqS;
    char *name, *dst_name;
    char *vl=(char *)"VL", *vs=(char *)"VS";
    int fn_vl, fn_vs, *ctr, level_bias;
    float *fptr, *ipptr, X, Y, scale, bias, rcp_scale;
    static float f_err = 1.0;
    int vorfast_fix = NO;

    struct fix_vortex_vels *fvv;
    struct dds_structs *dds=dgi->dds;
    struct radar_d *radd=dgi->dds->radd;
    struct parameter_d *parm;

    short *ss, *zz, *vl_ptr, *vs_ptr, *dst;



    if(!radar_count) {
	for(ii=0; ii < MAX_SENSORS; fvvs[ii++] = NULL) ;
    }

    ss = (short *)dds->qdat_ptrs[fn];
    nc = dgi->clip_gate +1;
    zz = ss + nc;

    if((fn_vs = dd_find_field(dgi, vs)) < 0) {
	return(-1);
    }
    parm = dds->parm[fn_vs];
    scale = dds->parm[fn_vs]->parameter_scale;
    rcp_scale = 1./scale;
    bias = dds->parm[fn_vs]->parameter_bias;

    if((fn_vl = dd_find_field(dgi, vl)) < 0) {
	return(-1);
    }
    vs_ptr = (short *)dds->qdat_ptrs[fn_vs];
    vl_ptr = (short *)dds->qdat_ptrs[fn_vl];

    if(!strcmp(dname, "VQB1") || !strcmp(dname, "VQC1")) {
      if (dgi->radar_name.find("TA") != std::string::npos) { /* aft antenna! */
	    vorfast_fix = YES;
	}
    }
    if (dgi->time > D970401) {
       vorfast_fix = NO;
    }
    


    if(dgi->new_vol) {

	f_err = dgi->time < D950502 ? 1.0106326 : 1.0212653;

	if(!(fvv = fvvs[dgi->radar_num])) {
	    fvv = fvvs[dgi->radar_num] = (struct fix_vortex_vels *)malloc
		(sizeof(struct fix_vortex_vels));
	    memset(fvv, 0, sizeof(struct fix_vortex_vels));
	  fvv->d_round = .5;
	  radar_count++;
	}
	/*
	 * this assumes the scale and the bias are the same for
	 * all three fields
	 */
	fptr = &radd->freq1;

	for(d=0, ii=0, kk=1; ii < 5; ii++, kk<<=1) {
	    if(kk & parm->xmitted_freq) {
		d += *(fptr +ii);
	    }
	}
	/* frequency is in Ghz */
	wvl = SPEED_OF_LIGHT/((d/(double)(radd->num_freq_trans)) * 1.e9);
	/*
	 * get the PRFs
	 */
	for(ii=0, kk=1; ii < 5; ii++, kk<<=1) {
	    if(kk & parm->interpulse_time) {
		break;
	    }
	}
	/* prfS is the greater prf
	 * the interpulse period is in milliseconds
	 */
	ipptr = &radd->interpulse_per1;
	prfS = 1./(*(ipptr + ii) *.001);
	parm = dds->parm[fn_vl];

	for(ii=0, kk=1; ii < 5; ii++, kk<<=1) {
	    if(kk & parm->interpulse_time) {
		break;
	    }
	}
	prfL = 1./(*(ipptr + ii) *.001);

	X = prfS/(prfS - prfL);
	Y = prfL/(prfS - prfL);
	ratioXY = prfS/prfL;
	levels = (int)(X + Y + .1);
	level_bias = fvv->level_bias = levels/2;
	av_nyqS = .25 * wvl * prfS;
	av_nyqL = .25 * wvl * prfL;
	scaled_av_nyqS = (int)DD_SCALE( av_nyqS, scale, bias );
	scaled_av_nyqL = (int)DD_SCALE( av_nyqL, scale, bias );
	fvv->rcp_half_nyqL = 1./DD_SCALE((.5 * av_nyqL), scale, bias);
	ctr = &fvv->vConsts[level_bias];
	*ctr = 0;

	for(ii=1; ii <= fvv->level_bias; ii++) {
	    switch(ii) {
		/* these cases represent (vL - vS) normalized
		 * by half the nyquist velocity
		 */
	    case 1:		/*  */
		d = DD_SCALE(av_nyqL * (1. + ratioXY), scale, bias);
		*(ctr - ii) = (int)d;
		*(ctr + ii) = (int)(-d);
		break;

	    case 2:
		d = DD_SCALE(2. * av_nyqL * (1. + ratioXY), scale, bias);
		*(ctr - ii) = (int)d;
		*(ctr + ii) = (int)(-d);
		break;

	    case 3:
		d = DD_SCALE(av_nyqL * (2. + ratioXY), scale, bias);
		*(ctr - ii) = (int)(-d);
		*(ctr + ii) = (int)d;
		break;

	    case 4:
		d = DD_SCALE(av_nyqL, scale, bias);
		*(ctr - ii) = (int)(-d);
		*(ctr + ii) = (int)d;
		break;
	    }
	}
    }
    fvv = fvvs[dgi->radar_num];
    rcp_half_nyqL = fvv->rcp_half_nyqL;
    level_bias = fvv->level_bias;
    d_round = fvv->d_round;
    ctr = fvv->vConsts;

    /*
     * now loop through the data
     */

    for(; ss < zz; ss++,vl_ptr++,vs_ptr++) {
	vS = *vs_ptr;
	vL = *vl_ptr;

	if( vorfast_fix ) {
	    vS = (int)(f_err * (float)vS +.5);
	    vL = (int)(f_err * (float)vL +.5);
	    if( vS > scaled_av_nyqS )
		{ vS = scaled_av_nyqS; }
	    if( vL > scaled_av_nyqL )
		{ vL = scaled_av_nyqL; }
	}
	d = ((double)vS - (double)vL) * rcp_half_nyqL + level_bias
	      + d_round;
	kk = (int)d;
	*ss = ((vS + vL) >> 1) + (*(ctr + kk));
    }
}
/* c------------------------------------------------------------------------ */

int 
dd_gp_threshold (DGI_PTR dgi, int fn, char *thr_name, double thr_val1, double thr_val2, int thr_type)
{
    /* general purpose thresholding
     */
    DDS_PTR dds=dgi->dds;
    int bad=dds->parm[fn]->bad_data;

    int nc=dgi->clip_gate+1;
    int nchar = strlen(thr_name);
    int gg, ii, scaled_thr1, scaled_thr2, mark, fthr;
    short *ss, *zz, *thr=NULL;

    /* find the thr field
     */
    
    if((fthr = dd_find_field(dgi, thr_name)) < 0) {
	/* thr field not found
	 */
	printf("Threshold field: %s not found\n", thr_name);
	return(-1);
    }
# ifdef NEW_ALLOC_SCHEME
    thr = (short *)dds->qdat_ptrs[fthr];
# else
    thr = (short *)((char *)dds->rdat[fthr]
			    + sizeof(struct paramdata_d));
# endif
    strncpy(dgi->dds->parm[fn]->threshold_field, "          ", 8);
    strncpy(dgi->dds->parm[fn]->threshold_field, thr_name, nchar);

# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn]
			 + sizeof(struct paramdata_d));
# endif
    zz = ss + nc;

    for(gg=0; gg < fgg && ss < zz; gg++,ss++,thr++) {
	*ss = bad;
    }


    switch(thr_type) {

    case THRESHOLD_LOW:
	dgi->dds->parm[fn]->threshold_value = thr_val1;
	scaled_thr1 = (int)DD_SCALE(thr_val1, dds->parm[fthr]->parameter_scale
			       , dds->parm[fthr]->parameter_bias);
	for(; ss < zz; gg++,ss++,thr++) {
	    if(*ss != bad && (*thr < scaled_thr1)) {
		*ss = bad;
	    }
	}
	break;

    case THRESHOLD_HIGH:
	dgi->dds->parm[fn]->threshold_value = thr_val2;
	scaled_thr2 = (int)DD_SCALE(thr_val2, dds->parm[fthr]->parameter_scale
			       , dds->parm[fthr]->parameter_bias);
	for(; ss < zz; gg++,ss++,thr++) {
	    if(*ss != bad && (*thr > scaled_thr2)) {
		*ss = bad;
	    }
	}
	break;

    case THRESHOLD_INSIDE:
	dgi->dds->parm[fn]->threshold_value = thr_val1;
	scaled_thr1 = (int)DD_SCALE(thr_val1, dds->parm[fthr]->parameter_scale
			       , dds->parm[fthr]->parameter_bias);
	scaled_thr2 = (int)DD_SCALE(thr_val2, dds->parm[fthr]->parameter_scale
			       , dds->parm[fthr]->parameter_bias);
	for(; ss < zz; gg++,ss++,thr++) {
	    if(*ss != bad && ((*thr >= scaled_thr1 &&
			       *thr <= scaled_thr2))) {
		*ss = bad;
	    }
	}
	break;

    case THRESHOLD_OUTSIDE:
	dgi->dds->parm[fn]->threshold_value = thr_val1;
	scaled_thr1 = (int)DD_SCALE(thr_val1, dds->parm[fthr]->parameter_scale
			       , dds->parm[fthr]->parameter_bias);
	scaled_thr2 = (int)DD_SCALE(thr_val2, dds->parm[fthr]->parameter_scale
			       , dds->parm[fthr]->parameter_bias);
	for(; ss < zz; gg++,ss++,thr++) {
	    if(*ss != bad && ((*thr < scaled_thr1 ||
					   *thr > scaled_thr2))) {
		*ss = bad;
	    }
	}
	break;

    default:
	break;
    }

    return(0);
}
/* c------------------------------------------------------------------------ */

int 
dd_legal_pname (char *name)
{
    /* parameter names can only contain upper case, lower case and
     * underscores for now
     */
    int ii;
    char *a=name;

    for(ii=0; ii < strlen(name); ii++,a++) {
	if(!(UPPER(*a) || LOWER(*a) || *a == '_'))
	      return(0);
    }
    if(ii == strlen(name))
	  return(1);
    else
	  return(0);
}
/* c------------------------------------------------------------------------ */

void 
dd_lidar_db (DGI_PTR dgi, int fnd, struct lidar_parameters *lps)
{
    /* generate a lidar db field
     */
    int ii, jj, nn, bad;
    int nc=dgi->clip_gate+1;
    struct cell_d *celv=dgi->dds->celvc;
    double d, dns, baseline, scale=10., bias=0;
    double end_range, log_end_range, ff=1000., rcp288=1./288.;
    short *ss, *zz;
    float *rng, *rlut, cell_spacing, ant_orientation;


    dgi->dds->parm[fnd]->parameter_scale = scale;
    dgi->dds->parm[fnd]->parameter_bias = bias;
    bad = dgi->dds->parm[fnd]->bad_data;
    ss = (short *)dgi->dds->qdat_ptrs[fnd]; /* original data is replaced by
					     * derived field */
    zz = ss +nc;
    baseline = lps->baseline ? lps->baseline : .5 * (*ss + (*(ss +1)));
    if(lps->fudge_factor) ff = lps->fudge_factor;
    ant_orientation = (FABS(dgi->dds->asib->rotation_angle - 180.) < .1)
      ? -1 : 1;
    cell_spacing = dgi->dds->celvc->dist_cells[1] -
      dgi->dds->celvc->dist_cells[0];
    /*
     * a rotation angle of 180 => pointing down
     */
    if(lps->max_cells < dgi->dds->celvc->number_cells ||
       ant_orientation != lps->ant_orientation ||
       lps->cell_spacing != cell_spacing) {

       lps->cell_spacing = cell_spacing;
       lps->ant_orientation = ant_orientation;
	/*
	 * rebuilt the range correction look up table
	 */
       if(lps->max_cells < dgi->dds->celvc->number_cells) {
	  nn = lps->max_cells = celv->number_cells;
	  if(lps->rng_cor_lut) free(lps->rng_cor_lut);
	  rlut = lps->rng_cor_lut = (float *)malloc(nn * sizeof(float));
	  memset(rlut, 0, nn * sizeof(float));
       }
       rlut = lps->rng_cor_lut;
       nn = lps->max_cells;
	rng = celv->dist_cells;
	if(!lps->end_range) {
	    end_range = celv->dist_cells[celv->number_cells -1];
	}
	else {
	    end_range = lps->end_range;
	}
	if(!lps->counts_per_db) {
	   lps->counts_per_db = 100.;
	}
       for(; nn--; rng++, rlut++) {
	  *rlut = 0;
	  if(*rng > 0) {
	     d = (288. - 0.0065 * (*rng) * ant_orientation) * rcp288;
	     dns = log10(d) * 4.256;
# ifdef obsolete
	     d = (*rng)/end_range;
	     *rlut =  20. * log10(d);
# else
	     *rlut =  20. * LOG10(M_TO_KM(*rng));
# endif
	     *rlut += dns;
	  }
       }
    }
    
    /*
     * now generate a db field
     */
    rlut = lps->rng_cor_lut;

    for(; ss < zz; ss++, rlut++) {
	if((d = ((double)(*ss) - baseline)) <= 0) {
	    *ss = bad;
	}
	else {
	    d /= lps->counts_per_db;
	    d = 10. * log10(d) + (*rlut);

	    if(fabs(d = DD_SCALE(d, scale, bias)) > 32767.) {
		*ss = bad;
	    }
	    else {
		*ss = (short)d;
	    }
	}
    }
}
/* c------------------------------------------------------------------------ */

int 
dd_new_param (DGI_PTR dgi, char *srs_name, char *dst_name)
{
    DDS_PTR dds=dgi->dds;
    int ii, fn, fns, fnd, size=0, mark;
    int ns=strlen(srs_name), ndst=strlen(dst_name);
    char *a=NULL, *b=NULL, *cc, tmp_name[16], dname[16];
    /*
     * find the source field
     */
    if((fns = dd_find_field(dgi, srs_name)) < 0) {
	/* source field not found
	printf("Source parameter %s not found\n", srs_name);
	 */
	return(-1);
    }
    a = dds->qdat_ptrs[fns];
    size = dds->sizeof_qdat[fns];
    strcpy( dname, dst_name );
    if( ndst > 8 )
	{ ndst = 8; *( dname +8 ) = '\0'; }
    /*
     * deal with the destination field (it may already be there)
     */
    if((fnd = dd_find_field(dgi, dname)) < 0 ) {

	for(ii=0; ii < MAX_PARMS; ii++) {
	    if( dds->field_present[ii])
		{ continue; }	/* already checked with dd_find_field() */
	    cc = dds->parm[ii]->parameter_name;
	    str_terminate( tmp_name, cc, 8 );
	    if( !(*cc) )
		{ break; }	/* never been used */

	    if( !strcmp(dst_name, tmp_name)) {
		dds->field_present[ii] = YES;
		dgi->num_parms++;
		fnd = ii;
		break;
	    }
	}
    }
    if( fnd < 0 ) {	/* never existed--create it! */
	dgi->num_parms++;
	/* create the new parameter
	 */
	for(ii=0; ii < MAX_PARMS; ii++) {
	    if(!dgi->dds->field_present[ii]) {
		fnd = ii;
		dds->field_present[ii] = YES;
		break;
	    }
	}
	if(ii == MAX_PARMS) {
	   printf("\n**** Exhausted the space for %d fields! ****\n"
		  , ii);
	   exit(1);
	}
	memcpy((char *)dds->parm[fnd], (char *)dds->parm[fns]
	       , sizeof(struct parameter_d));
	strncpy(dds->parm[fnd]->parameter_name, dst_name, 8);
	memcpy(dds->qdat[fnd], dds->qdat[fns], sizeof(struct qparamdata_d));
	strncpy(dds->qdat[fnd]->pdata_name, dst_name, 8);
	dds->field_id_num[fnd] =
	      dd_return_id_num(dds->parm[fnd]->parameter_name);
	dgi->parm_type[fnd] = dgi->parm_type[fns];
	dds->number_cells[fnd] = dds->number_cells[fns];
	dds->field_present[fnd] = YES;
	dds->last_cell_num[fnd] = dds->last_cell_num[fns];
    }
    if(size > dds->sizeof_qdat[fnd]) {
       dd_alloc_data_field(dgi, fnd);
    }
    size = dds->sizeof_qdat[fnd]; /* can be smaller now */
    b = dds->qdat_ptrs[fnd];
    memcpy(b, a, size);	/* copy srs data to dst */
    return(fnd);
}  
/* c------------------------------------------------------------------------ */

void 
dd_add_bias (DGI_PTR dgi, int fn, double add_bias)
{
    DDS_PTR dds=dgi->dds;
    int nc=dgi->clip_gate+1;
    int bad=dds->parm[fn]->bad_data;
    int g, i, k=250, mark;
    float scale;
    float bias;
    short scaled_bias;
    short *ss, *zz;

    ss = (short *)dds->qdat_ptrs[fn];
    zz = ss +nc;
    scale = dds->parm[fn]->parameter_scale;
    bias = dds->parm[fn]->parameter_bias;
    scaled_bias = (short)DD_SCALE(add_bias, scale, bias);

    for(; ss < zz; ss++) {
      if(*ss != bad) {
	*ss += scaled_bias;
      }
    }
    mark = 0;
}
/* c------------------------------------------------------------------------ */

void 
dd_nix_air_motion (DGI_PTR dgi, int fn, double ac_vel, int center_on_zero)
{
    DDS_PTR dds=dgi->dds;
    int nc=dgi->clip_gate+1;
    int bad=dds->parm[fn]->bad_data;
    float nyqv;
    float scale;
    float bias;
    int g, i, k=250, mark;
    short scaled_nyqv, scaled_nyqi, scaled_ac_vel, adjust, vx;
    short *ss, *zz;

# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)((char *)dds->rdat[fn] + sizeof(struct paramdata_d));
# endif
    zz = ss +nc;
    nyqv = dds->radd->eff_unamb_vel;
    scale = dds->parm[fn]->parameter_scale;
    bias = dds->parm[fn]->parameter_bias;
    scaled_nyqv = (short)DD_SCALE(nyqv, scale, bias);
    scaled_nyqi = 2*scaled_nyqv;
    scaled_ac_vel = (short)DD_SCALE(ac_vel, scale, bias);

    if(center_on_zero) {
	adjust = scaled_ac_vel % scaled_nyqi;

	if(IABS(adjust) > scaled_nyqv) {
	    adjust = adjust > 0 ? adjust-scaled_nyqi : adjust+scaled_nyqi;
	}
	for(; ss < zz; ss++) {
	    if(*ss != bad) {
		vx = *ss + adjust;
		if(IABS(vx) > scaled_nyqv) {
		    vx = vx > 0 ? vx-scaled_nyqi : vx+scaled_nyqi;
		}
		*ss = vx;
	    }
	}
    }
    else {			/* just add the correction for ac motion */
	for(; ss < zz; ss++) {
	    if(*ss != bad) {
		*ss += scaled_ac_vel;
	    }
	}
    }
    mark = 0;
}
/* c------------------------------------------------------------------------ */

void 
dd_pct_stats (DGI_PTR dgi)
{
    /* routine to do some simple statics on a particular field
     * for printout with the catalog information
     */
# define        BELOW 0
# define        ABOVE 1
# define      BETWEEN 2
# define       BEYOND 3

    int ii, gg, ng, nt, fn, scaled_below, scaled_above, mis;
    DDS_PTR dds=dgi->dds;
    static int pct_stats_flag=YES, count=0, which;
    char *a, *b, string_space[256], *str_ptrs[32];
    static char parameter_name[12];
    static float  below=0, above=25.;
    short *ss;
    float f;

    if(!pct_stats_flag)
	  return;

    if(!count++) {
	pct_stats_flag = NO;
	if(a=get_tagged_string("PCT_STATS")) {
	    strcpy(string_space, a);
	    nt = dd_tokens(string_space, str_ptrs);
	    if(nt < 3) {
		which = ABOVE;
		strcpy(string_space, "DZ DBZ");
		for(fn=0; fn < dgi->num_parms; fn++) {
		    for(ii=0; ii < nt; ii++) {
			if(strstr(dds->parm[fn]->parameter_name
				  , str_ptrs[ii])) {
			    break;
			}
		    }
		    if(ii < nt) break;
		}
		if(fn >= dgi->num_parms)
		      return;
		strcpy(parameter_name, str_ptrs[ii]);
		pct_stats_flag = YES;
	    }
	    else if(strstr(str_ptrs[0], "BELOW") ||
		    strstr(str_ptrs[0], "ABOVE")) {
		if((ii=sscanf(str_ptrs[1], "%f", &f)) == 1) {
		    below = f;
		}
		which = strstr(str_ptrs[0], "BELOW") ? BELOW : ABOVE;
		strcpy(parameter_name, str_ptrs[2]);
		pct_stats_flag = YES;
	    }
	    else if(strstr(str_ptrs[0], "BETWEEN") ||
		    strstr(str_ptrs[0], "BEYOND")) {
		if(nt < 4) return;
		if((ii=sscanf(str_ptrs[1], "%f", &f)) == 1) {
		    below = f;
		}
		if((ii=sscanf(str_ptrs[2], "%f", &f)) == 1) {
		    above = f;
		}
		which = strstr(str_ptrs[0], "BETWEEN") ? BETWEEN : BEYOND;
		strcpy(parameter_name, str_ptrs[3]);
		pct_stats_flag = YES;
	    }
	    else
		  return;
	}
	else
	      return;
    }
    /* find the parameter
     */
    for(fn=0; fn < dgi->num_parms; fn++) {
	if(strstr(dds->parm[fn]->parameter_name, parameter_name)) {
	    break;
	}
    }
    if(fn >= dgi->num_parms)
	  return;

    dgi->num_good_cells = 0;
    scaled_below = (int)DD_SCALE(below, dds->parm[fn]->parameter_scale
			    , dds->parm[fn]->parameter_bias);
    scaled_above = (int)DD_SCALE(above, dds->parm[fn]->parameter_scale
			    , dds->parm[fn]->parameter_bias);
    mis = dds->parm[fn]->bad_data;
# ifdef NEW_ALLOC_SCHEME
    ss = (short *)dds->qdat_ptrs[fn];
# else
    ss = (short *)dds->rdat[fn] + sizeof(struct paramdata_d);
# endif
    ng = dds->celv->number_cells;

    if(which == BELOW) {
	for(gg=0; gg < ng; gg++, ss++) {
	    if(!(*ss == mis || *ss >= scaled_below))
		  dgi->num_good_cells++;
	}
    }
    else if(which == ABOVE) {
	for(gg=0; gg < ng; gg++, ss++) {
	    if(!(*ss == mis || *ss <= scaled_above))
		  dgi->num_good_cells++;
	}
    }
    else if(which == BETWEEN) {
	for(gg=0; gg < ng; gg++, ss++) {
	    if(!(*ss == mis || *ss <= scaled_below || *ss >= scaled_above))
		  dgi->num_good_cells++;
	}
    }
    else if(which == BEYOND) {
	for(gg=0; gg < ng; gg++, ss++) {
	    if(!(*ss == mis || (*ss > scaled_below && *ss < scaled_above)))
		  dgi->num_good_cells++;
	}
    }
}
/* c------------------------------------------------------------------------ */

int 
dd_thr_parms (char *a, char *name, float *val)
{
    /* routine to extract the name and value from the string
     * this routines assumes it's ok to put nulls in the source string
     */
    char string_space[256], *str_ptrs[32];
    double d;
    int ii, nt;

    strcpy(string_space, a);
    if(!(nt = dd_tokens(string_space, str_ptrs))) {
	return(nt);
    }

    if(!dd_legal_pname(str_ptrs[0])) {
	/* assume we will use the default name and this is a value */
	sscanf(str_ptrs[0], "%f", val);
	return(nt);
    }
    strcpy(name, str_ptrs[0]);
    if(nt < 2)
	  return(nt);		/* just a name with no value */

    sscanf(str_ptrs[1], "%f", val);
    if(nt < 3)
	  return(nt);

    sscanf(str_ptrs[2], "%f", val+1);
    return(nt);
}
/* c------------------------------------------------------------------------ */

int dder_lidar_opts(struct derived_fields *derfs, char *opts, char *dst_flds[], int num_dst)
{
    int ii, jj, nt, ndx;
    char *aa=opts, *bb=opts, *cc, *pp;
    short *ss;
    char string_space[256], *str_ptrs[32];
    float f;
    struct lidar_parameters *lps;

    strcpy(string_space, opts);
    nt = dd_tokenz(string_space, str_ptrs, " ,;\t\n");
    if(nt < 3)
	  return(0);

    for(ii=0; ii < nt; ii += 3) { /* process each ordered triple */

	aa = str_ptrs[ii]; bb = str_ptrs[ii+2];
	lps = NULL;

	if(!strncmp(aa, "RED", 3)) {
	    lps = derfs->RDB_lps;
	}
	else if(!strncmp(aa, "GREEN", 3)) {
	    lps = derfs->GDB_lps;
	}
	else {
	    continue;
	}
	if(!lps)
	      continue;

	if(strstr(aa, "BASEL")) {
	    if(sscanf(bb, "%f", &f) == 1) {
		lps->baseline = f;
		printf("%s baseline: %.2f\n", aa, f);
	    }
	}
	else if(strstr(aa, "COUNTS")) {
	    if(sscanf(bb, "%f", &f) == 1) {
		lps->counts_per_db = f;
		printf("%s counts per db: %.2f\n", aa, f);
	    }
	}
	else if(strstr(aa, "END_R")) {
	    if(sscanf(bb, "%f", &f) == 1) {
		lps->end_range = KM_TO_M(f);
		printf("%s end range: %.2f\n", aa, f);
	    }
	}
	else if(strstr(aa, "FUDGE")) {
	    if(sscanf(bb, "%f", &f) == 1) {
		lps->fudge_factor = f;
		printf("%s fudge factor: %.2f\n", aa, f);
	    }
	}
    }
    return(1);

}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */


