/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

/*
 * This file contains the following routines
 * 
 * produce_uf
 * 
 * ufp_init
 * ufp_stuff_ray
 * ufp_uf_stuff
 * ufp_writ
 * 
 */

#include <LittleEndian.hh>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <dd_math.h>
#include "dorade_uf.hh"
#include "ddb_common.hh"
#include "dda_common.hh"
#include "dd_crackers.hh"
//#include <iostream>
//#include <fstream>
#include "dorade_share.hh"

# define PMODE 0666
# define NO_UF_CLIPPING 0x1
# define    UF_VOL_TRAP 0x2

static struct ufp_control_struct *ucs;
static int UF_null_dev=0;
static int UF_num_radars=0;
static int UF_output=YES;
static int UF_EOFs=YES;
static int num_output_fields=0;
static char output_fields_list[MAX_PARMS][16];
static float MB_trip_inc=2000.;	/* 2GB or 2000 MB */
static int num_parms=0;
static double max_media_size = 4200 * 1.e6; /* 4.2 GB */
static struct field_name_aliases *field_aliases = NULL;


/* c------------------------------------------------------------------------ */

void produce_uf (struct dd_general_info *dgi)
{
    struct field_name_aliases *fna, *prev_fna;
    struct uf_production *ufp;
    char *a;
    char str[256], *str_ptrs[36], *aa, *bb, *cc;
    double d;
    int ii, nt, mark;
    static int count = 0, n_catch=123;  //Jul 26, 2011 keyword issue, change catch to n_catch
    struct uf_out_dev *uod, *prev;

    if(!UF_output)
	  return;

    count++;
    if(count >= n_catch) {
       mark = 0;
    }

    if(!UF_num_radars) {
	/*
	 * first time through
	 */
	UF_output = NO;
	if(dd_catalog_only_flag()) {
	    return;
	}
	if(a=get_tagged_string("OUTPUT_FLAGS")) {
	    if(strstr(a,"UFDATA") || strstr(a,"UF_DATA")) {
		UF_output = YES;
	    }
	    if(strstr(a, "NO_UFEOFS")) {
		UF_EOFs = NO;
	    }
	}
	if(!UF_output)
	      return;

	ucs = (struct ufp_control_struct *)
	      malloc(sizeof(struct ufp_control_struct));
	memset((char *)ucs, 0, sizeof(struct ufp_control_struct));
	ucs->dev_list = uod = (struct uf_out_dev *)malloc(sizeof(struct uf_out_dev));
	memset( uod, 0, sizeof(struct uf_out_dev));
	uod->io_type = FB_IO;

	if((a=get_tagged_string("UF_DEV"))) {
	    strcpy( ucs->out_devs, a );
	    nt = dd_tokens( ucs->out_devs, str_ptrs );
	    /*
	     * looking for ordered triples
	     */
	    for(ii = 0; ii < nt-2; ii += 3 ) {
		if( !ii ) {	/* first one */
		   uod = ucs->dev_list;
		}
		else {
		    uod = (struct uf_out_dev *)malloc(sizeof(struct uf_out_dev));
		    memset( uod, 0, sizeof(struct uf_out_dev));
		    uod->io_type = FB_IO;
		    prev->next = uod;
		}
		prev = uod;

		uod->radar_name = str_ptrs[ii];
		uod->dev_name = str_ptrs[ii+2];

		if( dd_itsa_physical_dev( uod->dev_name )) {
		    uod->io_type = PHYSICAL_TAPE;
		}
	    }
	}
	else if( a=get_tagged_string("UF_DIRECTORY") ) {
	    uod->dev_name = a;
	}
	else {
	  uod->dev_name = strdup(dgi->directory_name.c_str());
	}

	if((a=get_tagged_string("UF_OUTPUT_FIELDS"))) { 
	    strcpy( str, a );
	    nt = dd_tokens( str, str_ptrs );
	    for(ii = 0; ii < nt; ii++ ) {
	       strcpy( output_fields_list[ii], str_ptrs[ii] );
	       num_output_fields++;
	    }
	}

	if((a=get_tagged_string("OPTIONS"))) { 
	    if( strstr( a, "NO_UF_CLIP" )) { /* NO_UF_CLIPPING */
		ucs->options |= NO_UF_CLIPPING;
	    }
	    if( strstr( a, "UF_VOL_TRAP" )) { /* NO_UF_CLIPPING */
		ucs->options |= UF_VOL_TRAP;
	    }
	}

	if((a=get_tagged_string("UF_ALIASES"))) { 
	    strcpy( str, a );
	    nt = dd_tokens( str, str_ptrs );
	    for(ii = 0; ii < nt-2; ii += 3 ) {
		fna = (struct field_name_aliases *)
		    malloc( sizeof(struct field_name_aliases));
		memset( fna, 0, sizeof(struct field_name_aliases));
		if( !ii )
		    { field_aliases = fna; }
		else
		    { prev_fna->next = fna; }
		strcpy( fna->fname, str_ptrs[ii] );
		strcpy( fna->alias, str_ptrs[ii+2] );
		prev_fna = fna;
	    }
	}
	if((a=get_tagged_string("MAX_MEDIA_SIZE"))) { /* in MB */
	    d = 0;
	    d = atof( a );
	    if( d > 0 )
		{ max_media_size = d * 1.e6; }
	}
	printf( "Max. media size: %.4fGB\n", max_media_size * 1.e-9 );
	/*
	 * end initialization
	 */
    }


# ifdef notyet
    if(dgi->ray_quality & UNACCEPTABLE)
	  return;
# endif

    if(ucs->encountered[dgi->radar_num]) {
	ufp = ucs->ufp[dgi->radar_num];
    }
    else {
	UF_num_radars++;
	ucs->encountered[dgi->radar_num] = YES;

	ufp = ucs->ufp[dgi->radar_num] = (struct uf_production *)
	      malloc(sizeof(struct uf_production));
	memset((char *)ufp, 0, sizeof(struct uf_production));
	ufp_init(dgi, ufp);
    }

    ufp_stuff_ray(dgi, ufp);
    ucs->last_radar_num = dgi->radar_num;
    ufp->new_vol = ufp->new_sweep = NO;
}
/* c------------------------------------------------------------------------ */

int 
ufp_check_media (struct uf_production *ufp)
{
    int ii;
    char str[256], command[256];
    struct uf_out_dev *uod = NULL;

    if( ufp->current_media_size > 0 &&
       ( ufp->current_media_size < max_media_size || !ufp->new_vol )) {
       return 1;
    }
    ufp->current_media_size = 0;

    /* otherwise see if we can open a new media dev
     */
    if( !ufp->dev_count++ ) {
       ufp->dev_list = ucs->dev_list;
    }

    if( !ufp->dev_list ) {
	return 0;		/* we've used up all the devices */
    }
    /* find the device corresponding to this radar
     * we may only have one directory name for all the data 
     */
    uod = ufp->dev_list;

    if( ufp->dev_list->radar_name ) {
       /* try and find this radar in the list
	*/
       uod = NULL;
       for(; ufp->dev_list; ufp->dev_list = ufp->dev_list->next ) {
	  if( strstr( ufp->radar_name, ufp->dev_list->radar_name )) {
	     uod = ufp->dev_list;
	     break;
	  }
       }
       if( !uod ) { /* not found! */
	  printf( "\nCould not find a match for %s in UF_DEVS\n"
		 , ufp->radar_name );
	  return 0;
       }
    }
    /* set up for the next device which may be NULL
     */
    ufp->dev_list = ufp->dev_list->next;

    if(( ufp->io_type = uod->io_type ) == FB_IO ) {
       /* construct the full path name
	*/
       slash_path( str, uod->dev_name ); /* dir name */
       dd_file_name( "ufd", (time_t)ufp->time, ufp->radar_name, 0
		    , str+strlen(str));
       strcat( str, ".tape" );
    }
    else {
	strcpy( str, uod->dev_name ); /* media dev name */
    }
    if( ufp->uf_fid > 0 ) {
	close( ufp->uf_fid );

	if( ufp->prev_dev->io_type !=  FB_IO ) {
	    /* issue an eject */
	    sprintf( command, "mt -f %s offline",  ufp->prev_dev->dev_name );
	    ii = system( command );
	}
    }
    ufp->prev_dev = uod;

    printf( "Opening UF output file %s", str );

    if((ufp->uf_fid = creat( str, PMODE )) < 0 ) {
	printf( "\nUnable to open %s %d\n", str, ufp->uf_fid );
	exit(1);
    }
    UF_null_dev = strstr( str, "/dev/null" ) ? YES : NO;

    printf(" fid: %d\n", ufp->uf_fid);

    return 1;
}
/* c------------------------------------------------------------------------ */

void 
ufp_init (struct dd_general_info *dgi, struct uf_production *ufp)
{
    /*
     * initialize uf structures
     */
    struct dds_structs *dds=dgi->dds;
    struct volume_d *vold=dds->vold;
    struct radar_d *radd=dds->radd;
    UF_MANDITORY *man;
    UF_OPTIONAL *opt;
    UF_DATA_HED *dhed;
    UF_FLD_ID_ARRAY *fida;
    int ii, m, n, nt;
    char *uf_ptr, str[512], *a, *ufn;
    short *w, tdate[6];

    ufp->last_ray_num = -1;
    ufp->last_sweep_num = -1;
    ufp->last_vol_num = -1;
    ufp->last_num_parms = -1;
    ufp->uf_range1 = -1.e11;
    ufp->uf_range2 = 1.e11;
    ufp->last_fxd_ang = -32768.;
    ufp->dd_radar_num = dgi->radar_num;
    ufp->io_type = FB_IO;
    str_terminate( ufp->radar_name, dgi->dds->radd->radar_name, 8 );

    ufp->uf_buf = (short *)malloc( BLOCK_SIZE );
    uf_ptr = (char *)ufp->uf_buf;
    memset(uf_ptr, 0, BLOCK_SIZE);
    w = ufp->uf_buf;

    for(n=BYTES_TO_SHORTS(BLOCK_SIZE); n-- > 0;) 
	 *w++ = EMPTY_FLAG; /* initialize all header values to EMPTY_FLAG */

    ufp->man = man = (UF_MANDITORY *)uf_ptr;
    man->opt_hed_ptr = 1 + BYTES_TO_SHORTS(sizeof(UF_MANDITORY));
    man->loc_hed_ptr = man->opt_hed_ptr
	  + BYTES_TO_SHORTS(sizeof(UF_OPTIONAL));
    uf_ptr += sizeof(UF_MANDITORY);

    ufp->opt = opt = (UF_OPTIONAL *)uf_ptr;
    uf_ptr += sizeof(UF_OPTIONAL);

    if( radd->radar_type == DD_RADAR_TYPE_SHIP ) {
	ufp->luship = (UF_LOC_USE_SHIP *)uf_ptr;
	uf_ptr += sizeof(UF_LOC_USE_SHIP);
	/*
	 * NCAR ship positioning info in the local use header
	 */
	strncpy( ufp->luship->id, "SHIP", 4 );
	man->data_hed_ptr = man->loc_hed_ptr
	      +BYTES_TO_SHORTS(sizeof(UF_LOC_USE_SHIP));
    }
    else if( radd->radar_type != DD_RADAR_TYPE_GROUND ) {
	ufp->luac = (UF_LOC_USE_AC *)uf_ptr;
	uf_ptr += sizeof(UF_LOC_USE_AC);
	/*
	 * NCAR aircraft positioning info in the local use header
	 */
	strcpy( ufp->luac->id, "AIR" );
	man->data_hed_ptr = man->loc_hed_ptr
	      +BYTES_TO_SHORTS(sizeof(UF_LOC_USE_AC));
    }
    else {			/* no local use header */
	man->data_hed_ptr = man->loc_hed_ptr;
    }

    ufp->dhed = dhed = (UF_DATA_HED *)uf_ptr;
    uf_ptr += sizeof(UF_DATA_HED);

    ufp->fida = fida = (UF_FLD_ID_ARRAY *)uf_ptr;
    uf_ptr += (MAX_UF_FLDS+1)*sizeof(UF_FLD_ID);

    ufp->fhed[0] = (UF_FLD_HED *)uf_ptr;
    uf_ptr += sizeof(UF_FLD_HED);

    ufp->fdata[0] = (short *)uf_ptr;

    strncpy( man->id, "UF", 2 );
    /*
     * UF style (FORTRAN index) pointer to first field header 
     */
    fida->field[0].fld_hed_ptr = man->data_hed_ptr
	  + BYTES_TO_SHORTS(sizeof(UF_DATA_HED))
		+ (MAX_UF_FLDS+1)*BYTES_TO_SHORTS(sizeof(UF_FLD_ID));
    /*
     * FORTRAN index of first data field 
     */
    ufp->fhed[0]->fld_data_ptr = fida->field[0].fld_hed_ptr
	  + BYTES_TO_SHORTS(sizeof(UF_FLD_HED));

    todays_date( tdate );
    man->gen_date_yy = tdate[0];
    man->gen_date_mm = tdate[1];
    man->gen_date_dd = tdate[2];
    man->vol_num = 0;
    man->swp_num = 0;
    man->prec_in_ray = 1;
    man->missing_data_flag = EMPTY_FLAG;
    dhed->num_recs_this_ray = 1;

    strncpy( man->radar, radd->radar_name, 8 );

    strncpy(man->site,"UNKNOWN ", 8);
    if(a=get_tagged_string("SITE_NAME")) {
	if((m=strlen(a)) > 8 ) m = 8;
	strncpy( man->site, a, m );
	for(; m < 8; man->site[m++]=' ');
    }
    else {
	/* see if site name is stored after project name
	 */
	for(n=0; n < 20 && vold->proj_name[n++];);

	if( n < 20 ) {
	    for(ii=n,m=0; ii < 20 && vold->proj_name[ii++]; m++);
	    if( m = m > 8 ? 8 : m ) {
		strncpy(man->site,vold->proj_name+n,m);
		for(; m < 8; man->site[m++]=' ');
	    }
	}
    }
    strncpy( man->facility_id, "NCAR/RDP   ", 8 );

    strncpy( man->time_zone, "GMT   ", 2 );

    strncpy( opt->project_id, vold->proj_name, 8 );

    strncpy( opt->fld_tape_id, "UNKNOWN ", 8 );

    opt->range_info_flag = 0;	/* field info same for volume */
}
/* c------------------------------------------------------------------------ */

void 
ufp_stuff_ray (struct dd_general_info *dgi, struct uf_production *ufp)
{
    struct dds_structs *dds=dgi->dds;
    struct radar_d *radd=dds->radd;
    struct cell_d *celv=dds->celvc;
    struct sweepinfo_d *swib=dds->swib;
    struct ray_i *ryib=dds->ryib;
    struct platform_i *asib=dds->asib;
    struct field_name_aliases *fna;

    UF_MANDITORY *man=ufp->man;
    UF_OPTIONAL *opt=ufp->opt;
    UF_DATA_HED *dhed=ufp->dhed;
    UF_LOC_USE_AC *luac=ufp->luac;
    UF_LOC_USE_SHIP *luship=ufp->luship;
    UF_FLD_ID_ARRAY *fida=ufp->fida;
    UF_FLD_HED *fhed;

    unsigned char *uc;
    int bad, cg, i, ii, jj, k, n, mm, nn, xcount=0, clip_gate, wlen, far_gate;
    int mark, mdf, ng, pn, fn, scale, rescale=NO, new_vol=NO;
    int serious_new_vol = NO;
    float f, gs, r, rcp_scale=0, offset;
    double d;
    char *b, *c, *buf, fname[16], *aa, *bb;
    short short_vals[MAXCVGATES];
    short *ss, *dd;
    static int count=0;
    DD_TIME *dts=dgi->dds->dts;

				/* c...mark */

    ufp->time = dgi->time;
    dts->time_stamp = dgi->time;
    d_unstamp_time(dts);

    if( ufp->last_scan_mode != radd->scan_mode ) {
      serious_new_vol = YES;
    }
    new_vol = dgi->new_vol;

    if (dgi->new_sweep && ucs->options & UF_VOL_TRAP) {
      if (ufp->last_fxd_ang >= swib->fixed_angle)
	{ new_vol = YES; }
    }

    if( dgi->new_sweep && ufp->vol_sweep_count > 0 ) {
      if( new_vol &&
	  ufp->vol_sweep_count < dd_min_sweeps_per_volume()) {
	new_vol = serious_new_vol ? YES : NO;
      }
      else if( ufp->vol_sweep_count >= dd_max_sweeps_per_volume()) {
	new_vol = YES;
      }
    }

    if( new_vol ) {
	ufp->new_vol = YES;
	if( ufp->uf_file_size > 0 && UF_EOFs == YES ) {
	    i = ufp_write(ufp, 0L); /* write an EOF! */
	    if( i < 0 ) {
		printf( "Error during UF EOF write\n" );
		exit(1);
	    }
	    man->swp_num = 0;
	} 
	printf("UF New volume fid: %d at %.2fMB  media: %.2fMB\n"
	       , ufp->uf_fid, ufp->MB, ufp->current_media_size * 1.e-6);
	ufp->uf_file_size = 0;
	ufp_uf_stuff(dgi, ufp);
	man->vol_num++;
	ufp->last_sweep_num = -1;
	ufp->last_vol_num = dgi->vol_num;
	man->rec_num = 0;
	man->ray_num = 0;
	if(!strncmp(opt->project_id, "NEXRAD_", 7)) {
	    /* so solo won't try special plots based on prf info */
	    strncpy(opt->project_id, "NEXRAD ", 7);
	}
	ufp->vol_sweep_count = 0;
    }
    if(dgi->new_sweep) {
       ufp->new_sweep = YES;
       man->swp_num++;
       ufp->vol_sweep_count++;
       ufp->last_sweep_num = dgi->sweep_num;
       ufp->last_scan_mode = radd->scan_mode;
       ufp->last_fxd_ang = swib->fixed_angle;
       man->fixed_angle_x64 = (short)S64(swib->fixed_angle);
       for(num_parms=pn=0; pn < MAX_PARMS; pn++) {
	  if(dgi->dds->field_present[pn]) {
	     if( num_output_fields ) {
	       str_terminate( fname, dgi->dds->parm[pn]->parameter_name, 8 );
	       for( ii = 0; ii < num_output_fields; ii++ ) {
		 if( !strcmp( fname, output_fields_list[ii] ))
		   { num_parms++; break; }
	       }
	     }
	     else
	       { num_parms++; }
	  }
       }
    }

//Jul 26, 2011 start
/*
    man->rec_num++; man->ray_num++; man->lat_deg = f =
    dd_latitude(dgi); man->lat_min = f = (f -(float)man->lat_deg)*60.;
    man->lat_sec_x64 = S64((f-(float)man->lat_min)*60.); man->lon_deg
    = f = dd_longitude(dgi); man->lon_min = f = (f
    -(float)man->lon_deg)*60.; man->lon_sec_x64 =
    S64((f-(float)man->lon_min)*60.);
*/
    man->rec_num++; man->ray_num++; 
    f = dd_latitude(dgi);
    man->lat_deg = (short) f; 
    f = (f -(float)man->lat_deg)*60.;
    man->lat_min = (short)f;
    man->lat_sec_x64 = (short)S64((f-(float)man->lat_min)*60.); 
    f = dd_longitude(dgi);
    man->lon_deg = (short)f; 
    f = (f-(float)man->lon_deg)*60.;
    man->lon_min = (short)f;
    man->lon_sec_x64 = (short)S64((f-(float)man->lon_min)*60.);
//Jul 26, 2011 end

    man->altitude_msl = (short)(dd_altitude(dgi) * 1000.);
    man->azimuth_x64 = (short)S64(dd_azimuth_angle(dgi));
    man->elevation_x64 = (short)S64(dd_elevation_angle(dgi));

    man->fixed_angle_x64 = (short)S64(swib->fixed_angle);
    man->sweep_rate_x64 = (short)S64(ryib->true_scan_rate);

    man->year = dts->year-1900;
    man->month = dts->month;
    man->day = dts->day;
    man->hour = dts->hour;
    man->minute = dts->minute;
    man->second = dts->second;
    opt->seconds_xe3 = dts->millisecond;

    if( radd->radar_type == DD_RADAR_TYPE_SHIP ) {
    }
    else if( radd->radar_type != DD_RADAR_TYPE_GROUND ) {
	luac->ac_pitch_x64 = (short)S64(dd_pitch(dgi));
	luac->ac_roll_x64 = (short)S64(dd_roll(dgi));
	luac->ac_phdg_x64 = (short)S64(dd_heading(dgi));
	luac->ac_drift_x64 = (short)S64(dd_drift(dgi));
	d = FMOD360( asib->rotation_angle + dgi->dds->cfac->rot_angle_corr);
	luac->ac_rotation_x64 = (short)S64(d);
	d = FMOD360( asib->tilt + dgi->dds->cfac->tilt_corr);
	luac->ac_tilt_x64 = (short)S64(d);
	luac->ac_phdg_change_x64 = (short)S64(asib->heading_change);
	luac->ac_pitch_change_x64 = (short)S64(asib->pitch_change);
	luac->ac_ant_rot_angle_x64 = (short)S64(dd_rotation_angle(dgi));
	luac->ac_tilt_angle_x64 = (short)S64(dd_tilt_angle(dgi));

	luac->ac_vns_x10 = (short)S10(asib->ns_velocity);
	luac->ac_vew_x10 = (short)S10(asib->ew_velocity);
	luac->ac_wp3_x10 = (short)S10(asib->vert_velocity);
	luac->ac_ui_x10 = (short)S10(asib->ns_horiz_wind);
	luac->ac_vi_x10 = (short)S10(asib->ew_horiz_wind);
	luac->ac_wi_x10 = (short)S10(asib->vert_wind);
    }

    dhed->num_flds_this_ray = num_parms;
    dhed->num_flds_this_rec = num_parms;
    /* 
     * update field related information
     * 
     * for now assume the bias is zero and just use the scaled data as is
     * 
     */
    k = fida->field[0].fld_hed_ptr; /* k counts the number of shorts */
    c = (char*)ufp->fhed[0];
    mdf = man->missing_data_flag;
    far_gate =
	ucs->options & NO_UF_CLIPPING ? celv->number_cells -1 : dgi->clip_gate;

    r = celv->dist_cells[far_gate];

//    clip_gate = f = (r - ufp->uf_range_g1)/ufp->uf_gate_spacing +.022;
    f = (r - ufp->uf_range_g1)/ufp->uf_gate_spacing +.022;
    clip_gate = (int)f;

    for(fn=-1,pn=0; pn < MAX_PARMS; pn++, xcount=0 ) {
	count++;
	if(!dgi->dds->field_present[pn])
	      { continue; }
	str_terminate( fname, dds->parm[pn]->parameter_name, 8);
	/* see if output field list and if this is a selected field
	 */
	if( num_output_fields  ) {
	   for( ii = 0; ii < num_output_fields; ii++ ) {
	      if( !strcmp( fname, output_fields_list[ii] )) {
		 break;
	      }
	   }
	   if( ii == num_output_fields )
	     { continue; }
	}
	fn++;
	fhed = ufp->fhed[fn];
	if( fn > 0 ) {
	    /* use the first field as the template for others
	     */
	    memcpy(c, (char *)ufp->fhed[0], sizeof(UF_FLD_HED));
	    fhed = (UF_FLD_HED *)c;
	    fida->field[fn].fld_hed_ptr = k;
	}
	aa = fname;

	for( fna = field_aliases; fna ; fna = fna->next ) {
	    if( !strcmp( fname, fna->fname ))
		{ aa = fna->alias; }
	}

	if(strlen(aa) < 2) {
	    /* some people use 1 char field names...just repeat it */
	    b = fida->field[fn].id;
	    *b = *aa;
	    *(b+1) = *b;
	}
	else {
	    strncpy(fida->field[fn].id, aa, 2);
	}

	k += BYTES_TO_SHORTS(sizeof(UF_FLD_HED));
	c += sizeof(UF_FLD_HED);
	fhed->fld_data_ptr = k;

	offset = dds->parm[pn]->parameter_bias;
	f = dds->parm[pn]->parameter_scale;
	scale = (int)(f +.000001);
	if(fabs((double)(f-(float)scale)) > .01 ||
	   fabs((double)offset) > .01) {
	    /* need to rescale the data */
	    scale =  (int)(f +.5);
	    rescale = YES;
	}
	rcp_scale = 1./f;
	fhed->scale = scale;

	if( dgi->parm_type[pn] == VELOCITY ) {
	    fhed->word_20 = (short)UF_SCALE(radd->eff_unamb_vel, scale);
	    strncpy((char *)&fhed->word_21, "  ", 2 );
	}
	else {
	    fhed->word_20 = man->missing_data_flag;
	    fhed->word_21 = (short)UF_SCALE(radd->noise_power, scale);
	    fhed->word_22 = (short)UF_SCALE(radd->receiver_gain, scale);
	    fhed->word_23 = (short)UF_SCALE(radd->peak_power, scale);
	    fhed->word_24 = (short)UF_SCALE(radd->antenna_gain, scale);
	    fhed->word_25 =
		  (short)S64(1.e6*dds->parm[pn]->pulse_width/SPEED_OF_LIGHT);
	}
	buf = dgi->dds->qdat_ptrs[pn];
	bad = dds->parm[pn]->bad_data;
	ng = far_gate+1;
				/* c...mark */
# ifdef obsolete
# endif
	if(!dds->field_present[pn]) {
	    /* fill with bad flags */
	    for(ss=short_vals,jj=0; jj < ng; jj++ ) {
		*ss++ = mdf;
	    }
	    ss = short_vals;
	}
	else if( dds->parm[pn]->binary_format == DD_16_BITS ) { 
	    ss = (short *)buf;
	    if(rescale) {
		for(i=0,dd=short_vals; i < ng; i++,ss++,dd++) {
		    if(*ss == bad) 
			  *dd = mdf;
		    else 
			  *dd = (short)UF_SCALE
				(DD_UNSCALE
				 ((float)(*ss), rcp_scale, offset), scale);
		}
		ss = short_vals;
	    }
	    else {
	        dd = short_vals;
		for(i=0; i < ng; i++,dd++,ss++) {
		  *dd = (*ss == bad) ? mdf : *ss;
		}		
		ss = short_vals;
	    }
	}
	else if( dds->parm[pn]->binary_format == DD_8_BITS ) { 
	    uc = (unsigned char *)buf;
	    /* assume byte data needs to be rescaled */
	    for(ss=dd=short_vals; ng--; uc++) {
		if(*uc == bad) {
		    *dd++ = mdf;
		}
		else {
		    *dd++ = (short)UF_SCALE
			  (DD_UNSCALE
			   ((float)(*uc), rcp_scale, offset), scale);
		}
	    }
	}
	else if( dds->parm[pn]->binary_format == DD_24_BITS ) { 
	}
	else if( dds->parm[pn]->binary_format == DD_32_BIT_FP ) { 
	}


	ng = fhed->num_gates = clip_gate +1;
	/*
	 * use the gate look up table to generate the UF field
	 */
	if(dds->field_present[pn]) {
	    for(jj=0,dd=(short *)c; jj < ng; jj++ ) {
		*dd++ = (ufp->dd_gate_lut[jj] < 0) ? mdf
		  : *(ss+ufp->dd_gate_lut[jj]);
	    }
	}
	c += SHORTS_TO_BYTES(ng);
	k += ng;
    }
    /* write the record
     */
    man->rec_len = k-1;
    wlen = ufp_write( ufp, (int)SHORTS_TO_BYTES(man->rec_len));
    mark = man->rec_len * sizeof(short);

    if( wlen < mark ) {
	printf("Error during UF write--result: %d  tried: %d\n"
	       , wlen, man->rec_len * sizeof(short));
	exit(1);
    }
}
 /* c----------------------------------------------------------------------- */

void 
ufp_uf_stuff (struct dd_general_info *dgi, struct uf_production *ufp)
{
    /*
     * Application specific UF stuff
     */
    struct dds_structs *dds=dgi->dds;
    struct radar_d *radd=dds->radd;
    struct cell_d *celv=dds->celvc;
    UF_MANDITORY *man=ufp->man;
    UF_OPTIONAL *opt=ufp->opt;
    UF_DATA_HED *dhed=ufp->dhed;
    UF_FLD_HED *fhed;
    int g, i, ii, jj, pn, mm, nn;
    float gs, r, x, y;
    int g2;
    DD_TIME *dts=dgi->dds->dts;
    char *aa, fname[16];


    opt->vol_hour = dts->hour;
    opt->vol_minute = dts->minute;
    opt->vol_second = dts->second;
    man->sweep_mode = radd->scan_mode;
    
    
    for(num_parms=pn=0; pn < MAX_PARMS; pn++) {
       if(dgi->dds->field_present[pn]) {
	  if( num_output_fields ) {
	     str_terminate( fname, dgi->dds->parm[pn]->parameter_name, 8 );
	     for( ii = 0; ii < num_output_fields; ii++ ) {
		if( !strcmp( fname, output_fields_list[ii] ))
		   { num_parms++; break; }
	     }
	  }
	  else
	    { num_parms++; }
       }
    }
    dhed->num_flds_this_ray = num_parms;
    dhed->num_flds_this_rec = num_parms;
    
    fhed = ufp->fhed[0];
    fhed->sample_vol_xe3 = (dds->parm[0]->pulse_width);
    fhed->h_beam_width_x64 = (unsigned short)S64(radd->horz_beam_width);
    fhed->v_beam_width_x64 = (unsigned short)S64(radd->vert_beam_width);
    fhed->rec_bw_xe6 = (unsigned short) dds->parm[0]->recvr_bandwidth;
    fhed->polarization = dds->parm[0]->polarization;
    fhed->waveln_x6400 = (unsigned short)S64(100.*SPEED_OF_LIGHT/(radd->freq1*1.e9));
    fhed->num_samples = dds->parm[0]->num_samples;
    fhed->prT_xe6 = (unsigned short)(radd->interpulse_per1*1.e6);
    fhed->bits_per_datum = 16;
    
    /* Now analyse gate spacing info to determine
     * the gate spacing and the number of gates used
     *
     * also perhaps to be input is the nominal gate spacing and
     * the ranges of the desired data
     */

    dd_range_gate(dgi, ufp->uf_range1, &ufp->dd_gate1, &r);
    ufp->uf_range_g1 = r;
    i = (int)(r +.5);
    fhed->range_g1 = i/1000;
    fhed->g1_adjust_xe3 = i % 1000;
    /* returns a fortran type gate number
     */
    dd_range_gate(dgi, ufp->uf_range2, &g2, &r);

    ufp->uf_gate_spacing = 
	gs = celv->dist_cells[ufp->dd_gate1]
	-celv->dist_cells[ufp->dd_gate1-1];
    fhed->gate_spacing_xe3 = (short) ufp->uf_gate_spacing;
    fhed->gt_sp_adjust_xe6 =
	  (short)S1000(ufp->uf_gate_spacing-fhed->gate_spacing_xe3);

    /* construct a gate lookup array for the idealized gate spacing
     */
    g = ufp->dd_gate1 -1;
    r = celv->dist_cells[g];
    ufp->uf_num_gates = (int) ((celv->dist_cells[g2-1] -r)/gs  +1.1);
    /*
     * create a gate lookup table if the gate spacings
     * are not to be the same in the celv
     */
    for(i=0; i < ufp->uf_num_gates; i++, r+=ufp->uf_gate_spacing ) {
	x = fabs((double)(r-celv->dist_cells[g]));
	y = .5*fabs((double)(celv->dist_cells[g+1] -celv->dist_cells[g]));

	if( x > y )
	  { g++; }
	ufp->dd_gate_lut[i] = ( g < celv->number_cells ) ? g : -1;
    }
}
/* c------------------------------------------------------------------------ */

int 
ufp_write (struct uf_production *ufp, int size)
{
    int ii, jj, nn;
    static short *le_buf=NULL;
    short *ss, *tt;
    short *buf = ufp->uf_buf;
    UF_MANDITORY *man;
    UF_OPTIONAL *opt;
    UF_LOC_USE *loc;
    UF_DATA_HED *dhed;
    UF_FLD_ID *fid, *fidx;
    UF_FLD_ID_ARRAY *fida;
    UF_FLD_HED *fh;
    
    if(hostIsLittleEndian() && size > 0) {
       if(!le_buf) {
	  le_buf = (short *)malloc(65536);
	  memset(le_buf, 0, 65536);
       }
       /* remember uf_buf is a short pointer not a character pointer
	*/
       man = (UF_MANDITORY *)ufp->uf_buf;
       opt = (UF_OPTIONAL *)(ufp->uf_buf + FsCs(man->opt_hed_ptr));
       loc = (UF_LOC_USE *)(ufp->uf_buf + FsCs(man->loc_hed_ptr));
       dhed = (UF_DATA_HED *)(ufp->uf_buf + FsCs(man->data_hed_ptr));
       fid = fidx = (UF_FLD_ID *)((char *)dhed + sizeof(UF_DATA_HED));

       ss = le_buf;
//       uf_crack_man(man, ss, (int)0);  //Jul 26, 2011
       uf_crack_man((char *)man, (char *)ss, (int)0);  //Jul 26, 2011
       ss = le_buf + FsCs(man->opt_hed_ptr);
//       uf_crack_opt(opt, ss, (int)0);  //Jul 26, 2011
       uf_crack_opt((char *)opt, (char *)ss, (int)0);  //Jul 26, 2011
       ss = le_buf + FsCs(ufp->man->loc_hed_ptr);
       nn = man->data_hed_ptr - man->loc_hed_ptr;

       if(strncmp((char *)loc, "AIR", 3) == 0) {
//	  uf_crack_loc_use_ncar(loc, ss, (int)0);  //Jul 26, 2011
	  uf_crack_loc_use_ncar((char *)loc, (char *)ss, (int)0);  //Jul 26, 2011
       }
       else {
//	  swack_short(loc, ss, nn);  //Jul 26, 2011
	  swack_short((char *)loc, (char *)ss, nn);  //Jul 26, 2011
       }
       ss  = le_buf + FsCs(man->data_hed_ptr);
//       swack_short(dhed, ss, 3);  //Jul 26, 2011
       swack_short((char *)dhed, (char *)ss, 3);  //Jul 26, 2011
       ss += 3;
       nn = dhed->num_flds_this_ray;
       ii = FsCs(man->data_hed_ptr) + 3;
       jj = FsCs(fid->fld_hed_ptr);
       /* zero out extra space in field id array if there is any
	*/
       memset(ss, 0, (jj - ii)*sizeof(short));

       for(; nn--; fidx++) {	/* for each field */
	  memcpy(ss, fidx->id, 2); ss++;
//	  swack2(&fidx->fld_hed_ptr, ss); ss++;  //Jul 26, 2011
	  swack2((char *)&fidx->fld_hed_ptr, (char *)ss); ss++;  //Jul 26, 2011

	  ii = FsCs(fidx->fld_hed_ptr);
	  fh = (UF_FLD_HED *)(ufp->uf_buf + ii);
//	  uf_crack_fhed(fh, le_buf + ii, (int)0);  //Jul 26, 2011
	  uf_crack_fhed((char *)fh, (char *)(le_buf + ii), (int)0);  //Jul 26, 2011

	  ii = FsCs(fh->fld_data_ptr);
//	  swack_short(ufp->uf_buf + ii, le_buf + ii, (int)fh->num_gates);  //Jul 26, 2011
	  swack_short((char *)(ufp->uf_buf + ii), (char *)(le_buf + ii), (int)fh->num_gates);  //Jul 26, 2011
       }
       buf = le_buf;
    }

    if( size > 0 ) {
	if( !ufp_check_media( ufp )) {
	    printf( "\nNo more output media names\n" );
	    return -1;
	}
    }
    ufp->uf_file_size += size;
    ufp->MB += BYTES_TO_MB(size);

    if(ufp->MB > ufp->MB_trip && ufp->io_type == PHYSICAL_TAPE) {
	/* klooge for writing past 2 gig
	 */
	printf("UFp reseting output!...fid: %d at %.2f MB\n"
	       , ufp->uf_fid, ufp->MB);
	nn = lseek(ufp->uf_fid, 0L, SEEK_SET);
	ufp->MB_trip += MB_trip_inc;
    }
    ufp->current_media_size += size;
    ii = size;

    if( UF_null_dev )
      { ii = size; }
    else
      { ii = gp_write( ufp->uf_fid, (char *)buf, size, ufp->io_type ); }
    return(ii);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */



