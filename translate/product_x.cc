/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "product_x.hh"
#include "dorade_share.hh"
#include "ddb_common.hh"
#include <fcntl.h>
#include <dd_math.h>

//# include "dorade_headers.h"

# define PMODE 0666

struct px_control_struct {
    int omit[MAX_SENSORS];
    int fid[MAX_SENSORS];
    int sweep_num[MAX_SENSORS];
    int encountered[MAX_SENSORS];
    char *pbp;
    char *pb_lim;
    char pb[BLOCK_SIZE];
};

static int local_px_flag=YES;
static int num_px_radars=0, fid;
static struct px_control_struct *pxpx=NULL;
static float earth_radius=6366.8056;
static double trip_time=0;

/* c------------------------------------------------------------------------ */

//int dd_product_x (DGI_PTR dgi)  //Jul 26, 2011
void dd_product_x (DGI_PTR dgi)  //Jul 26, 2011 change to void
{
    DDS_PTR dds=dgi->dds;
    int i, mark, rn=dgi->radar_num;
    char *a, str[256];

    if(!local_px_flag)
	  return;
    
    if(!num_px_radars) {
	/* first time through
	 */
	local_px_flag = NO;
	
	if(a = get_tagged_string("OUTPUT_FLAGS")) {
	    if(strstr(a, "SPECIAL_DATA")) {
		local_px_flag = YES;
	    }
	    else
		  return;
	}
	else {
	    return;
	}
# ifdef obsolete
# endif
	trip_time = (int)(dgi->time +1);
	pxpx = (struct px_control_struct *)
	      malloc(sizeof(struct px_control_struct));
	memset((char *)pxpx, 0, sizeof(struct px_control_struct));
	pxpx->pbp = pxpx->pb;
	pxpx->pb_lim = pxpx->pb +BLOCK_SIZE -1;

	if(a = get_tagged_string("EARTH_RADIUS"))
	      earth_radius = atof(a);
	/*
	 * build a filename and open the file
	 */
	strcpy(str, dgi->directory_name.c_str());
	dd_file_name("xxx", (time_t)dgi->time
		     , dd_radar_name(dds)
		     , 0, str+strlen(str));
	if((fid = pxpx->fid[rn] = creat(str, PMODE)) < 0 ) {
	    printf("Error: %d opening file: %s\n", pxpx->fid[rn], str );
	    pxpx->omit[rn] = YES;
	}
	printf( " file %s  %d \n", str, pxpx->fid[rn]);
    }

    if(!pxpx->encountered[rn]) {
	/* first time for this radar
	 */
	pxpx->encountered[rn] = YES;
	pxpx->fid[rn] = fid;
	num_px_radars++;
	pxpx->omit[rn] = NO;
    }
# ifdef obsolete
    if(dgi->time >= trip_time ) {
	trip_time = (int)(dgi->time +time_inc);
	dd_px(dgi, pxpx->fid[rn], rn);
    }
# else
    dd_px(dgi, pxpx->fid[rn], rn);
# endif
    return;
}
/* c------------------------------------------------------------------------ */

int 
dd_px (DGI_PTR dgi, int fid, int rn)
{
    /* 
     * routine to generate a px file
     * How do you deal with more than one radar?
     * 
     */
    DDS_PTR dds=dgi->dds;
    DD_TIME *dts=dgi->dds->dts;
    static int count=0, trip=250;
    char *a, str[256], date[32];
    struct ray_i *ryib=dds->ryib;
    struct platform_i *asib=dds->asib;
    double d;
    int i, n=0, mark;


    if(!(count % trip)) {
	mark = 0;
    }
    count++;

    str[0] = '\0';
    if(pxpx->sweep_num[rn] != dgi->sweep_num) {
	sprintf(str,"vol:%3d sweep:%3d %s\n",
		dgi->vol_num,
		dgi->sweep_num,
		dgi->radar_name.c_str());
	n = strlen(str);
    }
    a = str +n;
    pxpx->sweep_num[rn] = dgi->sweep_num;
    dts->time_stamp = dgi->time;
    d_unstamp_time(dts);
# ifdef obsolete
    sprintf(a, "%3d %-9s%s%5.1f%6.1f%4.0f%4.0f%4.0f %8.4f %8.4f"
	    , ryib->sweep_num
	    , dgi->radar_name
	    , dts_print(dts)
	    , asib->pitch
	    , asib->roll
	    , asib->heading
	    , asib->rotation_angle
	    , DEGREES(dgi->dds->ra->rotation_angle)
	    , asib->latitude
	    , asib->longitude
	    );
# else
    sprintf(a, "%s %3d%6.1f%6.1f%6.1f%6.1f%6.2f %6.1f%6.1f%6.1f%6.1f%6.1f%6.1f"
	    , dts_print(dts)
	    , ryib->sweep_num
	    , asib->heading
	    , asib->drift_angle
	    , asib->roll
	    , asib->pitch
	    , asib->tilt
	    , asib->rotation_angle
	    , DEGREES(dgi->dds->ra->rotation_angle)
	    , ryib->azimuth
	    , DEGREES(dgi->dds->ra->azimuth)
	    , ryib->elevation
	    , DEGREES(dgi->dds->ra->elevation)
	    );
# endif
    n += strlen(a);
    a = str +n;
# ifdef obsolete
    if((ndx=dd_ndx_name(dgi, "DB")) >= 0 ) {
	dd_givfld( dgi, ndx, g1, ng, refl, &bad_val);
	strcat(a, "\n");
	for(i=0; i < ng; i++) {
	    sprintf(a, " %.1f", refl[i]);
	    n += strlen(a);
	    a = str +n;
	}
    }
# endif
    strcat(a, "\n"); n++;
    n = strlen(str);
    mark = write(fid, str, n);
    if(pxpx->pbp+n > pxpx->pb_lim)
	  pxpx->pbp = pxpx->pb;
    strcpy(pxpx->pbp, str);
    pxpx->pbp += n;
    mark = 0;
}
/* c------------------------------------------------------------------------ */

