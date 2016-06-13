/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <dd_math.h>
#include "input_limits.hh"
#include "eldb_dd.hh"
#include "dd_stats.h"
#include "dorade_share.hh"
#include "dd_der_flds.hh"


static float bigL=29.8;		/* distance between INS and antenna */

/* c------------------------------------------------------------------------ */

void 
eld_thr_flds (struct eldora_unique_info *eui)
{

    /*
     * create a thresholded reflectivity field and a velocity field
     * if they are not there
     * then copy/threshold data from the source fields into them
     */
    DGI_PTR dgi=eui->dgi;
    DDS_PTR dds=dgi->dds;
    int nc=dgi->clip_gate+1;

    static int a_speckle=A_SPECKLE;
    static int count=0, trippin=222;
    static int fgg=FIRST_GOOD_GATE;
    static short *sscr;
    int i, j, n, flag, tf, opts, vd;
    int mark, thr_bad;
    short *dst, thrv, *thrf, *aa, *ss, *tt, *zz;
    char *a;


    if(!count) {		
	/* first time through
	 */
	eui->ncp_threshold = NCP_THRESHOLD_VAL;
	if(a=get_tagged_string("NCP_THRESHOLD_VAL")) {
	    eui->ncp_threshold = atof(a);
	}
	printf( "NCP threshold: %f\n", eui->ncp_threshold);

	if((a=get_tagged_string("A_SPECKLE"))) {
	    if((i=atoi(a)) > 0)
		  a_speckle = i;
	}
	printf("A speckle is %d or less cells\n", a_speckle);

	if((a=get_tagged_string("FIRST_GOOD_GATE"))) {
	    if((i=atoi(a)) > 0)
		  fgg = i;
	}
	printf("First good gate is %d\n", fgg);

	/* c...mark */
	sscr = (short *)malloc(MAXCVGATES*sizeof(short));
    }

    count++;
    if( count >= trippin) {
	mark = 0;
    }

    /* threshold field */
    for(tf=0; tf < dgi->num_parms; tf++) {
	if(dds->field_id_num[tf] == NCP_ID_NUM)
	      break;
    }
    thrv = (short)DD_SCALE(eui->ncp_threshold, dds->parm[tf]->parameter_scale
		    , dds->parm[tf]->parameter_bias);
    thrf = (short *)((char *)dds->rdat[tf] + sizeof(struct paramdata_d));
    thr_bad = dds->parm[tf]->bad_data;

    for(i=0,tt=thrf,ss=sscr,zz=sscr+nc;
	i < fgg && ss < zz; i++,tt++) {
	/* flag the thresholded gates */
	*ss++ = 1;
    }
    for(j=0; ss < zz; ss++,tt++) {
	if(*tt < thrv) {
	    *ss = 1;
	}
	else {
	    *ss = 0;
	    j++;
	}
    }
    dgi->num_good_cells = j;

    if(eui->options & ELD_DESPECKLING) {
	/* now look for speckles
	 */
	for(ss=sscr; ss < zz;) {
	    for(; *ss  && ss < zz; ss++); /* find the next good gate */
	    if(ss == zz) break;
	    /* now count the number of good gates */
	    for(aa=ss,n=0; !(*ss) && ss < zz; ss++,n++);
	    if(ss == zz) break;
	    if(n <= a_speckle) {
		for(; aa < ss;)	/* zap em */
		      *aa++ = 1;
	    }
	}
    }
    /* create a field that can be thresholded and denotched
     */
    vd = dd_new_param(dgi, (char *)"VR", (char *)"VD");
    flag = dds->parm[vd]->bad_data;
    dst = ss = (short *)((char *)dds->rdat[vd] + sizeof(struct paramdata_d));
    thrf = sscr;
    for(zz=dst+nc; ss < zz; thrf++, ss++)
	  if(*thrf) *ss = flag;
    strncpy(dds->parm[vd]->threshold_field
		  , dds->parm[tf]->parameter_name, 8);
    dds->parm[vd]->threshold_value = eui->ncp_threshold;
	
    if(TOGACOARE_FIXES SET_IN eui->options) {
	opts = ELD_NOTCH_REMOVAL;
	// eld_tweak_vel(eui, vd, dst, fgg, opts);
    }
}
# ifdef obsolete
/* c------------------------------------------------------------------------ */

int 
eld_tweak_vel (
    struct eldora_unique_info *eui,
    int fn,
    short *dst,
    int fgg,
    int ok_options		/* field num and first good gate */
)
{
    /* remove platform motion and folding from velocity data
     * should have current version of radar angles at this point
     */
    DGI_PTR dgi=eui->dgi;
    DDS_PTR dds=dgi->dds;
    struct radar_angles *ra=dds->ra;
    struct platform_i *asib=dds->asib;
    struct prev_rays *prs=dgi->ray_que;
    double d;
    double insitu_wind, ac_vel, u, v, w, vert;
    float nyqv=dds->radd->eff_unamb_vel;
    float scale=dds->parm[fn]->parameter_scale;
    float bias=dds->parm[fn]->parameter_bias;
    double d2, dH=0, dP=0, dt, cosP, sinP, cosH, sinH;
    double cosPHI, cosLAM, sinPHI, sinLAM;
    int nc=dgi->clip_gate+1;
    int bad=dds->parm[fn]->bad_data;
	  
    char *a;
    int g, i, mark, off, vx;
    int adjust, a_fold, scaled_nyqv, scaled_nyqi;
    int scaled_wind, scaled_ac_vel;
    int small_shear, notch_shear, fold_shear, notch_max;

    short *aa, *ss, *zz;
    short *vs;

    float f, rcp_scale, nyqi;
    float f_notch_max, f_notch_shear, f_fold_shear;

    static int count=0, trippin=88, new_sweep[MAX_SENSORS];
    struct ve_que_val *this, *last;
    static struct ve_que_val *vqv;
    static short *sscr, **sptrs;
    static int qsize=VUFQ_SIZE;
    static int mbc=VUF_MIN_BAD_COUNT;


    /* c...mark */
    if(count >= trippin) {
	mark = 0;
    }
    /* the work is being done in 16-bit integer space
     * thus various constants are scaled and biased to work here
     */
    aa = dst +fgg;
    zz = dst +nc;
    nyqi = 2.*nyqv;
    f_notch_max = VUF_NOTCH_MAX;
    f_notch_shear = VUF_NOTCH_SHEAR;
    f_fold_shear = VUF_FOLD_SHEAR;

    if(!count++) {
	/*
	 * Initialization first time through
	 */
	if((a=get_tagged_string("OPTIONS"))) {
	    if(strstr(a, "NO_REVERSE")) {
		eui->options |= ELD_NO_REVERSE;
		printf("No reverse notch removal\n");
	    }
	    if(strstr(a, "INITIAL_UNF")) {
		eui->options |= ELD_INITIAL_UNFOLD;
		printf("Do initial unfold\n");
	    }
	    if(strstr(a, "AZ_CONT")) {
		eui->options |= ELD_AZ_CONTINUITY;
		printf("Azimuth continuity\n");
	    }
	}
	if((a=get_tagged_string("MAX_NOTCH_VELOCITY"))) {
	    f_notch_max = atof(a);
	}
	printf( "Notch max: %f\n", f_notch_max);
	if((a=get_tagged_string("NOTCH_SHEAR"))) {
	    f_notch_shear = atof(a);
	}
	printf( "Notch shear: %f\n", f_notch_shear);
	if((a=get_tagged_string("FOLD_SHEAR"))) {
	    f_fold_shear = atof(a);
	}
	printf( "Fold shear: %f\n", f_fold_shear);
	if((a=get_tagged_string("UNFOLD_QUE_SIZE"))) {
	    if((i=atoi(a)) >= 0)
		  qsize = i;
	}
	printf("Que size: %d\n", qsize);
	if((a=get_tagged_string("MIN_BAD_COUNT"))) {
	    if((i=atoi(a)) >= 0)
		  mbc = i;
	}
	printf("Min bad count: %d\n", mbc);
	if((a=get_tagged_string("INCLUDE_DH_DP"))) {
	    eui->options |= ELD_INCLUDE_DH_DP;
	    printf("Include dH/dt and dP/dt\n");
	}

	if((a=get_tagged_string("x"))) {
	}

	/* set up the que */
	for(i=0; i < qsize; i++) {
	    this = (struct ve_que_val *)malloc(sizeof(struct ve_que_val));
	    if(!i)
		  vqv = this;
	    else {
		this->last = last;
		last->next = this;
	    }
	    last = this;
	}
	this->next = vqv;
	vqv->last = this;

	sscr = (short *)malloc(MAXCVGATES*sizeof(short));
	sptrs = (short **)malloc(MAXCVGATES*sizeof(short *));

	/*
	 * End initialization
	 */
    }

    if(!dgi->ray_que->data[fn]) {
	if(!dgi->ray_que->last->data[fn]) {
	    /* allocate space for this field */
	    dgi->ray_que->data[fn] = (int32_t *)
		  malloc(MAXCVGATES*sizeof(int32_t));
	}
	else {
	    /* there will be only one scratch field per radar 
	     * keep using the same field
	     */
	    dgi->ray_que->data[fn] =
		  dgi->ray_que->last->data[fn];
	}
    }
    vs = (short *)dgi->ray_que->data[fn];

    if(dgi->swp_que->num_rays == 1)
	  new_sweep[dgi->radar_num] = YES;

    notch_max = DD_SCALE(f_notch_max, scale, bias);
    notch_shear = DD_SCALE(f_notch_shear, scale, bias);
    fold_shear = DD_SCALE(f_fold_shear, scale, bias);

    small_shear = .5*notch_shear;
    scaled_nyqv = DD_SCALE(nyqv, scale, bias);
    scaled_nyqi = 2*scaled_nyqv;
    rcp_scale = scale ? 1./scale : 1.;
    a_fold = qsize*fold_shear;

    if(!(ss = non_shear_segment(aa, zz, notch_shear
				, qsize, bad, FORWARD, &off))) {
	dgi->ray_quality |= UNACCEPTABLE;
    }

    if(eui->options AND ELD_NOTCH_REMOVAL AND ok_options) {
	/* remove the averaged velocities between folds
	 */
	eld_denotch(dgi, aa, zz, bad, mbc, notch_max, small_shear
		    , notch_shear, scaled_nyqv, FORWARD);
    }
    u = asib->ns_horiz_wind;
    v = asib->ew_horiz_wind;
    w = asib->vert_wind != -999 ? asib->vert_wind : 0;

    insitu_wind =
	  cos(ra->elevation) * (u*cos(ra->azimuth) + v*sin(ra->azimuth))
		+ w*sin(ra->elevation);
    scaled_wind = DD_SCALE(insitu_wind, scale, bias);

    vert =  asib->vert_velocity != -999 ? asib->vert_velocity : 0;
    d = sqrt((double)(SQ(asib->ew_velocity) + SQ(asib->ns_velocity)));
    d += dds->cfac->ew_gndspd_corr; /* klooge to correct ground speed */
    ac_vel = d*sin((double)ra->tilt) + vert*sin((double)ra->elevation);
    if((eui->options AND ELD_INCLUDE_DH_DP) &&
       !dgi->new_sweep && dds->swib->num_rays > 3) {
# ifdef obsolete
	dH = RADIANS(angdiff(dgi->ray_que->last->heading, asib->heading));
	dP = RADIANS(asib->pitch -dgi->ray_que->last->pitch);
	dt = dgi->time -dgi->ray_que->last->time;
# else
	dH = RADIANS(prs->dH +prs->last->dH +prs->last->last->dH);
	dP = RADIANS(prs->dP +prs->last->dP +prs->last->last->dP);
	dt = prs->time - prs->last->last->last->time;
# endif	
	cosP = cos((double)RADIANS(asib->pitch));
	sinP = sin((double)RADIANS(asib->pitch));
	cosH = cos((double)RADIANS(asib->heading));
	sinH = sin((double)RADIANS(asib->heading));
	cosLAM = cos((double)(PIOVR2-ra->azimuth));
	sinLAM = sin((double)(PIOVR2-ra->azimuth));
	cosPHI = cos((double)ra->elevation);
	sinPHI = sin((double)ra->elevation);
# ifdef obsolete
	d2 = bigL*
	      ((1.+cosP)*(cosPHI*cosLAM*cosH -cosPHI*sinLAM*sinH)*(dH/dt)
		-(sinP*(cosPHI*cosLAM*sinH +cosPHI*sinLAM*cosH)
		  -sinPHI*cosP)*(dP/dt));
# else
	d2 = bigL*
	      ((1.+cosP)*(cosPHI*cosLAM*cosH -cosPHI*sinLAM*sinH)*
	       (RADIANS(asib->heading_change))
	       -(sinP*(cosPHI*cosLAM*sinH +cosPHI*sinLAM*cosH)
		 -sinPHI*cosP)*(dP/dt));
# endif
	ac_vel -= d2;
    }
    scaled_ac_vel = DD_SCALE(ac_vel, scale, bias);

# ifdef obsolete
    if(rota > lima && rota < limb) {
	mark = 0;
    }
# endif
    /*
     * unfold the velocities
     */
    if((eui->options AND ELD_UNFOLDING AND ok_options)) {
	wlee_unfolding(dgi, vqv, aa, zz, bad, qsize
		       , scaled_nyqi, a_fold, scaled_wind, scaled_ac_vel);
	if((eui->options AND ELD_AZ_CONTINUITY)) {
	    if(new_sweep[dgi->radar_num]) {
		/* this is a new sweep, don't do anything */
		memcpy((char *)vs, (char *)dst
		       , (dgi->clip_gate+1)*sizeof(short));
	    }
	    else {
		eld_az_continuity(dgi, vqv, dst, zz, bad, vs
				  , fold_shear, scaled_nyqi, qsize, a_fold);
	    }
	}
    }    
    else if(eui->options AND ELD_AIR_MOTION AND ok_options) {
	adjust = scaled_ac_vel % scaled_nyqi;
	if(iabs(adjust) > scaled_nyqv) {
	    adjust = adjust > 0 ? adjust-scaled_nyqi : adjust+scaled_nyqi;
	}
	for(g=fgg,ss=aa; ss < zz; ss++,g++) {
	    if(*ss != bad) {
		vx = *ss +adjust;
		if(iabs(vx) > scaled_nyqv) {
		    vx = vx > 0 ? vx-scaled_nyqi : vx+scaled_nyqi;
		}
		*ss = vx;
	    }
	}
    }
 almost_the_end:

    new_sweep[dgi->radar_num] = NO; /* only need to flag the first good ray */

    return;
}
/* c------------------------------------------------------------------------ */

int 
eld_az_continuity (DGI_PTR dgi, struct ve_que_val *vqv, short *new_ray, short *zz, int bad, short *prev, int fold_shear, int scaled_nyqi, int qsize, int a_fold)
{
    /* loop through the data counting and marking the azimuthal shears
     * new ray starts at "aa" and old ray starts at "pp"
     */
    int i, n, ns, nv, nlast, nnew, off;
    int nc=dgi->dds->celv->number_cells;
    register int diff;
    short *aa, *pp, *ppend;
    float ratio;

    nlast = dgi->ray_que->last->clip_gate +1;
    nnew = dgi->clip_gate+1;
    n = nlast > nnew ? nnew : nlast;

    pp = non_shear_segment(prev, prev+n, fold_shear
			   , qsize, bad, FORWARD, &off);
    if(!pp) goto almost_the_end;
    ppend = prev+n;

    for(aa=new_ray,pp=prev,nv=ns=0; pp < ppend; pp++,aa++) {
	if( *aa != bad && *pp != bad) {
	    nv++;
	    diff = *aa - *pp;
	    if(iabs(diff) > fold_shear) {
		ns++;
	    }
	}
    }

    if((ratio = nv > 0 ? (float)ns/(float)nv : 0) > .5) {
	for(aa=new_ray,pp=prev; pp < ppend; aa++, pp++) {
	    if( *aa != bad && *pp != bad) {
		diff = *aa - *pp;
		if(iabs(diff) > fold_shear) {
		    *aa = diff < 0 ? *aa +scaled_nyqi : *aa -scaled_nyqi;
		}
	    }
	}
	if(n < nnew) {
	    eld_unfolding(dgi, vqv, aa-qsize, zz, bad, qsize
			  , scaled_nyqi, a_fold, FORWARD);
	}
    }

 almost_the_end:
    memcpy(prev, new_ray, nc*sizeof(short));
}
/* c------------------------------------------------------------------------ */

int 
eld_denotch2 (DGI_PTR dgi, short *aa, short *zz, int bad, int mbc, int notch_max, int small_shear, int notch_shear, int scaled_nyqv, int qsize)
{
    int i, mark, off, slip=qsize-2;
    short *bb, *cc, *ss;
    static int count=0, trippin=111, lims=34;
    struct platform_i *asib=dgi->dds->asib;
    float rota=asib->rotation_angle;
    struct ray_i *ryib=dgi->dds->ryib;
    static float lima=214., limb=223.;


    if(++count >= trippin) {
	mark = 0;
    }

    if(!(ss = non_shear_segment
	 (aa, zz, notch_shear, slip, bad, FORWARD, &off)))
	  return;

    if(rota > lima && rota < limb && ryib->second > lims) {
# ifdef obsolete
	cctype16(aa, ss+24);
# endif
	mark = 0;
    }
    /*
     * loop through the data removing notches
     * ss should always be pointing to the beginning of a
     * non_shear_segment
     */
    for(; ss < zz;) {
	bb = bad_segment(aa-1, ss-1, mbc, bad, BACKWARD);
	/* look backwards for notches */
	eld_denotch(dgi, bb, ss, bad, mbc, notch_max, small_shear
		    , notch_shear, scaled_nyqv, BACKWARD);
	/* now forwards */
	cc = bad_segment(ss, zz, mbc, bad, FORWARD);
	eld_denotch(dgi, ss, cc, bad, mbc, notch_max, small_shear
		    , notch_shear, scaled_nyqv, FORWARD);
	if(!(ss = non_shear_segment
	     (cc, zz, notch_shear, slip, bad, FORWARD, &off)))
	      return;
    }
    return;
}
/* c------------------------------------------------------------------------ */

int 
eld_denotch (DGI_PTR dgi, short *ss, short *zz, int bad, int mbc, int notch_max, int small_shear, int notch_shear, int scaled_nyqv, int dir)
  /*
   * routine to remove notches or false zeroes in the velocities
   * in 16-bit integer space
   *
   * "ss" points to the first cell of data
   * "zz" points to the last cell plus one
   * "bad" is the bad flag
   * "mbc" is the minimum bad count. A number of consecutive bad cell
   *     greater or equal to this count will terminate certain loops
   * "notch_max" a scaled integer that the absolute value of a notch value
   *     should not exceed 
   * "notch_shear" used as the first step in detecting a notch
   * "scaled_nyqv" is the scaled Nyquist velocity
   * "dir" indicates direction and requires BACKWARD to be defined or
   *     replaced
   * 
   */
{
    int g=0, nb, mark;
    short *ok, *anchor, *rr, *tt;
    register int diff;
    static int count=0, trippin=111;


    if(++count >= trippin) {
	mark = 0;
    }

    if(dir == BACKWARD) goto backwards_loop;

    /* skip over bad flagged data */
    for(nb=0; *ss == bad && ss < zz; ss++,g++,nb++);
    /*
     * loop through the data removing notches
     */
    for(; ss < zz;) {
	anchor = ss;
	/* to find the next notch, locate next shear point
	 */
	for(;;) {		
	    ok = ss++; g++;
	    /* skip over bad flagged data */
	    for(nb=0; *ss == bad && ss < zz; ss++,g++,nb++);
	    if(ss == zz || nb > mbc) break;
	    diff = *ok -*ss;
	    diff = diff < 0 ? -diff : diff;
	    if(diff > notch_shear) {
		if(diff > notch_shear && iabs(*ss) < notch_max) {
		    /* we appear to be in a notch
		     * find the end of the notch
		     */
		    for(tt=ss; tt < zz; tt++,g++) {
			for(nb=0; *tt == bad && tt < zz; tt++,g++,nb++);
			/* terminate on too many bad flags or a non-notch value */
			if(nb > mbc) break;
			if(iabs(*tt) > notch_max) break;
		    }
		    for(; ss < tt; ss++) {
			if(*ss != bad)
			      *ss += *ss < 0 ? scaled_nyqv : -scaled_nyqv;
		    }
		    break;
		}
		else if(iabs(*ok) < notch_max && iabs(*ss) > notch_max) {
		    /* we appear to be at the end of a previously
		     * undetected notch
		     */
		    for(rr=ok; rr >= anchor; rr--) {
			if(*rr != bad) {
			    diff = *ss - *rr;
			    if(iabs(diff) > notch_shear &&
			       iabs(*rr) < notch_max)
				  *rr += *rr < 0 ? scaled_nyqv : -scaled_nyqv;
			    else
				  break;
			}
		    }
		    break;
		}
	    }
	}
	mark = 0;
    }
    return;

 backwards_loop:

    /* skip over bad flagged data */
    for(nb=0; *zz == bad && ss < zz; zz--,g++,nb++);

    for(; ss < zz;) {
	anchor = zz;
	/* to find the next notch, locate next shear point
	 */
	for(;;) {		
	    ok = zz--; g++;
	    /* skip over bad flagged data */
	    for(nb=0; *zz == bad && ss < zz; zz--,g++,nb++);
	    if(ss == zz || nb > mbc) break;
	    diff = *ok -*zz;
	    diff = diff < 0 ? -diff : diff;
	    if(diff > notch_shear) {
		if(diff > notch_shear && iabs(*zz) < notch_max) {
		    /* we appear to be in a notch
		     * find the end of the notch
		     */
		    for(tt=zz; ss < tt; tt--,g++) {
			for(nb=0; *tt == bad && tt < zz; tt--,g++,nb++);
			/* terminate on too many bad flags or a non-notch value */
			if(nb > mbc) break;
			if(iabs(*tt) > notch_max) break;
		    }
		    for(; zz > tt; zz--) {
			if(*zz != bad)
			      *zz += *zz < 0 ? scaled_nyqv : -scaled_nyqv;
		    }
		    break;
		}
		else if(iabs(*ok) < notch_max && iabs(*zz) > notch_max) {
		    /* we appear to be at the end of a previously
		     * undetected notch
		     */
		    for(rr=ok; rr <= anchor; rr++) {
			if(*rr != bad) {
			    diff = *zz - *rr;
			    if(iabs(diff) > notch_shear &&
			       iabs(*rr) < notch_max)
				  *rr += *rr < 0 ? scaled_nyqv : -scaled_nyqv;
			    else
				  break;
			}
		    }
		    break;
		}
	    }
	}
	mark = 0;
    }
    return;
}
/* c------------------------------------------------------------------------ */

short *
non_shear_segment (short *aa, short *zz, int shear, int num, int bad, int dir, int *offs)
{
    /* try to find a run of "num" points where the difference
     * between adjacent points is less than shear
     * this is assummed to be 16-bit data
     */
    register int diff;
    short *rr, *ss, *tt;
    int n;

    *offs = 0;

    if(dir == FORWARD) {	/* forward */
	for(; aa < zz;) {
	    for(n=0; *aa == bad && aa < zz; aa++,n++); /* skip bad data */
	    if(aa == zz)
		  return(NULL);
	    *offs += n;
	    for(n=0,rr=ss=aa++,tt=ss+num; aa < zz; aa++,ss++,n++) {
		if(aa == tt || *aa == bad) break;
		diff = *aa - *ss;
		if(iabs(diff) > shear) break;
	    }
	    if(aa == zz)
		  return(NULL);
	    if(aa == tt) {
		return(rr);
	    }
	    *offs += n;
	}
	return(NULL);
    }
    else {			/* backward */
	for(; aa < zz;) {
	    for(n=0; *zz == bad && aa < zz; zz--,n++); /* skip bad data */
	    if(aa == zz)
		  return(NULL);
	    *offs += n;
	    for(n=0,rr=ss=zz--,tt=ss-num; aa < zz; zz--,ss--,n++) {
		if(zz == tt || *zz == bad) break;
		diff = *zz - *ss;
		if(iabs(diff) > shear) break;
	    }
	    if(aa == zz)
		  return(NULL);
	    if(zz == tt) {
		return(rr);
	    }
	    *offs += n;
	}
    }
    return(NULL);
}
/* c------------------------------------------------------------------------ */

short *
bad_segment (short *aa, short *zz, int num, int bad, int dir)
{
    /* try to find a run of "num" consecutive bad vals
     * this is assummed to be 16-bit data
     */
    short *rr=aa, *ss, *tt=zz;
    int nb;

    if(dir == FORWARD) {	/* forward */
	for(; rr < zz;) {
	    for(nb=0,ss=rr; *ss == bad && nb < num && ss < zz; ss++,nb++);
	    if(nb >= num)
		  return(rr);
	    if(ss == zz)
		  break;
	    for(rr=ss+1; *rr != bad && rr < zz; rr++);
	    if(rr == zz)
		  break;
	}
	return(zz);
    }
    else {			/* backward */
	for(; aa < tt;) {
	    for(nb=0,ss=tt; *ss == bad && nb < num && aa < ss; ss--,nb++);
	    if(nb >= num)
		  return(tt);
	    if(ss == aa)
		  break;
	    for(tt=ss-1; *tt != bad && aa < tt; tt--);
	    if(tt == aa)
		  break;
	}
	return(aa);
    }
}
/* c------------------------------------------------------------------------ */

int 
wlee_unfolding (DGI_PTR dgi, struct ve_que_val *vqv, short *aa, short *zz, int bad, int qsize, int scaled_nyqi, int a_fold, int scaled_wind, int scaled_ac_vel)
{
    struct ve_que_val *this;
    int i, g, nb, fold_count, sum, mark, vx;
    short *ss;
    double folds, rcp_nyqi=1./((float)scaled_nyqi);
    double rcp_qsize=1./(float)qsize;


    for(sum=0,this=vqv,i=0; i < qsize; i++) { /* load up the que */
	    this->vel = scaled_wind;
	    sum += scaled_wind;
	    this = this->next;
    }

    /* do the unfolding
     */
    for(g=qsize,ss=aa; ss < zz; g++) {
	for(nb=0; *ss == bad && ss < zz; ss++,nb++,g++);
	if(ss == zz) break;
	vx = *ss;

	folds = ((rcp_qsize*sum -vx)-scaled_ac_vel)*rcp_nyqi;
	if(fabs(folds) > .0001) {
	    fold_count = folds < 0 ? folds -0.5 : folds +0.5;
	}
	else
	      fold_count = 0;

	sum -= this->vel;
	vx += fold_count*scaled_nyqi +scaled_ac_vel;
	this->vel = vx;
	sum += vx;
	*ss++ = vx;
	this = this->next;
    }
    return;
}
/* c------------------------------------------------------------------------ */

int 
eld_unfolding (DGI_PTR dgi, struct ve_que_val *vqv, short *aa, short *zz, int bad, int qsize, int scaled_nyqi, int a_fold, int dir)
{
    struct ve_que_val *this;
    int i, g, nb, fold_count, adjust, vx, sum, mark;
    register int diff;
    short *ss;

    if(dir == BACKWARD) goto backwards_loop;

    if(aa+qsize >= zz) return;

    for(sum=0,this=vqv,ss=aa,i=0; i < qsize; ss++,i++) { /* load up the que */
	    this->vel = *ss;
	    sum += *ss;
	    this = this->next;
    }
    fold_count = 0;
    adjust = fold_count*scaled_nyqi;

    for(g=qsize,ss=aa+qsize; ss < zz;) {
	for(; ss < zz;) {	/* find a possible fold */
	    for(nb=0; *ss == bad && ss < zz; ss++,nb++,g++);
	    if(ss == zz) break;
	    if(nb > 11) {
		mark = 0;
	    }
	    vx = *ss;
	    diff = sum -qsize*(vx +adjust);
	    if(iabs(diff) > a_fold) {
		fold_count = diff < 0 ? fold_count-1 : fold_count+1;
		adjust = fold_count*scaled_nyqi;
		if(iabs(fold_count) > 1) {
		    mark = 0;
		}
	    }
	    sum -= this->vel;
	    vx += adjust;
	    this->vel = vx;
	    sum += vx;
	    this = this->next;
	    *ss++ = vx;
	}
	if(ss == zz) break;
    }
    return;

 backwards_loop:

    if(zz-qsize <= aa) return;

    for(sum=0,this=vqv,ss=zz,i=0; i < qsize; ss--,i++) { /* load up the que */
	    this->vel = *ss;
	    sum += *ss;
	    this = this->next;
    }
    fold_count = 0;
    adjust = fold_count*scaled_nyqi;

    for(g=qsize,ss=zz-qsize; ss > aa;) {
	for(; ss > aa;) {	/* find a possible fold */
	    for(nb=0; *ss == bad && ss > aa; ss--,nb++,g++);
	    if(ss == aa) break;
	    if(nb > 11) {
		mark = 0;
	    }
	    vx = *ss;
	    diff = sum -qsize*(vx +adjust);
	    if(iabs(diff) > a_fold) {
		fold_count = diff < 0 ? fold_count-1 : fold_count+1;
		adjust = fold_count*scaled_nyqi;
		if(iabs(fold_count) > 1) {
		    mark = 0;
		}
	    }
	    sum -= this->vel;
	    vx += adjust;
	    this->vel = vx;
	    sum += vx;
	    this = this->next;
	    *ss-- = vx;
	}
	if(ss == aa) break;
    }
    return;
}
/* c------------------------------------------------------------------------ */

short *
non_notch_segment (short *aa, short *zz, int shear, int num, int bad, int dir, int min_val, int *offs)
{
    /* try to find a run of "num" points where the difference
     * between adjacent points is less than shear
     * this is assummed to be 16-bit data
     */
    register int diff;
    short *rr, *ss, *tt;
    int n;

    *offs = 0;

    if(dir == FORWARD) {	/* forward */
	for(; aa < zz;) {
	    for(n=0; *aa == bad && aa < zz; aa++,n++); /* skip bad data */
	    if(aa == zz)
		  return(NULL);
	    *offs += n;
	    for(n=0,rr=ss=aa++,tt=ss+num; aa < zz; aa++,ss++,n++) {
		if(aa == tt || *aa == bad) break;
		diff = *aa - *ss;
		if(iabs(diff) > shear || iabs(*ss) < min_val)
		      break;
	    }
	    if(aa == zz)
		  return(NULL);
	    if(aa == tt) {
		return(rr);
	    }
	    *offs += n;
	}
	return(NULL);
    }
    else {			/* backward */
	for(; aa < zz;) {
	    for(n=0; *zz == bad && aa < zz; zz--,n++); /* skip bad data */
	    if(aa == zz)
		  return(NULL);
	    *offs += n;
	    for(n=0,rr=ss=zz--,tt=ss-num; aa < zz; zz--,ss--,n++) {
		if(zz == tt || *zz == bad) break;
		diff = *zz - *ss;
		if(iabs(diff) > shear || iabs(*ss) < min_val)
		      break;
	    }
	    if(aa == zz)
		  return(NULL);
	    if(zz == tt) {
		return(rr);
	    }
	    *offs += n;
	}
    }
    return(NULL);
}
/* c------------------------------------------------------------------------ */

int 
eld_mark_shear (short *aa, short *zz, short *ss, int shear, int bad)
{
    /* move between aa and zz and mark and count shear points */
    int i, n, sval, mark;
    register int diff;
    short *rr;
    

    for(; *aa == bad && aa < zz; aa++,*ss++=0);
    if(aa >= zz) return(0);

    sval = *aa++;
    *ss = 0;
    rr = ss++;
    
    for(n=0,zz--; aa < zz; aa++, ss++) {
	if(*aa != bad) {
	    diff = *aa -sval;
	    mark = iabs(diff);
	    if(iabs(diff) > shear) {
		*rr = diff < 0 ? -1: 1;
		n++;
	    }
	    else
		  *ss = 0;
	    rr = ss;
	    sval = *aa;
	}
	else
	      *ss = 0;
    }
    return(n);
}
/*c----------------------------------------------------------------------*/

int 
cctype16 (	/* type as 16 bit integers */
    short *aa,
    short *zz
)
{
    int r=0, i, s;
    
    for(s=0; aa < zz; r++,aa++) {
	if( s == 0 ) {
	    printf("%5d)", r+1);	/* new line label */
	    s = 6;
	}
	printf( "%6d", *aa);
	s += 6;
	
	if( s >= 66 ) {
	    printf( "\n" );	/* start a new line */
	    s = 0;
	}
    }
    if( s > 0 ) {
	printf( "\n" );
    }
}
# endif
/* c------------------------------------------------------------------------ */
