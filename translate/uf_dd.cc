/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define IOTEST

/*
 * this file contains the following routines
 * 
 * uf_dd_conv
 * 
 * map_uf_ptrs
 * rdp_ac_header
 * ucla_ac_header
 * uf_ini
 * uf_inventory
 * uf_next_ray
 * uf_positioning
 * uf_print_data
 * uf_print_dhed
 * uf_print_fldh
 * uf_print_headers
 * uf_print_lus
 * uf_print_lus_ac
 * uf_print_lus_gen
 * uf_print_lus_ucla
 * uf_print_man
 * uf_print_opt
 * uf_print_stat_line
 * uf_rename_str
 * uf_reset
 * uf_select_ray
 * uf_time_stamp
 * uf_vol_update
 * 
 * 
 */

# define NOT_FOUND -1
# define MAX_RADARS 8
# define TOGA_PROJECTS "TAMEX TOGACORE "
# define SIGMET_RADARS "SIGMET MIT-SIGMET "

#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <dd_math.h>
#include "uf_fmt.h"
#include "dd_io_mgmt.hh"
#include "dorade_share.hh"
#include "generic_radar_info.h"
#include "input_limits.hh"
#include "dd_stats.h"
#include "uf_dd.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "gneric_dd.hh"
#include "dd_crackers.hh"
#include <LittleEndian.hh>

extern int Sparc_Arch;

UF_MANDITORY *man, *xman;
UF_OPTIONAL *opt, *xopt;
UF_LOC_USE_AC *loc, *xloc;
UF_DATA_HED *dhed, *xdhed;
UF_FLD_ID_ARRAY *fida, *xfida;
UF_FLD_ID *fidp[MAX_UF_FLDS], *xfidp[MAX_UF_FLDS];
UF_FLD_HED *fhed[MAX_UF_FLDS], *xfhed[MAX_UF_FLDS];

# define    UF_360_MAX_SECTOR 0x1
# define    UF_FORCE_EOF_VOLS 0x2
# define OLD_INDEXED_UF_FILES 0x4

/* 
 * pointers to where the actual data is in the record
 */
short *fdata[MAX_UF_FLDS];
static struct UF_for_each_radar *ufer[MAX_RADARS];
static struct generic_radar_info *gri;
static struct uf_useful_items *uui;

static char *power_fields=(char *)POWER_FIELDS;
static char *velocity_fields=(char *)VELOCITY_FIELDS;
static char *toga_projects=(char *)TOGA_PROJECTS;
static char *uf_buf=NULL;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats=NULL;
static float min_fxd_delta=.222;
static struct input_read_info *irq;
static int pbuf_max=2000;
static char preamble[24], postamble[64];
static char *current_file_name;
static char *tmp_radar_name = (char *)"XXXX";

/* c------------------------------------------------------------------------ */

void 
uf_dd_conv (int interactive)
{
    int i, n, rn;
    static int count=0, nok_count=0;


    if(!count) {
	ufx_ini();
    }
    if(interactive) {
	dd_intset();
	uf_positioning();
    }
    uf_reset();

    /* loop through the data
     * c...mark 
     */

    while(1){
	count++;
	if(difs->stop_flag &&
	   (difs->abrupt_start_stop || uui->new_sweep))
	      break;

	if(uf_select_ray()) {
	    rn = uui->current_radar_ndx;
	    if(uui->new_sweep) {
		if(difs->max_sweeps) {
		    if(++ufer[rn]->sweep_count > difs->max_sweeps)
			  break;
		}
		if(uui->new_vol) {
		    if(difs->max_vols)
			  if(++ufer[rn]->vol_count > difs->max_vols)
				break;
		}
	    }
	    if(difs->max_beams) {
		if(++ufer[rn]->ray_count > difs->max_beams )
		      break;
	    }
	    dd_stuff_ray();	/* pass it off to dorade code */
	}
	else {			/* not selected */
	    if(!(nok_count++ % 1000)) {
		uf_print_stat_line(count);
	    }
	}
	uui->new_sweep = NO;
	uui->new_vol = NO;

	if((n = uf_next_ray()) < 1) {
	    break;
	}
    }
    uf_reset();
}
/* c----------------------------------------------------------------------- */

void 
map_uf_ptrs (short *sbuf)
{
   int ii, nn, mark;
   char *cc;
   short *ss, *tt, *vv;
   /*
    * FsCs is FORTRAN subscript to C subscript conversion (subtract 1)
    */
   if(!hostIsLittleEndian()) {
      man = (UF_MANDITORY *)sbuf;
      opt = (UF_OPTIONAL *)(sbuf + FsCs(man->opt_hed_ptr));
      loc = (UF_LOC_USE_AC *)(sbuf + FsCs(man->loc_hed_ptr));
      dhed = (UF_DATA_HED *)(sbuf + FsCs(man->data_hed_ptr));

      cc = (char *)dhed +sizeof(UF_DATA_HED);

      for(ii=0; ii < dhed->num_flds_this_ray; ii++ ) {
	 fidp[ii] = (UF_FLD_ID *)cc;
	 vv = (short *)cc;
	 cc += sizeof(UF_FLD_ID);
	 if ( *vv == 0 || strncmp ("  ", fidp[ii]->id, 2) == 0) {
	   strncpy (fidp[ii]->id, "BB", 2);
	 }
      }
      for(ii=0; ii < dhed->num_flds_this_ray; ii++ ) {
	 fhed[ii] = (UF_FLD_HED *)(sbuf + FsCs(fidp[ii]->fld_hed_ptr));
      }
      for(ii=0; ii < dhed->num_flds_this_ray; ii++ ) {
	 fdata[ii] = sbuf + FsCs(fhed[ii]->fld_data_ptr);
      }
      for(ii=0; ii < dhed->num_flds_this_ray; ii++ ) {
	 gri->actual_num_bins[ii] = fhed[ii]->num_gates;
	 gri->scaled_data[ii] = fdata[ii];
      }

	 /* get length of field header */
	 nn = fhed[0]->fld_data_ptr - fidp[0]->fld_hed_ptr;
	 nn += fhed[0]->num_gates; /* field header plus data */

	 if ( fidp[0]->fld_hed_ptr + dhed->num_flds_this_ray * nn >
	     man->rec_len +1)
	   { 
	      dhed->num_flds_this_ray = 1;
	   }
      mark = 0;
   }
   else {
//      uf_crack_man(sbuf, man, (int)0);  //Jul 26, 2011
      uf_crack_man((char *)sbuf, (char *)man, (int)0);  //Jul 26, 2011
      ss = sbuf + FsCs(man->opt_hed_ptr);
//      uf_crack_opt(ss, opt, (int)0);  //Jul 26, 2011
      uf_crack_opt((char *)ss, (char *)opt, (int)0);  //Jul 26, 2011
      nn = man->data_hed_ptr - man->loc_hed_ptr;

      if(nn > 0 && nn > uui->sizeof_loc_use) {
	 if(xloc) free(xloc);
	 xloc = (UF_LOC_USE_AC *)malloc(nn*sizeof(short));
	 memset(xloc, 0, nn*sizeof(short));
      }
      loc = (UF_LOC_USE_AC *)(sbuf + FsCs(man->loc_hed_ptr));
      ss = sbuf + FsCs(man->data_hed_ptr);
//      swack_short(ss, dhed, 3);  //Jul 26, 2011
      swack_short((char *)ss, (char *)dhed, 3);  //Jul 26, 2011
      ss += 3;

      for(ii=0; ii < dhed->num_flds_this_ray; ii++) {
	 vv = ss;
	 memcpy(fidp[ii]->id, ss++, 2);
	 if ( *vv == 0 || strncmp ("  ", fidp[ii]->id, 2) == 0) {
	   strncpy (fidp[ii]->id, "BB", 2);
	 }
//	 swack2(ss++, &fidp[ii]->fld_hed_ptr);  //Jul 26, 2011
	 swack2((char *)ss++, (char *)&fidp[ii]->fld_hed_ptr);  //Jul 26, 2011
	 tt = sbuf + FsCs(fidp[ii]->fld_hed_ptr);

//	 uf_crack_fhed(tt, fhed[ii], (int)0);  //Jul 26, 2011
	 uf_crack_fhed((char *)tt, (char *)fhed[ii], (int)0);  //Jul 26, 2011
	 gri->actual_num_bins[ii] = fhed[ii]->num_gates;

	 /* get length of field header */
	 nn = fhed[ii]->fld_data_ptr - fidp[ii]->fld_hed_ptr;
	 nn += fhed[ii]->num_gates; /* field header plus data */

	 if ( FsCs( fidp[ii]->fld_hed_ptr) + nn >
	     man->rec_len)
	   { 
	      dhed->num_flds_this_ray = ii;
	      break; 
	   }

	 fdata[ii] = tt = sbuf + FsCs(fhed[ii]->fld_data_ptr);
//	 swack_short(tt, gri->scaled_data[ii], (int)fhed[ii]->num_gates);  //Jul 26, 2011
	 swack_short((char *)tt, (char *)gri->scaled_data[ii], (int)fhed[ii]->num_gates);  //Jul 26, 2011
      }
      mark = 0;
   }
}
/* c----------------------------------------------------------------------- */

void 
rdp_ac_header (int rdn)
{
    int i;
    UF_LOC_USE_AC *lus=(UF_LOC_USE_AC *)loc;

    if(hostIsLittleEndian()) {
//       uf_crack_loc_use_ncar (loc, xloc, (int)0);  //Jul 26, 2011
       uf_crack_loc_use_ncar ((char *)loc, (char *)xloc, (int)0);  //Jul 26, 2011
       lus = xloc;
    }

    if(strstr(ufer[rdn]->radar_name,"SIG") &&
       strstr(ufer[rdn]->project_name, toga_projects))
	  {
	      gri->radar_type = DD_RADAR_TYPE_SHIP;
	  }
    else
	  {
	      gri->radar_type = DD_RADAR_TYPE_AIR_TAIL;
	      gri->scan_mode = DD_SCAN_MODE_AIR;
	  }

    gri->pitch = US64(lus->ac_pitch_x64);
    gri->roll = US64(lus->ac_roll_x64);
    gri->heading = US64(lus->ac_phdg_x64);
    gri->vns = US10(lus->ac_vns_x10);
    gri->vew = US10(lus->ac_vew_x10);
    gri->vud = US10(lus->ac_wp3_x10);
    gri->ui = US10(lus->ac_ui_x10);
    gri->vi = US10(lus->ac_vi_x10);
    gri->wi = US10(lus->ac_wi_x10);

    gri->drift = US64(lus->ac_drift_x64);
    gri->rotation_angle = US64(lus->ac_rotation_x64);
    gri->tilt = US64(lus->ac_tilt_angle_x64);
    gri->corrected_rotation_angle = US64(lus->ac_ant_rot_angle_x64);
}
/* c----------------------------------------------------------------------- */

void 
ucla_ac_header (void)
{
    int i;
    UCLA_LOC_USE_AC *lus=(UCLA_LOC_USE_AC *)loc;

    if(hostIsLittleEndian()) {
//       uf_crack_loc_use_ucla (loc, xloc, (int)0);  //Jul 26, 2011
       uf_crack_loc_use_ucla ((char *)loc, (char *)xloc, (int)0);  //Jul 26, 2011
       lus = (UCLA_LOC_USE_AC *)xloc;
    }

    gri->radar_type = DD_RADAR_TYPE_AIR_TAIL;
    gri->scan_mode = DD_SCAN_MODE_AIR;

    /* you may have to look at the radar name and the
     project name to determine the correct radar type
     */

    gri->pitch = US64(lus->ac_pitch_x64);
    gri->roll = US64(lus->ac_roll_x64);
    gri->heading = US64(lus->ac_phdg_x64);
    gri->vns = US64(lus->tail_vns_x64);
    gri->vew = US64(lus->tail_vew_x64);
    gri->vud = US64(lus->tail_vud_x64);
    gri->ui = EMPTY_FLAG;
    gri->vi = EMPTY_FLAG;
    gri->wi = EMPTY_FLAG;

    gri->drift = EMPTY_FLAG;
    gri->rotation_angle = US64(lus->ac_ant_rot_angle_x64);
    gri->tilt = US64(lus->ac_tilt_angle_x64);
    gri->corrected_rotation_angle = US64(lus->ac_ant_rot_angle_x64);
}
/* c------------------------------------------------------------------------ */

void 
ufx_ini (void)
{
    int ii, jj, nn, len, lenx, mark;
    char *a, *aa, *buf = NULL;
    double d;
    FILE *stream;

    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();


    gri = return_gri_ptr();
# ifdef notyet
    for(ii=0; ii < SRS_MAX_FIELDS; ii++ ) {
	gri->range_bin[ii] = (float *)malloc(SRS_MAX_GATES*sizeof(float));
    }
# endif
    uui = (struct uf_useful_items *)malloc(sizeof(struct uf_useful_items));
    memset((char *)uui, 0, sizeof(struct uf_useful_items));
    
    for(ii=0; ii < MAX_RADARS; ii++ ) {
	ufer[ii] = (struct UF_for_each_radar *)
	      malloc(sizeof(struct UF_for_each_radar));
	memset((char *)ufer[ii], 0, sizeof(struct UF_for_each_radar));
	for(jj=0; jj < SRS_MAX_FIELDS; jj++ ) {
	    ufer[ii]->range_bin[jj] =
		  (float *)malloc(SRS_MAX_GATES*sizeof(float));
	    *ufer[ii]->range_bin[jj] = (float)EMPTY_FLAG;
	}
	ufer[ii]->prev_fxd_angle = -1000.;
    }

    if(hostIsLittleEndian()) {
       man = xman = (UF_MANDITORY *)malloc(sizeof(UF_MANDITORY));
       memset(xman, 0, sizeof(UF_MANDITORY));
       opt = xopt = (UF_OPTIONAL *)malloc(sizeof(UF_OPTIONAL));
       memset(xopt, 0, sizeof(UF_OPTIONAL));
       dhed = xdhed = (UF_DATA_HED *)malloc(sizeof(UF_DATA_HED));
       memset(xdhed, 0, sizeof(UF_DATA_HED));

       for(ii=0; ii < MAX_UF_FLDS; ii++) {
	  fidp[ii] = xfidp[ii] = (UF_FLD_ID *)malloc(sizeof(UF_FLD_ID));
	  memset(xfidp[ii], 0, sizeof(UF_FLD_ID));

	  fhed[ii] = xfhed[ii] = (UF_FLD_HED *)malloc(sizeof(UF_FLD_HED));
	  memset(xfhed[ii], 0, sizeof(UF_FLD_HED));
       }
    }
    uui->radar_count = 0;
    if(a=get_tagged_string("MAX_UF_CELLS")) {
	if((ii = atoi(a)) > 0 )
	      uui->max_num_bins = ii;
    }
    dd_min_rays_per_sweep();	/* trigger min rays per sweep */
    gri->source_format = UF_FMT;
    gri->compression_scheme = NO_COMPRESSION;
    
    if(a=get_tagged_string("RENAME")) {
	uf_rename_str(a);
    }
    if(a=get_tagged_string("WATCH_FIXED_ANGLE")) {
	uui->watch_fxd_angle = YES;
	if(strlen(a)) {
	    d = atof(a);
	    if(d > 0) {
		min_fxd_delta = d;
	    }
	}
    }
    if(a=get_tagged_string("GENERATE_SUBSECOND_TIMES")) {
	uui->do_subsecond_times = YES;
	if(strlen(a)) {
	    d = atof(a);
	    if(d > 0)
		  uui->subsec_increment = d;
	}
    }
    if(a=get_tagged_string("LIDAR_SWEEP_TIME_LIMIT")) {
	if((d = atof(a)) > 0) {
	    uui->sweep_time_limit = d;
	    printf( "Sweep_time_limit: %.3f seconds\n"
		   , uui->sweep_time_limit);
	}
    }
    if((a=get_tagged_string("OPTIONS"))) {
       if(strstr(a, "360_MAX")) {
	  uui->options |= UF_360_MAX_SECTOR;
       }
       if(strstr(a, "FORCE_EOF_VOLS")) {
	  uui->options |= UF_FORCE_EOF_VOLS;
       }
       if(strstr(a, "INDEXED_UF")) {
	  uui->options |= OLD_INDEXED_UF_FILES;
       }
    }
    irq = dd_return_ios(4, UF_FMT);
    aa = NULL;

    if ((aa=get_tagged_string("INPUT_FILES_LIST"))) {
      if(!(stream = fopen(aa, "r"))) {
	printf("Unable to open input files list %s\n", aa);
	exit (1);
      }
      ii = fseek(stream, 0L, SEEK_END); /* at end of file */
      len = ftell(stream);	/* how big is the file */
      rewind(stream);
      
      buf = (char *)malloc(len);
      memset (buf, 0, len);
      if((lenx = fread(buf, sizeof(char), len, stream)) < len) {
	mark = 0;
	/* complain */
      }
      fclose(stream);
    }
    else {
      buf = get_tagged_string("SOURCE_DEV");
    }
    if (!buf) {
      printf ("No SOURCE_DEV\n");
      exit (1);
    }
    dd_establish_source_dev_names((char *)"UF", buf);
    aa = dd_next_source_dev_name((char *)"UF");
    current_file_name = aa;
    dd_input_read_open(irq, aa);

    /* read in the first ray */
    nn = uf_next_ray();
}
/* c------------------------------------------------------------------------ */

int 
uf_inventory (struct input_read_info *irq)
{
    /* the purpose of this routine is to facilitate a more detailed
     * examination of the data
     */
    int ii=0, max=gri_max_lines();
    int nn, ival, really_break=NO;
    float val;
    char str[256];
    double d;


    for(;;) {
	for(ii=0; ii < max; ii++) {
	    if((nn=uf_next_ray()) <= 0) {
		really_break = YES;
		break;
	    }
	    uui->new_sweep = NO;
	    uui->new_vol = NO;
	    sprintf(preamble, "%2d", irq->top->b_num);
	    gri_interest(gri, 0, preamble, postamble);
	}
	if(really_break)
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

int 
uf_logical_read (void)
{
    int nn, eof_count = 0, size = -1, ii, mark;
    unsigned short rlen = 0;
    char *aa, *fofsv=(char *)"FOFSV0.0";
    static int local_rec_count = 0, ray_trip = 111;
    static FILE_HEADER *fthp;
    TABLE_HEADER *thp;
    static RECORD_TABLE_HEADER *rthp;
    static RECORD_TABLE *rectbl;
    static RECORD_TABLE_ENTRY *rtep;



    if (dd_stats->ray_count > ray_trip) {
       mark = 0;
    }

    for(;;) {
	if(irq->io_type == BINARY_IO ) {
	    if (irq->top->bytes_left > strlen (fofsv) &&
		strncmp (irq->top->at, fofsv, strlen (fofsv)) == 0) {
	       fthp = (FILE_HEADER *)irq->top->at;
	       rthp = (RECORD_TABLE_HEADER *)(irq->top->at + sizeof (*fthp));
	       rectbl = (RECORD_TABLE *)(irq->top->at
					     +rthp->header.bytes2table);
	       rtep = (RECORD_TABLE_ENTRY *)(irq->top->at
					     +rthp->header.bytes2table);
	       /* skip 64K bytes of header info */
	       irq->top->at += K64;
	       irq->top->bytes_left -= K64;
	       uui->options |= OLD_INDEXED_UF_FILES;
	       continue;
	    }
	    if( irq->top->bytes_left >= 4 ) {
		if(hostIsLittleEndian()) {
		    swack2( irq->top->at +2, (char *)&rlen );  //Jul 26, 2011
		}
		else {
		    rlen = *((unsigned short *)(irq->top->at + 2));
		}
		if( rlen * sizeof(short) <= irq->top->bytes_left ) {
		    /*
		     * we have the next ray completely with the
		     * record buffer
		     */
		    size = rlen * sizeof(short);
		    if (uui->options & OLD_INDEXED_UF_FILES &&
			rlen == 1604) {
		       irq->top->at += rlen * sizeof (short);
		       irq->top->bytes_left -= rlen * sizeof (short);
		       continue;
		    }
		    break;
		}
	    }
	    if( irq->top->bytes_left > 0 ) {
		dd_io_reset_offset(irq, irq->top->offset
				   + irq->top->sizeof_read
				   - irq->top->bytes_left);
	    }
	    irq->top->bytes_left = 0;
	}

	dd_logical_read(irq, FORWARD);

	if(irq->top->read_state < 0 || eof_count > 2 ) {
	    printf("Last read: %d\n", irq->top->read_state);
	    nn = irq->top->read_state;
	    dd_input_read_close(irq);
	    eof_count = 0;
	    /*
	     * see if there is another file to read
	     */
	    if(aa = dd_next_source_dev_name((char *)"UF")) {
		current_file_name = aa;
		if((ii = dd_input_read_open(irq, aa)) <= 0) {
		    return(-1);
		}
		local_rec_count = 0;
	    }
	    else
		{ return -1; }
	    irq->top->bytes_left = 0;
	    continue;
	}
	else if(irq->top->eof_flag) {
	    eof_count++;
	    dd_stats->file_count++;
	    printf( "EOF number: %d at %.2f MB\n"
		    , dd_stats->file_count
		    , dd_stats->MB);
	    irq->top->bytes_left = 0;
	    if( uui->options & UF_FORCE_EOF_VOLS )
		{ uui->new_vol = YES; }
	    continue;
	}
	else if( irq->top->bytes_left < sizeof( UF_MANDITORY ) ) {
	    irq->top->bytes_left = 0;
	    continue;
	}
	else if(irq->io_type == BINARY_IO ) {
	    continue;
	}
	else {
	    eof_count = 0;
	    size = irq->top->bytes_left;
	    break;
	}
    }
    local_rec_count++;
    return( size );
}
/* c------------------------------------------------------------------------ */

int 
uf_next_ray (void)
{
    /* read in the next ray */

    static int rec_num=0;
    char radar_name[12], *aa;
    int i, ii, n, nn, rdn, eof_count=0, err_count=0, new_vol=NO, size = -1;
    double del;
    DD_TIME *dts=gri->dts;
    double d;
    UF_FLD_HED *fh;
    struct rename_str *rns;

# define ENABLE_BINARY_IO

# ifndef ENABLE_BINARY_IO
    while(1) {			/* original code */
	if(dd_control_c()) {
	    dd_reset_control_c();
	    return(-1);
	}
	dd_logical_read(irq, FORWARD);
	
	if( ( nn = irq->top->read_state ) < 0) {
	    if( ++err_count < 4 ) {
		printf("Last read: %d errno: %d\n"
		       , irq->top->read_state, err_count);
		continue;
	    }
	    dd_input_read_close(irq);
	    /*
	     * see if there is another file to read
	     */
	    if(aa = dd_next_source_dev_name("UF")) {
		current_file_name = aa;
		if((ii = dd_input_read_open(irq, aa)) <= 0) {
		    return(nn);
		}
		err_count = 0;
		eof_count = 0;
		continue;
	    }
	    return(nn);
	}
	else if(irq->top->eof_flag) {
	    dd_stats->file_count++;
	    printf("EOF: %d at %.2f MB\n"
		   , dd_stats->file_count, dd_stats->MB);
	    if(++eof_count > 3) {
		dd_input_read_close(irq);
		/*
		 * see if there is another file to read
		 */
		if(aa = dd_next_source_dev_name("UF")) {
		    current_file_name = aa;
		    if((ii = dd_input_read_open(irq, aa)) <= 0) {
			return(-1);
		    }
		    err_count = eof_count = 0;
		    continue;
		}
		return(-1);
	    }
	}
	else {
	    err_count = 0; eof_count = 0;
	    dd_stats->rec_count++;
	    dd_stats->MB += BYTES_TO_MB(irq->top->sizeof_read);
	    rec_num++;
	    uf_buf = irq->top->buf;
	    map_uf_ptrs((short *)irq->top->buf);
	    break;
	}
    }
#else
    /* new BINARY_IO code
     * assumes the record lengths in the manditory header are spot on
     */
    uui->sizeof_this_ray = 0;
    if(( size = uf_logical_read()) <= 0 ) {
	return( size );
    }
    dd_stats->rec_count++;
    dd_stats->MB += BYTES_TO_MB(irq->top->sizeof_read);
    rec_num++;
    uf_buf = irq->top->at;
    map_uf_ptrs((short *)irq->top->at);
    irq->top->at += size;
    irq->top->bytes_left -= size;
    uui->sizeof_this_ray = size;
# endif

    /* identify the radar
     */
    str_terminate(gri->radar_name, man->radar, 8);
    if (strlen (gri->radar_name) < 1)
      { strcpy (gri->radar_name, tmp_radar_name); }

    if(rns=uui->top_ren) {
	for(;;) {
	    if(strstr(gri->radar_name, rns->old_name)) {
		strcpy(gri->radar_name, rns->new_name);
		break;
	    }
	    if((rns=rns->next) == uui->top_ren)
		  break;
	}
    }    
    strcpy (radar_name, gri->radar_name);
# ifdef obsolete
    str_terminate(radar_name, man->radar, 8);
    if (strlen (radar_name) < 1)
      { strcpy (radar_name, tmp_radar_name); }
# endif

    for(rdn=0; rdn < uui->radar_count; rdn++ ) {
	if(strcmp(radar_name, ufer[rdn]->radar_name) == 0) {
	    break;
	}
    }
    if( rdn == uui->radar_count ) {
	/* new radar!
	 */
	strcpy(ufer[rdn]->radar_name, radar_name);
	str_terminate(ufer[rdn]->project_name, opt->project_id, 8);
	ufer[rdn]->radar_num = dd_assign_radar_num(ufer[rdn]->radar_name);
	dd_radar_selected(ufer[rdn]->radar_name, ufer[rdn]->radar_num
			  , difs);
	uui->radar_count++;
	uui->prev_radar_num = EMPTY_FLAG;
    }
    uui->current_radar_ndx = rdn;
    gri->dd_radar_num = ufer[rdn]->radar_num;
    gri->time = uf_time_stamp(dts);
    if(uui->do_subsecond_times) {
	if(gri->time != uui->trip_time) {
	    uui->count_since_trip = 0;
	    uui->trip_time = gri->time;
	}
	else {
	    if(uui->subsec_increment) {
		gri->time += uui->count_since_trip * uui->subsec_increment;
	    }
	    else {		/* nab num hits and prt from first field */
		gri->time += uui->count_since_trip * fhed[0]->num_samples *
		      (double)fhed[0]->prT_xe6 * 1.e-6;
	    }
	}
	uui->count_since_trip++;
    }
    gri->azimuth = US64(man->azimuth_x64);
    gri->elevation = US64(man->elevation_x64);

    if( uui->options & UF_360_MAX_SECTOR ) {
       d = angdiff( ufer[rdn]->prev_rotang, gri->azimuth );
       if( fabs( d ) < 5. ) {
	  ufer[rdn]->sum_rotang_diff += d;
	  
       }
       ufer[rdn]->prev_rotang = gri->azimuth;
    }

    gri->dts->time_stamp = gri->time;
    uui->ok_ray = YES;
    dd_stats->ray_count++;

    new_vol = ufer[rdn]->srs_vol_num != man->vol_num
       || gri->dd_radar_num != uui->prev_radar_num;
    if( new_vol || uui->new_vol ) {
	/*
	 * different radar or new volume
	 * update radar specific parameters
	 */
	uui->prev_radar_num = gri->dd_radar_num;
	ufer[rdn]->srs_vol_num = man->vol_num;
	gri->source_vol_num = man->vol_num;
	gri->sweep_num = 0;
	gri->vol_num = ++ufer[rdn]->vol_num;
	/* this assumes what follows is a contiguous volume of data
	 */
	uf_vol_update();
	uui->new_vol = YES;
	uui->new_sweep = YES;
	dd_stats->vol_count++;
    }
    if(uui->watch_fxd_angle) {
	del = US64(man->fixed_angle_x64) - ufer[rdn]->prev_fxd_angle;
	if(fabs(del) > min_fxd_delta) {
	    uui->new_sweep = YES;
	}
    }
    else if(ufer[rdn]->srs_sweep_num != man->swp_num)
      { uui->new_sweep = YES;
      }
    else if( ufer[rdn]->sweep_ray_num >= dd_max_rays_per_sweep()) 
      { uui->new_sweep = YES;
      }
    else if(uui->sweep_time_limit) {
	if(gri->time > ufer[rdn]->sweep_reference_time) {
	    uui->new_sweep = YES;
	}
    }
    else if( uui->options & UF_360_MAX_SECTOR ) {
       if( FABS(ufer[rdn]->sum_rotang_diff) >= 360. ) {
	    uui->new_sweep = YES;
       }
    }
    /*
     * update generic info
     */
    if(uui->new_sweep) {
	ufer[rdn]->sweep_reference_time = gri->time + uui->sweep_time_limit;
	ufer[rdn]->srs_sweep_num = man->swp_num;
	ufer[rdn]->sum_rotang_diff = 0;
	ufer[rdn]->sweep_ray_num = 0;
	gri->source_sweep_num = man->swp_num;
	gri->sweep_num++;
	ufer[rdn]->prev_fxd_angle = 
	      gri->fixed = US64(man->fixed_angle_x64);
 	dd_stats->sweep_count++;
    }
    ufer[rdn]->sweep_ray_num++;
# ifdef obsolete
    gri->latitude = man->lat_deg +man->lat_min*RCP60
	  + man->lat_sec_x64*RCP64*RCP3600;
    gri->longitude = man->lon_deg +man->lon_min*RCP60
	  + man->lon_sec_x64*RCP64*RCP3600;
# else
    d = FABS(man->lat_min*RCP60) + FABS(man->lat_sec_x64*RCP64*RCP3600);
    gri->latitude = man->lat_deg < 0 ? (double)man->lat_deg -d :
	  (double)man->lat_deg +d;
    d = FABS(man->lon_min*RCP60) + FABS(man->lon_sec_x64*RCP64*RCP3600);
    gri->longitude = man->lon_deg < 0 ? (double)man->lon_deg -d :
	  (double)man->lon_deg +d;
# endif
    gri->altitude = man->altitude_msl;

# ifdef obsolete
    gri->year = man->year;
    gri->month = man->month;
    gri->day = man->day;
    gri->hour = man->hour;
    gri->minute = man->minute;
    gri->second = man->second;
    if((man->loc_hed_ptr-man->opt_hed_ptr) >= 15 &&
	opt->seconds_xe3 != man->missing_data_flag)
	  gri->millisecond = (float)opt->seconds_xe3;
    else
	  gri->millisecond = 0;
# endif

    switch(man->sweep_mode) {
    case RHI:
	gri->rotation_angle = gri->corrected_rotation_angle
	      = US64(man->elevation_x64);
	gri->tilt = US64(man->azimuth_x64);
	break;
    default:
	gri->rotation_angle = gri->corrected_rotation_angle
	      = US64(man->azimuth_x64);
	gri->tilt = US64(man->elevation_x64);
	break;
    }
    
    uui->moving_platform = NO;
    
    if((n=man->data_hed_ptr-man->loc_hed_ptr) >= 20 ) {
	/* see if this is one of the two known
	 * moving platform headers
	 */
	if(strncmp(loc->id, "AIR", 3) == 0) {
	    rdp_ac_header(rdn);
	    uui->moving_platform = YES;
	}
	else if(strncmp(loc->id, "INE", 3) == 0) {
	    ucla_ac_header();
	    uui->moving_platform = YES;
	}
    }
    return(irq->top->sizeof_read);
}
/* c------------------------------------------------------------------------ */

void 
uf_positioning (void)
{
    int ii, jj, kk, mm, mark, direction;
    int nn, ival;
    float val;
    char str[256];
    DD_TIME dts;
    static double skip_secs=0;
    static float fx1=.8, fx2=1.2;
    static float fxd_ang_tol=.2;
    float f;
    double d, dtarget;
    static int view_char=0, view_char_count=200;
    static char *pbuf=NULL, **pbuf_ptrs=NULL;

    
    gri_interest(gri, 1, preamble, postamble);
    pbuf = (char *)malloc(80*pbuf_max);
    memset(pbuf, 0, 80*pbuf_max);
    pbuf_ptrs = (char **)malloc(sizeof(char *)*pbuf_max);
    memset(pbuf_ptrs, 0, sizeof(char *)*pbuf_max);
    *pbuf_ptrs = pbuf;

 menu2:
    dd_reset_control_c();

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
 9 = Fixed angle search\n\
10 = Set input limits\n\
11 = Display headers\n\
12 = Display data\n\
Option = "
	   );
    nn = getreply(str, sizeof(str));
    if( cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 14 ) {
	if(ival == -2) exit(1);
	printf( "\nIllegal Option!\n" );
	goto menu2;
    }

    if(ival == -1) {
	free(pbuf);
	free(pbuf_ptrs);
	return;
    }
    else if(ival < 0)
	  exit(0);

    uui->new_sweep = NO;
    uui->new_vol = NO;

    if(ival == 0 || ival == 7) {
	printf("Type record skip # or hit <return> to read next rec:");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > K64) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	direction = ival >= 0 ? FORWARD : BACKWARD;

	if( irq->io_type == BINARY_IO ) {
	    if( direction == BACKWARD ) {
		printf( "Can't go BACKWARD for BINARY_IO!\n" );
	    }
	    else {
		for(ii = ival; ii--; ) {
		    if((jj = uf_next_ray()) < 1 )
			{ break; }
		}
	    }
	}
	else {
	    dd_skip_recs(irq, direction, ival > 0 ? ival : -ival);
	}
	uf_reset();
	uf_next_ray();
	nn = uui->sizeof_this_ray;
	printf("\n Read %d bytes\n", nn);
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 1) {
	if((nn = uf_inventory(irq)) == -2)
	      exit(0);
    }
    else if( ival == 2 && irq->io_type == BINARY_IO ) {
	printf( "Can't do it for BINARY_IO!\n" );
    }
    else if(ival == 2) {
	printf("Type skip: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > K64) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	if(ival) {
	    direction = ival >= 0 ? FORWARD : BACKWARD;
	    dd_skip_files(irq, direction, ival >= 0 ? ival : -ival);
	}
	dd_stats_reset();
	uf_reset();
	uf_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 3) {
	dd_rewind_dev(irq);
	dd_stats_reset();
	uf_reset();
	uf_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 4) {
	if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
	    printf("\n");
	    ctypeu16((unsigned char *)irq->top->buf, view_char, view_char_count);  //Jul 26, 2011
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
    else if(ival == 8) {
	dtarget = 0;
	dd_clear_dts(&dts);

	printf("Type target time (e.g. 14:22:55 or +2h, +66m, +222s): ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) == 1 && !ival) {
	    /* assume they have hit a <return>
	     */
	    if(skip_secs > 0)
		 dtarget = gri->time + skip_secs; 
	}
	else if(skip_secs = dd_relative_time(str)) {
	    dtarget = gri->time + skip_secs;
	}
	else if(kk = dd_crack_datime(str, nn, &dts)) {
	    if(!dts.year) dts.year = gri->dts->year;
	    if(!dts.month) dts.month = gri->dts->month;
	    if(!dts.day) dts.day = gri->dts->day;
	    dtarget = d_time_stamp(&dts);
	}
	if(!dtarget) {
	    printf( "\nCannot derive a time from %s!\n", str);
	    goto menu2;
	}
	printf("Skipping ahead %.3f secs\n", dtarget-gri->time);
	/* loop until the target time
	 */
	for(mm=1;; mm++) {
	    if((nn = uf_next_ray()) <= 0 || gri->time >= dtarget ||
	       dd_control_c()) {
		break;
	    }
	    if(!(mm % 1000))
		  gri_interest(gri, 1, preamble, postamble);
	    mark = 0;
	}
	if(nn)
	      gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 9) {
	printf("Fixed angle: (e.g. \"1.0  0.2\" => (0.8 <= fxd < 1.2))  ");
	nn = getreply(str, sizeof(str));
	if((mm=cdcode(str, nn, &ival, &val)) == 1 && !ival) {
	    /* assume they have hit a <return>
	     */
	}
	else if((mm=sscanf(str, "%f%f", &f, &fxd_ang_tol)) == 2) {
	    fx1 = f -fxd_ang_tol;
	    fx2 = f +fxd_ang_tol;
	}
	else if((mm=sscanf(str, "%f", &f)) == 1) {
	    fx1 = f -fxd_ang_tol;
	    fx2 = f +fxd_ang_tol;
	}
	else {
	    printf( "\nBad fixed angle spec: %s!\n", str);
	    goto menu2;
	}
	printf("Condition: %.2f <= fxd < %.2f\n", fx1, fx2);
	/* loop until the target time
	 */
	for(mm=1;; mm++) {
	    if((nn = uf_next_ray()) <= 0 || in_sector(gri->fixed, fx1, fx2)
	       || dd_control_c()) {
		break;
	    }
	    if(!(mm % 1000))
		  gri_interest(gri, 1, preamble, postamble);
	    mark = 0;
	}
	if(nn)
	      gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 10) {
	gri_nab_input_filters(gri->time, difs, 0);
    }
    else if(ival == 11) {
	uf_print_headers((short *)irq->top->buf, pbuf_ptrs);
    }
    else if(ival == 12) {
	uf_print_data((short *)irq->top->buf, pbuf_ptrs);
    }
    else if(ival == 14) {
    }
    preamble[0] = '\0';

    goto menu2;


}
/* c------------------------------------------------------------------------ */

char **
uf_print_data (short *uf, char **ptrs)
{
    int gg, ii, nf, ngx=0;
    short *ss, *dh;
    char ts[8];
    char **pt=ptrs;
    float unscale[7], ff;
    short *flds[7];
    int ng[7];
    UF_FLD_HED *fh;


    /* setup pointers to the field headers
     */
    nf = dhed->num_flds_this_ray;
    nf = nf > 7 ? 7 : nf;

    sprintf(*pt, "\nContents of the UF fields");
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "name ");

    for(ii=0; ii < nf; ii++) {
	sprintf(*pt+strlen(*pt), "        %s"
		, str_terminate(ts, fidp[ii]->id, 2)); /* nab the name */

	fh = fhed[ii];
	flds[ii] = gri->scaled_data[ii];
	unscale[ii] = fh->scale ? 1./fh->scale : 1.0;
	ng[ii] = fh->num_gates;
	if(ng[ii] > ngx) ngx = ng[ii];
    }
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "r0:  ");

    for(ii=0; ii < nf; ii++) {
	ff = fhed[ii]->range_g1 + .001 * fhed[ii]->g1_adjust_xe3;
	sprintf(*pt+strlen(*pt), "%10.3f", ff);
    }

    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "gs:  ");

    for(ii=0; ii < nf; ii++) {
	sprintf(*pt+strlen(*pt), "%10.3f", .001 * fhed[ii]->gate_spacing_xe3);
    }

    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ng:  ");

    for(ii=0; ii < nf; ii++) {
	sprintf(*pt+strlen(*pt), "%10d", fhed[ii]->num_gates);
    }

    *(pt+1) = *pt +strlen(*pt) +1;

    for(gg=0; gg < ngx; gg++) {
	sprintf(*(++pt), "%4d)", gg);

	for(ii=0; ii < nf; ii++) {
	    if(gg < ng[ii]) {
		sprintf(*pt+strlen(*pt), "%10.2f"
			, (float)(*(flds[ii]+gg)) * unscale[ii]);
	    }
	    else {
		sprintf(*pt+strlen(*pt), "          ");
	    }
	}
	*(pt+1) = *pt +strlen(*pt) +1;
    }
    *(++pt) = NULL;
    gri_print_list(ptrs);

    return(pt);
}
/* c------------------------------------------------------------------------ */

char **
uf_print_dhed (short *dh, char **ptrs)
{
    int ii, nf;
    short *ss=dh;
    char str[128];
    char **pt=ptrs;


    sprintf(*pt, "\nContents of the UF data header");
    *(pt+1) = *pt +strlen(*pt) +1;

    sprintf(*(++pt), " num_flds_this_ray %d", *ss++);
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), " num_recs_this_ray %d", *ss++);
    *(pt+1) = *pt +strlen(*pt) +1;
    nf = *ss;
    sprintf(*(++pt), " num_flds_this_rec %d", *ss++);
    *(pt+1) = *pt +strlen(*pt) +1;

    for(ii=0; ii < nf; ii++) {
        sprintf(*(++pt), "field:%2d  %s  pointer:%4d"
		, ii+1, str_terminate(str, fidp[ii]->id, 2)
		, fidp[ii]->fld_hed_ptr);
        *(pt+1) = *pt +strlen(*pt) +1;
    }
    return(++pt);
}
/* c------------------------------------------------------------------------ */

char **
uf_print_fldh (UF_FLD_HED *fld, char **ptrs, char *name)
{
    int ii;
    char str[128];
    char **pt=ptrs;


    sprintf(*pt, "\nContents of the field header for %s"
	    , str_terminate(str, name, 2));
    *(pt+1) = *pt +strlen(*pt) +1;

    sprintf(*(++pt), "fld_data_ptr     %d", fld->fld_data_ptr);     
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "scale            %d", fld->scale);            
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "range_g1         %d", fld->range_g1);         
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "g1_adjust_xe3    %d", fld->g1_adjust_xe3);    
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "gate_spacing_xe3 %d", fld->gate_spacing_xe3); 
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "num_gates        %d", fld->num_gates);        
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "sample_vol_xe3   %d", fld->sample_vol_xe3);   
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "h_beam_width_x64 %.2f", US64(fld->h_beam_width_x64)); 
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "v_beam_width_x64 %.2f", US64(fld->v_beam_width_x64)); 
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "rec_bw_xe6       %.6f", 1.e-6*fld->rec_bw_xe6);       
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "polarization     %d", fld->polarization);     
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "waveln_x6400     %.2f", US64(fld->waveln_x6400));     
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "num_samples      %d", fld->num_samples);      
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "thr_fld[2]       %s"
	    , str_terminate(str, fld->thr_fld, 2));       
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "thr_val          %d", fld->thr_val);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "thr_val_scale    %d", fld->thr_val_scale);    
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "edit_code[2]     %s"
	    , str_terminate(str, fld->edit_code, 2));     
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "prT_xe6          %.6f", 1.e-6*fld->prT_xe6);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "bits_per_datum   %d", fld->bits_per_datum);   
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_20	       %d", fld->word_20);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_21	       %d", fld->word_21);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_22	       %d", fld->word_22);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_23	       %d", fld->word_23);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_24	       %d", fld->word_24);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_25	       %d", fld->word_25);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_26	       %d", fld->word_26);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_27	       %d", fld->word_27);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "az_adjust_xe3    %d", fld->az_adjust_xe3);    
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "el_adjust_xe3    %d", fld->el_adjust_xe3);    
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "gt_sp_adjust_xe6 %d", fld->gt_sp_adjust_xe6); 
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "alt_scale        %d", fld->alt_scale);        
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "alt_bias         %d", fld->alt_bias);         
    *(pt+1) = *pt +strlen(*pt) +1;

    return(++pt);
}
/* c------------------------------------------------------------------------ */

void 
uf_print_headers (short *uf, char **ptrs)
{
    char **pt=ptrs;

    short *ss;
    int ii, jj, nf;
    char *str[16];

    pt = uf_print_man(man, pt);

    if(man->loc_hed_ptr > man->opt_hed_ptr) {
       pt = uf_print_opt(opt, pt);
    }
    pt = uf_print_lus(uf, pt);

    pt = uf_print_dhed(&dhed->num_flds_this_ray, pt);

    for(ii=0; ii < dhed->num_flds_this_ray; ii++) {
       pt = uf_print_fldh(fhed[ii], pt, fidp[ii]->id);
    }
    *pt = NULL;
    gri_print_list(ptrs);
}
/* c------------------------------------------------------------------------ */

char **
uf_print_lus (short uf[], char **ptrs)
{
    int ii, lundx, len;
    short *ss;
    char **pt=ptrs;
    char *aa = (char *)loc;

# ifdef obsolete
    lundx = uf[FsCs(4)];
    ss = uf + FsCs(lundx);
    len = uf[FsCs(5)] -lundx;
# endif
    lundx = man->loc_hed_ptr;
    len = man->data_hed_ptr - man->loc_hed_ptr;

    if(len > 0 && strncmp(aa, "AIR", 3) == 0) {
	pt = uf_print_lus_ac(loc, ptrs);
    }
    else if(len > 0 && strncmp(aa, "INE", 3) == 0) {
	pt = uf_print_lus_ucla((UCLA_LOC_USE_AC *)loc, ptrs);
    }
    else {
       if(hostIsLittleEndian()) {
//	  swack_short(loc, xloc, len);  //Jul 26, 2011
	  swack_short((char *)loc, (char *)xloc, len);  //Jul 26, 2011
	  pt = uf_print_lus_gen((short *)xloc, lundx, len, ptrs);
       }
       else {
	  pt = uf_print_lus_gen((short *)loc, lundx, len, ptrs);
       }
    }
    return(pt);
}
/* c------------------------------------------------------------------------ */

char **
uf_print_lus_ac (UF_LOC_USE_AC *lus, char **ptrs)
{
    int ii;
    char str[128];
    char **pt=ptrs;


    if(hostIsLittleEndian()) {
       lus = (UF_LOC_USE_AC *)xloc;
    }
    sprintf(*pt, "\nContents of the UF local use header");
    *(pt+1) = *pt +strlen(*pt) +1;

    sprintf(*(++pt), "id[4]                %s"
	    , str_terminate(str, lus->id, 4));
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_pitch_x64         %.2f", US64(lus->ac_pitch_x64));        
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_roll_x64          %.2f", US64(lus->ac_roll_x64));         
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_phdg_x64          %.2f", US64(lus->ac_phdg_x64));         
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_vns_x10           %.1f", US10(lus->ac_vns_x10));          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_vew_x10           %.1f", US10(lus->ac_vew_x10));          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_wp3_x10           %.1f", US10(lus->ac_wp3_x10));          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_hgme              %d", lus->ac_hgme);             
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_ui_x10            %.1f", US10(lus->ac_ui_x10));           
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_vi_x10            %.1f", US10(lus->ac_vi_x10));           
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_wi_x10            %.1f", US10(lus->ac_wi_x10));           
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_drift_x64         %.2f", US64(lus->ac_drift_x64));        
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_rotation_x64      %.2f", US64(lus->ac_rotation_x64));     
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_tilt_x64          %.2f", US64(lus->ac_tilt_x64));         
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_phdg_change_x64   %.2f"
	    , US64(lus->ac_phdg_change_x64));  
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_pitch_change_x64  %.2f"
	    , US64(lus->ac_pitch_change_x64)); 
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_ant_rot_angle_x64 %.2f"
	    , US64(lus->ac_ant_rot_angle_x64));
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_tilt_angle_x64    %.2f"
	    , US64(lus->ac_tilt_angle_x64));   
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_20              %d", lus->word_20);             
    *(pt+1) = *pt +strlen(*pt) +1;

    return(++pt);
}
/* c------------------------------------------------------------------------ */

char **
uf_print_lus_gen (short *lus, int lundx, int len, char **ptrs)
{
    int ii, jj, kk;
    short *ss=lus;
    char str[128];
    char **pt=ptrs;
    
    
    if(len < 1) {
	sprintf(*pt, "\nNo UF local use header!");
	*(pt+1) = *pt +strlen(*pt) +1;
	return(++pt);
    }

    sprintf(*pt, "\nContents of the UF local use header");
    *(pt+1) = *pt +strlen(*pt) +1;
    
    for(ii=0; len--;) {
	sprintf(*(++pt), " %2d)(%2d) %d", lundx++, ii++, *ss++);
	*(pt+1) = *pt +strlen(*pt) +1;
    }
    return(++pt);
}
/* c------------------------------------------------------------------------ */

char **
uf_print_lus_ucla (UCLA_LOC_USE_AC *lus, char **ptrs)
{
    int ii;
    char str[128];
    char **pt=ptrs;


    if(hostIsLittleEndian()) {
       lus = (UCLA_LOC_USE_AC *)xloc;
    }
    sprintf(*pt, "\nContents of the UF-UCLA local use header");
    *(pt+1) = *pt +strlen(*pt) +1;

    sprintf(*(++pt), "id[10]                %s"
	, str_terminate(str, lus->id, 10));                   
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "flight_id[8]          %s"
	, str_terminate(str, lus->flight_id, 8));            
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_10               %d", lus->word_10);              
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_11               %d", lus->word_11);              
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_phdg_x64           %.2f", US64(lus->ac_phdg_x64));          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_roll_x64           %.2f", US64(lus->ac_roll_x64));          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_pitch_x64          %.2f", US64(lus->ac_pitch_x64));         
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_drift_x64          %.2f", US64(lus->ac_drift_x64));         
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_vud_x64            %.2f", US64(lus->ac_vud_x64));           
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "tail_vew_x64          %.2f", US64(lus->tail_vew_x64));         
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "tail_vns_x64          %.2f", US64(lus->tail_vns_x64));         
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "tail_vud_x64          %.2f", US64(lus->tail_vud_x64));         
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_20               %d", lus->word_20);              
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_21               %d", lus->word_21);              
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_ant_rot_angle_x64  %.2f"
	    , US64(lus->ac_ant_rot_angle_x64)); 
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ac_tilt_angle_x64     %.2f"
	    , US64(lus->ac_tilt_angle_x64));    
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_24               %d", lus->word_24);              
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_25               %d", lus->word_25);              
    *(pt+1) = *pt +strlen(*pt) +1;

    return(++pt);
}
/* c------------------------------------------------------------------------ */

char **
uf_print_man (UF_MANDITORY *mh, char **ptrs)
{
    int ii;
    char str[128];
    char **pt=ptrs;


    sprintf(*pt, "\nContents of the UF manditory header");

    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "id[2]             %s"
	    , str_terminate(str, mh->id, 2));
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "rec_len           %d", mh->rec_len);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "opt_hed_ptr       %d", mh->opt_hed_ptr);      
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "loc_hed_ptr       %d", mh->loc_hed_ptr);      
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "data_hed_ptr      %d", mh->data_hed_ptr);     
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "rec_num           %d", mh->rec_num);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "vol_num           %d", mh->vol_num);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "ray_num           %d", mh->ray_num);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "prec_in_ray       %d", mh->prec_in_ray);      
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "swp_num           %d", mh->swp_num);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "radar[8]          %s"
	    , str_terminate(str, mh->radar, 8));
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "site[8]           %s"
	    , str_terminate(str, mh->site, 8));
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "lat_deg           %d", mh->lat_deg);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "lat_min           %d", mh->lat_min);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "lat_sec_x64       %.2f", US64(mh->lat_sec_x64));      
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "lon_deg           %d", mh->lon_deg);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "lon_min           %d", mh->lon_min);          
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "lon_sec_x64       %.2f", US64(mh->lon_sec_x64));      
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "altitude_msl      %d", mh->altitude_msl);     
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "year              %d", mh->year);             
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "month             %d", mh->month);            
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "day               %d", mh->day);              
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "hour              %d", mh->hour);             
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "minute            %d", mh->minute);           
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "second            %d", mh->second);           
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "time_zone[2]      %s"
	    , str_terminate(str, mh->time_zone, 2));
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "azimuth_x64       %.2f", US64(mh->azimuth_x64));      
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "elevation_x64     %.2f", US64(mh->elevation_x64));    
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "sweep_mode        %d", mh->sweep_mode);       
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "fixed_angle_x64   %.2f", US64(mh->fixed_angle_x64));  
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "sweep_rate_x64    %.2f", US64(mh->sweep_rate_x64));   
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "gen_date_yy       %d", mh->gen_date_yy);      
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "gen_date_mm       %d", mh->gen_date_mm);      
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "gen_date_dd       %d", mh->gen_date_dd);      
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "facility_id[8]    %s"
	    , str_terminate(str, mh->facility_id, 8));
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "missing_data_flag %d", mh->missing_data_flag);
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "word_46           %d", mh->word_46);          
    *(pt+1) = *pt +strlen(*pt) +1;

    return(++pt);
}
/* c------------------------------------------------------------------------ */

char **
uf_print_opt (UF_OPTIONAL *opt, char **ptrs)
{
    int ii;
    char str[128];
    char **pt=ptrs;


    sprintf(*pt, "\nContents of the UF optional header");

    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "project_id[8]   %s"
	    , str_terminate(str, opt->project_id, 8));     
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "baseline_az_x64 %d", opt->baseline_az_x64);
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "baseline_el_x64 %d", opt->baseline_el_x64);
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "vol_hour        %d", opt->vol_hour);       
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "vol_minute      %d", opt->vol_minute);     
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "vol_second      %d", opt->vol_second);     
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "fld_tape_id[8]  %s"
	    , str_terminate(str, opt->fld_tape_id, 8));    
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "range_info_flag %d", opt->range_info_flag);
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "seconds_xe3     %d", opt->seconds_xe3);    
    *(pt+1) = *pt +strlen(*pt) +1;
    sprintf(*(++pt), "software_rev    %d", opt->software_rev);   
    *(pt+1) = *pt +strlen(*pt) +1;

    return(++pt);
}
/* c------------------------------------------------------------------------ */

void 
uf_print_stat_line (int count)
{
    time_t i;
    double d;
    DD_TIME *dts=gri->dts;
    
    d_unstamp_time(dts);
    
    d = gri->time;
    i = (time_t)d;
    printf(" %5d %3d %6.2f %.2f %s"
	   , count
	   , gri->sweep_num
	   , gri->fixed
	   , d
	   , dts_print(dts)
	   );
}
/* c------------------------------------------------------------------------ */

void 
uf_rename_str (char *a)
{
    /* routine to construct the list of aliases */
    struct rename_str *this_name, *last;  //Jul 26, 2011 *this issue
    char string_space[256], *str_ptrs[32];
    int ii, nt;

    strcpy(string_space, a);
    nt = dd_tokens(string_space, str_ptrs);

    for(ii=0; ii+2 < nt; ii+=3){
	this_name = (struct rename_str *)malloc(sizeof(struct rename_str));  //Jul 26, 2011 this issue

	/* get the first name */
	strcpy(this_name->old_name, str_ptrs[ii]);

	/* get the second name */
	strcpy(this_name->new_name, str_ptrs[ii+2]);

	if(!uui->top_ren) {
	    uui->top_ren = this_name;
	}
	else {
	    last->next = this_name;
	    this_name->last = last;
	}
	uui->top_ren->last = this_name;
	this_name->next = uui->top_ren;
	last = this_name;
    }
}
/* c------------------------------------------------------------------------ */

void 
uf_reset (void)
{
    int rdn;

    for(rdn=0; rdn < MAX_RADARS; rdn++) {
	ufer[rdn]->vol_count = 
	      ufer[rdn]->sweep_count = ufer[rdn]->ray_count = 0;
	ufer[rdn]->sweep_reference_time = 0;

	if(rdn != uui->current_radar_ndx) {
	    ufer[rdn]->prev_fxd_angle = -1000.;
	    ufer[rdn]->srs_vol_num = ufer[rdn]->srs_sweep_num = -1;
	}
    }
    uui->trip_time = 0;
    uui->new_sweep = uui->new_vol = YES;
    rdn = uui->current_radar_ndx;
    gri->vol_num = ufer[rdn]->vol_num;
    gri->sweep_num = 0;
    difs->stop_flag = NO;
}
/* c------------------------------------------------------------------------ */

int 
uf_select_ray (void)
{
    int ii, jj, mark;
    int ok;

    ok = uui->ok_ray;

    if(gri->time >= difs->final_stop_time)
	  difs->stop_flag = YES;

    if(difs->num_time_lims) {
	for(ii=0; ii < difs->num_time_lims; ii++ ) {
	    if(gri->time >= difs->times[ii]->lower &&
	       gri->time <= difs->times[ii]->upper)
		  break;
	}
	if(ii == difs->num_time_lims)
	      ok = NO;
    }

    if(!difs->radar_selected[gri->dd_radar_num]) 
	  ok = NO;

    if(difs->sweep_skip) {
	if(!((gri->sweep_num-1) % difs->sweep_skip)) {
	    mark = 0;
	}
	else
	      ok = NO;
    }

    if(difs->num_prf_lims) {
	for(ii=0; ii < difs->num_prf_lims; ii++ ) {
	    if(gri->PRF >= difs->prfs[ii]->lower &&
	       gri->PRF <= difs->prfs[ii]->upper)
		  break;
	}
	if(ii == difs->num_prf_lims)
	      ok = NO;
    }

    if(difs->num_fixed_lims) {
	for(ii=0; ii < difs->num_fixed_lims; ii++ ) {
	    if(gri->fixed >= difs->fxd_angs[ii]->lower &&
	       gri->fixed <= difs->fxd_angs[ii]->upper)
		  break;
	}
	if(ii == difs->num_fixed_lims)
	      ok = NO;
    }

    if(difs->num_az_sectors) {
	for(ii=0; ii < difs->num_az_sectors; ii++) {
	    if(in_sector(gri->azimuth
			 , (float)difs->azs[ii]->lower
			 , (float)difs->azs[ii]->upper)) {
		break;
	    }
	}
	if(ii == difs->num_az_sectors)
	      ok = NO;
    }

    if(difs->num_el_sectors) {
	for(ii=0; ii < difs->num_el_sectors; ii++) {
	    if(in_sector(gri->elevation
			 , (float)difs->els[ii]->lower
			 , (float)difs->els[ii]->upper)) {
		break;
	    }
	}
	if(ii == difs->num_el_sectors)
	      ok = NO;
    }

    if(difs->num_az_xouts) {
	for(ii=0; ii < difs->num_az_xouts; ii++ ) {
	    if(in_sector(gri->azimuth, (float)difs->xout_azs[ii]->lower
			 , (float)difs->xout_azs[ii]->upper)) {
		if(difs->num_el_xouts) {
		    for(jj=0; jj < difs->num_el_xouts; jj++) {
			if(in_sector(gri->elevation
				     , (float)difs->xout_els[jj]->lower
				     , (float)difs->xout_els[jj]->upper)) {
			    ok = NO;
			}
		    }
		}
		else {
		    ok = NO;
		}
	    }
	}
    }

    if(difs->num_modes) {
	if(difs->modes[gri->scan_mode]) {
	    mark = 0;
	}
	else
	      ok = NO;
    }

    return(ok);
}
/* c------------------------------------------------------------------------ */

double 
uf_time_stamp (DD_TIME *dts)
{
    double d;
    int yy;

    dts->day_seconds = D_SECS(man->hour, man->minute, man->second, 0);

    if((man->loc_hed_ptr-man->opt_hed_ptr)*sizeof(short) >
       sizeof(OLD_UF_OPTIONAL) &&
       opt->seconds_xe3 != man->missing_data_flag)
	  {
	      dts->day_seconds += (float)opt->seconds_xe3 * 1.e-3;
	  }
    yy = man->year < 70 ? man->year +100 : man->year;
    dts->year = yy < 1900 ? yy +1900 : yy;
    dts->month = man->month;
    dts->day = man->day;
    d = d_time_stamp(dts);
    return(d);
 }
/* c----------------------------------------------------------------------- */

void 
uf_vol_update (void)
{
    int i, j, k, n, pick_a_field, fhlen;
    char str[256], *a;
    float unscale, r, gs;
    struct rename_str *rns;

# ifdef obsolete
    str_terminate(gri->radar_name, man->radar, 8);
    if (strlen (gri->radar_name) < 1)
      { strcpy (gri->radar_name, tmp_radar_name); }

    if(rns=uui->top_ren) {
	for(;;) {
	    if(strstr(gri->radar_name, rns->old_name)) {
		strcpy(gri->radar_name, rns->new_name);
		break;
	    }
	    if((rns=rns->next) == uui->top_ren)
		  break;
	}
    }    
# endif
    strncpy(gri->project_name, opt->project_id, 8);
    gri->project_name[8] = '\0';
    
    strncpy(gri->site_name, man->site, 8);
    gri->site_name[8] = '\0';
    
    /* update constants look for a power field first
     */
    pick_a_field = EMPTY_FLAG;
    a = str;
    str[2] = '\0';
    for(i=0; i < dhed->num_flds_this_ray; i++ ) {
	strncpy(str,fidp[i]->id,2);

	if(strstr(power_fields,str)) {
	    break;
	}
    }
    if( i < dhed->num_flds_this_ray ) {
	/* power field
	 */
	pick_a_field = i;
	unscale = fhed[i]->scale > 0 ? 1./fhed[i]->scale : 1.;
	gri->radar_constant = fhed[i]->word_20*unscale;
	gri->noise_power = fhed[i]->word_21*unscale;
	gri->rcvr_gain = fhed[i]->word_22*unscale;
	gri->peak_power = fhed[i]->word_23*unscale;
	gri->ant_gain = fhed[i]->word_24*unscale;
    }
    
    for(i=0; i < dhed->num_flds_this_ray; i++ ) {
	/* now look for a velocity field
	 */
	strncpy(str,fidp[i]->id,2);

	if(strstr(velocity_fields,str)) {
	    break;
	}
    }
    if( i < dhed->num_flds_this_ray ) {
	/* velocity field
	 */
	if(pick_a_field == EMPTY_FLAG)
	      pick_a_field = i;

	unscale = fhed[i]->scale > 0 ? 1./fhed[i]->scale : 1.;
	gri->nyquist_vel = fhed[i]->word_20*unscale;
    }
    if(pick_a_field == EMPTY_FLAG ) {
	/* Complain!
	 */
	printf("Could not find a power or velocity field\n" );
	pick_a_field = 0;
    }
    k = pick_a_field;
    fhlen = fhed[k]->fld_data_ptr - fidp[k]->fld_hed_ptr;
    gri->polarization = fhed[k]->polarization;
    gri->PRF = 1./(fhed[k]->prT_xe6*1.e-6) +.000001;
    gri->wavelength  = US100(US64(fhed[k]->waveln_x6400));
    if(gri->wavelength > 0 && gri->PRF > 0)
	  gri->nyquist_vel = gri->wavelength*gri->PRF*.25;
    gri->num_freq = 1;
    gri->freq[0] = SPEED_OF_LIGHT/gri->wavelength*1.e-9; /* GHz */
    gri->num_ipps = 1;
    gri->ipps[0] = fhed[k]->prT_xe6*1.e-6;
    gri->h_beamwidth = US64(fhed[k]->h_beam_width_x64);
    gri->v_beamwidth = US64(fhed[k]->v_beam_width_x64);
    gri->num_samples = fhed[k]->num_samples;
    gri->rcv_bandwidth = fhed[k]->rec_bw_xe6*1.e-6;
# ifdef obsolete
    gri->pulse_width = fhed[k]->sample_vol_xe3;
# else
    gri->pulse_width = 2.* (float)fhed[k]->sample_vol_xe3/SPEED_OF_LIGHT;
# endif
    gri->scan_mode = man->sweep_mode;
    gri->radar_type = DD_RADAR_TYPE_GROUND;
    gri->sweep_speed = US64(man->sweep_rate_x64);
    gri->missing_data_flag = man->missing_data_flag;
    
    /* range gate info
     */
    r = fhed[k]->range_g1*1000. +fhed[k]->g1_adjust_xe3;
    gri->range_b1 = r;
    gs = fhed[k]->gate_spacing_xe3;
    if( fhlen >= 30 && fhed[k]->gt_sp_adjust_xe6 > 0 ) {
	gs += fhed[k]->gt_sp_adjust_xe6*.001; /* get back to meters */
    }
    gri->bin_spacing = gs;
    gri->num_bins = n = uui->max_num_bins > 0 ?
	  uui->max_num_bins : fhed[k]->num_gates;
    
    for(j=0; j < n; j++ ) {
	gri->range_value[j] = r;
	r += gs;
    }
    gri->num_fields_present = 0;

    for(i=0; i < dhed->num_flds_this_ray; i++ ) {
	gri->num_fields_present++;
	gri->dd_scale[i] = fhed[i]->scale;
	gri->dd_offset[i] = 0;
# ifdef obsolete
	gri->scaled_data[i] = fdata[i];
# endif
	gri->actual_num_bins[i] = fhed[i]->num_gates;
	strncpy(gri->field_name[i],fidp[i]->id,2);
	gri->field_name[i][2] = '\0';
    }
}
/* c----------------------------------------------------------------------- */
