/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define NEW_ALLOC_SCHEME
# define NEW_STUFF

/*
 * This file contains routines
 *
 * dap_givfld_
 * dap_mount_
 * dap_ndx_name
 * dap_next_ray_
 * dap_parm_resent_
 * dap_range_gate_
 * dap_rng_info_
 * dap_setdpr_
 *
 * kdate_
 * kday_
 * khour_
 * kfld1_
 * kfldtn_
 * klrect_
 * kminit_
 * kminut_
 * kmonth_
 * kprnv_
 * kproj_
 * kradar_
 * ksecnd_
 * ksite_
 * kswepm_
 * kswepn_
 * ktapen_
 * ktime_
 * ktimez_
 * ktzone_
 * kvoln_
 * kyear_
 * uazim_
 * udmprf_
 * udmrc_
 * uelev_
 * ufixed_
 * uhbwid
 * uhight_
 * ulatit_
 * ulongt_
 * uvenyq_
 *
 *
 * time2unix_
 *
 */

# include <time.h>
# include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <LittleEndian.hh>
#include "dd_stats.h"
#include <input_sweepfiles.h>
#include "swp_file_acc.hh"
#include "dap_common.hh"
#include "ddin.hh"
#include <Pdata.h>
#include "dda_common.hh"
#include "ddb_common.hh"
#include "dorade_share.hh"
#include "dd_crackers.hh"

# define NOTIFY_TIME_DIFF 60
# define END_OF_TIME_SPAN -9
# define ACCESS_FAILURE 999

struct dorade_unique_info {
    struct dd_general_info *dgi;
    time_t current_time;
    int io_type;
    int dorade_fid;
    int radar_num;
    int fore_radar_num;
    int aft_radar_num;
    int vol_count[MAX_SENSORS];
    int sweep_count[MAX_SENSORS];
    int beam_count[MAX_SENSORS];
    int use_this_sweep[MAX_SENSORS];
    char *ddin_buf;
    char site_name[12];
};

static struct dorade_unique_info *dui=NULL;

static struct dd_input_sweepfiles_v3 *dis;
static struct unique_sweepfile_info_v3 *usi;
static struct dd_general_info *dgi=NULL, *save_dgi=NULL;
static int Diagnostic_Print=NO;
static int sweep_files=NO, dd_radar_num=-1;
static char local_radar_name[32];
static struct volume_d *vold=NULL;
static struct dd_stats *dd_stats=NULL;
static struct dd_input_filters *difs;
static int32_t dd_start_time=0, dd_stop_time=ETERNITY;


# ifdef sparc
int Sparc_Arch=YES;
# else
int Sparc_Arch=NO;
# endif

/* c------------------------------------------------------------------------ */

int 
process_cappi_beam (void) { int i=0; } /* KLOOGE! */ 

/* c------------------------------------------------------------------------ */

void 
solo_message (	/* some of the access routines call
				 * this routine
				 */
    const char *message
)
{
    printf("%s", message);
}
# ifdef obsolete
/* c------------------------------------------------------------------------ */

struct dd_general_info *
dap_return_dgi (void)
{
    return(dgi);
}
# endif
/* c------------------------------------------------------------------------ */

void 
dap_givfld_ (int *ndx, int *g1, int *n, float *dd, float *badval)
{
    dd_givfld( dgi, *ndx, *g1, *n, dd, badval );
}

/* c------------------------------------------------------------------------ */

int 
dap_mnt_f_ (char *input_file_name, char *radar_name, int32_t *start_time, int32_t *stop_time, int *ivol, int *iscan, int *irec)
{
   /* this version now uses access routines in ddin.c
    *
    * Try to open the file and get to the first useful record
    */
    static int first=YES;
    int i, n, ierr=0;
    char *a=input_file_name;

    if(first) {
	first = NO;
	put_tagged_string((char *)"SOURCE_DEV", input_file_name);
	if(ddin_ini(0) < 0) {	/* from ddin.c */
	   ierr = ACCESS_FAILURE;
	   exit(1);
	}
    }

    dd_start_time = *start_time;
    dd_stop_time = *stop_time;
    strcpy(local_radar_name, radar_name);

    /* get the first record for this radar
     */
    if((n=dap_next_ray_loop()) < 1 ) {
	printf("Unable to get the first record for %s from %s\n"
	       , local_radar_name, a);
	ierr = ACCESS_FAILURE;
	exit(1);
    }
    return(0);
}
/* c------------------------------------------------------------------------ */

int 
dap_mnt_s_ (char *dir, char *radar_name, int32_t *start_time, int32_t *stop_time, int *ivol, int *iscan, int *irec)
{
    /* See if there is a file name and
     * a volume name in the vicinity of the start time
     */
    int ii, ierr=0, ok;

    put_tagged_string((char *)"DORADE_DIR", dir);
    put_tagged_string((char *)"SELECT_RADARS", radar_name);
    difs = dd_return_difs_ptr();
    difs->num_time_lims = 1;
    difs->times[0] = (struct d_limits *)malloc(sizeof(struct d_limits));
    difs->times[0]->lower = *start_time;
    difs->times[0]->upper = *stop_time;
    dis = dd_setup_sweepfile_structs_v3(&ok);

    if(!ok) {
	ierr = ACCESS_FAILURE;
	return(ierr);
    }
    sweep_files = YES;
    usi = dis->usi_top;
    dgi = dd_window_dgi(usi->radar_num);
    return(0);
}
/* c------------------------------------------------------------------------ */
int 
dap_ndx_name_ (char *name2)
{
    DDS_PTR dds=dgi->dds;
    int pn;

    for(pn=0; pn < dgi->num_parms; pn++ ) {
	if(strncmp(name2, dds->parm[pn]->parameter_name, 2 ) == 0 ) 
	      return(pn);
    }
    return(-1);
}
/* c------------------------------------------------------------------------ */

int 
dap_next_ray_ (int *lun, char *buf)
{
    int ii;
    if(sweep_files) {
	if((ii=ddswp_next_ray_v3(usi)) == END_OF_TIME_SPAN) {
	    return(ii);
	}
    }
    else if((ii=dap_next_ray_loop()) == END_OF_TIME_SPAN) {
	/* loop until we find a ray for this radar */
	return(ii);
    }
    dgi->new_sweep = dgi->new_vol = NO;
    return(ii);
}
/* c------------------------------------------------------------------------ */

int 
dap_parm_present_ (char *name2)
{
    int i;

    if((i = dap_ndx_name_( name2 )) == -1 )
	  return(NO);
    return(YES);
}
/* c------------------------------------------------------------------------ */

void 
dap_range_gate_ (float *req_range, int *gate, float *val)
{
    /* return the gate and range of the gate nearest the requested range
     */
    float r= *req_range*1000.; /* convert to meters */
    dd_range_gate(dgi, r, gate, val);
    *val *= .001;		/* convert back to km. */
    return;
}
/* c------------------------------------------------------------------------ */
    
int 
dap_read (int fid, char *buf, int size)
{
    struct generic_descriptor *gd, xgd;
    char name[8];
    int i, m, n;
    int32_t k, mark;
    char *c, str[16];
    static int first=YES, rec_len=0;

    memset( name, 0, 8 );

    if(first) {			
	/* do the first read and try to discover the format */
	first = NO;
	dui->io_type = -1;
	if((n=read( fid, buf, BLOCK_SIZE )) <
	   sizeof(struct generic_descriptor)) {
	    printf("First read in ddin_read unsuccessful...status: %d fid: %d\n"
		   , n, fid);
	    return(n);
	}
	if(n < MAX_REC_SIZE) {
	    /* assume physical io */
	    dui->io_type = PHYSICAL_TAPE;
	    return(n);
	}
	str[4] = '\0';

	/* make sure there are 4 non-zero characters */
	for(i=0,c=buf+4; *c && i < 4; c++, i++);

	if(i == 4) {
	    /* this might be FB_IO */
	    memcpy(str, buf+4, 4);
	    if(strstr(DD_ID_LIST, str)) {
		/* assume fortran-binary io */
		dui->io_type = FB_IO;
	    }
	}
	else {
	    for(i=0,c=buf; *c && i < 4; c++, i++);
	    memcpy(str, buf, 4);
	    if(i == 4 && strstr(DD_ID_LIST, str)) {
		/* assume binary io */
		dui->io_type = BINARY_IO;
	    }
	}
	if(dui->io_type == -1) {
	    /* we've got a problem */
	    printf("\nUnable to identify the data io_type\n");
	    printf("ASCII\n");
	    cascdmp(buf, 0, 280);
	    printf("HEX\n");
	    chexdmp(buf, 0, 128);
	    exit(1);
	}
	mark = lseek(fid, 0L, SEEK_SET); /* rewind */
    }



    if(dui->io_type == PHYSICAL_TAPE) {
	n = read( fid, buf, size );
	return(n);
    }
    else if(dui->io_type == BINARY_IO) {
	mark = lseek(fid, 0L, SEEK_CUR); /* mark offset */
	if((n=read( fid, buf, BLOCK_SIZE )) <
	   sizeof(struct generic_descriptor)) {
	    return(n);
	}
	/*
	 * now end the input on a ray boundary
	 */
	c = buf;
	for(k=m=0,c=buf; m <= n;) {
	   if(hostIsLittleEndian()) {
	      gd = &xgd;
	      memcpy(gd->name_struct, c, 4);
	      swack4(c+4, (char *)&gd->sizeof_struct);
	   }
	   else {
	      gd = (struct generic_descriptor *)c;
	   }
	   strncpy( name, c, 4 );
	   if(strstr(DD_RAY_BOUNDARIES, name)) {
	      k = m;
	   }
	   c += gd->sizeof_struct;
	   m += gd->sizeof_struct;
	}
	mark = lseek(fid, mark+k, SEEK_SET); /* move offset to last ray boundary */
	return(k);
    }
    else if( dui->io_type == FB_IO ) {
	n = fb_read( fid, buf, size );
	return(n);
    }
    return(n);
}
/* c------------------------------------------------------------------------ */

void 
dap_rng_info_ (int *g1, int *m, float *rngs, int *ng)
{
    /* routine to return "m" range values */
    int i=(*g1-1), n;

    dd_rng_info(dgi, g1, m, rngs, ng);
    
    if((n=i+(*m)) > *ng)
	  n = *ng;

    for(; i < n; i++, rngs++)
	  *rngs *= .001;	/* return in km. */
}
/* c------------------------------------------------------------------------ */

void 
dap_setdpr_ (int *val)
{
    Diagnostic_Print = *val ? YES : NO;
}
/* c------------------------------------------------------------------------ */

int 
dap_next_ray_loop (void)
{
    
    while(1) {
	/* keep looping until we're read a ray from the radar
	 * we are interested in
	 */
	if(ddin_next_ray() < 1) { /* from ddin.c */
	    dgi = save_dgi;	/* some routines need last ray info */
	    return(END_OF_TIME_SPAN);
	}
	dgi = dgi_last();

	if(dd_radar_num < 0 ) {
	    /* haven't encountered a ray for this radar yet */
	  if (dgi->radar_name.find(local_radar_name) != std::string::npos) {
		dd_radar_num = dgi->radar_num;
	    }
	}
	if(dgi->radar_num == dd_radar_num) {
	    if( dgi->new_sweep ) {
                dgi->new_sweep = NO;
                dgi->sweep_count++;
            }

	    if( dgi->new_vol ) {
                dgi->new_vol = NO;
                dgi->vol_num++;
            }

	    if(dgi->time > dd_stop_time)
		  return(END_OF_TIME_SPAN);

	    else if(dgi->time >= dd_start_time) {
		save_dgi = dgi;
		if(dgi->dds->radd->scan_mode == DD_SCAN_MODE_AIR)
		    dd_radar_angles(dgi->dds->asib, dgi->dds->cfac
				    , dgi->dds->ra, dgi );
		dgi->clip_gate = dgi->dds->celv->number_cells-1;
		return(1);	/* finally found one */
	    }
	}
    }
    return(0);
}
/* c------------------------------------------------------------------------ */

DGI_PTR 
dap_another_ray (void)
{
    /* read in the next ray of pure dorade data as opposed to sweepfiles
     */
    static int count=0, ddin_bytes_left=0, rec_count=0, eof_count=0;
    static char *ddin_next_block=NULL;
    static struct generic_descriptor *gd;
    static int trip=0, rippin=7000;
    DGI_PTR dgi=NULL;
    DDS_PTR dds;
    int ii, j, mm, n, ryib_flag=NO, mark, trip_inc=1000, final_bad_count;
    int num_rdats=0, pn, ncopy, nmax;
    char *a, *b, *c, str[32];

    struct paramdata_d *rdat;
    struct qparamdata_d *qdat;
    struct radar_d *radd;
    struct sweepinfo_d *swib;
    struct ray_i *ryib;
    struct platform_i *asib;
    /* c...mark */


    while(1) {
	/* absorb the next ray as well as all intervening headers
	 */
	if( count >= rippin ) {
	    mark = 0;
	}
	count++;

	while(ddin_bytes_left <= sizeof(struct generic_descriptor)) {
	    /* read in the next record */
	    rec_count++;
	    if((n = dap_read(dui->dorade_fid
			      , dui->ddin_buf, BLOCK_SIZE)) < 0 )
		  {
		      ddin_bytes_left = 0;
		      return(NULL);
		  }
	    if( n == 0 ) {
		eof_count++;
		dd_stats->file_count++;
		printf( "EOF number: %d at %.2f MB\n"
		       , dd_stats->file_count
		       , dd_stats->MB);
		if( ryib_flag )	/* assume last ray in the file */
		      return(dgi);
		else if( eof_count > 1 )
		      return(NULL);
	    }
	    else if( n < sizeof(struct generic_descriptor)) {
		printf("Record length: %d too small\n", n);
		return(NULL);
	    }
	    else {
		eof_count = 0;
		dd_stats->MB += BYTES_TO_MB(n);
		dd_stats->rec_count++;
		ddin_bytes_left = n;
		ddin_next_block = dui->ddin_buf;
		break;
	    }
	}
	gd = (struct generic_descriptor *)ddin_next_block;

	if(strncmp(ddin_next_block,"RDAT",4) == 0 ||
	   strncmp(ddin_next_block,"QDAT",4) == 0) {
	    pn = num_rdats++;
# ifdef NEW_ALLOC_SCHEME
	    qdat = (struct qparamdata_d *)ddin_next_block;
	    ncopy = *ddin_next_block == 'R'
		  ? sizeof(struct paramdata_d) : dds->parm[pn]->offset_to_data;
	    memcpy((char *)dds->qdat[pn], ddin_next_block, ncopy);
		       
	    if(dds->radd->data_compress == NO_COMPRESSION) {
		mm = gd->sizeof_struct - ncopy;
		memcpy(dds->qdat_ptrs[pn], ddin_next_block, mm);
	    }
	    else {
		a = ddin_next_block + ncopy;
		b = dds->qdat_ptrs[pn];
		nmax = dds->celv->number_cells;
		n = dd_hrd16_uncompressx
		      ((short *)a, (short *)b, (int)dds->parm[pn]->bad_data
		       , &final_bad_count, nmax);
		mm = n * dd_datum_size((int)dds->parm[pn]->binary_format);
	    }
	    dds->qdat[pn]->pdata_length = ncopy + dds->sizeof_qdat[pn];
# else
	    if(dds->radd->data_compress == NO_COMPRESSION) {
		memcpy((char *)dds->rdat[pn], ddin_next_block
		       , gd->sizeof_struct);
	    }
	    else {
		memcpy((char *)dds->rdat[pn], ddin_next_block
		       , sizeof(struct paramdata_d));
		a = ddin_next_block + sizeof(struct paramdata_d);
		b = (char *)dds->rdat[pn] + sizeof(struct paramdata_d);
		n = dd_hrd16_uncompressx
		      ((short *)a, (short *)b, (int)dds->parm[pn]->bad_data
		       , &final_bad_count, dds->celv->number_cells);
		dds->rdat[pn]->pdata_length = sizeof(struct paramdata_d)
		      + n * dd_datum_size((int)dds->parm[pn]->binary_format);
	    }
# endif
	}
# ifdef obsolete
# endif
	else if( ryib_flag && (strncmp(ddin_next_block,"RYIB",4)==0 ||
			       strncmp(ddin_next_block,"SWIB",4)==0 ||
			       strncmp(ddin_next_block,"VOLD",4)==0)) {
	    break;
	}
	else if(strncmp(ddin_next_block,"RYIB",4)==0 ) {
	    num_rdats = 0;
	    ryib_flag = YES;
	    dgi = dgi_last();
	    dds = dgi->dds;
	    for(ii=0; ii < MAX_PARMS; dds->field_present[ii++]=NO);
	    memcpy((char *)dgi->dds->ryib, ddin_next_block
		   , gd->sizeof_struct);
	    dd_stats->ray_count++;
	    dgi->time = dds->dts->time_stamp =
		  eldora_time_stamp( dds->vold, dds->ryib );
	}
	else if(strncmp(ddin_next_block,"ASIB",4)==0) {
	    memcpy((char *)dgi->dds->asib, ddin_next_block
		   , gd->sizeof_struct);
	}
	else if(strncmp(ddin_next_block,"SWIB",4)==0 ) {
	    num_rdats = 0;
	    ryib_flag = NO;
	    swib = (struct sweepinfo_d *)ddin_next_block;
	    dgi = dd_get_structs(dd_assign_radar_num(swib->radar_name));
	    dds = dgi->dds;
	    memcpy(dds->swib, swib, sizeof(struct sweepinfo_d));
	    dgi->new_sweep = YES;
	    dgi->sweep_count++;
	    dgi->file_byte_count = 0;
	}
	else if(strncmp(ddin_next_block,"PARM",4)==0 ) {
	    /* assume dds & vs6 defined by the previous
	     * radd block encounter
	     */
	    pn = dgi->num_parms++;
	    dds->field_present[pn] = YES;
	    memcpy((char *)dds->parm[pn]
		   , ddin_next_block, sizeof(struct parameter_d));
	    dds->parm[pn]->parameter_des_length = sizeof(struct parameter_d);
	}
	else if(strncmp(ddin_next_block,"RADD",4)==0 ) {
	    radd = (struct radar_d *)ddin_next_block;
	    /* set structs for subsequent descriptors */
	    dgi = dd_get_structs(dd_assign_radar_num(radd->radar_name));
	    dds = dgi->dds;
	    memcpy((char *)dds->radd, ddin_next_block
		   , sizeof(struct radar_d));
	    memcpy((char *)dds->vold, (char *)vold
		   , sizeof(struct volume_d));

	    dds->vold->number_sensor_des = 1;
	    dgi->new_vol = dgi->new_sweep = YES;
	    dgi->num_parms = 0;
	    dgi->vol_num++;
	    dds->asib->longitude = dds->radd->radar_longitude;
	    dds->asib->latitude = dds->radd->radar_latitude;
	    dds->asib->altitude_msl = dds->radd->radar_altitude;
	}
	else if(strncmp(ddin_next_block,"CFAC",4)==0 ) {
	    memcpy((char *)dds->cfac, ddin_next_block
		   , sizeof(struct correction_d));
	    for(ii=0; ii < dds->celv->number_cells; ii++) {
		  dds->celvc->dist_cells[ii] += dds->cfac->range_delay_corr;
	    }
	}
	else if(strncmp(ddin_next_block,"CELV",4)==0 ) {
	    memcpy((char *)dds->celv, ddin_next_block
		   , gd->sizeof_struct);
	    memcpy((char *)dds->celvc, ddin_next_block
		   , gd->sizeof_struct);
	    dd_set_uniform_cells(dds);
# ifdef NEW_ALLOC_SCHEME
	    for(ii=0; ii < MAX_PARMS; ii++) {
		if(dgi->dds->field_present[ii])
		      dd_alloc_data_field(dgi, ii);
	    }
# endif
	}
	else if(strncmp(ddin_next_block,"VOLD",4)==0) {
	    num_rdats = 0;
	    ryib_flag = NO;
	    memcpy((char *)vold, ddin_next_block
		   , sizeof(struct volume_d));
	}
	else if(strncmp(ddin_next_block,"WAVE",4)==0 ||
		strncmp(ddin_next_block,"FRIB",4)==0 ||
		strncmp(ddin_next_block,"FRAD",4)==0 ||
		strncmp(ddin_next_block,"CSPD",4)==0)
	      {
		  /* this a field tape */
		  printf("This is a field tape not a DORADE tape\n");
		  exit(1);
	      }
	else {
	    mark = 0;
	}
	ddin_next_block += gd->sizeof_struct;
	ddin_bytes_left -= gd->sizeof_struct;
	if(num_rdats > 0) {
	    if(num_rdats >= dgi->num_parms)
		break;
	}
    }
    if(dgi->dds->radd->scan_mode == DD_SCAN_MODE_AIR ||
       dgi->dds->radd->radar_type == DD_RADAR_TYPE_AIR_LF ||
	dgi->dds->radd->radar_type == DD_RADAR_TYPE_AIR_NOSE )
      {
	 dd_radar_angles( dds->asib, dds->cfac, dds->ra, dgi );
      }
    dgi->clip_gate = dds->celv->number_cells-1;
    return(dgi);
}
/* c------------------------------------------------------------------------ */

void 
kdate_ (char *buf, char *name8)
{
    char tmp[16];
    int y = dgi->dds->vold->year;
    
    sprintf( tmp, "%02d/%02d/%02d"
	    , dgi->dds->vold->month, dgi->dds->vold->day
	    , y > 1900 ? y-1900 : y );
    strncpy( name8, tmp, 8 );
}
/* c------------------------------------------------------------------------ */
int 
kday_ (void) { return(dgi->dds->vold->day); }
/* c------------------------------------------------------------------------ */

void 
kfld1_ (char *buf, char *name2)
{
    /* return the name of the first field */
    strncpy(name2, dgi->dds->parm[0]->parameter_name, 8);
}
/* c------------------------------------------------------------------------ */

void 
kfldtn_ (char *buf, char *name8)
{
    /* return the field tape name */
    strncpy( name8, "UNKNOWN ", 8 );
}
/* c------------------------------------------------------------------------ */
int 
khour_ (void) { return(dgi->dds->ryib->hour); }
/* c------------------------------------------------------------------------ */
int 
klrect_ (void) { return( dgi->ray_num ); }
/* c------------------------------------------------------------------------ */
int 
kminit_ (void) { return(dgi->dds->ryib->minute); }
/* c------------------------------------------------------------------------ */
int 
kminut_ (void) { return(dgi->dds->ryib->minute); }
/* c------------------------------------------------------------------------ */
int 
kmonth_ (void) { return(dgi->dds->vold->month); }
/* c------------------------------------------------------------------------ */
int 
kprnv_ (void) { return(dgi->ray_num); }
/* c------------------------------------------------------------------------ */

void 
kproj_ (char *buf, char *name8)
{
    strncpy( name8, dgi->dds->vold->proj_name, 8 );
}
/* c------------------------------------------------------------------------ */

void 
kradar_ (char *buf, char *name8)
{
    strncpy( name8, dgi->dds->radd->radar_name, 8 );
}
/* c------------------------------------------------------------------------ */
int 
ksecnd_ (void) { return(dgi->dds->ryib->second); }
/* c------------------------------------------------------------------------ */

void 
ksite_ (char *buf, char *name8)
{
    strncpy( name8, "ELECTRA   ", 8 );
}
/* c------------------------------------------------------------------------ */
int 
kswepm_ (void) { return(dgi->dds->radd->scan_mode); }
/* c------------------------------------------------------------------------ */
int 
kswepn_ (void) { return(dgi->sweep_count); }
/* c------------------------------------------------------------------------ */

void 
ktapen_ (char *buf, char *name8)
{
    strncpy( name8, "UNKNOWN ", 8 );
}
/* c------------------------------------------------------------------------ */

void 
ktime_ (char *buf, char *name10)
{
    char tmp[16];
    
    sprintf( tmp, "%02d:%02d:%02dUT"
	    , dgi->dds->ryib->hour, dgi->dds->ryib->minute, dgi->dds->ryib->second );
    strncpy( name10, tmp, 10 );
}
/* c------------------------------------------------------------------------ */

void 
ktimez_ (int *yy, int *mo, int *dd, int *hh, int *mm, int *ss, int *ms)
{
    int y = dgi->dds->vold->year;

    *yy = y > 1900 ? y-1900 : y;
    *mo = dgi->dds->vold->month;
    *dd = dgi->dds->vold->day;
    *hh = dgi->dds->ryib->hour;
    *mm = dgi->dds->ryib->minute;
    *ss = dgi->dds->ryib->second;
    *ms = dgi->dds->ryib->millisecond;
}
/* c------------------------------------------------------------------------ */

void 
ktzone_ (char *buf, char *name2)
{
    strncpy( name2, "UTC", 2 );
}
/* c------------------------------------------------------------------------ */
int 
kvoln_ (void) { return(dgi->vol_num); }
/* c------------------------------------------------------------------------ */
int 
kyear_ (void) {int y = dgi->dds->vold->year; return(y > 1900 ? y-1900 : y); }
/* c------------------------------------------------------------------------ */
void 
cuazim_ (float *x)
{
    double d;
     d = dd_azimuth_angle(dgi);
    *x = d;
   return;
}
/* c------------------------------------------------------------------------ */
void 
cudmrc_ (float *x)
{ 
    *x = dgi->dds->radd->radar_const;
    return;
}
/* c------------------------------------------------------------------------ */

void 
cudmprf_ (char *name2, float *x)
{
    int i;
    float z;

    /* calculate the effective prf */
    i = dap_ndx_name_(name2);
    if((z = dgi->dds->radd->interpulse_per1) > 0 ) {
	*x = 1./z;
    }
    else
	  *x = -1;

    return;
}
/* c------------------------------------------------------------------------ */
void 
cuelev_ (float *x)
{ 
    *x = dd_elevation_angle(dgi);
    return;
}
/* c------------------------------------------------------------------------ */
void 
cufixed_ (float *x)
{
    *x = dgi->dds->swib->fixed_angle;
    return;
}
/* c------------------------------------------------------------------------ */
void 
cugealt_ (float *x)
{
    *x = dgi->dds->asib->altitude_agl;
    /* return km. */
    return; 
}
/* c------------------------------------------------------------------------ */
void 
cuhbwid_ (float *x)
{
    *x = dgi->dds->radd->horz_beam_width;
    /* return deg. */
    return; 
}
/* c------------------------------------------------------------------------ */
void 
cuhight_ (float *x)
{
    *x = dgi->dds->asib->altitude_msl+dgi->dds->cfac->pressure_alt_corr;
    /* return km. */
    return; 
}
/* c------------------------------------------------------------------------ */
void 
culatit_ (float *x)
{ 
    *x = dgi->dds->asib->latitude;
    return;
}
/* c------------------------------------------------------------------------ */
void 
culongt_ (float *x)
{ 
    *x = dgi->dds->asib->longitude;
    return;
}
/* c------------------------------------------------------------------------ */

void 
cuvenyq_ (float *x)
{
    *x = dgi->dds->radd->eff_unamb_vel;
    return;
}
/* c------------------------------------------------------------------------ */

void 
curotat_ (float *x)
{
    if( dgi->dds->radd->scan_mode == DD_SCAN_MODE_AIR ) {
	*x = dgi->dds->asib->rotation_angle;
    }
    else if( dgi->dds->radd->scan_mode == DD_SCAN_MODE_RHI ) {
	*x = dgi->dds->ryib->elevation;
    }
    else {
	*x = dgi->dds->ryib->azimuth;
    }
}
/* c------------------------------------------------------------------------ */

void 
time2unix_ (int time[6], int32_t *unixtime)
{
    DD_TIME dts;
    double d;
    
    dts.day_seconds = time[3]*3600. +time[4]*60. +time[5];
    dts.year = time[0] > 1900 ? time[0] : time[0] +1900;
    dts.month = time[1];
    dts.day = time[2];

    *unixtime = (int32_t)d_time_stamp(&dts);
}
/* c------------------------------------------------------------------------ */

# define WORKAROUND

# ifdef WORKAROUND
/* c------------------------------------------------------------------------ */
int 
dap_ndx_name__ (char *name2)
{
   return( dap_ndx_name_(name2) );
}
/* c------------------------------------------------------------------------ */

void 
dap_rng_info__ (int *g1, int *m, float *rngs, int *ng)
{
   dap_rng_info_( g1, m, rngs, ng );
}
/* c------------------------------------------------------------------------ */

void 
dap_givfld__ (int *ndx, int *g1, int *n, float *dd, float *badval)
{
    dd_givfld( dgi, *ndx, *g1, *n, dd, badval );
}
/* c------------------------------------------------------------------------ */

void 
dap_range_gate__ (float *req_range, int *gate, float *val)
{
   dap_range_gate_( req_range, gate, val );
}
/* c------------------------------------------------------------------------ */

int 
dap_parm_present__ (char *name2)
{
    return( dap_parm_present_( name2 ) );
}
/* c------------------------------------------------------------------------ */

int 
dap_next_ray__ (int *lun, char *buf)
{
    return( dap_next_ray_(lun, buf ) );
}
/* c------------------------------------------------------------------------ */

int 
dap_mnt_s__ (char *dir, char *radar_name, int32_t *start_time, int32_t *stop_time, int *ivol, int *iscan, int *irec)
{
    return( dap_mnt_s_( dir, radar_name, start_time, stop_time
	   , ivol, iscan, irec ) );
}
/* c------------------------------------------------------------------------ */

int 
dap_mnt_f__ (char *input_file_name, char *radar_name, int32_t *start_time, int32_t *stop_time, int *ivol, int *iscan, int *irec)
{
    return( dap_mnt_f_( input_file_name, radar_name, start_time, stop_time
	   , ivol, iscan, irec ) );
}
/* c------------------------------------------------------------------------ */

void 
dap_setdpr__ (int *val)
{
    dap_setdpr_( val );
}
# endif
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */





