/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NEW_ALLOC_SCHEME
# define RESET_FOR_VOL_HEADERS -1
# define WRITE_VOL_HEADERS 0

/*
 * This file contains the following routines
 * 
 * 
 * ddout_ini
 * ddout_headers_dump
 * ddout_loop
 * ddout_nab_vol_stuff
 * ddout_new_swp
 * ddout_next_ray
 * ddout_really
 * ddout_sweep_dump
 * ddout_sweep_list
 * ddout_sweeps_loop
 * ddout_vol_data
 * ddout_vol_detector
 * ddout_vols_loop
 * ddout_write
 * 
 * 
 */

#include <iostream>
#include <LittleEndian.hh>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
///#include <iostream>
#include <fstream>
#include <fcntl.h>
#include <dd_files.h>
#include "dd_files.hh"
#include "input_limits.hh"
#include "dd_stats.h"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "swp_file_acc.hh"
#include "dorade_share.hh"
#include "ddout.hh"
#include <Qdata.h>
#include <Sweep.h>
#include <Pdata.h>
#include <RadarDesc.h>
#include <Volume.h>
#include "dd_time.hh"
#include "dd_crackers.hh"

# define PMODE 0666
# define TMP_BUF_SIZE MAX_REC_SIZE

static struct dd_input_sweepfiles_v3 *dis;
static int sweep_interleave=YES;
static double reference_time=0;	/* start of last vol written */
static double new_vol_time=0;
static double ddout_len=0;
static int ddout_fid;
static int time_span=240;
static int min_vol_et=20;
static int sweeps_since_headers=999;
static int IO_type=FB_IO;
static int Really_write_it=YES;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats;
static char* tmp_buf=NULL, *swap_buf;
static int ddout_num_parms[MAX_SENSORS], ddout_sweep_num[MAX_SENSORS];
static int ddout_vol_num[MAX_SENSORS];
static int time_defined_volumes=NO;
static int last_time_span=-1;
static int real_rec_count = 0;

/* c------------------------------------------------------------------------ */

void 
ddout_loop (void)
{
    /* c...mark */
    int ii, mark;

    mark = ddout_ini();
    if(mark)
	  exit(1);

    if(sweep_interleave == YES )
	  ddout_sweeps_loop(time_span);
    else
	  ddout_vols_loop(time_span);
    
    /* clean up! */
    ddout_write(ddout_fid,0,0);	/* write an EOF */
# ifdef not_right_now
# endif
    printf("Wrote %.3f MB\n", (float)BYTES_TO_MB(ddout_len));
    close(ddout_fid);

    dd_stats->rec_count = dis->rec_count;
    dd_stats->ray_count = dis->ray_count;
    dd_stats->sweep_count = dis->sweep_count;
    dd_stats->vol_count = dis->vol_count;
    dd_stats->file_count = dis->file_count;
    dd_stats->MB = dis->MB;
}
/* c------------------------------------------------------------------------ */

int 
ddout_ini (void)
{
    int ii, mark;
    int ok=YES;
    char *a, *name, str[256], *dfn;
    struct dd_general_info *dgi;
    struct unique_sweepfile_info_v3 *usi;
    struct dd_file_name_v3 *fn;
    double lower;


    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();
    if(!tmp_buf) {
	tmp_buf = (char *)malloc(TMP_BUF_SIZE);
	memset(tmp_buf, 0, TMP_BUF_SIZE);
	if(hostIsLittleEndian()) {
	   swap_buf = (char *)malloc(TMP_BUF_SIZE);
	   memset(swap_buf, 0, TMP_BUF_SIZE);
	}
    }

    if(!(dis = dd_setup_sweepfile_structs_v3(&ok)))
	  return(-1);

    for(ii=0,usi=dis->usi_top; ii++ < dis->num_radars; usi=usi->next) {
	/* now try and find files for each radar by
	 * reading the first record from each file
	 */
	if(ddswp_next_ray_v3(usi) == END_OF_TIME_SPAN) {
	  printf( "Problems reading first record of %s\n",
		  usi->radar_name.c_str());
	    ok = NO;
	}
	dgi = dd_window_dgi(usi->radar_num);
    }
    if(!ok)
	  return(-1);

    for(ii=0; ii < MAX_SENSORS; ii++) {
	ddout_num_parms[ii] = ddout_sweep_num[ii] = ddout_vol_num[ii] = 0;
    }

    if((a=get_tagged_string("TIME_DEFINED_VOLUMES"))) {
	time_defined_volumes = YES;
    }
    if((a=get_tagged_string("DORADE_VOLUME_INTERVAL"))) {
	if((ii = atoi(a)) >= 0 )
	      time_span = ii;
    }
    if((a=get_tagged_string("MIN_VOLUME_TIME_SPAN"))) {
	if((ii = atoi(a)) >= 0 )
	      min_vol_et = ii;
    }
    if((a=get_tagged_string("INTERLEAVE"))) {
	if(strstr(a, "SWEEP"))
	      sweep_interleave=YES;
	else
	      sweep_interleave=NO;
    }
    if((a=get_tagged_string("DORADE_IO_TYPE"))) {
	if(strstr(a,"BINARY_IO")) {
	    IO_type = BINARY_IO;
	}
	else if(strstr(a,"PHYSICAL_TAPE")) {
	    IO_type = PHYSICAL_TAPE;
	}
	/* FB_IO is the default */
    }
    if((dfn=get_tagged_string("DORADE_DEV"))) {
	/* see if the string contains a physical device name */
	if(dd_itsa_physical_dev(dfn))
	      IO_type = PHYSICAL_TAPE;
	/* otherwise assume it contains a directory name
	 * and is either FB_IO or BINARY_IO
	 */
    }
    if(IO_type != PHYSICAL_TAPE) { /* manufacture the name */
        if(a=get_tagged_string("DORADE_DEV")) {
	    slash_path(str, a);
	}
	else {
	  slash_path(str, dis->usi_top->directory.c_str());
	}
	/* find the earliest time
	 */
	lower = ETERNITY;
	usi = dis->usi_top;
	for(ii=0; ii++ < dis->num_radars; usi=usi->next) {
	    fn = *usi->final_swp_list; /* first sweep */
	    if(fn->time_stamp < lower) {
		lower = fn->time_stamp;
		name = fn->radar_name;
	    }
	}
	dd_file_name("dor", (int32_t)lower, name, 0, str+strlen(str));
	if(IO_type == FB_IO)
	      strcat(str, ".tape");
	dfn = str;
    }
    printf( "Opening DORADE output file %s ", dfn );
   if((ddout_fid = creat( dfn, PMODE )) < 0 ) {
	printf( " Unable to open %s\n", dfn );
	exit(1);
    }
    printf( " fid: %d\n", ddout_fid);
    return(0);
}
/* c------------------------------------------------------------------------ */

void ddout_sweeps_loop (int time_span)
{
    static int count=0;
    int ii, mark;
    struct dd_file_name_v3 *fn;
    double d, diff;
    struct dd_general_info *dgi;
    struct unique_sweepfile_info_v3 *usi, *usi_save;
    char *aa=NULL;

    /*
     * write out all the sweeps
     */

    for(diff=2.e22 ;; diff=2.e22) { /* all sweeps */
	usi = dis->usi_top->last;
	usi_save=NULL;
	/*
	 * loop backwards so that if there are two equal delta_ts
	 * we will use the first radar in the select list first
	 */
	for(ii=0; ii++ < dis->num_radars; usi=usi->last) {
	    count++;
	    if(!usi->forget_it) {
		fn = *(usi->final_swp_list+usi->swp_count-1);
		/* there's still sweeps for this radar */
		if((d = fn->time_stamp -reference_time)
		   < diff ) {
		    diff = d;
		    usi_save = usi;
		}
	    }
	}
	if(!usi_save) break;	/* no sweeps at or beyond the reference time */

	/* c...mark */
	fn = *(usi_save->final_swp_list+usi_save->swp_count-1);
	dis->reference_time = 
	      reference_time = fn->time_stamp;
	if(ddout_vol_detector(time_span)) {
	    /* rewrite the prev set of headers
	     */
	    ddout_hedr_saver( ddout_fid, aa, WRITE_VOL_HEADERS );
	    ddout_write( ddout_fid, aa, 0 ); /* write an EOF */
	    ddout_headers_dump(usi_save, YES); /* assemble new vol headers */
	    /*
	     * write new volume headers for all radars
	     */
	    ddout_hedr_saver( ddout_fid, 0, WRITE_VOL_HEADERS );

	    new_vol_time = reference_time+time_span;
	    sweeps_since_headers = 0;
	    for(usi=dis->usi_top,ii=0; ii++ < dis->num_radars; usi=usi->next) {
		dgi = dd_window_dgi(usi->radar_num);
		dgi->new_vol = NO;
	    }
	}
	sweeps_since_headers++;
	/*
	 * do the next sweep
	 */
	ddout_sweep_dump(usi_save);
	dgi->new_sweep = NO;
    }
    /* write last volume header */
    ddout_hedr_saver( ddout_fid, aa, WRITE_VOL_HEADERS );
}
/* c------------------------------------------------------------------------ */

int 
ddout_really (int fid, char *buf, int len)
{
    int mark=0;
    static int err_count=0;

    if(!Really_write_it) {
	mark = len;
    }
    mark = gp_write(fid, buf, len, IO_type);
    real_rec_count++;

    if( mark < len ) {
	err_count++;
	printf( "Error during dorade tape write: %d  %d\n"
	       , mark, len );
	if(err_count > 11)
	      exit(1);
    }
    else
	  ddout_len += len;

    return(mark);
}
/* c------------------------------------------------------------------------ */

void 
ddout_sweep_dump (struct unique_sweepfile_info_v3 *usi)
{
    static int count=0, loop_count;
    static double xtime=736902252;
    char *a, *b, *c, str[32], *cc;
    struct dd_general_info *dgi;
    struct paramdata_d *rdat;
    struct qparamdata_d *qdat;
    struct sweepinfo_d tswib;
    double d;
    DDS_PTR dds;
    DD_TIME *dts;
    int ii, nc, nn, pn, mark, num_short, bytes, ssn, ncopy;
    /*
     * write out the swib for this scan and all the rays
     * and position to the first ray of the next sweep
     */
    dgi = dd_window_dgi(usi->radar_num);

    if(ddout_num_parms[dgi->radar_num] != dgi->num_parms) {
	printf("Number of parameters (%d) in %s/%s ",
	       dgi->num_parms, usi->directory.c_str(), usi->filename.c_str());
	printf(" does not agree with previous sweep (%d) for the same radar\n"
	       , ddout_num_parms[dgi->radar_num]);
	exit(1);
    }
    ddout_num_parms[dgi->radar_num] = dgi->num_parms;
    dds = dgi->dds;
    dts = dgi->dds->dts;
    nc = dgi->dds->celv->number_cells;
    memcpy(&tswib, dds->swib, sizeof(struct sweepinfo_d));
    ssn = dgi->source_sweep_num;
    tswib.num_rays = dgi->source_num_rays;
    tswib.sweep_num = ++ddout_sweep_num[dgi->radar_num];
    if(hostIsLittleEndian()) {
//       ddin_crack_swib(&tswib, swap_buf, (int)0);  //Jul 26, 2011
       ddin_crack_swib((char *)&tswib, swap_buf, (int)0);  //Jul 26, 2011
       cc = swap_buf;
    }
    else { cc = (char *)&tswib; }
    mark = ddout_write(ddout_fid, cc, dds->swib->sweep_des_length);

    loop_count = 0;
    dgi->new_sweep = NO;
    d_unstamp_time(dts);

    printf( "Sweep: %2d %s  %s %d %d  %.1f MB ", tswib.sweep_num
	   , str_terminate(str, dds->radd->radar_name,8)
	   , dts_print(dts), ssn, dgi->source_vol_num
	   , (float)BYTES_TO_MB(ddout_len));

    while(1) {
	loop_count++;
	count++;
	if(dgi->time > xtime) {
	    mark = 0;
	}
	if(hostIsLittleEndian()) {
//	   ddin_crack_ryib(dds->ryib, swap_buf, (int)0);   //Jul 26, 2011
	   ddin_crack_ryib((char *)dds->ryib, swap_buf, (int)0);   //Jul 26, 2011
	   cc = swap_buf;
	}
	else { cc = (char *)dds->ryib; }
	mark = ddout_write
	  (ddout_fid, cc, dds->ryib->ray_info_length);

	if(hostIsLittleEndian()) {
//	   ddin_crack_asib(dds->asib, swap_buf, (int)0);   //Jul 26, 2011
	   ddin_crack_asib((char *)dds->asib, swap_buf, (int)0);   //Jul 26, 2011
	   cc = swap_buf;
	}
	else { cc = (char *)dds->asib; }
	mark = ddout_write
	  (ddout_fid, cc, dds->asib->platform_info_length);

	for(pn=0; pn < dgi->num_parms; pn++ ) {

	    ncopy = *dgi->dds->qdat[pn]->pdata_desc == 'R'
		  ? sizeof(struct paramdata_d)
			: sizeof(struct qparamdata_d);

	    if(dgi->compression_scheme == NO_COMPRESSION) {
	       if(hostIsLittleEndian()) {
//		  ddin_crack_qdat(dds->qdat[pn], swap_buf, (int)0);  //Jul 26, 2011
		  ddin_crack_qdat((char *)dds->qdat[pn], swap_buf, (int)0);  //Jul 26, 2011
		  mark = ddout_write(ddout_fid, swap_buf, ncopy);
		  nn = dds->qdat[pn]->pdata_length -ncopy;
		  swack_short(dds->qdat_ptrs[pn], swap_buf, nn/sizeof(short));
		  mark = ddout_write(ddout_fid, swap_buf, nn);
	       }
	       else {
//		  mark = ddout_write(ddout_fid, dds->qdat[pn], ncopy);  //Jul 26, 2011
		  mark = ddout_write(ddout_fid, (char *)dds->qdat[pn], ncopy); //comment : dds->qdat[pn] is pointing to dds->qdat[pn]->pdata_name (length = 4) Jun 27, 2011
		  mark = ddout_write(ddout_fid, dds->qdat_ptrs[pn], dds->qdat[pn]->pdata_length -ncopy);
	       }
	    }
	    else {
		num_short = dgi->dds->parm[pn]->binary_format == DD_8_BITS
		      ? BYTES_TO_SHORTS(nc) : nc;
		qdat = (struct qparamdata_d *)tmp_buf;
		memcpy(tmp_buf, dds->qdat[pn], ncopy);
		b = dds->qdat_ptrs[pn];
		c = tmp_buf + ncopy;
		num_short = HRD_recompress
//		      ((short *)b, (short *)c  //Jul 26, 2011
		      ((unsigned short *)b, (unsigned short *)c  //Jul 26, 2011
		       , dgi->dds->parm[pn]->bad_data, num_short
		       , (int)dgi->compression_scheme);
		bytes = LONGS_TO_BYTES(SHORTS_TO_LONGS(num_short));
		qdat->pdata_length = ncopy + bytes;
		if(hostIsLittleEndian()) {
//		   ddin_crack_qdat(qdat, swap_buf, (int)0);  //Jul 26, 2011
		   ddin_crack_qdat((char *)qdat, swap_buf, (int)0);  //Jul 26, 2011
		   mark = ddout_write(ddout_fid, swap_buf, ncopy);
		   swack_short(c, swap_buf, num_short);
		   mark = ddout_write(ddout_fid, swap_buf, bytes);
		}
		else {
		   mark = ddout_write(ddout_fid, tmp_buf
				      , qdat->pdata_length);
		}
	    }
	}
	if(ddswp_next_ray_v3(usi) == END_OF_TIME_SPAN) {
	    break;
	}
	if(dgi->new_sweep)
	      break;
    }
    printf( " %d \n", loop_count );

    if(usi->swp_count > usi->num_swps)
	  usi->forget_it = YES;
}
/* c------------------------------------------------------------------------ */

void 
ddout_vol_data (struct unique_sweepfile_info_v3 *usi, int time_span)
{
    struct dd_general_info *dgi;

    dgi = dd_window_dgi(usi->radar_num);

    do {
	ddout_sweep_dump(usi);
	if(time_span && dgi->time >= new_vol_time)
	      break;
    } while(!dgi->new_vol);
}
/* c------------------------------------------------------------------------ */

int 
ddout_vol_detector (int volume_interval)
{
    struct unique_sweepfile_info_v3 *usi;
    struct dd_general_info *dgi;
    int ii, doit=NO;


    if(time_defined_volumes) {
	/* get the time span for this sweep */
	for(ii=0; ii < difs->num_time_lims; ii++ ) {
	    if(reference_time >= difs->times[ii]->lower &&
	       reference_time <= difs->times[ii]->upper)
		  break;
	}
	if(ii == difs->num_time_lims) {
	}
	doit = ii != last_time_span ? YES : NO;
	last_time_span = ii;
	return(doit);
    }

    for(ii=0,usi=dis->usi_top; ii++ < dis->num_radars; usi=usi->next) {
	dgi = dd_window_dgi(usi->radar_num);

	if(!usi->forget_it) {
	    if(dgi->new_vol || (volume_interval && reference_time >=
				new_vol_time)) {
		doit = YES;
	    }
	}
    }
    doit = doit && sweeps_since_headers >= dis->num_radars ? YES : NO;
    return(doit);
}
/* c------------------------------------------------------------------------ */

void 
ddout_vols_loop (int volume_interval)
{
    static int count=0;
    int ii, mark;
    struct dd_file_name_v3 *fn;
    double d, diff;
    struct unique_sweepfile_info_v3 *usi, *usi_save=NULL;
    char *aa = NULL;

    /*
     * write out all the vols
     */
    for(diff=2.e22 ;; diff=2.e22) { 
	/* which vol to write next?
	 * at this point we should always have the headers and
	 * the first ray for each radar at our disposal
	 */
	usi = dis->usi_top->last;
	for(ii=0; ii++ < dis->num_radars; usi=usi->last) {
	    count++;
	    if(!usi->forget_it) {
		fn = *(usi->final_swp_list+usi->swp_count-1);
		if((d = fn->time_stamp -reference_time)
		   < diff ) {
		    diff = d;
		    usi_save = usi;
		}
	    }
	}
	if(!usi_save) break;	/* no sweeps at or beyond the reference time */

	fn = *(usi_save->final_swp_list+usi_save->swp_count-1);
	reference_time = fn->time_stamp;
	new_vol_time = reference_time+volume_interval;

	ddout_write( ddout_fid, aa, 0 ); /* write an EOF */
	ddout_headers_dump(usi_save, NO); /* assemble new vol headers */
	ddout_hedr_saver( ddout_fid, 0, WRITE_VOL_HEADERS ); /* new headers */
	/*
	 * write the sweeps for this volume
	 */
	ddout_vol_data(usi_save, volume_interval);
	/*
	 * write vol headers again
	 */
	ddout_hedr_saver( ddout_fid, 0, WRITE_VOL_HEADERS );
    }
}
/* c------------------------------------------------------------------------ */

int ddout_write (int fid, char *buf, int len)
{
    /* len = 0 => write an EOF
     */
    int i, n, mark;
    char *aa;
    static char *tbuf=NULL, *tbuf_ptr, *last_RYIB;
    static int tbuf_size=0, upto_last_RYIB;
    

    if(!tbuf) {			/* initialize */
	tbuf_ptr = tbuf = (char *)malloc(2*MAX_REC_SIZE);
    }


    if(!len || (strncmp(buf,"SWIB", 4) == A_MATCH) ||
        (strncmp(buf,"VOLD", 4) == A_MATCH) ) {
	/* EOF, EOV or start of sweep
	 */
	if( tbuf_size ) {	/* but write out leftovers first */
	    aa = tbuf;
	    if(tbuf_size > MAX_REC_SIZE) {
		mark = ddout_really(fid, aa, upto_last_RYIB);
		tbuf_size -= upto_last_RYIB;
		aa += upto_last_RYIB;
	    }
	    if( tbuf_size > 0 ) {
		mark = ddout_really(fid, aa, tbuf_size);
	    }
	}
	tbuf_ptr = tbuf;
	tbuf_size = 0;

	if(!len && real_rec_count > 0 ) {
	    mark = ddout_really(fid, buf, len); /* really write the EOF! */
	    return(mark);
	}
    }
    else if(strncmp(buf, "RYIB", 4) == A_MATCH) {
	/* start of a new ray */
	if(tbuf_size > MAX_REC_SIZE) {
	    mark = ddout_really(fid, tbuf, upto_last_RYIB);
	    tbuf_size -= upto_last_RYIB;
	    memcpy(tbuf, last_RYIB, tbuf_size);
	    tbuf_ptr = tbuf + tbuf_size;
	}
	last_RYIB = tbuf_ptr;
	upto_last_RYIB = tbuf_size;
    }
    memcpy(tbuf_ptr, buf, len );
    tbuf_ptr += len;
    tbuf_size += len;

    return(tbuf_size);
}
/* c------------------------------------------------------------------------ */

int 
ddout_hedr_saver (int fid, char *buf, int len)
{
    int ii, nn, mark;
    static char *hedr=NULL, *at;
    static int hedr_size;


    if(!hedr) {			/* initialize */
	hedr = (char *)malloc( MAX_REC_SIZE );
        at = hedr;
        memset( hedr, 0, MAX_REC_SIZE);
	hedr_size = 0;
    }

    if( len == RESET_FOR_VOL_HEADERS ) {             /* begin a new header */
        at = hedr;
        hedr_size = 0;
        return( hedr_size );
    }
    else if(len == WRITE_VOL_HEADERS && hedr_size > 0) { /* dump the header */
        mark = ddout_write( fid, hedr, hedr_size );
        return( hedr_size );
    }
    else if( len > 0 ) {
	memcpy( at, buf, len );
	hedr_size += len;
	at += len;
    }
    return(hedr_size);
}
/* c------------------------------------------------------------------------ */

void 
ddout_headers_dump (struct unique_sweepfile_info_v3 *usix, int all)
{
    int ii, jj, mark, num_sensors=0;
    struct dds_structs *dds;
    struct unique_sweepfile_info_v3 *usi;
    struct dd_general_info *dgi;
    struct radar_d *raddo;
    struct volume_d *vold;
    char str[256], *a;
    DD_TIME xdts, *dts;
    char *cc;

    /*
     * assemble all the volume headers
     * first reset header saver
     */    
    ddout_hedr_saver( ddout_fid, 0, RESET_FOR_VOL_HEADERS );

    if(all)
	  for(ii=0,usi=dis->usi_top; ii++ < dis->num_radars; usi=usi->next)
		if(!(usi->swp_count >= usi->num_swps)) num_sensors++;
    else
	  num_sensors = 1;

    usi = all ? dis->usi_top : usix;
    dgi = dd_window_dgi(usi->radar_num);
    dds = dgi->dds;
    dts = dds->dts;
    vold = dgi->dds->vold;

    dgi->vol_count++;
    dgi->vol_num++;
    dgi->vol_time0 = dgi->time;
    dgi->volume_time_stamp = (int32_t) dgi->vol_time0;
    vold->volume_num = ++ddout_vol_num[dgi->radar_num];
    printf("Output Volume num: %d\n", vold->volume_num);
    d_unstamp_time(dts);
    vold->year = dts->year;
    vold->month = dts->month;
    vold->day = dts->day;
    vold->data_set_hour = dts->hour;
    vold->data_set_minute = dts->minute;
    vold->data_set_second = dts->second;
    vold->number_sensor_des = num_sensors;
    if(a=get_tagged_string("GEN_FACILITY")) {
	  strncpy(vold->gen_facility, "         ", 8 );
	  jj = strlen(a) > 8 ? 8 : strlen(a);
	  strncpy(vold->gen_facility, a, jj);
    }
    else
	  strncpy( vold->gen_facility, "NCAR/ATD   ", 8 );
    xdts.time_stamp = time_now();
    d_unstamp_time(&xdts);
    vold->gen_year = xdts.year;
    vold->gen_month = xdts.month;
    vold->gen_day = xdts.day;


    if(hostIsLittleEndian()) {
       ddin_crack_vold((char *)vold, swap_buf, (int)0);
       cc = swap_buf;
    }
    else { cc = (char *)dds->vold; }	  
    mark = ddout_hedr_saver
      (ddout_fid, cc, dds->vold->volume_des_length);

    for(ii=0,usi=dis->usi_top; ii++ < dis->num_radars; usi=usi->next) {

	if(!usi->forget_it) {

	    if(all || usi == usix) {
		dgi = dd_window_dgi(usi->radar_num);
		dds = dgi->dds;
		dts = dds->dts;
		d_unstamp_time(dts);
		printf( "Headers for: %s %s \n"
		       , str_terminate(str, dds->radd->radar_name, 8)
		       , dts_print(dts));

		memcpy(tmp_buf, (char *)dds->radd
		       , dds->radd->radar_des_length);
		raddo = (struct radar_d *)tmp_buf;
		raddo->data_compress = dgi->compression_scheme;

		if(hostIsLittleEndian()) {
		   ddin_crack_radd(tmp_buf, swap_buf, (int)0);
		   cc = swap_buf;
		}
		else { cc = tmp_buf; }
		mark = ddout_hedr_saver
		  (ddout_fid, cc, dds->radd->radar_des_length);

		ddout_num_parms[dgi->radar_num] = dgi->num_parms;
		
		for(jj=0; jj < dgi->num_parms; jj++ ) {
		   if(hostIsLittleEndian()) {
//		      ddin_crack_parm(dds->parm[jj], swap_buf, (int)0);  //Jul 26, 2011
		      ddin_crack_parm((char *)dds->parm[jj], swap_buf, (int)0);  //Jul 26, 2011
		      cc = swap_buf;
		   }
		   else { cc = (char *)dds->parm[jj]; }
		   mark = ddout_hedr_saver
		     (ddout_fid, cc, dds->parm[jj]->parameter_des_length);
		}
		if(hostIsLittleEndian()) {
		   ddin_crack_celv((char *)dds->celv, swap_buf, (int)0);	//Jul 26, 2011 ??
		   swack_long((char *)&dds->celv->dist_cells[0], swap_buf +12  //Jul 26, 2011
			      , (int)dds->celv->number_cells);
		   cc = swap_buf;
		}
		else { cc = (char *)dds->celv; }
		mark = ddout_hedr_saver(ddout_fid, cc, dds->celv->cell_des_len);
		
		if (dds->frib) {
		  if(hostIsLittleEndian()) {
		    ddin_crack_frib((char *)dds->frib, swap_buf, (int)0);
		    cc = swap_buf;
		  }
		  else { cc = (char *)dds->frib; }
		  mark = ddout_hedr_saver
		    (ddout_fid, cc, dds->frib->field_radar_info_len);
		}

		if(hostIsLittleEndian()) {
		   ddin_crack_cfac((char *)dds->cfac, swap_buf, (int)0);
		   cc = swap_buf;
		}
		else { cc = (char *)dds->cfac; }
		mark = ddout_hedr_saver
		  (ddout_fid, cc, dds->cfac->correction_des_length);
	    }
	}
    } 
    dgi->new_vol = NO;
}
/* c------------------------------------------------------------------------ */









