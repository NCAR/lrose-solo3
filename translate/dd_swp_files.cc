/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

/*
 * This file contains the following routines
 * 
 * dd_dump_headers
 * dd_dump_ray
 * dd_flush
 * dd_insert_headers
 * dd_new_vol
 * 
 */
#include <iostream>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dd_time.h>
#include <Parameter.h>
#include <Correction.h>
#include "dd_stats.h"
#include "nssl_mrd.hh"
#include "dorade_ncdf.hh"
#include "swp_file_acc.hh"
#include "dd_swp_files.hh"
#include "dd_time.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "dd_catalog.hh"
#include "dorade_tape.hh"
//#define MAXCVGATES 1500


static int sizeof_prev_file=0;
static int unique_vol_id = NO;

/* c------------------------------------------------------------------------ */

void 
dd_set_unique_vol_id (void)
{
   unique_vol_id = YES;
}

/* c------------------------------------------------------------------------ */

int 
dd_sizeof_prev_file (void)
{
  return( sizeof_prev_file );
}

/* c------------------------------------------------------------------------ */

void 
dd_dump_headers (struct dd_general_info *dgi)
{
    dd_new_vol(dgi);
}
/* c------------------------------------------------------------------------ */

void 
dd_dump_ray (struct dd_general_info *dgi)
{
    char *aa, *b, *c;
    int bytes, pn, n, num_short, rlen=0, mark, ncopy;
    int nc=dgi->dds->celv->number_cells;
    static int count=0, trip=3127;
    struct sweepinfo_d *swib=dgi->dds->swib;
    struct radar_d *radd=dgi->dds->radd;
    struct paramdata_d *rdat;
    struct qparamdata_d *qdat;
    struct dd_input_sweepfiles_v3 *dis;

    /* c...mark */
    dis = dd_return_sweepfile_structs_v3();

    if(count >= trip) {
	mark = 0;
    }
    if(dgi->new_sweep) {
	dd_insert_headers(dgi);
    }
    c = (char *)dgi->dd_buf + dgi->sizeof_dd_buf;
    dgi->dds->ryib->sweep_num = dgi->dds->swib->sweep_num;
    swib->num_rays++;

    switch(radd->scan_mode) {
     case DD_SCAN_MODE_AIR:
       swib->stop_angle = dd_rotation_angle(dgi);
       break;
     case DD_SCAN_MODE_RHI:
       swib->stop_angle = dd_elevation_angle(dgi);
       break;
    case DD_SCAN_MODE_TAR:
       swib->start_angle = radd->radar_type != DD_RADAR_TYPE_GROUND ?
	 dd_rotation_angle(dgi) : dd_azimuth_angle(dgi);
	break;
     default:
       swib->stop_angle = dd_azimuth_angle(dgi);
       break;
    }
    dgi->dds->sswb->stop_time = (int32_t) dgi->time;
    c = (char *)dgi->dd_buf + dgi->sizeof_dd_buf;

    memcpy(c, (char *)dgi->dds->ryib, sizeof(struct ray_i));
    c += sizeof(struct ray_i);
    rlen = sizeof(struct ray_i);

    memcpy(c, (char *)dgi->dds->asib, sizeof(struct platform_i));
    c += sizeof(struct platform_i);
    rlen += sizeof(struct platform_i);


    for(pn=0; pn < MAX_PARMS; pn++) {
	if(!dgi->dds->field_present[pn])
	      continue;
	count++;
	if(count >= trip) {
	    mark = 0;
	}
	qdat = (struct qparamdata_d *)c;
	ncopy = *dgi->dds->qdat[pn]->pdata_desc == 'R'
	      ? sizeof(struct paramdata_d) : sizeof(struct qparamdata_d);
	memcpy(c, (char *)dgi->dds->qdat[pn], ncopy);
	c += ncopy;
	rlen += ncopy;

	if(dgi->compression_scheme == NO_COMPRESSION) {
	    bytes = dgi->dds->parm[pn]->binary_format == DD_8_BITS
		  ? nc : LONGS_TO_BYTES(SHORTS_TO_LONGS(nc));
	    memcpy(c, dgi->dds->qdat_ptrs[pn], bytes);
	}
	else {
	    num_short = dgi->dds->parm[pn]->binary_format == DD_8_BITS
		  ? BYTES_TO_SHORTS(nc) : nc;

	    b = dgi->dds->qdat_ptrs[pn];

	    num_short = HRD_recompress
		  ((unsigned short *)b, (unsigned short *)c
		   , dgi->dds->parm[pn]->bad_data, num_short
		   , (int)dgi->compression_scheme);
	    bytes = LONGS_TO_BYTES(SHORTS_TO_LONGS(num_short));
	}
	qdat->pdata_length = ncopy + bytes;
	c += bytes;
	rlen += bytes;
    }
    dgi->ray_que->disk_offset = dgi->dds->sswb->sizeof_file;
    dgi->ray_que->sizeof_ray = rlen;
    dgi->dds->sswb->sizeof_file += rlen;
    dgi->swp_que->sweep_file_size += rlen;

    dd_rotang_table(dgi, 0);
    if( dgi->sizeof_dd_buf +rlen >= MAX_REC_SIZE ) {
	dd_write( dgi->sweep_fid, dgi->dd_buf, dgi->sizeof_dd_buf );
	if(dgi->disk_output) {
	   /* this may be an overlap copy - memmove() is safe,
	    *  memcpy() may not be */
           memmove( dgi->dd_buf, dgi->dd_buf+dgi->sizeof_dd_buf, rlen );
	}
	dgi->sizeof_dd_buf = rlen;
    }
    else
	  dgi->sizeof_dd_buf += rlen;

    if(!dis->editing)
	  dd_catalog(dgi, (time_t)dgi->time, NO);
}
/* c------------------------------------------------------------------------ */

void 
dd_flush (int flush_radar_num)
{
    /*
     * dump out the partially filled buffer for the current
     * sweep and close the file
     */
    int ii, jj, nn;
    char str[256];
    struct dd_input_sweepfiles_v3 *dis;
    struct dd_general_info *dgi;
    struct dds_structs *dds;
    struct super_SWIB *sswb;
    struct sweepinfo_d *swib;
    struct generic_descriptor *gd;


    dis = dd_return_sweepfile_structs_v3();

    if( flush_radar_num == DD_FLUSH_ALL ) {
	ii = 0;
	jj = dd_num_radars();
    }
    else {
	ii = flush_radar_num;
	jj = ii+1;
    }

    for(; ii < jj; ii++ ) {
	if(dis->editing)
	      dgi = dd_window_dgi(ii);
	else 
	      dgi = dd_get_structs(ii);

	if(!dgi->sweep_fid)	/* file not open */
	{
	      continue;
	}
	
	dds = dgi->dds;	
	sswb = dds->sswb;
	swib = dds->swib;

	if( dgi->sizeof_dd_buf ) { /* dump buffer */
	    dd_write( dgi->sweep_fid, dgi->dd_buf, dgi->sizeof_dd_buf );
	    dgi->sizeof_dd_buf = 0;
	}
	if(sswb->sizeof_file > 0
	   && swib->num_rays >= dd_min_rays_per_sweep() &&
	   !dgi->ignore_this_sweep) {
	  
	    /* there really is some data
	     *
	     * a write NULL descriptor
	     * and update the sweep file
	     */
	    dd_write(dgi->sweep_fid, (char *)dds->NULL_d
		     , sizeof(struct null_d));
	    sswb->sizeof_file += sizeof(struct null_d);

	    sswb->num_key_tables = 2;
	    /* write the rotation angle table and reset it
	     */
	    sswb->key_table[NDX_ROT_ANG].offset = sswb->sizeof_file;
	    sswb->key_table[NDX_ROT_ANG].size = dgi->rat->sizeof_struct;
	    sswb->key_table[NDX_ROT_ANG].type = KEYED_BY_ROT_ANG;
	    dd_write(dgi->sweep_fid, (char *)dgi->rat
		     , dgi->rat->sizeof_struct);
	    sswb->sizeof_file += dgi->rat->sizeof_struct;
	    dd_rotang_table(dgi, RESET);
	    /*
	     * save the solo edit summary
	     */
	    if(dgi->sizeof_seds > 0) {
		gd = (struct generic_descriptor *)dgi->seds;
		/* save the solo edit summary
		 */
		sswb->key_table[NDX_SEDS].offset = sswb->sizeof_file;
		sswb->key_table[NDX_SEDS].type = SOLO_EDIT_SUMMARY;
		sswb->key_table[NDX_SEDS].size = gd->sizeof_struct;
		dd_write(dgi->sweep_fid, dgi->seds, gd->sizeof_struct);
		sswb->sizeof_file += gd->sizeof_struct;
	    }
	    
	    /* rewrite the SSWB and SWIB
	     */
	    dd_position( dgi->sweep_fid, 0 ); /* rewind */
	    dd_write( dgi->sweep_fid, (char *)sswb
		     , sizeof(struct super_SWIB));
	    dd_position( dgi->sweep_fid, dgi->offset_to_swib );
	    dd_write( dgi->sweep_fid, (char *)swib
		     , sizeof(struct sweepinfo_d));
	    dd_close( dgi->sweep_fid );
	    sizeof_prev_file = sswb->sizeof_file;
	    dgi->sweep_fid = 0;
	    if(dis->editing) {
	      slash_path(str, dgi->directory_name.c_str());
	      strcat(str, dgi->orig_sweep_file_name.c_str());
	      dd_unlink(str);
      
	    }
	    /*
	     * rename the sweepfile by removing the ".tmp" suffix
	     */
	    dd_rename_swp_file(dgi);
	}
	else if(dgi->sweep_fid) { /* too short...zap it */
	    dd_close( dgi->sweep_fid );
	    dgi->sweep_fid = 0;
	    dd_rotang_table(dgi, RESET);
	    slash_path(str, dgi->directory_name.c_str());
	    strcat(str, dgi->sweep_file_name.c_str());
	    dd_unlink(str);
	}

	if( flush_radar_num == DD_FLUSH_ALL ) {
	    dd_catalog(dgi, 0, YES );
	    produce_nssl_mrd(dgi, YES);
	    produce_ncdf( dgi, YES );
	}
    }
    if( flush_radar_num == DD_FLUSH_ALL ) {
	dd_tape(dgi, DD_FLUSH_ALL);
    }
}
/* c------------------------------------------------------------------------ */

void dd_insert_headers (struct dd_general_info *dgi)
{
    struct dds_structs *dds=dgi->dds;
    int ii, np, nn, bsize=0, ncomm, ssize;
    char *c;
    struct super_SWIB *sswb=dds->sswb;
    struct sweepinfo_d *swib=dds->swib;
    struct volume_d *vold=dds->vold, *voldo;
    struct radar_d *radd=dds->radd, *raddo;
    struct comment_d *comm=dds->comm, *commo;
    struct correction_d *cfac=dds->cfac, *cfaco;
    struct cell_d *celv=dds->celv, *celvo;
    struct parameter_d *parmo;
    struct dd_stats *dd_stats;
    struct dd_input_sweepfiles_v3 *dis;
    char scan_mode_mne[16];

    dis = dd_return_sweepfile_structs_v3();
    
    dd_file_name_ms("swp", dgi->sweep_time_stamp, dd_radar_name(dds),
		    dgi->version, dgi->sweep_file_name,
		    dgi->swp_que->segment_num);
    dgi->sweep_file_name += ".tmp"; /* this will be removed later */
    
    dd_scan_mode_mne( dgi->dds->radd->scan_mode, scan_mode_mne );
    dd_stats = dd_return_stats_ptr();
    if(!dis->editing)
	  printf("at %.1fMB f%d r%d %d %.1f_%s_v%d"
		 , dd_stats->MB
		 , dd_stats->file_count
		 , dd_stats->rec_count
		 , dd_stats->ray_count
		 , swib->fixed_angle
		 , scan_mode_mne
		 , dgi->dds->vold->volume_num
		 );

    dgi->sweep_fid = dd_open(dgi->directory_name, dgi->sweep_file_name);
    dgi->sizeof_dd_buf = 0;

    if(!dgi->dd_buf) {
	if(dgi->dd_buf = (char *)malloc(MAX_READ))
	      memset(dgi->dd_buf, 0, MAX_READ);
	else {
	    printf("Unable to allocate dgi->dd_buf\n");
	    exit(1);
	}
    }
    c = dgi->dd_buf;
    dgi->swp_que->num_parms = sswb->num_params = dgi->num_parms;
    /* reset size because of the difference on LE and BE machines */
    sswb->sizeof_struct = sizeof(struct super_SWIB);
    memcpy( c, (char *)sswb, sizeof(struct super_SWIB));
    c += sizeof(struct super_SWIB);
    bsize += sizeof(struct super_SWIB);

    memcpy(c, (char *)vold, sizeof(struct volume_d));
    voldo = (struct volume_d *)c;
    voldo->volume_des_length = sizeof(struct volume_d);
    c += sizeof(struct volume_d);
    bsize += sizeof(struct volume_d);

    if( dgi->sizeof_comments > 0 ) {
	memcpy(c, (char *)comm, sizeof(struct comment_d));
	commo = (struct comment_d *)c;
	commo->comment_des_length = sizeof(struct comment_d);
	bsize += sizeof(struct comment_d);
	c += sizeof(struct comment_d);
    }
    /* put out the size of the old descripter for now
     */
    ssize = sizeof(struct radar_d_v01);
    memcpy(c, (char *)radd, ssize);
    raddo = (struct radar_d *)c;
    raddo->radar_des_length = ssize;
    /* Compression scheme may have changed from input compression scheme */
    raddo->data_compress = dgi->compression_scheme;
    for(np=ii=0; ii < MAX_PARMS; ii++) {
	if(dgi->dds->field_present[ii])
	      np++;
    }
    raddo->num_parameter_des = np;
    raddo->total_num_des = np + 2;
    bsize += ssize;
    c += ssize;
    /*
     * put out the size of the old descripter for now
     */
    ssize = sizeof(struct parameter_d_v00);
    for(ii=0; ii < MAX_PARMS; ii++ ) {
	if(!dds->field_present[ii])
	      continue;
	memcpy(c, (char *)dds->parm[ii], ssize);
	parmo = (struct parameter_d *)c;
	parmo->parameter_des_length  = ssize;
	bsize += ssize;
	c += ssize;
    }
    /* cell vector */

    nn = (celv->number_cells > MAXCVGATES)
	? 12 + celv->number_cells * sizeof(float) : sizeof(struct cell_d);
    memcpy(c, (char *)celv, nn);
    celvo = (struct cell_d *)c;
    celvo->cell_des_len = nn;
    bsize += nn;
    c += nn;

    /* frib from ELDORA  */
    if (dds->frib) {
       nn = sizeof (*dds->frib);
       memcpy(c, (char *)dds->frib, nn);
       bsize += nn;
       c += nn;
    }

    /* cfac header */
    memcpy(c, (char *)cfac, sizeof(struct correction_d));
    cfaco = (struct correction_d *)c;
    cfaco->correction_des_length = sizeof(struct correction_d);
    bsize += sizeof(struct correction_d);
    c += sizeof(struct correction_d);

    swib->sweep_num = dis->editing ?
	  dgi->source_sweep_num : ++dgi->sweep_num;

    switch(radd->scan_mode) {
     case DD_SCAN_MODE_AIR:
       swib->start_angle = dd_rotation_angle(dgi);
       break;
     case DD_SCAN_MODE_RHI:
       swib->start_angle = dd_elevation_angle(dgi);
       break;
    case DD_SCAN_MODE_TAR:
       swib->start_angle = radd->radar_type != DD_RADAR_TYPE_GROUND ?
	 dd_rotation_angle(dgi) : dd_azimuth_angle(dgi);
	break;
     default:
       swib->start_angle = dd_azimuth_angle(dgi);
       break;
    }
    swib->num_rays = 0;
    dgi->offset_to_swib = bsize;

    memcpy( c, (char *)swib, sizeof(struct sweepinfo_d));
    c += sizeof(struct sweepinfo_d);
    bsize += sizeof(struct sweepinfo_d);

    if(ncomm = dd_return_num_comments()) {
	for(ii=0; ii < ncomm; ii++) {
	    if(comm = dd_return_comment(ii)) {
		memcpy(c, (char *)comm, comm->comment_des_length);
		c += comm->comment_des_length;
		bsize += comm->comment_des_length;
	    }
	}
	dd_reset_comments();
    }

    sswb->sizeof_file = bsize;
    dgi->sizeof_dd_buf += bsize;
}
/* c------------------------------------------------------------------------ */

void 
dd_new_vol (struct dd_general_info *dgi)
{
    int ii;
    struct dd_input_sweepfiles_v3 *dis;
    struct dds_structs *dds=dgi->dds;
    struct volume_d *vold=dds->vold;
    DD_TIME *dts=dgi->dds->dts;

    dis = dd_return_sweepfile_structs_v3();
    dgi->vol_count++;
    dgi->new_vol = YES;
    dgi->vol_time0 = dgi->time;
    dgi->volume_time_stamp = (int32_t) dgi->vol_time0;

    d_unstamp_time(dts);

    vold->year = dts->year;
    vold->month = dts->month;
    vold->day = dts->day;
    vold->data_set_hour = dts->hour;
    vold->data_set_minute = dts->minute;
    vold->data_set_second = dts->second;
    if (unique_vol_id) {
       vold->volume_num = 
	 dgi->vol_num = dts->hour * 60 + dts->minute;
    }
    else {
       dgi->vol_num++;
       vold->volume_num = dis->editing ? dgi->source_vol_num : dgi->vol_num;
    }
}
/* c------------------------------------------------------------------------ */


