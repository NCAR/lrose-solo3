/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include <fcntl.h>
#include <dd_math.h>
#include <radar_angles.h>
#include "dorade_share.hh"
#include "gecho.hh"
#include "ddb_common.hh"
#include "dda_common.hh"

# define PMODE 0666

struct gecho_control_struct {
    int omit[MAX_SENSORS];
    int encountered[MAX_SENSORS];
    int fid[MAX_SENSORS];
    struct gecho_stats *gstats[MAX_SENSORS];
};

struct gecho_stats {
    int alt_count;
    float sum_alts;
    int in_sector_count;
    int gnd_intxn_count;
};

static struct gecho_control_struct *gcs=NULL;
static struct radar_angles *ra=NULL;
static char *gecho_refl=(char *)"DB DBZ DZ", *gecho_vel=(char *)"VU VE VG VH";
static char *gecho_ncp=(char *)"NCP NC";
static int num_refl_names, num_vel_names, num_ncp_names;
static char refl_names[128], *refl_ptrs[16], vel_names[128], *vel_ptrs[16];
static char ncp_names[128], *ncp_ptrs[16];
static int local_gecho_flag=YES, gecho_msl=NO;
static int num_gecho_radars=0;
static int first_10_only = NO;
static float earth_radius=6366.8056, min_rot_angle=100.;
static float max_rot_angle=260., reasonable_gecho=10.;
static int min_gates=11, min_gatex;

/* c------------------------------------------------------------------------ */

void 
dd_gecho (DGI_PTR dgi)
{
    int i, jj, mark, rn=dgi->radar_num;
    char *a, str[256], name[12];
    int nt;
    char string_space[256], *str_ptrs[32];

    if(!local_gecho_flag)
	  return;
    
    if(!num_gecho_radars) {
	/* first time through */

	local_gecho_flag = NO;
	if(dd_catalog_only_flag()) {
	    return;
	}
	if(a = get_tagged_string("OUTPUT_FLAGS")) {
	    if(strstr(a,"NO_GECHO")) {
		return;
	    }
	    if(strstr(a, "GECHO_DATA")) {
		local_gecho_flag = YES;
	    }
	    else
		  return;
	}
	else {
	    return;
	}
	gcs = (struct gecho_control_struct *)
	      malloc(sizeof(struct gecho_control_struct));
	memset((char *)gcs, 0, sizeof(struct gecho_control_struct));

	for(i=0; i < MAX_SENSORS; i++) {
	    gcs->gstats[i] = ( struct gecho_stats*)
		  malloc(sizeof(struct gecho_stats));
	    memset(gcs->gstats[i], 0, sizeof(struct gecho_stats));
	}
	ra = (struct radar_angles *)malloc(sizeof(struct radar_angles));
	
	if(a = get_tagged_string("GECHO_MSL"))
	      gecho_msl = YES;
	if(a = get_tagged_string("GECHO_REFL"))
	      gecho_refl = a;
	if(a = get_tagged_string("GECHO_VEL"))
	      gecho_vel = a;
	if(a = get_tagged_string("EARTH_RADIUS"))
	      earth_radius = atof(a);
	if(a = get_tagged_string("GECHO_MAX_ROT_ANGLE"))
	      max_rot_angle = atof(a);
	if(a = get_tagged_string("GECHO_MIN_ROT_ANGLE"))
	      min_rot_angle = atof(a);
	if(a = get_tagged_string("GECHO_MIN_GATES"))
	      min_gates = atoi(a);

	if(a = get_tagged_string("OPTIONS")) {
	    if( strstr( a, "GECHO_10" )) {
		first_10_only = YES;
	    }
	}

	min_gatex = (int)(1.0*min_gates);
	if(!(min_gatex & 1)) min_gatex++; /* make it odd */
	earth_radius *= 1000.;	/* meters! */
	strcpy(refl_names, gecho_refl);
	num_refl_names = dd_tokens(refl_names, refl_ptrs);
	strcpy(vel_names, gecho_vel);
	num_vel_names = dd_tokens(vel_names, vel_ptrs);
	strcpy(ncp_names, gecho_ncp);
	num_ncp_names = dd_tokens(ncp_names, ncp_ptrs);
    }

    if(!gcs->encountered[rn]) {
	/* first time for this radar
	 */
	gcs->encountered[rn] = YES;
	num_gecho_radars++;

	for(;;) {
	    if(dgi->dds->radd->scan_mode != DD_SCAN_MODE_AIR) {
		gcs->omit[rn] = YES;
		break;
	    }
	    gcs->omit[rn] = NO;
	    /* look for the reflectivity field
	     */
	    for(jj=0; jj < num_refl_names; jj++) {
		if(dd_ndx_name(dgi, refl_ptrs[jj]) >= 0)
		      break;
	    }
	    if(jj == num_refl_names) {
		printf("Requested gecho reflectivity field(s)");
		printf(" \"%s\" not present.\n", gecho_refl);
		gcs->omit[rn] = YES;
	    }
	    else {
		gecho_refl = refl_ptrs[jj];
	    }

	    for(jj=0; jj < num_vel_names; jj++) {
		if(dd_ndx_name(dgi, vel_ptrs[jj]) >= 0)
		      break;
	    }
	    if(jj == num_vel_names) {
		printf("Requested gecho velocity field(s)");
		printf(" \"%s\" not present.\n", gecho_vel);
		gcs->omit[rn] = YES;
	    }
	    else {
		gecho_vel = vel_ptrs[jj];
	    }

	    for(jj=0; jj < num_ncp_names; jj++) {
		if(dd_ndx_name(dgi, ncp_ptrs[jj]) >= 0)
		      break;
	    }
	    if(jj == num_ncp_names) {
		gcs->omit[rn] = YES;
	    }
	    else {
		gecho_ncp = ncp_ptrs[jj];
	    }

	    if(gcs->omit[rn])
		  break;
	    /*
	     * build a filename and open the file
	     */
	    str_terminate(name, dgi->radar_name.c_str(), 8);
	    strcpy(str, dgi->directory_name.c_str());
	    dd_file_name("gde", (time_t)dgi->time, name, 0, str+strlen(str));
	    if((gcs->fid[rn] = creat(str, PMODE)) < 0 ) {
		printf("Error: %d opening file: %s\n", gcs->fid[rn], str );
		gcs->omit[rn] = YES;
		break;
	    }
	    printf( " file %s  %d \n", str, gcs->fid[rn]);
	    break;
	}
    }

    if(gcs->omit[rn])
	  return;
    else
	  dd_gecho_intxn( dgi, gcs->fid[rn]
			 , min_rot_angle, max_rot_angle
			 , earth_radius, min_gates, min_gatex
			 , reasonable_gecho
			 , gecho_refl, gecho_vel, rn, gecho_ncp
			 );
    return;
}
/* c------------------------------------------------------------------------ */

void 
dd_gecho_intxn (DGI_PTR dgi, int fid, double min_rot_angle, double max_rot_angle, double earth_radius, int min_gates, int min_gatex, double reasonable_gecho, char *gecho_refl, char *gecho_vel, int radar_num, char *gecho_ncp)
{
    /* 
     * routine to generate a gecho file
     * How do you deal with more than one radar?
     * 
     */
    DDS_PTR dds=dgi->dds;
    static int count=0, trip=500;
    static float bigL=29.8;		/* distance between INS and antenna */

    struct cell_d *celv=dds->celvc; /* corrected cell vector */
    struct ray_i *ryib=dds->ryib;
    struct platform_i *asib=dds->asib;
    struct radar_angles *ra=dds->ra;
    struct prev_rays *prs=dgi->ray_que;
    struct gecho_stats *gstats = gcs->gstats[radar_num];

    float rotation_angle=asib->rotation_angle+asib->roll;
    float tilt_angle=asib->tilt;
    float tan_elev, ground_intersect, footprint, range1, range2;
    float range_val, max_ref = -MAX_FLOAT, bad_val, max_range;
    float alt, elev, min_range;
    float refl[MAXCVGATES];
    float vel[MAXCVGATES];
    float rng[MAXCVGATES];
    float ncp[MAXCVGATES];
    double d;
    double d2, dH=0, dP=0, dt, cosP, sinP, cosH, sinH;
    double cosPHI, cosLAM, sinPHI, sinLAM;
    double u, v, w, insitu_wind;

    int i, j, n, nx, nc, g1, g2, gx, ng, mark, ndx, slen;
    int last_cell = celv->number_cells-1, max_ref_gate;
    char *a, str[BLOCK_SIZE];

    if(!(count % trip)) {
	mark = 0;
    }
    count++;

    if(dgi->ray_quality & UNACCEPTABLE) {
	return;
    }
    rotation_angle = fmod((double)rotation_angle, (double)360.);

    if(!(count % 1111)) {
	printf("count: %d rn: %d", count, radar_num);
	printf(" in_sector: %d ", gstats->in_sector_count);
	printf(" in_range: %d ", gstats->gnd_intxn_count);
	d = gstats->alt_count ? gstats->sum_alts/gstats->alt_count : 0;
	printf(" Ave. Alt. %.3f ", d);
	printf("\n");
	gstats->alt_count = 0;
	gstats->sum_alts = 0;
    }
    a = str;

    if( !first_10_only ) {
      if( rotation_angle <= min_rot_angle ||
	  rotation_angle >= max_rot_angle )
	{ return; }
      dd_radar_angles(asib, dds->cfac, ra, dgi);
      elev = ra->elevation;
      tan_elev = tan(elev);
      min_range = celv->dist_cells[0];
      max_range = celv->dist_cells[last_cell];

      alt = gecho_msl ? asib->altitude_msl : asib->altitude_agl;
      gstats->sum_alts += alt;
      gstats->alt_count++;
      gstats->in_sector_count++;

      alt = (alt +dds->cfac->pressure_alt_corr)*1000.;

      ground_intersect = (-(alt)/sin(elev))*
	(1.+alt/(2.*earth_radius*tan_elev*tan_elev));

      if(ground_intersect > max_range || ground_intersect <= 0 )
	return;

      dd_range_gate( dgi, ground_intersect, &gx, &range_val);
      footprint = ground_intersect*RADIANS
	(dds->radd->vert_beam_width)/tan_elev;
      footprint = fabs((double)footprint);

      range2 = ground_intersect + (footprint*.5);
      if(range2 > max_range)
	return;
      range1 = range2 - footprint;
      /* get the corresponding gate numbers
     */
      dd_range_gate( dgi, range1, &g1, &range_val);
      dd_range_gate( dgi, range2, &g2, &range_val);

      ng = nx = g2 -g1 +1;
      if(nx < min_gatex ) {
	g2 = gx + min_gatex/2;
	if( g2 > last_cell )
	  return;
	g1 = gx - min_gatex/2;
	nx = min_gatex;
      }
      g1 = g1 < 2 ? 2 : g1;
      nx = g2 -g1 +1;

      if(ng < min_gates) ng = min_gates;
      /*
       * the assumption here is that g1 is much greater than 1!
     */
      if(!(ng & 1)) ng++;		/* make it an odd number */
      /*
     * grab the interesting gates
     */
      if((ndx=dd_ndx_name( dgi, gecho_refl )) >= 0)
	dd_givfld( dgi, ndx, g1, nx, refl, &bad_val);

      for(i=g1,j=0; i <= g2; i++,j++ ) {
	if(refl[j] != bad_val && refl[j] > max_ref) {
	  max_ref = refl[j];
	  max_ref_gate = i;
	}
      }
      if(max_ref < reasonable_gecho) {
	return;
      }
      if((g2=max_ref_gate+ng/2) > last_cell)
	return;

      gstats->gnd_intxn_count++;

      g1 = max_ref_gate -ng/2 < 2 ? 2 : max_ref_gate -ng/2;
      ng = g2 -g1 +1;
    }
    else {
      g1 = 1; ng = 10;
    }
    /*
     * now grab the really interesting gates for
     * reflectivity
     */
    if((ndx=dd_ndx_name( dgi, gecho_refl )) >= 0)
	  dd_givfld( dgi, ndx, g1, ng, refl, &bad_val);
    if((ndx = dd_ndx_name(dgi, gecho_vel)) >= 0 )
	  dd_givfld( dgi, ndx, g1, ng, vel, &bad_val);
    dd_rng_info(dgi, &g1, &ng, rng, &n);

    if((ndx = dd_ndx_name(dgi, gecho_ncp)) >= 0 )
	  dd_givfld( dgi, ndx, g1, ng, ncp, &bad_val);

    sprintf( a,
"%3d%7.1f%6.1f%6.1f%5.2f%7.2f%8.1f%8.1f%8.1f%7.2f%8.1f%8.1f%8.1f%8.1f%8.1f%8.1f\n"
	    , ng
	    , rotation_angle +dds->cfac->rot_angle_corr
	    , tilt_angle +dds->cfac->tilt_corr
	    , asib->drift_angle + dds->cfac->drift_corr
	    , asib->pitch + dds->cfac->pitch_corr
	    , asib->heading
	    , asib->ew_velocity
	    , asib->ns_velocity
	    , asib->vert_velocity
	    , alt*.001
	    , ground_intersect
	    , dds->cfac->rot_angle_corr
	    , dds->cfac->drift_corr
	    , dds->cfac->pitch_corr
	    , dds->cfac->pressure_alt_corr
	    , dds->cfac->ew_gndspd_corr
	    );

    nc = slen = strlen(a);
    a = str +slen;
    dH = (prs->dH +prs->last->dH +prs->last->last->dH);
    dP = (prs->dP +prs->last->dP +prs->last->last->dP);
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
		 -sinPHI*cosP)*(RADIANS(dP)/dt));

    u = asib->ew_horiz_wind;
    v = asib->ns_horiz_wind;
    w = asib->vert_wind != -999 ? asib->vert_wind : 0;
    insitu_wind =
	  cos(ra->elevation) * (u*sin(ra->azimuth) + v*cos(ra->azimuth))
		+ w*sin(ra->elevation);

    sprintf( a
 , "%6d%3d:%3d:%3d%8.4f%8.4f%8.4f%8.4f%8.4f%8.4f%8.4f%8.4f%8.4f%8.4f%8.4f%8.4f\n"
	    , (int)(celv->dist_cells[0])
	    , ryib->hour
	    , ryib->minute
	    , ryib->second
	    , asib->heading_change
	    , asib->pitch_change
	    , dH
	    , dP
	    , dt
	    , asib->vert_wind
	    , d2
	    , u
	    , v
	    , insitu_wind
	    , DEGREES(ra->azimuth)
	    , DEGREES(ra->elevation)
	    );

    nc = strlen(a);
    slen += nc;
    a += nc;

    for(i=g1,j=0; j < ng; i++, j++ ) {
	sprintf(a, "%6d%10.1f%10.2f%10.1f%10.3f\n", i, refl[j], vel[j], rng[j]
	    , ncp[j] );
	nc = strlen(a);
	slen += nc;
	a += nc;
    }
    mark = write(fid, str, slen);

}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

