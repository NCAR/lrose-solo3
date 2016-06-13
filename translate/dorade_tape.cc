/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */
/*
 * This file contains the following routines
 * 
 * dd_sweep_listit
 * dd_tape
 * dd_tape_dump
 */

# define PMODE 0666
# define SIZEOF_FILE_MARK 250000

//# include <dorade_headers.h>
# ifndef K32
# define K32 32768
# endif

#include <LittleEndian.hh>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "input_limits.hh"
#include "dorade_tape.hh"
#include "dorade_share.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "dd_swp_files.hh"
#include "to_fb.hh"
#include "dd_crackers.hh"

# define FILE_RESET_INTERVAL 1.6e9 /* to get past 2GB file size limitation */

static struct track_new_vols *nv_track=NULL;
static float max_dorade_tape_size=4.2 * 1.e9; /* gigabytes! */
static double f_bytes_total=0;
double sizeof_file_mark=SIZEOF_FILE_MARK;
static double sweep_keep_stamp[MAX_SENSORS];
static double sweep_keep_int=0;
static int num_tape_devs=0, stacker_flag=NO, dev_name_ndx=-1;
static char dev_names[256], *dev_name_ptrs[16];
static int stacker_num=0;
static int io_type=FB_IO;
static struct dd_input_filters *difs;
static int null_dev=NO;

/* c------------------------------------------------------------------------ */

static struct dd_sweep_list_entries *dd_sweep_listit(DGI_PTR dgi,
						     struct prev_swps *swp_que,
						     struct dd_sweep_list_entries *this_list,
						     int *sweep_count)
{
    /* enter this sweep in the sweep list.
     * this sweep should have been written out at this point
     */
    /* c...mark */
    int mark, ii, rn;

    if(swp_que->num_rays >= dd_min_rays_per_sweep() && !swp_que->listed
       && !dgi->ignore_this_sweep) {
	this_list->volume_time_stamp = swp_que->volume_time_stamp;
	this_list->sweep_time_stamp = swp_que->sweep_time_stamp;
	this_list->start_time = swp_que->start_time;
	this_list->end_time = swp_que->end_time;
	this_list->segment_num = swp_que->segment_num;
	this_list->source_sweep_num = swp_que->source_sweep_num;
	this_list->radar_num = rn = dgi->radar_num;
	this_list->new_vol = swp_que->new_vol;
	this_list->num_rays = swp_que->num_rays;
	this_list->num_parms = swp_que->num_parms;
	this_list->ignore_this_sweep = NO;
	this_list->next_segment = NULL;
	(*sweep_count)++;
	swp_que->listed = YES;
	strcpy(this_list->file_name, swp_que->file_name);
# ifdef obsolete
	dts.time_stamp = this_list->start_time;
	printf("Listing %s %s c:%d %d %.3f\n"
	       , dts_print(d_unstamp_time(&dts))
	       , dgi->radar_name, *sweep_count
	       , dgi->ray_que->last->source_sweep_num
	       , this_list->start_time
	       );
# endif
	if(nv_track->start_time < 0 && rn != nv_track->LF_radar_num) {
	    nv_track->start_time = this_list->start_time;
	}
	/* see if this radar has been encounter before
	 */
	for(ii=0; ii < nv_track->num_radars; ii++) {
	    if(rn == nv_track->list_radar_nums[ii])
		  break;
	}
	if(ii >= nv_track->num_radars) {
	    /* this also tracks the order in which radar
	     * was encountered first
	     */
	    ii = nv_track->num_radars++;
	    nv_track->list_radar_nums[ii] = rn;
	    nv_track->swle_last[rn] = NULL;
	    nv_track->sweep_count[rn] = nv_track->vol_count = 0;
	    nv_track->sizeof_volume = 0;
	}
	this_list->nv_track_last = nv_track->swle_last[rn];
	nv_track->swle_last[rn] = this_list;
	nv_track->sweep_count[rn]++;
	nv_track->end_time = this_list->end_time;
	nv_track->sizeof_volume += swp_que->sweep_file_size;

	this_list = this_list->next;
    }
    else {
	mark = 0;
    }
    return(this_list);
}

/* c------------------------------------------------------------------------ */

static void dd_tape_dump (struct dd_sweep_list_entries *top,
			  int sweep_count, int out_fid, int remove_them,
			  int io_type, int vol_num,
			  struct dd_sweep_list_entries **swp_list)
{
    static char *vtbuf, *tbuf, *sbuf, *swap_buf;
    static int first=YES, count=0, trip=11820;
    static double f_bytes=0, fbytes_trip=FILE_RESET_INTERVAL;
    static DD_TIME *dts;

    struct dd_sweep_list_entries *this_list, *that, *last, *dsle[MAX_SENSORS];  //Jul 26, 2011 *this
    struct dd_sweep_list_entries **swp_list_ptr=swp_list;
    struct volume_d *vold;
    struct radar_d *radd;
    struct parameter_d *parm;
    struct super_SWIB *sswb;
    struct sweepinfo_d *swib;
    struct cell_d *celv;
    struct generic_descriptor *gd;
    struct ray_i *ryib;
    struct comment_d *comm;

    double d,keep_time;
    int real_sweep_count=0;
    int radar_count=0,_fid, swp_fid, ryib_ndx, itsa_ryib, vsize;
    int size_gd=sizeof(struct generic_descriptor), mark, keep_count, flen;
    int i, j=BLOCK_SIZE, kk, n, nn, ncopy, pn, qdat_data_offs, gdsos;
    int rn, sn, snn, rsize=0, slen, npd;
    int32_t cdl;
    char *a, *sb, *tb, *first_id;
    char str[256];
    DD_TIME ddts;
	DGI_PTR dgi;

    if(first) {
	first = NO;
	vtbuf = (char *)malloc(K32);
	tbuf = (char *)malloc(2*MAX_REC_SIZE);
	sbuf = (char *)malloc(K32);
	dts = (DD_TIME *)malloc(sizeof(DD_TIME));
	if(hostIsLittleEndian()) {
	   swap_buf = (char *)malloc(K32);
	   memset(swap_buf, 0, K32);
	}
    }

    /*
     * set up to reassemble the data in chronological order
     */
    for(this_list=top,sn=0; sn < sweep_count; sn++,this_list=this_list->next) {
	/* assemble the segments for this sweep
	 */
	if(!this_list->ignore_this_sweep) {
	    real_sweep_count++;
	    *swp_list_ptr++ = this_list;
	    last = this_list;
	    for(that=this_list->next,snn=sn+1; snn < sweep_count; snn++) {
		if(!that->ignore_this_sweep &&
		   this_list->radar_num == that->radar_num &&
		   this_list->source_sweep_num == that->source_sweep_num) {
		    /*
		     * this is another segment of the same sweep
		     */
		    printf("Segmented sweep %d %3d %.3f "
			   , this_list->radar_num
			   , this_list->source_sweep_num
			   , last->start_time);
		    ddts.time_stamp = last->start_time;
		    d_unstamp_time(&ddts);
		    printf("%02d:%02d.%03d "
			   , ddts.minute, ddts.second, ddts.millisecond);
		    printf("  %3f ", that->start_time);
		    ddts.time_stamp = that->start_time;
		    d_unstamp_time(&ddts);
		    printf("%02d:%02d.%03d "
			   , ddts.minute, ddts.second, ddts.millisecond);
		    printf("\n");

		    last->next_segment=that;
		    last = that;
		    that->ignore_this_sweep = YES;
		    this_list->num_rays += that->num_rays; /* accululate num_rays */
		}
		that = that->next;
	    }
	}
    }
    /* we now have a list of the first segments of each sweep
     * to be written to tape
     * Now sort the real sweep list by time
     */
    for(sn=0; sn < real_sweep_count; sn++) {
	keep_time = (*(swp_list+sn))->start_time;
	for(snn=sn+1; snn < real_sweep_count; snn++) {
	    that = *(swp_list+snn);
	    if(that->start_time < keep_time) {
		keep_time = that->start_time;
		*(swp_list+snn) = *(swp_list+sn);
		*(swp_list+sn) = that;
	    }
	}
    }
    /* the real sweep list should now be sorted by time
     *
     * construct a list of unique the radars
     */
    for(i=0; i < real_sweep_count; i++) {
	this_list = *(swp_list+i);
# ifdef obsolete
	printf(" %d %.3f %3d %3d\n"
	       , this_list->radar_num
	       , this_list->start_time
	       , this_list->source_sweep_num
	       , this_list->num_rays
	       );
# endif
	for(j=0; j < radar_count; j++) {
	    if(dsle[j]->radar_num == this_list->radar_num)
		  break;
	}
	if(j == radar_count) {	/* add this radar */
	    /* make sure we can rally access this sweep */
	    dgi = dd_get_structs(this_list->radar_num);
	    strcpy(str, dgi->directory_name.c_str());
	    strcat(str, this_list->file_name);
	    if((swp_fid = open(str,0)) < 1 ) {
		printf("\nUnable to open %s status: %d ... Ignoring it!\n"
		       , str, swp_fid);
		this_list->ignore_this_sweep = YES;
	    }
	    else if((n = read(swp_fid, sbuf, K32)) <= size_gd) {
		printf("\nUnable to read %s status:%d ... Ignoring it!\n"
		       , str, swp_fid);
		this_list->ignore_this_sweep = YES;
	    }
	    else {
		radar_count++;
		dsle[j] = this_list;
		close(swp_fid);
	    }
	}
	this_list = this_list->next;
    }
    if(!f_bytes) {
	dgi = dd_get_structs(dsle[0]->radar_num);
	/* Add comments and an eof
	 */
	a = (char *)dgi->dds->comm;
	gd = (struct generic_descriptor *)a;
	gdsos = gd->sizeof_struct;

	if(hostIsLittleEndian()) {
	   memcpy(swap_buf, a, 4);
	   swack4(a+4, swap_buf+4);
	   memcpy(swap_buf + 8, a + 8, gdsos - 8);
	   a = swap_buf;
	}
	if((mark = gp_write(out_fid, a, gdsos, io_type)) < 1 ) {
	    printf("Write error fid: %d status: %d\n"
		   , out_fid, mark);
	}
	/* Write an eof if appropriate */
	n = gp_write(out_fid, a, 0, io_type);
    }
    /*
     * generate the volume header record
     */

    for(rn=0; rn < radar_count; rn++) {
	count++;
	this_list = dsle[rn];
	dgi = dd_get_structs(this_list->radar_num);
	strcpy(str, dgi->directory_name.c_str());
	strcat(str, this_list->file_name);
	printf("Vol Dump %s %d ", str, this_list->sweep_time_stamp);

	if((swp_fid = open(str,0)) < 1 ) {
	    printf("\nUnable to open %s status: %d\n", str, swp_fid);
	    exit(1);
	}
	printf(" fid: %d\n", swp_fid);

	/* absorb a chunk of the sweep file */
	if((slen = read(swp_fid, sbuf, K32)) <= 0) {
	    printf("vs Unable to read %s status: %d\n", str, slen);
	    exit(1);
	}	
	sswb = (struct super_SWIB *)sbuf;
	if(!(a = dd_find_desc(sbuf, sbuf+slen, (char *)"RYIB"))) {
	    printf("Could not find ryib in first buffer\n");
	    exit(1);
	}
	close(swp_fid);
	ryib = (struct ray_i *)a;

	if(!rn) {
	    /* first time through; do the volume descriptor
	     */
	    if(!(a = dd_find_desc(sbuf, a, (char *)"VOLD"))) {
		mark = 0;
	    }
	    gd = (struct generic_descriptor *)a;
	    if(hostIsLittleEndian()){
	       memcpy(swap_buf, a, gd->sizeof_struct);
	       vold = (struct volume_d *)swap_buf;
	    }
	    else {
	       memcpy(vtbuf, a, gd->sizeof_struct);
	       vold = (struct volume_d *)vtbuf;
	    }
	    vsize = gd->sizeof_struct;
	    vold->number_sensor_des = radar_count;
	    vold->volume_num = vol_num;
	    d = dorade_time_stamp(vold, ryib, dts);
	    d_unstamp_time(dts);
	    vold->year = dts->year;
	    vold->month = dts->month;
	    vold->day = dts->day;
	    vold->data_set_hour = dts->hour;
	    vold->data_set_minute = dts->minute;
	    vold->data_set_second = dts->second;

	    if(hostIsLittleEndian()){
	       ddin_crack_vold(swap_buf, vtbuf, (int)0);
	    }

	    if((a = dd_find_desc(sbuf, a, (char *)"COMM"))) {
	       /* loop through the comments
		*/
		gd = (struct generic_descriptor *)a;
		memcpy(vtbuf+vsize, a, gd->sizeof_struct);
		if(hostIsLittleEndian()){
		   comm = (struct comment_d *)vtbuf+vsize;
		   cdl = comm->comment_des_length;
//		   swack4(&cdl, &comm->comment_des_length);  //Jul 26, 2011
		   swack4((char *)&cdl, (char *)&comm->comment_des_length);  //Jul 26, 2011
		}
		vsize += gd->sizeof_struct;
	    }
	}
	/* sensor specific stuff
	 */
	if(!(a = dd_find_desc(sbuf, a, (char *)"RADD"))) {
	   mark = 0;
	}
	gd = (struct generic_descriptor *)a;
	radd = (struct radar_d*)a;
	if(hostIsLittleEndian()){
	   ddin_crack_radd(a, vtbuf+vsize, (int)0);
	}
	else {
	   memcpy(vtbuf+vsize, a, gd->sizeof_struct);
	}
	vsize += gd->sizeof_struct;
	npd = radd->num_parameter_des;
	/*
	 * get the parameter descriptors from the sweep file
	 */
	if(!(a = dd_find_desc(sbuf, a, (char *)"PARM"))) {
	    /* complain */
	   mark = 0;
	}
	for(i=0; i < npd; i++) {
	    gd = (struct generic_descriptor *)a;
	    if(hostIsLittleEndian()){
	       parm = (parameter_d *)a;
	       qdat_data_offs = parm->offset_to_data;
	       ddin_crack_parm(a, vtbuf+vsize, (int)0);
	    }
	    else {
	       memcpy(vtbuf+vsize, a, gd->sizeof_struct);
	    }
	    a += gd->sizeof_struct;
	    vsize += gd->sizeof_struct;
	}
	if(!(a = dd_find_desc(sbuf, a, (char *)"CELV"))) {
	   mark = 0;
	}
	gd = (struct generic_descriptor *)a;
	if(hostIsLittleEndian()){
	   celv = (struct cell_d *)a;
	   ddin_crack_celv(a, vtbuf+vsize, (int)0);
	   swack_long((char *)&celv->dist_cells[0], vtbuf+vsize +12  //Jul 26, 2011
		      , (int)celv->number_cells);
	}
	else {
	   memcpy(vtbuf+vsize, a, gd->sizeof_struct);
	}
	vsize += gd->sizeof_struct;

	if(!(a = dd_find_desc(sbuf, a, (char *)"CFAC"))) {
	   mark = 0;
	}
	gd = (struct generic_descriptor *)a;
	if(hostIsLittleEndian()){
	   ddin_crack_cfac(a, vtbuf+vsize, (int)0);
	}
	else {
	   memcpy(vtbuf+vsize, a, gd->sizeof_struct);
	}
	vsize += gd->sizeof_struct;
    }
    /* write out the buffer
     */
    if((mark = gp_write(out_fid, vtbuf, vsize, io_type)) < 1 ) {
	printf("Write error fid: %d  count: %d  status: %d\n"
	       , out_fid, count, mark);
    }
    f_bytes += mark;
    swp_list_ptr = swp_list;
    /*
     * Loop through all the sweeps
     */

    for(sn=0; sn < real_sweep_count; sn++,swp_list_ptr++) {
	this_list = that = *swp_list_ptr;
	for(;;) {
	    /* for each segment, open up the file
	     */
	    dgi = dd_get_structs(this_list->radar_num);
	    strcpy(str, dgi->directory_name.c_str());
	    strcat(str, this_list->file_name);
	    printf("Copy s:%2d %s %.3f %.2f MB %d"
		   , sn, strrchr(str, '/'), this_list->start_time
		   , f_bytes*1.e-6, this_list->source_sweep_num);
	    if((swp_fid = open(str,0)) < 1 ) {
		printf("\nUnable to open %s status: %d\n", str, swp_fid);
		break;
	    }
	    printf(" fid: %d\n", swp_fid);
	    rsize = ryib_ndx = 0;
	    sb = sbuf;
	    flen = n = read(swp_fid, sb, size_gd);
	    if(flen < size_gd) {
		printf("Zero length file for this sweep\n");
		continue;
	    }
	    if(this_list == that) 
		first_id = (char *)"SWIB";
	    else
		first_id = (char *)"RYIB"; /* segmented sweep */
	    /*
	     * loop to the first id
	     */
	    for(;;) {
		if(strncmp(sb, first_id, 4) == 0 )
		      break;
		gd = (struct generic_descriptor *)sb;
		if((n = read(swp_fid, sb+size_gd, gd->sizeof_struct)) < 1 ) {
		    printf("Error looping to %s\n", first_id);
		    exit(1);
		}
		flen += n;
		sb += gd->sizeof_struct;
	    }
	    if(this_list == that) {
		/* first and probably only segment although
		 * this may have been a segmented sweep so
		 * update the "num_rays" in the swib
		 */
		swib = (struct sweepinfo_d *)sb;
		swib->num_rays = this_list->num_rays;
	    }
	    /* move the first part of the first descriptor to the tape buffer
	     */
	    memcpy(tbuf, sb, size_gd);
	    tb = tbuf;
	    keep_count = count;
	    /*
	     * read in the rest of the file directly into the tape buffer
	     */
	    for(;;) {		/* each descriptor */
		count++;
		if(count >= trip) {
		    mark = 0;
		}
		if((itsa_ryib = strncmp(tb, "RYIB", 4) == 0) &&
		   rsize > MAX_REC_SIZE) {
		    /*
		     * write out this record
		     */
		    if((mark = gp_write(out_fid, tbuf, ryib_ndx, io_type)) < 1 ) {
			printf("Write error fid: %d  count: %d  status: %d\n"
			       , out_fid, count, mark);
			exit(1);
		    }
		    f_bytes += mark;
		    n = rsize -ryib_ndx;
		    memcpy(tbuf, tbuf+ryib_ndx, n+size_gd);
		    rsize = ryib_ndx = n;
		    tb = tbuf+n;
		}
		else if(itsa_ryib) { /* found a RYIB */
		    ryib_ndx = rsize;
		}
		else if(strncmp(tb, "NULL", 4) == 0) { /* end of useful data */
		    if(rsize) {
			n = rsize > MAX_REC_SIZE ? ryib_ndx : rsize;
			if((mark = gp_write(out_fid, tbuf, n, io_type)) < 1 ) {
			    printf(
"Write error fid: %d  count: %d  status: %d %d\n"
				   , out_fid, count, mark, f_bytes);
			    exit(1);
			}
			f_bytes += mark;
			if(rsize > MAX_REC_SIZE) {
			    if((mark = gp_write(out_fid, tbuf+ryib_ndx
						, rsize-ryib_ndx, io_type)) < 1 ) {
				printf(
"Write err fid: %d count: %d status: %d %d\n"
				       , out_fid, count, mark, f_bytes);
				exit(1);
			    }
			    f_bytes += mark;
			}
		    }
		    break;
		} /* end of "itsa_ryib check */

		gd = (struct generic_descriptor *)tb;
		gdsos = gd->sizeof_struct;
		if((n = read(swp_fid, tb+size_gd, gd->sizeof_struct)) < 1 ) {
		    printf("Sweep file read error: %d count: %d %s\n"
			   , n, count, str);
		    if(ryib_ndx) {
			tb = tbuf+ryib_ndx;
			strncpy(tb, "NULL", 4); /* fake an EOD */
		    }
		    else
			  exit(1);
		}
		else {
		    flen += n;
		    if(itsa_ryib) {
			ryib = (struct ray_i *)tb;
		    }
		    if(hostIsLittleEndian()){
		       if(strncmp(tb, "RDAT", 4) == 0 ||
			       strncmp(tb, "QDAT", 4) == 0) {
			  ncopy = (*tb == 'R') ? sizeof(struct paramdata_d) :
			    qdat_data_offs;
			  nn = gdsos - ncopy;
			  ddin_crack_qdat(tb, swap_buf, (int)0);
			  memcpy(tb, swap_buf, ncopy);
			  swack_short(tb+ncopy, swap_buf, nn/2);
			  memcpy(tb+ncopy, swap_buf, nn);
		       }
		       else if(strncmp(tb, "RYIB", 4) == 0) {
			  ddin_crack_ryib(tb, swap_buf, (int)0);
			  memcpy(tb, swap_buf, gdsos);
		       }
		       else if(strncmp(tb, "ASIB", 4) == 0) {
			  ddin_crack_asib(tb, swap_buf, (int)0);
			  memcpy(tb, swap_buf, gdsos);
		       }
		       else if(strncmp(tb, "SWIB", 4) == 0) {
			  ddin_crack_swib(tb, swap_buf, (int)0);
			  memcpy(tb, swap_buf, gdsos);
		       }
		    }
		    tb += gdsos;
		    rsize += gdsos;
		}
	    }
	    close(swp_fid);
	    if(remove_them) {
		if(sweep_keep_int > 0 && sweep_keep_stamp[this_list->radar_num] +
		   sweep_keep_int <= this_list->start_time) {
		    sweep_keep_stamp[this_list->radar_num] = this_list->start_time;
		}
		else
		      n = unlink(str);
	    }
	    if(this_list->next_segment)
		  this_list = this_list->next_segment;
	    else
		  break;
	}
    }

    /* write out the volume header again  */
    if((mark = gp_write(out_fid, vtbuf, vsize, io_type)) < 1 ) {
	printf("Write error fid: %d  count: %d  status: %d\n"
	       , out_fid, count, mark);
    }
    f_bytes += mark;

    /* Write an eof if appropriate
     */
    if(!null_dev) {
       n = gp_write(out_fid, tbuf, 0, io_type);
    }
    if(io_type == PHYSICAL_TAPE)
	  f_bytes += sizeof_file_mark;

    if(io_type == PHYSICAL_TAPE && f_bytes > max_dorade_tape_size) {
	f_bytes_total += f_bytes;

	if(stacker_flag) {
	    close(out_fid);
	    sprintf( str, "mt -f %s offline"
		     , dev_name_ptrs[stacker_num+1] );
	    kk = system( str );

	    printf("Switching to next tape dev\n");
	    stacker_num = ( stacker_num +1 ) % num_tape_devs;
	    a = dev_name_ptrs[stacker_num +1];
	    printf("Opening next DORADE output file %s", a);
	    if((out_fid = creat(a, PMODE )) < 0 ) {
		printf( "\nUnable to open %s\n", a );
		difs->stop_flag = difs->abrupt_start_stop = YES;
		return;
	    }
	    printf( " fid: %d\n", out_fid);
	    f_bytes = 0;
	    fbytes_trip = FILE_RESET_INTERVAL;
	}
	else {
	    close(out_fid);
	    if(++dev_name_ndx >= num_tape_devs) {
		printf("Ran out of DORADE output devs at %.3f GB\n"
		       , f_bytes_total*1.e-9);
		difs->stop_flag = difs->abrupt_start_stop = YES;
		return;
	    }
	    printf("Switching to next tape dev\n");
	    a = dev_name_ptrs[dev_name_ndx];
	    printf("Opening next DORADE output file %s", a);
	    if((out_fid = creat(a, PMODE )) < 0 ) {
		printf( "\nUnable to open %s\n", a );
		difs->stop_flag = difs->abrupt_start_stop = YES;
		return;
	    }
	    null_dev = strstr(a, "/dev/null") ? YES : NO;
	    printf( " fid: %d\n", out_fid);
	    f_bytes = 0;
	    fbytes_trip = FILE_RESET_INTERVAL;
	}
    }
    else if(io_type == PHYSICAL_TAPE && f_bytes > fbytes_trip) {
	/* klooge to get a tape past 2 GB on some systems
	 */
	fbytes_trip += FILE_RESET_INTERVAL;
	mark = lseek(out_fid, 0L, SEEK_SET);
	printf("Reset output dev at %.3f MB\n", f_bytes*1.e-6);
    }
}

/* c------------------------------------------------------------------------ */

void 
dd_tape (DGI_PTR dgi, int flush)
{
    /* this routine controls the output of DORADE tape format data
     * by assembling a list of sweeps that constitute a volume
     * and then cycling through them when the voluem is compelte
     */
    static double time_span=240; /* seconds */
    static float max_vol_size=99.e6;
    static int sweep_count=0, out_fid, remove_them=YES;
    static int max_sweep_count=0, min_time_gap=20;
    static int produce_dorade=YES, min_vol_et=20;
    static int vol_num=0;
    static struct dd_sweep_list_entries *top=NULL
	  , *this_list=NULL, *next=NULL, *last, **swp_list=NULL;  //Jul 26, 2011 *this

    int i=0, n, rn, dump_em=NO, mark, radar_num = -1, nt;
    double d, et;
    char *a, *dfn, str[256];
	struct dd_general_info *dgii;

    if(!produce_dorade)
	  return;


    if(!top) {
	/* Initialization!
	 */
	produce_dorade = NO;
	if(dd_catalog_only_flag()) {
	    return;
	}
	if((a=get_tagged_string("OUTPUT_FLAGS"))) {
	    if(strstr(a, "DORADE_DATA")) {
		produce_dorade = YES;
		if(strstr(a, "SWEEP_FIL")) {
		    remove_them = NO;
		}
	    }
	}
	if(!produce_dorade)
	      return;

	difs = dd_return_difs_ptr();
	dd_output_flag(YES);

	for(i=0; i < MAX_SENSORS; i++) {
	    sweep_keep_stamp[i] = 0;
	}
	if((a=get_tagged_string("MAX_VOLUME_SIZE"))) { /* in MB */
	   if((i = atoi(a)) >= 0 ) {
	       max_vol_size = i*1000000;
	   }
	}
	if((a=get_tagged_string("DORADE_VOLUME_INTERVAL"))) {
	   if((i = atoi(a)) >= 0 )
		 time_span = i;
	}
	if(a=get_tagged_string("MAX_DORADE_TAPE_SIZE")) {
	   if((d = atof(a)) > 0) {
	      max_dorade_tape_size = d * 1.e9; 
	   }
	}
	if( a=get_tagged_string("MAX_MEDIA_SIZE")) {
	   if((d = atof(a)) > 0) {
	      max_dorade_tape_size = d * 1.e6;
	   }
	}
	if((a=get_tagged_string("MIN_VOLUME_TIME_SPAN"))) {
	   if((i = atoi(a)) >= 0 )
		 min_vol_et = i;
	}
	if((a=get_tagged_string("MIN_TIME_GAP"))) {
	   if((i = atoi(a)) > 0 )
		 min_time_gap = i;
	}
	if((a=get_tagged_string("PRESERVE_SWEEP_FILES"))) {
	    if(strlen(a)) {
		if((i = atoi(a)) > 0) {
		    sweep_keep_int = i;
		}
	    }
	    else
		  remove_them = NO;
	}
	if((a=get_tagged_string("DORADE_IO_TYPE"))) {
	    if(strstr(a,"BINARY_IO")) {
		io_type = BINARY_IO;
	    }
	    else if(strstr(a,"PHYSICAL_TAPE")) {
		io_type = PHYSICAL_TAPE;
	    }
	    /* FB_IO is the default */
	}

	if( dfn=get_tagged_string("DORADE_DEV") ) {
	    /* see if the string contains a physical device name */

	    if(dd_itsa_physical_dev(dfn)) {
		io_type = PHYSICAL_TAPE;
		strcpy(dev_names, dfn);
		num_tape_devs = nt =
		      dd_tokens(dev_names, dev_name_ptrs);
		if(num_tape_devs < 1) {
		    strcpy(dev_names, "/dev/null");
		    dfn = dev_names;
		    dev_name_ptrs[0] = dev_names;
		}
		else if(strstr(dev_name_ptrs[0], "STACKER")) {
		    stacker_flag = YES;
		    if( --num_tape_devs < 2 ) {
		      printf( "Less than 2 stackers specified!\n" );
		      exit(1);
		    }
		    stacker_num = 0;
		    /* remaining args are the stacker dev names */
		    dfn = dev_name_ptrs[ stacker_num +1];
		}
		else {		/* assume this is a list of devs
				 * to cycle through */
		    dev_name_ndx = 0;
		    dfn = dev_name_ptrs[0];
		}
	    }
	    /* otherwise assume it contains a directory name
	     * and is either FB_IO or BINARY_IO
	     */
	}

	if(io_type != PHYSICAL_TAPE) { /* manufacture the name */
	    if(dfn) {
		slash_path(str, dfn);
	    }
	    else {
	      slash_path(str, dgi->directory_name.c_str());
	    }
	    n = strlen(str);
	    dd_file_name("dor", (time_t)dgi->time
		     , dd_radar_name(dgi->dds), 0, str+n);
	    if(io_type == FB_IO)
		  strcat(str, ".tape");
	    dfn = str;
	}

	printf( "Opening DORADE output file %s", dfn );
	if((out_fid = creat( dfn, PMODE )) < 0 ) {
	    printf( "\nUnable to open %s\n", dfn );
	    exit(1);
	}

	printf( " fid: %d\n", out_fid);
	null_dev = strstr(dfn, "/dev/null") ? YES : NO;

	nv_track = (struct track_new_vols *)
	      malloc(sizeof(struct track_new_vols));
	memset(nv_track, 0, sizeof(struct track_new_vols));
	nv_track->start_time = -1.;
	nv_track->LF_radar_num = -1;

	/*
	 * end of initialization
	 */
    }



    if(sweep_count == max_sweep_count) {
	/* allocate space for more sweep list entries
	 * and sweep list entry pointers
	 */
	for(i=0; i < 64; i++) {
	    this_list = (struct dd_sweep_list_entries *)
		  malloc(sizeof(struct dd_sweep_list_entries));
	    memset((char *)this_list, 0, sizeof(struct dd_sweep_list_entries));
	    if(!i) {
		next = this_list;
		if(!max_sweep_count) 
		      top = this_list;
	    }
	    if(max_sweep_count) {
		last->next = this_list;
		this_list->last = last;
	    }
	    top->last = last = this_list;
	    this_list->next = top;
	    max_sweep_count++;
	}
	if(!swp_list)
	      swp_list = (struct dd_sweep_list_entries **)
		    malloc(max_sweep_count*
			   sizeof(struct dd_sweep_list_entries *));
	else
	      swp_list = (struct dd_sweep_list_entries **)
		    realloc((char *)swp_list, max_sweep_count*
			   sizeof(struct dd_sweep_list_entries *));
    }


    if(flush == DD_FLUSH_ALL) {
	dump_em = YES;
    }
    else {
	/* save sweep information for the previous sweep
	 * remember you're always one behind on sweeps
	 * ie sweeps are not listed until they are complete
	 */
	if(!dgi)
	      return;
	radar_num = dgi->radar_num;
	if(dgi->dds->radd->radar_type == DD_RADAR_TYPE_AIR_LF) {
	    nv_track->LF_radar_num = radar_num;
	}
	next = dd_sweep_listit(dgi, dgi->swp_que->last, next, &sweep_count);
	et = nv_track->start_time > 0 ? dgi->time -nv_track->start_time : 0;
    }

    if(sweep_count && !dump_em) {
	if(et > time_span)
	      dump_em = YES;
	if(dgi->new_vol && nv_track->sweep_count[dgi->radar_num]) {
	    /* there must be at least one sweep for this radar */
	    dump_em = YES;
	}
	if(nv_track->sizeof_volume >= max_vol_size)
	      dump_em = YES;
    }

    if(dump_em) {
	/* information about the last sweep for each radar
	 * may not be in the table
	 */
	for(rn=0; rn < dd_num_radars(); rn++) {
	    dgii = dd_get_structs(rn);
	    if(flush != DD_FLUSH_ALL) {
		if(rn == radar_num)
		      continue;	/* it's already been done */
		if(dgii->dds->radd->radar_type == DD_RADAR_TYPE_AIR_LF)
		      continue;
		if(dgi->source_fmt == ELDORA_FMT)
		      continue;
	    }
	    /*
	     * It's either a flush or we're assuming
	     * that sweeps for other radars
	     * will also be ending and it would be nice to
	     * include them in the same volume...
	     * this is not true for
	     * eldora field fmt and P3 lower fuselage data
	     */
	    dd_flush(dgii->radar_num);
	    next = dd_sweep_listit(dgii, dgii->swp_que, next
				   , &sweep_count);
	}
# ifdef obsolete
	for(rn=0; rn < dd_num_radars(); rn++) {
	    dgj = dd_get_structs(rn);
	    if(flush != DD_FLUSH_ALL && rn == radar_num)
		  continue;	/* it's already been done */
	    if(flush == DD_FLUSH_ALL || dgj->dds->radd->radar_type != AIR_LF) {
		/* don't flush if lower fuselage data it's too confusing
		 */
		dd_flush(dgj->radar_num);
		next = dd_sweep_listit(dgj, dgj->swp_que, next, &sweep_count);
	    }
	}
# endif
	if(sweep_count)
	      dd_tape_dump( top, sweep_count, out_fid, remove_them, io_type
			   , ++vol_num, swp_list);
	nv_track->start_time = -1.;
	nv_track->num_radars = 0;
	sweep_count = 0;
	next = top;
    }
}
