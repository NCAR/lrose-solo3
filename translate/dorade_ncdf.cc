/* 	$Id$	 */

//#ifndef lint
//static char vcid[] = "$Id$";
//#endif /* lint */

/*
 * This file contains the following routines
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 * 
 */

# ifdef NETCDF
# include <netcdf.h>
# endif
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dd_math.h>
#include <dd_defines.h>
#include "piraq.hh"
#include "piraq_dd.hh"
#include <piraqx.h>
#include "dorade_share.hh"
#include "dd_time.hh"
#include "dorade_ncdf.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "dd_crackers.hh"
#include "dd_der_flds.hh"

# define NC_SCALE(x,scale,offset) (  (x) >= 0 \
      ? ((x)+(offset))*(scale)+.5 : ((x)+(offset))*(scale)-.5)
# define NC_UNSCALE(x,rcp_scale,offset) ((x)*(rcp_scale)-(offset)  )

/* c------------------------------------------------------------------------ */

static int 
dd_setFxdR1GsEtc (struct ncdf_production *ncp, int num_dims, int *dims);

static int 
dd_setLatLonAlt (struct ncdf_production *ncp, int num_dims, int *dims);

static int 
dd_setRcvrXmtrStuff (struct ncdf_production *ncp, int num_dims, int *dims);

static int 
dd_setAzEl (struct ncdf_production *ncp, int num_dims, int *dims);

static int 
dd_setACparams (struct dd_general_info *dgi, struct ncdf_production *ncp, int num_dims, int *dims);

/* c------------------------------------------------------------------------ */

struct rnc_info * return_rnc_info( char * name )
{
  static int count = 0;
  int ii, jj, mm, nn, nt;
  int kk, match = -1;
  char *aa, *bb, str[256], *sptrs[32];
  struct rnc_info * ri; 

  static char * known_aliases [] =
  {
    (char *)"DZ DB DBZ"
    , (char *)"DM"
    , (char *)"VE VR VF VQ VU"
    , (char *)"RH RX RHO RHOHV"
    , (char *)"PH DP PHI PHIDP"

    , (char *)"ZD DR ZDR"
    , (char *)"LD LC LDR"
    , (char *)"LV LVDR LDRV"
    , (char *)"NC NCP"
    , (char *)"SW"

    , (char *)"CH"
    , (char *)"CV"
    , (char *)"AH"
    , (char *)"AV"
    , (char *)"NIQ"

    , (char *)"AIQ"
    , (char *)"UNKNOWN"
    , (char *)"KDP NKDP CKDP"
    , (char *)"ZAC DAC NAC HAC"
    , (char *)"PD WPD"

    , (char *)"SPHI"
    , (char *)"SVR"
    , (char *)"SZDR"
    , (char *)"HDR"
    , (char *)"TEMP"
  };

  static char * long_names [] =
  {
    (char *)"Reflectivity factor"
    , (char *)"Reflected power in dBm"
    , (char *)"Doppler velocity"
    , (char *)"Copolar Cross Correlation"
    , (char *)"Differential Phase"

    , (char *)"Polarization_diversity"
    , (char *)"Linear Depolarization Ratio"
    , (char *)"Vertical Linear Depolarization Ratio"
    , (char *)"Normalized Coherent Power"
    , (char *)"Spectral width"

    , (char *)"Horizontal Rho value"
    , (char *)"Vertical Rho value"
    , (char *)"Horizontal Rho angle"
    , (char *)"Vertical Rho value"
    , (char *)"log (avg(I**2)+avg(Q**2))"

    , (char *)"Angle avg I,Q"
    , (char *)"unknown"
    , (char *)"Derivative of PhiDp"
    , (char *)"Rainfall Accumulation"
    , (char *)"Particle Identifier"

    , (char *)"Std. Dev. of PhiDp"
    , (char *)"Std. Dev. of Raw Doppler Velocities"
    , (char *)"Std. Dev. of Zdr"
    , (char *)"Hail Detection Ratio"
    , (char *)"Temperature"
  };

  static char * units[] =
  {
    (char *)"dBz"
    , (char *)"dBm"
    , (char *)"m/s"
    , (char *)" "
    , (char *)"deg"

    , (char *)"dBm"
    , (char *)"dBm"
    , (char *)"dBm"
    , (char *)" "
    , (char *)"m/s"

    , (char *)" "
    , (char *)" "
    , (char *)"deg"
    , (char *)"deg"
    , (char *)" "

    , (char *)"deg"
    , (char *)" "
    , (char *)"deg/km"
    , (char *)"mm"
    , (char *)" "

    , (char *)" "
    , (char *)" "
    , (char *)" "
    , (char *)" "
    , (char *)"deg"
  };
  
//  static int field_types[] =  //Jul 26, 2011
  static int field_types_lst[] =  //Jul 26, 2011 change array name to field_types_lst because field_types is an enum name
  {
    DBZ_FLD
  , DM_FLD
  , VEL_FLD
  , RHO_FLD
  , PHI_FLD

  , ZDR_FLD
  , LDR_FLD
  , LVDR_FLD
  , NCP_FLD
  , SW_FLD

  , CH_FLD
  , CV_FLD
  , AH_FLD
  , AV_FLD
  , NIQ_FLD

  , AIQ_FLD
  , UNK_FLD
  , KDP_FLD
  , RAC_FLD
  , PID_FLD

  , SDPHI_FLD
  , SDVR_FLD
  , SDZDR_FLD
  , HDR_FLD
  , TEMP_FLD

  };

  static float valid_ranges[] =
  {
    -30, 80
    , -130, 30
    , -100, 100
    , 0, 1
    , -180, 180

    , -10, 20 
    , -35, 10
    , -35, 10
    , 0, 1
    , 0, 50 

    , 0, 1
    , 0, 1
    , 0, 360
    , 0, 360
    , -999, 999

    , -180, 180
    , -999, 999
    , 0, 20
    , 0, 200
    , 0, 20

    , 0, 90
    , 0, 50
    , 0, 20
    , -100, 100
    , -100, 50

  };

  static float scale_and_offset[] =
  {
    100, 0
    , 100, 0
    , 100, 0
    , 100, 0
    , 100, 0

    , 100, 0
    , 100, 0
    , 100, 0
    , 100, 0
    , 100, 0

    , 100, 0
    , 100, 0
    , 100, 0
    , 100, 0
    , 100, 0

    , 100, 0
    , 100, 0
    , 100, 0
    , 100, 0
    , 100, 0

    , 100, 0
    , 100, 0
    , 100, 0
    , 100, 0
    , 100, 0
  };

  static float missing_and_fill_values[] =
  {
    -32768, -32767
    , -32768, -32767
    , -32768, -32767
    , -32768, -32767
    , -32768, -32767

    , -32768, -32767
    , -32768, -32767
    , -32768, -32767
    , -32768, -32767
    , -32768, -32767

    , -32768, -32767
    , -32768, -32767
    , -32768, -32767
    , -32768, -32767
    , -32768, -32767

    , -32768, -32767
    , -32768, -32767
    , -32768, -32767
    , -32768, -32767
    , -32768, -32767

    , -32768, -32767
    , -32768, -32767
    , -32768, -32767
    , -32768, -32767
    , -32768, -32767
  };

  jj = sizeof(known_aliases)/sizeof(char *); 
  nn = strlen( name );

  for( ii = 0; ii < jj; ii++ ) {
    strcpy( str,  known_aliases[ii] );
    nt = dd_tokens( str, sptrs );
    for( kk = 0; kk < nt; kk++ ) {
      if( !strcmp( name, sptrs[kk] )) {
	match = ii;
	break;
      }
    }
    if( match >= 0 )
      { break; }
  }
  ri = (struct rnc_info *)malloc( sizeof( struct rnc_info ));
  memset( ri, 0, sizeof( struct rnc_info ));

  if( match < 0 )
    { match = UNK_FLD; }
  ii = match * 2;

//  ri->field_type = field_types[match]; //Jul 26, 2011 enum field_types
  ri->field_type = (field_types) field_types_lst[match]; //Jul 26, 2011
  strcpy( ri->units, units[match] );
  strcpy( ri->long_name, long_names[match] );
  ri->valid_range[0] = valid_ranges[ii];
  ri->valid_range[1] = valid_ranges[ii+1];
  ri->ncdf_scale_factor =  scale_and_offset[ii];
  ri->ncdf_add_offset = scale_and_offset[ii+1];
  ri->missing_value = missing_and_fill_values[ii];
  ri->fill_value = missing_and_fill_values[ii+1];
  return( ri );
}
/* c------------------------------------------------------------------------ */


static struct input_read_info *irq;
static int num_vars = 0;
static char *nc_var_list;
static char *nc_var_name[64];
static int nc_var_id[64];
static int dd_indices[64];


struct ncdf_control_struct {
    int encountered[MAX_SENSORS];
    struct ncdf_production *ncp[MAX_SENSORS];
    int last_radar_num;
    int options;
    char dir_name[256];
    char prefix[16];
    float tst_pls_ranges[2];
};



static int ncdf_output=1;
static int count = 0;
static int ncdf_num_radars=0;
static struct ncdf_control_struct *ncs=NULL;
static float def_fill = -32767;
static float def_missing = -32768;
static int notSet = -32768;


//# ifdef obsolete
//stati int ();
//# endif


/* c------------------------------------------------------------------------ */
 /* c...mark */
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

static char *
horiz_fields_list (void)
{
  static char list[256];

  strcpy( list, "DZ DB DBZ DCZ DM VE VT VR VL VS VQ NC NCP SW" );
  strcat( list, " ZAC DAC SVR CH AH" );

  return( list );
}

/* c------------------------------------------------------------------------ */

static char *
vert_fields_list (void)
{
  static char list[256];

  strcpy( list, "DY DL CV AV" );

  return( list );
}

/* c------------------------------------------------------------------------ */

static char *
horiz_and_vert_fields_list (void)
{
  static char list[256];

  strcpy( list, "DX PH DP PHI PHIDP RH RX RHO RHOHV ZD DR ZDR LC LD LDR" );
  strcat( list, "LVDR LDRV KDP NKDP CKDP HDR SDZDR SPHI PD WPD KAC NAC HAC" );

  return( list );
}


/* c------------------------------------------------------------------------ */

static int 
in_list (char *name, char *list)
{
    char str[256], * sptrs[64], *aa, *bb, *cc;
    int ii, jj, kk, mm, nn, nt, ndx;

    if(( mm = strlen( name )) < 1 )
      { return NO; }

    strcpy( str, list );
    nt = dd_tokens( str, sptrs );

    for( ii = 0; ii < nt; ii++ ) {
      nn = strlen( sptrs[ii] );
      if( mm != nn )
	{ continue; }
      if( !strcmp( name, sptrs[ii] ))
	return YES;
    }
    return NO;
}

/* c------------------------------------------------------------------------ */

void 
produce_ncdf (struct dd_general_info *dgi, int finish)
{

    struct ncdf_production *ncp;
    char str[256], *sptrs[32], *aa, *bb, *cc;
    int ii, jj, kk, nt, mark;

    if(!ncdf_output)
	  return;

//    irq = return_pir_irq();  // July 06, 2011 There is no function declaration for return_ir_irq()
    count++;			/* c...mark */

    if(!ncdf_num_radars) {
	/*
	 * first time through
	 */
	ncdf_output = NO;

	if(aa = get_tagged_string("OUTPUT_FLAGS")) {
	    if( strstr(aa,"NETCDF_DATA")) {
		ncdf_output = YES;
	    }
	}

# ifndef NETCDF
	ncdf_output = NO;
# endif	
	if(!ncdf_output)
	      return;
	nc_var_list = (char *)malloc (1024);
	memset (nc_var_list, 0, 1024);

	if( aa = getenv( "NETCDF_FIELDS" )) {
	  strcpy( nc_var_list, aa );
	  num_vars = dd_tokens( nc_var_list, nc_var_name );
	  if( num_vars < 1 ) {
	    printf( "No NETCDF_FIELDS!\n" );
	    ncdf_output = NO;
	    return;
	  }
	}
	else {			/* get list from first record */
	  num_vars = 0;
	}
	ncs = (struct ncdf_control_struct *)
	      malloc(sizeof(struct ncdf_control_struct));
	memset((char *)ncs, 0, sizeof(struct ncdf_control_struct));
	strcpy( ncs->prefix, "tmp_" );

	if( aa = get_tagged_string("NC_DIR") ) {
	  slash_path( ncs->dir_name, aa );
	}
	else {
	  slash_path( ncs->dir_name, dgi->directory_name.c_str() );
	}
	if( aa = get_tagged_string( "OPTIONS" )) {
	  mark = 0;
	}
	if( aa = get_tagged_string( "TEST_PULSE_RANGES" )) {
	  strcpy( str, aa );
	  nt = dd_tokens( str, sptrs );
	  for( ii = 0; nt >= 2 && ii < 2; ii++ ) {
	    ncs->tst_pls_ranges[ii] = (float)atof( sptrs[ii] ) * 1000.;
	  }
	}
    }

    if( finish ) {
      /* close out all open sweepfiles */
      for( ii=0; ii < MAX_SENSORS; ii++ ) {
	if( ncs->ncp[ii] ) {
# ifdef NETCDF
	  ncdf_finish( ncs->ncp[ii] );
# endif
	}
      }
    }

    if(ncs->ncp[dgi->radar_num]) {
	ncp = ncs->ncp[dgi->radar_num];
    }
    else {
	ncdf_num_radars++;

	ncp = ncs->ncp[dgi->radar_num] = (struct ncdf_production *)
	      malloc(sizeof(struct ncdf_production));
	memset((char *)ncp, 0, sizeof(struct ncdf_production));
	ncp->new_vol = ncp->new_sweep = YES;
	ncp->source_format = dgi->source_fmt;
    }

# ifdef NETCDF
    ncp_stuff_ray(dgi, ncp);
# endif
    ncs->last_radar_num = dgi->radar_num;
    ncp->new_vol = ncp->new_sweep = NO;
}

# ifdef NETCDF

/* c------------------------------------------------------------------------ */

void 
ncdf_finish (struct ncdf_production *ncp)
{
  char str[256], str2[256], *aa, ii;

  if( ncp->ray_count < 1 )
    { return; }

    nc_close( ncp->ncid );
    slash_path(str, ncs->dir_name);
    strcat( str, ncs->prefix );
    strcat( str, ncp->filename );
 
    if( ncp->ray_count < dd_min_rays_per_sweep() ) {
      ii = unlink(str);
      return;
    }
    slash_path(str2, ncs->dir_name);
    strcat( str2, ncp->filename );
    ii = rename( str, str2 );
}
/* c------------------------------------------------------------------------ */

void 
ncdf_init_sweep (struct dd_general_info *dgi, struct ncdf_production *ncp)
{
   struct dds_structs *dds=dgi->dds;
   struct radar_d *radd=dds->radd;
   struct cell_d *celv=dds->celvc;
   struct sweepinfo_d *swib=dds->swib;
   struct ray_i *ryib=dds->ryib;
   struct platform_i *asib=dds->asib;
   struct correction_d *cfac=dds->cfac;
   DD_TIME *dts=dds->dts;
   XTRA_STUFF *xstf = NULL;
   static PIRAQX *prqx = NULL, *prqxs = NULL;
   static int sizeof_prqx = 0;
   struct timeval tp;
   struct timezone tzp;
   char *aa, *bb, *cc, str[256], smode[16], *xstf_first_item = NULL;
   char radar_name[16];
   double d, d_vals[8];
   float f, f_vals[8];
   int ii, jj, kk, nn, fn, mark;
   int ncid;
   int rays_did;
   int gates_did;
   int gates2_did;
   int systems_did;
   int nf_dim, ss_dim, ls_dim, short_string = 16, long_string = 80;
   int fld_names_id;
   int frib_name_id;
   int dims[4], num_dims = 0;
   int ndx, fld_ndx;
   float scale_factor;
   short sii;
   size_t start[4], count[4];
   RADARV * rhdr;
   HEADERV * dwel;
   struct radar_consts *r_consts;
   double nc_prf, nc_waveln;
				/* c...mark */
    
   if (dgi->dds->xstf &&
       dgi->dds->xstf->sizeof_struct > sizeof (XTRA_STUFF)) {
      xstf = dgi->dds->xstf;
      xstf_first_item = (char *)xstf +xstf->offset_to_first_item; 
   }
   str_terminate (radar_name, dgi->radar_name, 8);

   /* construct the file name */
   
   if( !num_vars ) {
      aa = nc_var_list;
      
      for( ii = 0; ii < dgi->num_parms; ii++ ) {
	 str_terminate( aa, dgi->dds->parm[ii]->parameter_name, 8 );
	 strcat( aa, " " );
	 aa += strlen( aa );
      }
      num_vars = dd_tokens( nc_var_list, nc_var_name );
    }

   if( dgi->new_vol ) {
     ncp->vol_count++;
     ncp->vol_start_time = (time_t)dts->time_stamp;
     sprintf (ncp->vol_stt
	      , "%d%02d%02d %02d%02d%02d"
	      , dts->year
	      , dts->month
	      , dts->day
	      , dts->hour
	      , dts->minute
	      , dts->second
	      );
   }
   ncp->sweep_count++;
   ncp->ray_count = 0;
   dd_scan_mode_mne(radd->scan_mode, smode);
   
   sprintf( ncp->filename
	   , "ncswp_%s_%d%02d%02d_%02d%02d%02d.%03d_u%d_s%d_%.1f_%s_.nc"
	   , radar_name
	   , dts->year
	   , dts->month
	   , dts->day
	   , dts->hour
	   , dts->minute
	   , dts->second
	   , dts->millisecond
	   , ncp->vol_count
	   , ncp->sweep_count
	   , swib->fixed_angle
	   , smode
	   );
   
   if( ncp->vol_count == 1 && ncp->sweep_count == 1 )
     { printf( "%s\n", ncs->dir_name ); }
   slash_path( str, ncs->dir_name );
   bb = str + strlen( str );
   strcat( str, ncs->prefix );
   strcat( str, ncp->filename );
   ii = nc_create( str, NC_CLOBBER, &ncid );
   aa = (char *)nc_strerror( ii );
   printf( "%s: %d %s\n", bb, ii, aa );
   if( ii )
     { exit(1); }
   /* c...mark */
   ncp->ncid = ncid;
   
   strcpy( str, "This file contains one scan of remotely sensed data" );
   ii = nc_put_att_text( ncid, NC_GLOBAL
			, "Content", strlen( str ) +1, str );
   aa = (char *)nc_strerror( ii );
   strcpy( str, "NCAR_ATD-NOAA_ETL/Scanning_Remote_Sensor" );
   ii = nc_put_att_text( ncid, NC_GLOBAL
			, "Conventions", strlen( str ) +1, str );
   strcpy( str, dgi->radar_name);
   ii = nc_put_att_text( ncid, NC_GLOBAL, "Instrument_Name"
			, strlen(str)+1, str );
   dd_radar_type_ascii( radd->radar_type, str );
   ii = nc_put_att_text( ncid, NC_GLOBAL, "Instrument_Type"
			, strlen(str)+1, str );
   strcpy( str, smode );
   ii = nc_put_att_text( ncid, NC_GLOBAL, "Scan_Mode"
			, strlen(str)+1, str );
   strcpy (str, ncp->vol_stt);
   ii = nc_put_att_text( ncid, NC_GLOBAL, "Volume_Start_Time"
			 , strlen(str)+1, str );
   strcpy( str, dts_print(dts) );
   ii = nc_put_att_text( ncid, NC_GLOBAL, "Start_Time"
			, strlen(str)+1, str );
   jj = dts->year;
   ii = nc_put_att_int( ncid, NC_GLOBAL, "Year"
		       , NC_INT, 1, &jj );
   jj = dts->month;
   ii = nc_put_att_int( ncid, NC_GLOBAL, "Month"
		       , NC_INT, 1, &jj );
   jj = dts->day;
   ii = nc_put_att_int( ncid, NC_GLOBAL, "Day"
		       , NC_INT, 1, &jj );
   jj = dts->hour;
   ii = nc_put_att_int( ncid, NC_GLOBAL, "Hour"
		       , NC_INT, 1, &jj );
   jj = dts->minute;
   ii = nc_put_att_int( ncid, NC_GLOBAL, "Minute"
		       , NC_INT, 1, &jj );
   jj = dts->second;
   ii = nc_put_att_int( ncid, NC_GLOBAL, "Second"
		       , NC_INT, 1, &jj );
   
   jj = ncp->vol_count;
   ii = nc_put_att_int( ncid, NC_GLOBAL, "Volume_Number"
		       , NC_INT, 1, &jj );
   jj = ncp->sweep_count;
   ii = nc_put_att_int( ncid, NC_GLOBAL, "Scan_Number"
		       , NC_INT, 1, &jj );
   jj = dds->parm[0]->num_samples;
   ii = nc_put_att_int( ncid, NC_GLOBAL, "Num_Samples"
		       , NC_INT, 1, &jj );
   jj = 0;
   ii = nc_put_att_int( ncid, NC_GLOBAL, "Index_of_horizontal_information"
		       , NC_INT, 1, &jj );
   aa = (char *)nc_strerror( ii );
   
   if( dgi->source_fmt == APIRAQ_FMT ) {
      jj = 1;
      ii = nc_put_att_int( ncid, NC_GLOBAL, "Index_of_vertical_information"
			  , NC_INT, 1, &jj );
   }
   str_terminate( str, dds->vold->proj_name, sizeof(dds->vold->proj_name));
   ii = nc_put_att_text( ncid, NC_GLOBAL, "Project_Name"
			, strlen(str)+1, str );
   ii = gettimeofday(&tp,&tzp);
   aa = ctime( &tp.tv_sec );
   *( aa +strlen(aa) -1 ) = '\0';
   strcpy( str, aa );
   ii = nc_put_att_text( ncid, NC_GLOBAL, "Production_Date"
			, strlen(str)+1, str );
   strcpy( str, "NSF/UCAR/NCAR/ATD" );
   ii = nc_put_att_text( ncid, NC_GLOBAL, "Producer_Name"
			, strlen(str)+1, str );
   strcpy( str, "xltrs" );
   ii = nc_put_att_text( ncid, NC_GLOBAL, "Software"
			, strlen(str)+1, str );
   strcpy (str, "Certain radars have variable cell spacings");
strcat( str, "defined in segments of constant cell spacing");
   
   ii = nc_put_att_text( ncid, NC_GLOBAL, "Range_Segments"
			, strlen(str)+1, str );
   
   
   /*  // define dimensions  */
   
   /* These declarations yield a dimension identifier which is used
    * in future variable declarations
    */
   
   ii = nc_def_dim( ncid, "Time", NC_UNLIMITED, &rays_did );
   ncp->num_cells = celv->number_cells;
   ii = nc_def_dim( ncid, "maxCells", ncp->num_cells, &gates_did );
   ii = nc_def_dim( ncid, "maxCells2", 11, &gates2_did );
   
   nn = 1;
   if (xstf && xstf->source_format == APIRAQ_FMT)
     { nn = 2; }
   else if (dgi->source_fmt == APIRAQ_FMT)
     { nn = 2; }
   ii = nc_def_dim( ncid, "numSystems", nn, &systems_did );
   
   ii = nc_def_dim( ncid, "fields", num_vars, &nf_dim );
   ii = nc_def_dim( ncid, "short_string", short_string, &ss_dim );
   ii = nc_def_dim( ncid, "long_string", long_string, &ls_dim );
   
   
   
   /*  // define variables  */
   
   ii = nc_def_var( ncid, "volume_start_time", NC_INT, 0, dims, &ncp->id_vst );
   strcpy( str, "Unix Date/Time value for volume start time" );
   ii = nc_put_att_text( ncid, ncp->id_vst
			 , "long_name", strlen( str ) +1, str );
   strcpy( str, "seconds since 1970-01-01 00:00 UTC" );
   ii = nc_put_att_text( ncid, ncp->id_vst
			 , "units", strlen( str ) +1, str );
   
   ii = nc_def_var( ncid, "base_time", NC_INT, 0, dims, &ncp->id_ut );
   strcpy( str, "Unix Date/Time value for first record" );
   ii = nc_put_att_text( ncid, ncp->id_ut
			, "long_name", strlen( str ) +1, str );
   strcpy( str, "seconds since 1970-01-01 00:00 UTC" );
   ii = nc_put_att_text( ncid, ncp->id_ut
			, "units", strlen( str ) +1, str );
   jj = 0;
   ii = nc_put_att_int( ncid, ncp->id_ut
		       , "missing_value", NC_INT, 1, &jj );
   ii = nc_put_att_int( ncid, ncp->id_ut
			, "_FillValue", NC_INT, 1, &jj );

  dims[0] = nf_dim;
  dims[1] = ss_dim;
  ii = nc_def_var( ncid, "field_names", NC_CHAR, 2, dims, &fld_names_id );

  if( radd->radar_type != GROUND ) {
    num_dims = 1;
    dims[0] = rays_did;
    if (dds->frib) {
       ii = nc_def_var( ncid, "frib_file_name", NC_CHAR, 1, &ls_dim
		       , &frib_name_id );
    }
  }

  ii = dd_setFxdR1GsEtc( ncp, num_dims, dims );
  if( ii != NC_NOERR ) {
    aa = (char *)nc_strerror( ii );
  }

  if( radd->radar_type == GROUND ) {
    num_dims = 0;
    ii = dd_setLatLonAlt( ncp, num_dims, dims );
  }

  num_dims = 1;
  dims[0] = systems_did;
  ii = dd_setRcvrXmtrStuff( ncp, num_dims, dims );

  num_dims = 1;
  dims[0] = rays_did;

  ii = nc_def_var( ncid, "time_offset", NC_DOUBLE, num_dims, dims, &ncp->id_tmoffs );
  strcpy( str, "time offset of the current record from base_time" );
  ii = nc_put_att_text( ncid, ncp->id_tmoffs
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "seconds" );
  ii = nc_put_att_text( ncid, ncp->id_tmoffs
			, "units", strlen( str ) +1, str );
  d = 0;
  ii = nc_put_att_double( ncid, ncp->id_tmoffs
			, "missing", NC_DOUBLE, 1, &d );
  ii = nc_put_att_double( ncid, ncp->id_tmoffs
			, "_FillValue", NC_DOUBLE, 1, &d );

  ii = dd_setAzEl( ncp, num_dims, dims );
  if( ii != NC_NOERR ) {
    aa = (char *)nc_strerror( ii );
  }

  if( radd->radar_type != GROUND ) {
    ii = dd_setLatLonAlt( ncp, num_dims, dims );
    ii = dd_setACparams( dgi, ncp, num_dims, dims );
  }

  num_dims = 2;
  dims[0] = rays_did;
  dims[1] = gates_did;
 
  for( ndx = 0; ndx < num_vars; ndx++ ) { /* for each field */

    fld_ndx = dd_find_field( dgi, nc_var_name[ndx] );
    
    if( fld_ndx < 0 ) {
      fld_ndx = dd_alias_index_num( dgi, nc_var_name[ndx] );
    }
    if( fld_ndx < 0 )
      { continue; }

    dd_indices[ndx] = fld_ndx;


    ii = nc_def_var( ncid, nc_var_name[ndx], NC_SHORT, num_dims, dims
		     , &nc_var_id[ndx] );
    aa = str;
    str_terminate( aa, (char *)dds->parm[fld_ndx]->param_description, 40 );
    ii = nc_put_att_text( ncid, nc_var_id[ndx], "long_name", strlen(aa)+1
			  , aa );
    aa = (char *)"data";
    ii = nc_put_att_text( ncid, nc_var_id[ndx], "variable_type"
			 , strlen(aa)+1, aa );

    aa = str;
    str_terminate( aa, (char *)dds->parm[fld_ndx]->param_units, 8 );
    if( !strcmp( aa, "m/s" )) { strcpy( aa, "meters/second" ); }
    ii = nc_put_att_text( ncid, nc_var_id[ndx], "units", strlen(aa)+1, aa );

    d = 1./dds->parm[fld_ndx]->parameter_scale;
    scale_factor = (float)d;
    ii = nc_put_att_float( ncid, nc_var_id[ndx]
			   , "scale_factor", NC_FLOAT, 1, &scale_factor );
    f = 0;
    ii = nc_put_att_float( ncid, nc_var_id[ndx]
			   , "add_offset", NC_FLOAT, 1, &f );

    sii = (short)dds->parm[fld_ndx]->bad_data;
    ii = nc_put_att_short( ncid, nc_var_id[ndx]
				 , "missing_value", NC_SHORT, 1, &sii );
    ii = nc_put_att_short( ncid, nc_var_id[ndx]
				 , "_FillValue", NC_SHORT, 1, &sii );
# ifdef obsolete
# endif
    aa = nc_var_name[ndx];
    cc = (char *)"Horizontal";

    if( in_list( aa, vert_fields_list() ))
      { cc = (char *)"Vertical"; }
    else if( in_list( aa, horiz_and_vert_fields_list() )) 
      { cc = (char *)"Horizontal and Vertical"; }

    strcpy( str, cc );
    ii = nc_put_att_text( ncid, nc_var_id[ndx], "polarization"
			  , strlen(str)+1, str );

    nn = dd_return_frequencies( dgi, fld_ndx, d_vals );
    nc_waveln = SPEED_OF_LIGHT/( d_vals[0] * 1.e9 );

    ii = nc_put_att_double( ncid, nc_var_id[ndx], "Frequencies_GHz", NC_DOUBLE
			    , nn, d_vals );
    nn = dd_return_interpulse_periods( dgi, fld_ndx, d_vals );
    nc_prf = d_vals[0] ? 1./d_vals[0] : 0;
    ii = nc_put_att_double( ncid, nc_var_id[ndx], "InterPulsePeriods_secs"
			    , NC_DOUBLE, nn, d_vals );

    jj = 1;
    ii = nc_put_att_int( ncid, nc_var_id[ndx]
			   , "num_segments", NC_INT, 1, &jj );
    jj = celv->number_cells;
    ii = nc_put_att_int( ncid, nc_var_id[ndx]
			   , "cells_in_segment", NC_INT, 1, &jj );
    f = celv->dist_cells[0];
    ii = nc_put_att_float( ncid, nc_var_id[ndx]
			   , "meters_to_first_cell", NC_FLOAT, 1, &f );
    f = celv->dist_cells[1] -celv->dist_cells[0];
    ii = nc_put_att_float( ncid, nc_var_id[ndx]
			   , "meters_between_cells", NC_FLOAT, 1, &f );

  }
  /* end definitions */

  ii = nc_enddef( ncid );
  aa = (char *)nc_strerror( ii );
				/*
  ii = nc_sync( ncid );
				 */
  aa = (char *)nc_strerror( ii );
  mark = 0;

  /* set parameters that are constant
   * throughout the sweep
   */
  if( radd->radar_type == GROUND ) {
    d = radd->radar_latitude;
    ii = nc_put_var_double( ncid, ncp->id_lat, &d );
    aa = (char *)nc_strerror( ii );
    d = radd->radar_longitude;
    ii = nc_put_var_double( ncid, ncp->id_lon, &d );
    d = radd->radar_altitude * 1000.;
    ii = nc_put_var_double( ncid, ncp->id_alt, &d );
  }
   if (dds->frib) { 
      strncpy (str, dds->frib->file_name, sizeof (dds->frib->file_name));
      ii = nc_put_var_text( ncid, frib_name_id, str );
   }
  f = swib->fixed_angle;
  ii = nc_put_var_float( ncid, ncp->id_fa, &f );
  f = celv->dist_cells[0];
  ii = nc_put_var_float( ncid, ncp->id_g1, &f );
  f = celv->dist_cells[1] - celv->dist_cells[0];
  ii = nc_put_var_float( ncid, ncp->id_gs, &f );
  f = radd->eff_unamb_vel;
  ii = nc_put_var_float( ncid, ncp->id_nyq, &f );
  f = radd->eff_unamb_range;
  ii = nc_put_var_float( ncid, ncp->id_ra, &f );
  f = radd->radar_const;
  ii = nc_put_var_float( ncid, ncp->id_rconst, &f );
  f = (float)nc_prf;
  ii = nc_put_var_float( ncid, ncp->id_prf, &f );
  f = (float)nc_waveln;
  ii = nc_put_var_float( ncid, ncp->id_wvlen, &f );

  f = cfac->azimuth_corr;
  ii = nc_put_var_float( ncid, ncp->id_azc, &f );
  f = cfac->elevation_corr;
  ii = nc_put_var_float( ncid, ncp->id_elc, &f );
  f = cfac->range_delay_corr;
  ii = nc_put_var_float( ncid, ncp->id_rngc, &f );
  f = cfac->longitude_corr;
  ii = nc_put_var_float( ncid, ncp->id_lonc, &f );
  f = cfac->latitude_corr;
  ii = nc_put_var_float( ncid, ncp->id_latc, &f );
  f = cfac->pressure_alt_corr;
  ii = nc_put_var_float( ncid, ncp->id_paltc, &f );
  f = cfac->radar_alt_corr;
  ii = nc_put_var_float( ncid, ncp->id_raltc, &f );
  f = cfac->ew_gndspd_corr;
  ii = nc_put_var_float( ncid, ncp->id_ewc, &f );
  f = cfac->ns_gndspd_corr;
  ii = nc_put_var_float( ncid, ncp->id_nsc, &f );
  f = cfac->vert_vel_corr;
  ii = nc_put_var_float( ncid, ncp->id_vrtc, &f );
  f = cfac->heading_corr;
  ii = nc_put_var_float( ncid, ncp->id_hdgc, &f );
  f = cfac->roll_corr;
  ii = nc_put_var_float( ncid, ncp->id_rollc, &f );
  f = cfac->pitch_corr;
  ii = nc_put_var_float( ncid, ncp->id_pitchc, &f );
  f = cfac->drift_corr;
  ii = nc_put_var_float( ncid, ncp->id_driftc, &f );
  f = cfac->rot_angle_corr;
  ii = nc_put_var_float( ncid, ncp->id_rotc, &f );
  f = cfac->tilt_corr;
  ii = nc_put_var_float( ncid, ncp->id_tiltc, &f );

  ncp->base_time = (time_t)dgi->time;
  ii = nc_put_var_int( ncid, ncp->id_ut, (int *)&ncp->base_time );
  ii = nc_put_var_int( ncid, ncp->id_vst, (int *)&ncp->vol_start_time );

  start[0] = 0;
  count[0] = 1;


  f = radd->peak_power;
  ii = nc_put_vara_float( ncid, ncp->id_pkpwr, start, count, &f );

  f = dgi->dds->parm[0]->recvr_bandwidth * 1.e6;
  if( f < 0 ) { f = 0; }
  ii = nc_put_vara_float( ncid, ncp->id_bndw, start, count, &f );

  f = dgi->dds->parm[0]->pulse_width/(.5*SPEED_OF_LIGHT);
				/*
  // from m. to seconds
				 */

  ii = nc_put_vara_float( ncid, ncp->id_plsw, start, count, &f );

  if (xstf && xstf->source_format == APIRAQ_FMT ) {
    prqxs = (PIRAQX *)xstf_first_item; /* if we need char data */

    if (xstf->one != 1) {	/* data are swapped */
      if (!prqx)
	{ prqx = (PIRAQX *)malloc (sizeof (PIRAQX)); }
      swack_long (xstf_first_item, (char *)prqx
		  , sizeof (PIRAQX)/sizeof (int4));
    }
    else {
      prqx = (PIRAQX *)xstf_first_item;
    }

    count[0] = 1;
# ifdef obsolete
    f_vals[0] = prqx->h_rconst;
    f_vals[1] = prqx->v_rconst;
# endif
    
    ii = nc_put_vara_float( ncid, ncp->id_rconst, start, count, f_vals );
    
    f_vals[0] = prqx->receiver_gain;
# ifdef obsolete
    f_vals[1] = prqx->vreceiver_gain;
# endif
    ii = nc_put_vara_float( ncid, ncp->id_rcvrg, start, count, f_vals );
    
    f_vals[0] = prqx->antenna_gain;
# ifdef obsolete
    f_vals[1] = prqx->vantenna_gain;
# endif
    ii = nc_put_vara_float( ncid, ncp->id_antnag, start, count, f_vals );
    
    f_vals[0] = prqx->noise_power;
# ifdef obsolete
    f_vals[1] = prqx->vnoise_power;
# endif
    ii = nc_put_vara_float( ncid, ncp->id_noispwr, start, count, f_vals );
    
    f_vals[0] = prqx->test_pulse_pwr;
# ifdef obsolete
    f_vals[1] = prqx->vtest_pulse_pwr;
# endif
    ii = nc_put_vara_float( ncid, ncp->id_tppwr, start, count, f_vals );
    
# ifdef obsolete
    f_vals[0] = prqx->hxmit_power;
    f_vals[1] = prqx->vxmit_power;
# endif
    ii = nc_put_vara_float( ncid, ncp->id_xmtpwr, start, count, f_vals );
    
    count[0] = 1;
    ii = nc_put_vara_float( ncid, ncp->id_tprng0, start, count
			    , ncs->tst_pls_ranges );
    ii = nc_put_vara_float( ncid, ncp->id_tprng1, start, count
			    , ncs->tst_pls_ranges +1 );
  }
    else if( dgi->source_fmt == APIRAQ_FMT ) {
      rhdr = (RADARV *)dgi->gpptr1;
      dwel = (HEADERV *)dgi->gpptr2;
      r_consts = (struct radar_consts *)dgi->gpptr3;

      count[0] = 2;
      f_vals[0] = r_consts->h_rconst;
      f_vals[1] = r_consts->v_rconst;

      ii = nc_put_vara_float( ncid, ncp->id_rconst, start, count, f_vals );

      f_vals[0] = rhdr->receiver_gain;
      f_vals[1] = rhdr->vreceiver_gain;
      ii = nc_put_vara_float( ncid, ncp->id_rcvrg, start, count, f_vals );

      f_vals[0] = rhdr->antenna_gain;
      f_vals[1] = rhdr->vantenna_gain;
      ii = nc_put_vara_float( ncid, ncp->id_antnag, start, count, f_vals );

      f_vals[0] = rhdr->noise_power;
      f_vals[1] = rhdr->vnoise_power;
      ii = nc_put_vara_float( ncid, ncp->id_noispwr, start, count, f_vals );

      f_vals[0] = rhdr->test_pulse_pwr;
      f_vals[1] = rhdr->vtest_pulse_pwr;
      ii = nc_put_vara_float( ncid, ncp->id_tppwr, start, count, f_vals );
 
      f_vals[0] = dwel->hxmit_power;
      f_vals[1] = dwel->vxmit_power;
      ii = nc_put_vara_float( ncid, ncp->id_xmtpwr, start, count, f_vals );
 
      count[0] = 1;
      ii = nc_put_vara_float( ncid, ncp->id_tprng0, start, count
			      , ncs->tst_pls_ranges );
      ii = nc_put_vara_float( ncid, ncp->id_tprng1, start, count
			      , ncs->tst_pls_ranges +1 );
  }
  else {
    f = radd->receiver_gain;
    ii = nc_put_vara_float( ncid, ncp->id_rcvrg, start, count, &f );
    f = radd->antenna_gain;
    ii = nc_put_vara_float( ncid, ncp->id_antnag, start, count, &f );
    f = radd->system_gain;
    ii = nc_put_vara_float( ncid, ncp->id_sysg, start, count, &f );
    f = radd->noise_power;
    ii = nc_put_vara_float( ncid, ncp->id_noispwr, start, count, &f );

  }
  count[0] = 2;
  f_vals[0] = radd->horz_beam_width;
  f_vals[1] = radd->vert_beam_width;
  ii = nc_put_vara_float( ncid, ncp->id_bmwdt, start, count, f_vals );
  count[0] = 1;
  
  memset( str, ' ', sizeof(str));

  for( fn = 0; fn < num_vars; fn++ ) {
    aa = nc_var_name[fn];
    bb = str + fn * short_string;
    strncpy( bb, aa, strlen( aa ));
  }
  ii = nc_put_var_text( ncid, fld_names_id, str );
				/*
  ii = nc_sync( ncid );
				 */

}
/* c------------------------------------------------------------------------ */

static int 
dd_setFxdR1GsEtc (struct ncdf_production *ncp, int num_dims, int *dims)
{
  char str[256], *aa, *bb, *cc;
  int ii, jj, mark, nn;
  float f, f_vals[8];
  double d, d_vals[8];
  int ncid = ncp->ncid;


  ii = nc_def_var( ncid, "Fixed_Angle", NC_FLOAT, 0, dims, &ncp->id_fa );
  strcpy( str, "Targeted fixed angle for this scan" );
  ii = nc_put_att_text( ncid, ncp->id_fa
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_fa
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_fa
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_fa
			, "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "Range_to_First_Cell", NC_FLOAT, 0, dims, &ncp->id_g1 );
  strcpy( str, "Range to the center of the first cell" );
  ii = nc_put_att_text( ncid, ncp->id_g1
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters" );
  ii = nc_put_att_text( ncid, ncp->id_g1
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_g1
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_g1
			, "_FillValue", NC_FLOAT, 1, &f );

  ii = nc_def_var( ncid, "Cell_Spacing", NC_FLOAT, 0, dims, &ncp->id_gs );
  strcpy( str, "Distance between cells" );
  ii = nc_put_att_text( ncid, ncp->id_gs
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters" );
  ii = nc_put_att_text( ncid, ncp->id_gs
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_gs
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_gs
			, "_FillValue", NC_FLOAT, 1, &f );

  ii = nc_def_var( ncid, "Nyquist_Velocity", NC_FLOAT, 0, dims, &ncp->id_nyq );
  strcpy( str, "Effective unambigous velocity" );
  ii = nc_put_att_text( ncid, ncp->id_nyq
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters/second" );
  ii = nc_put_att_text( ncid, ncp->id_nyq
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_nyq
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_nyq
			, "_FillValue", NC_FLOAT, 1, &f );

  ii = nc_def_var( ncid, "Unambiguous_Range", NC_FLOAT, 0, dims, &ncp->id_ra );
  strcpy( str, "Effective unambigous range" );
  ii = nc_put_att_text( ncid, ncp->id_ra
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters" );
  ii = nc_put_att_text( ncid, ncp->id_ra
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_ra
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_ra
			, "_FillValue", NC_FLOAT, 1, &f );
  return ii;
}

/* c------------------------------------------------------------------------ */

static int 
dd_setRcvrXmtrStuff (struct ncdf_production *ncp, int num_dims, int *dims)
{
  char str[256], *aa, *bb, *cc;
  int ii, jj, mark, nn;
  float f, f_vals[8];
  double d, d_vals[8];
  int ncid = ncp->ncid;


  ii = nc_def_var( ncid, "Radar_Constant", NC_FLOAT, num_dims, dims
		   , &ncp->id_rconst );
  strcpy( str, "Radar constant" );
  ii = nc_put_att_text( ncid, ncp->id_rconst
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "???" );
  ii = nc_put_att_text( ncid, ncp->id_rconst
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_rconst
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_rconst
			, "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "rcvr_gain", NC_FLOAT, num_dims, dims, &ncp->id_rcvrg );
  
  strcpy( str, "Receiver Gain" );
  ii = nc_put_att_text( ncid, ncp->id_rcvrg
			, "long_name", strlen( str ) +1, str );
  aa = (char *)"Most entries are 2 dimension arrays one for each polarity";
  strcpy( str, aa );
  ii = nc_put_att_text( ncid, ncp->id_rcvrg
			, "Comment", strlen( str ) +1, str );
  strcpy( str, "db" );
  ii = nc_put_att_text( ncid, ncp->id_rcvrg
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_rcvrg
			 , "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_rcvrg
			  , "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "ant_gain", NC_FLOAT, num_dims, dims, &ncp->id_antnag );
  
  strcpy( str, "Antenna Gain" );
  ii = nc_put_att_text( ncid, ncp->id_antnag
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "db" );
  ii = nc_put_att_text( ncid, ncp->id_antnag
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_antnag
			 , "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_antnag
			  , "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "sys_gain", NC_FLOAT, num_dims, dims, &ncp->id_sysg );
  
  strcpy( str, "System Gain" );
  ii = nc_put_att_text( ncid, ncp->id_sysg
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "db" );
  ii = nc_put_att_text( ncid, ncp->id_sysg
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_sysg
			 , "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_sysg
			  , "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "bm_width", NC_FLOAT, num_dims, dims, &ncp->id_bmwdt );
  
  strcpy( str, "Beam Width" );
  ii = nc_put_att_text( ncid, ncp->id_bmwdt
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_bmwdt
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_bmwdt
			 , "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_bmwdt
			  , "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "pulse_width", NC_FLOAT, num_dims, dims, &ncp->id_plsw );
  
  strcpy( str, "Pulse Width" );
  ii = nc_put_att_text( ncid, ncp->id_plsw
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "seconds" );
  ii = nc_put_att_text( ncid, ncp->id_plsw
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_plsw
			 , "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_plsw
			  , "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "band_width", NC_FLOAT, num_dims, dims, &ncp->id_bndw );
  
  strcpy( str, "Band Width" );
  ii = nc_put_att_text( ncid, ncp->id_bndw
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "hertz" );
  ii = nc_put_att_text( ncid, ncp->id_bndw
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_bndw
			 , "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_bndw
			  , "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "peak_pwr", NC_FLOAT, num_dims, dims, &ncp->id_pkpwr );
  
  strcpy( str, "Peak Power" );
  ii = nc_put_att_text( ncid, ncp->id_pkpwr
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "watts" );
  ii = nc_put_att_text( ncid, ncp->id_pkpwr
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_pkpwr
			 , "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_pkpwr
			  , "_FillValue", NC_FLOAT, 1, &f );
				/*
				 */


  ii = nc_def_var( ncid, "xmtr_pwr", NC_FLOAT, num_dims, dims, &ncp->id_xmtpwr );
  
  strcpy( str, "Transmitter Power" );
  ii = nc_put_att_text( ncid, ncp->id_xmtpwr
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "dBm" );
  ii = nc_put_att_text( ncid, ncp->id_xmtpwr
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_xmtpwr
			 , "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_xmtpwr
			  , "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "noise_pwr", NC_FLOAT, num_dims, dims, &ncp->id_noispwr );
  
  strcpy( str, "Noise Power" );
  ii = nc_put_att_text( ncid, ncp->id_noispwr
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "dBm" );
  ii = nc_put_att_text( ncid, ncp->id_noispwr
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_noispwr
			 , "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_noispwr
			  , "_FillValue", NC_FLOAT, 1, &f );
				/*
				 */


  ii = nc_def_var( ncid, "tst_pls_pwr", NC_FLOAT, num_dims, dims, &ncp->id_tppwr );
  
  strcpy( str, "Test Pulse Power" );
  ii = nc_put_att_text( ncid, ncp->id_tppwr
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "dBm" );
  ii = nc_put_att_text( ncid, ncp->id_tppwr
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_tppwr
			 , "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_tppwr
			  , "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "tst_pls_rng0", NC_FLOAT, num_dims, dims
		   , &ncp->id_tprng0 );
  strcpy( str, "Range to start of test pulse" );
  ii = nc_put_att_text( ncid, ncp->id_tprng0
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters" );
  ii = nc_put_att_text( ncid, ncp->id_tprng0
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_tprng0
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_tprng0
			, "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "tst_pls_rng1", NC_FLOAT, num_dims, dims
		   , &ncp->id_tprng1 );
  strcpy( str, "Range to end of test pulse" );
  ii = nc_put_att_text( ncid, ncp->id_tprng1
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters" );
  ii = nc_put_att_text( ncid, ncp->id_tprng1
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_tprng1
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_tprng1
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "Wavelength", NC_FLOAT, num_dims, dims
		   , &ncp->id_wvlen );
  strcpy( str, "System wavelength" );
  ii = nc_put_att_text( ncid, ncp->id_wvlen
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters" );
  ii = nc_put_att_text( ncid, ncp->id_wvlen
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_wvlen
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_wvlen
			, "_FillValue", NC_FLOAT, 1, &f );




  ii = nc_def_var( ncid, "PRF", NC_FLOAT, num_dims, dims
		   , &ncp->id_prf );
  strcpy( str, "System pulse repetition frequence" );
  ii = nc_put_att_text( ncid, ncp->id_prf
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "pulses/sec" );
  ii = nc_put_att_text( ncid, ncp->id_prf
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_prf
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_prf
			, "_FillValue", NC_FLOAT, 1, &f );

}

/* c------------------------------------------------------------------------ */

static int 
dd_setLatLonAlt (struct ncdf_production *ncp, int num_dims, int *dims)

{
  char str[256], *aa, *bb, *cc;
  int ii, jj, mark, nn;
  float f, f_vals[8];
  double d, d_vals[8];
  int ncid = ncp->ncid;

  ii = nc_def_var( ncid, "Latitude", NC_DOUBLE, num_dims, dims, &ncp->id_lat );
  
  strcpy( str, "Latitude of the instrument" );
  ii = nc_put_att_text( ncid, ncp->id_lat
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees_north" );
  ii = nc_put_att_text( ncid, ncp->id_lat
			, "units", strlen( str ) +1, str );
  f_vals[0] = -90.;
  f_vals[1] =  90.;
  ii = nc_put_att_float( ncid, ncp->id_lat
			 , "valid_range", NC_FLOAT, 2, f_vals );
  d = notSet;
  ii = nc_put_att_double( ncid, ncp->id_lat
			 , "missing_value", NC_DOUBLE, 1, &d );
  ii = nc_put_att_double( ncid, ncp->id_lat
			  , "_FillValue", NC_DOUBLE, 1, &d );

  
  ii = nc_def_var( ncid, "Longitude", NC_DOUBLE, num_dims, dims, &ncp->id_lon );

  strcpy( str, "Longitude of the instrument" );
  ii = nc_put_att_text( ncid, ncp->id_lon
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees_east" );
  ii = nc_put_att_text( ncid, ncp->id_lon
			, "units", strlen( str ) +1, str );
  f_vals[0] = -360.;
  f_vals[1] =  360.;
  ii = nc_put_att_float( ncid, ncp->id_lon
			 , "valid_range", NC_FLOAT, 2, f_vals );
  
  d = notSet;
  ii = nc_put_att_double( ncid, ncp->id_lon
			 , "missing_value", NC_DOUBLE, 1, &d );
  ii = nc_put_att_double( ncid, ncp->id_lon
			  , "_FillValue", NC_DOUBLE, 1, &d );
 

  ii = nc_def_var( ncid, "Altitude", NC_DOUBLE, num_dims, dims, &ncp->id_alt );

  strcpy( str, "Altitude in meters (asl) of the instrument" );
  ii = nc_put_att_text( ncid, ncp->id_alt
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters" );
  ii = nc_put_att_text( ncid, ncp->id_alt
			, "units", strlen( str ) +1, str );
  f_vals[0] = -10000.;
  f_vals[1] =  90000.;
  ii = nc_put_att_float( ncid, ncp->id_alt
			 , "valid_range", NC_DOUBLE, 2, f_vals );
  
  d = notSet;
  ii = nc_put_att_double( ncid, ncp->id_alt
			 , "missing_value", NC_DOUBLE, 1, &d );
  ii = nc_put_att_double( ncid, ncp->id_alt
			  , "_FillValue", NC_DOUBLE, 1, &d );


  return( ii );
}

/* c------------------------------------------------------------------------ */

static int 
dd_setAzEl (struct ncdf_production *ncp, int num_dims, int *dims)
{
  char str[256], *aa, *bb, *cc;
  int ii, jj, mark, nn, i_vals[8];
  float f, f_vals[8];
  double d, d_vals[8];
  int ncid = ncp->ncid;


  ii = nc_def_var( ncid, "Azimuth", NC_FLOAT, num_dims, dims, &ncp->id_az );

  strcpy( str, "Earth relative azimuth of the ray" );
  ii = nc_put_att_text( ncid, ncp->id_az
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "Degrees clockwise from true North" );
  ii = nc_put_att_text( ncid, ncp->id_az
			, "Comment", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_az
			, "units", strlen( str ) +1, str );
  f_vals[0] = -360.;
  f_vals[1] =  360.;
  ii = nc_put_att_float( ncid, ncp->id_az
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_az
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_az
			, "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "Elevation", NC_FLOAT, num_dims, dims, &ncp->id_el );

  strcpy( str, "Earth relative elevation of the ray" );
  ii = nc_put_att_text( ncid, ncp->id_el
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "Degrees from earth tangent towards zenith" );
  ii = nc_put_att_text( ncid, ncp->id_el
			, "Comment", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_el
			, "units", strlen( str ) +1, str );
  f_vals[0] = -360.;
  f_vals[1] =  360.;
  ii = nc_put_att_float( ncid, ncp->id_el
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_el
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_el
			, "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "clip_range", NC_FLOAT
		   , num_dims, dims, &ncp->id_clip );
  strcpy( str, "Range of last usefull cell" );
  ii = nc_put_att_text( ncid, ncp->id_clip
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters" );
  ii = nc_put_att_text( ncid, ncp->id_clip
			, "units", strlen( str ) +1, str );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_clip
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_clip
			, "_FillValue", NC_FLOAT, 1, &f );

  return( ii );
}


/* c------------------------------------------------------------------------ */

static int 
dd_setACparams (struct dd_general_info *dgi, struct ncdf_production *ncp, int num_dims, int *dims)
{
  char str[256], *aa, *bb, *cc;
  int ii, jj, mark, nn;
  float f, f_vals[8];
  double d, d_vals[8];
  int ncid = ncp->ncid;


  ii = nc_def_var( ncid, "Altitude_agl", NC_DOUBLE, num_dims, dims, &ncp->id_agl );

  strcpy( str, "Altitude in meters above ground level of the instrument" );
  ii = nc_put_att_text( ncid, ncp->id_agl
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters" );
  ii = nc_put_att_text( ncid, ncp->id_agl
			, "units", strlen( str ) +1, str );
  f_vals[0] = -10000.;
  f_vals[1] =  90000.;
  ii = nc_put_att_float( ncid, ncp->id_agl
			 , "valid_range", NC_FLOAT, 2, f_vals );
  
  d = notSet;
  ii = nc_put_att_double( ncid, ncp->id_agl
			 , "missing_value", NC_FLOAT, 1, &d );
  ii = nc_put_att_double( ncid, ncp->id_agl
			  , "_FillValue", NC_DOUBLE, 1, &d );



  ii = nc_def_var( ncid, "Rotation_Angle", NC_FLOAT, num_dims, dims
		   , &ncp->id_rotang );

  strcpy( str, "Rotation Angle of ray relative to platform" );
  ii = nc_put_att_text( ncid, ncp->id_rotang
			, "long_name", strlen( str ) +1, str );
  if( dgi->dds->radd->scan_mode == AIR ) {
				/*
				  // tail
				 */
    aa = (char *)"Degrees clockwise from zenith ";
  }
  else {
				/*
    // nose or lower fuselage
				 */
    aa = (char *)"Degrees clockwise from forward";
  }
  strcpy( str, aa );
  ii = nc_put_att_text( ncid, ncp->id_rotang
			, "Comment", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_rotang
			, "units", strlen( str ) +1, str );
  f_vals[0] = -360.;
  f_vals[1] =  360.;
  ii = nc_put_att_float( ncid, ncp->id_rotang
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_rotang
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_rotang
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "Tilt", NC_FLOAT, num_dims, dims, &ncp->id_tilt );

  strcpy( str, "Tilt of ray relative to rotational axis" );
  ii = nc_put_att_text( ncid, ncp->id_tilt
			, "long_name", strlen( str ) +1, str );
  if( dgi->dds->radd->scan_mode == AIR ) {
				/*
    // tail
				 */
    aa = (char *)"Degrees from starboard perpendicular towards forward";
  }
  else {
				/*
    // nose or lower fuselage
				 */
    aa = (char *)"Clockwise angle from platform horizontal towards the ray ";
  }
  strcpy( str, aa );
  ii = nc_put_att_text( ncid, ncp->id_tilt
			, "Comment", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_tilt
			, "units", strlen( str ) +1, str );
  f_vals[0] = -360.;
  f_vals[1] =  360.;
  ii = nc_put_att_float( ncid, ncp->id_tilt
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_tilt
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_tilt
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "Heading", NC_FLOAT, num_dims, dims, &ncp->id_hdg );

  strcpy( str,
        "Azimuth of forward vector in degrees clockwise from North" );
  ii = nc_put_att_text( ncid, ncp->id_hdg
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_hdg
			, "units", strlen( str ) +1, str );
  f_vals[0] = -360.;
  f_vals[1] =  360.;
  ii = nc_put_att_float( ncid, ncp->id_hdg
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_hdg
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_hdg
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "Roll", NC_FLOAT, num_dims, dims, &ncp->id_roll );

  strcpy( str, "Roll angle" );
  ii = nc_put_att_text( ncid, ncp->id_roll
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "Angle from earth tangent to starboard vector" );
  ii = nc_put_att_text( ncid, ncp->id_roll
			, "Comment", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_roll
			, "units", strlen( str ) +1, str );
  f_vals[0] = -360.;
  f_vals[1] =  360.;
  ii = nc_put_att_float( ncid, ncp->id_roll
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_roll
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_roll
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "Pitch", NC_FLOAT, num_dims, dims, &ncp->id_pitch );

  strcpy( str, "Pitch angle" );
  ii = nc_put_att_text( ncid, ncp->id_pitch
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "Angle from earth tangent to forward vector" );
  ii = nc_put_att_text( ncid, ncp->id_pitch
			, "Comment", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_pitch
			, "units", strlen( str ) +1, str );
  f_vals[0] = -360.;
  f_vals[1] =  360.;
  ii = nc_put_att_float( ncid, ncp->id_pitch
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_pitch
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_pitch
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "Drift", NC_FLOAT, num_dims, dims, &ncp->id_drift );

  strcpy( str,
        "Angle between Heading and platform true velocity vector" );
  ii = nc_put_att_text( ncid, ncp->id_drift
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "Degrees clockwise from Heading" );
  ii = nc_put_att_text( ncid, ncp->id_drift
			, "Comment", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_drift
			, "units", strlen( str ) +1, str );
  f_vals[0] = -360.;
  f_vals[1] =  360.;
  ii = nc_put_att_float( ncid, ncp->id_drift
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_drift
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_drift
			, "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "NS_Velocity", NC_FLOAT, num_dims, dims, &ncp->id_nsv );

  strcpy( str, "North component of the velocity of the platform" );
  ii = nc_put_att_text( ncid, ncp->id_nsv
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters/second" );
  ii = nc_put_att_text( ncid, ncp->id_nsv
			, "units", strlen( str ) +1, str );
  f_vals[0] = -777.;
  f_vals[1] = 777.;
  ii = nc_put_att_float( ncid, ncp->id_nsv
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_nsv
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_nsv
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "EW_Velocity", NC_FLOAT, num_dims, dims, &ncp->id_ewv );

  strcpy( str, "East component of the velocity of the platform" );
  ii = nc_put_att_text( ncid, ncp->id_ewv
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters/second" );
  ii = nc_put_att_text( ncid, ncp->id_ewv
			, "units", strlen( str ) +1, str );
  f_vals[0] = -777.;
  f_vals[1] = 777.;
  ii = nc_put_att_float( ncid, ncp->id_ewv
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_ewv
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_ewv
			, "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "Vertical_Speed", NC_FLOAT, num_dims, dims
		   , &ncp->id_vspd );

  strcpy( str, "Vertical speed of the platform" );
  ii = nc_put_att_text( ncid, ncp->id_vspd
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters/second" );
  ii = nc_put_att_text( ncid, ncp->id_vspd
			, "units", strlen( str ) +1, str );
  f_vals[0] = -777.;
  f_vals[1] = 777.;
  ii = nc_put_att_float( ncid, ncp->id_vspd
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_vspd
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_vspd
			, "_FillValue", NC_FLOAT, 1, &f );


  ii = nc_def_var( ncid, "NS_Wind", NC_FLOAT, num_dims, dims, &ncp->id_nsw );

  strcpy( str, "North horizontal wind component at the platform" );
  ii = nc_put_att_text( ncid, ncp->id_nsw
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters/second" );
  ii = nc_put_att_text( ncid, ncp->id_nsw
			, "units", strlen( str ) +1, str );
  f_vals[0] = -777.;
  f_vals[1] = 777.;
  ii = nc_put_att_float( ncid, ncp->id_nsw
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_nsw
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_nsw
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "EW_Wind", NC_FLOAT, num_dims, dims, &ncp->id_eww );

  strcpy( str, "East horizontal wind component at the platform" );
  ii = nc_put_att_text( ncid, ncp->id_eww
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters/second" );
  ii = nc_put_att_text( ncid, ncp->id_eww
			, "units", strlen( str ) +1, str );
  f_vals[0] = -777.;
  f_vals[1] = 777.;
  ii = nc_put_att_float( ncid, ncp->id_eww
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_eww
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_eww
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "Vertical_Wind_Speed", NC_FLOAT, num_dims, dims
		   , &ncp->id_vwspd );

  strcpy( str, "Vertical wind speed at the platform" );
  ii = nc_put_att_text( ncid, ncp->id_vwspd
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters/second" );
  ii = nc_put_att_text( ncid, ncp->id_vwspd
			, "units", strlen( str ) +1, str );
  f_vals[0] = -777.;
  f_vals[1] = 777.;
  ii = nc_put_att_float( ncid, ncp->id_vwspd
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_vwspd
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_vwspd
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "AC_Vel_Component", NC_FLOAT, num_dims, dims
		   , &ncp->id_acvc );

  strcpy( str, "Velocity correction for aircraft motion" );
  ii = nc_put_att_text( ncid, ncp->id_acvc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "Add to velocity to remove aircraft motion" );
  ii = nc_put_att_text( ncid, ncp->id_acvc
			, "Comment", strlen( str ) +1, str );
  strcpy( str, "meters/second" );
  ii = nc_put_att_text( ncid, ncp->id_acvc
			, "units", strlen( str ) +1, str );
  f_vals[0] = -777.;
  f_vals[1] = 777.;
  ii = nc_put_att_float( ncid, ncp->id_acvc
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_acvc
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_acvc
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "Heading_Change", NC_FLOAT, num_dims, dims
		   , &ncp->id_hchng );

  strcpy( str, "Angular rate of change of the Heading" );
  ii = nc_put_att_text( ncid, ncp->id_hchng
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees/second" );
  ii = nc_put_att_text( ncid, ncp->id_hchng
			, "units", strlen( str ) +1, str );
  f_vals[0] = -360.;
  f_vals[1] = 360.;
  ii = nc_put_att_float( ncid, ncp->id_hchng
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_hchng
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_hchng
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "Pitch_Change", NC_FLOAT, num_dims, dims
		   , &ncp->id_pchng );

  strcpy( str, "Angular rate of change of the Pitch" );
  ii = nc_put_att_text( ncid, ncp->id_pchng
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees/second" );
  ii = nc_put_att_text( ncid, ncp->id_pchng
			, "units", strlen( str ) +1, str );
  f_vals[0] = -360.;
  f_vals[1] = 360.;
  ii = nc_put_att_float( ncid, ncp->id_pchng
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_pchng
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_pchng
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "True_Scan_Rate", NC_FLOAT, num_dims, dims
		   , &ncp->id_tsrt );

  strcpy( str, "Angular rate of change of the Pitch" );
  ii = nc_put_att_text( ncid, ncp->id_tsrt
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees/second" );
  ii = nc_put_att_text( ncid, ncp->id_tsrt
			, "units", strlen( str ) +1, str );
  f_vals[0] = -360.;
  f_vals[1] = 360.;
  ii = nc_put_att_float( ncid, ncp->id_tsrt
			, "valid_range", NC_FLOAT, 2, f_vals );
  f = notSet;
  ii = nc_put_att_float( ncid, ncp->id_tsrt
			, "missing_value", NC_FLOAT, 1, &f );
  ii = nc_put_att_float( ncid, ncp->id_tsrt
			, "_FillValue", NC_FLOAT, 1, &f );



  ii = nc_def_var( ncid, "azimuth_corr", NC_FLOAT, 0, dims
		   , &ncp->id_azc );

  strcpy( str, "Additive azimuth correction" );
  ii = nc_put_att_text( ncid, ncp->id_azc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_azc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "elevation_corr", NC_FLOAT, 0, dims
		   , &ncp->id_elc );

  strcpy( str, "Additive elevation correction" );
  ii = nc_put_att_text( ncid, ncp->id_elc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_elc
			, "units", strlen( str ) +1, str );



  ii = nc_def_var( ncid, "range_delay_corr", NC_FLOAT, 0, dims
		   , &ncp->id_rngc );

  strcpy( str, "Additive range correction" );
  ii = nc_put_att_text( ncid, ncp->id_rngc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters" );
  ii = nc_put_att_text( ncid, ncp->id_rngc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "longitude_corr", NC_FLOAT, 0, dims
		   , &ncp->id_lonc );

  strcpy( str, "Additive longitude correction" );
  ii = nc_put_att_text( ncid, ncp->id_lonc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_lonc
			, "units", strlen( str ) +1, str );



  ii = nc_def_var( ncid, "latitude_corr", NC_FLOAT, 0, dims
		   , &ncp->id_latc );

  strcpy( str, "Additive latitude correction" );
  ii = nc_put_att_text( ncid, ncp->id_latc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_latc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "pressure_alt_corr", NC_FLOAT, 0, dims
		   , &ncp->id_paltc );

  strcpy( str, "Additive pressure altitude correction" );
  ii = nc_put_att_text( ncid, ncp->id_paltc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters" );
  ii = nc_put_att_text( ncid, ncp->id_paltc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "radar_alt_corr", NC_FLOAT, 0, dims
		   , &ncp->id_raltc );

  strcpy( str, "Additive radar altitude correction" );
  ii = nc_put_att_text( ncid, ncp->id_raltc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters" );
  ii = nc_put_att_text( ncid, ncp->id_raltc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "ew_gndspd_corr", NC_FLOAT, 0, dims
		   , &ncp->id_ewc );

  strcpy( str, "Additive east-west ground speed correction" );
  ii = nc_put_att_text( ncid, ncp->id_ewc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters/second" );
  ii = nc_put_att_text( ncid, ncp->id_ewc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "ns_gndspd_corr", NC_FLOAT, 0, dims
		   , &ncp->id_nsc );

  strcpy( str, "Additive north-south ground speed correction" );
  ii = nc_put_att_text( ncid, ncp->id_nsc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters/second" );
  ii = nc_put_att_text( ncid, ncp->id_nsc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "vert_vel_corr", NC_FLOAT, 0, dims
		   , &ncp->id_vrtc );

  strcpy( str, "Additive vertical velocity correction" );
  ii = nc_put_att_text( ncid, ncp->id_vrtc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "meters/second" );
  ii = nc_put_att_text( ncid, ncp->id_vrtc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "heading_corr", NC_FLOAT, 0, dims
		   , &ncp->id_hdgc );

  strcpy( str, "Additive heading correction" );
  ii = nc_put_att_text( ncid, ncp->id_hdgc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_hdgc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "roll_corr", NC_FLOAT, 0, dims
		   , &ncp->id_rollc );

  strcpy( str, "Additive roll correction" );
  ii = nc_put_att_text( ncid, ncp->id_rollc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_rollc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "pitch_corr", NC_FLOAT, 0, dims
		   , &ncp->id_pitchc );

  strcpy( str, "Additive pitch correction" );
  ii = nc_put_att_text( ncid, ncp->id_pitchc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_pitchc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "drift_corr", NC_FLOAT, 0, dims
		   , &ncp->id_driftc );

  strcpy( str, "Additive drift correction" );
  ii = nc_put_att_text( ncid, ncp->id_driftc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_driftc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "rot_angle_corr", NC_FLOAT, 0, dims
		   , &ncp->id_rotc );

  strcpy( str, "Additive rotation angle correction" );
  ii = nc_put_att_text( ncid, ncp->id_rotc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_rotc
			, "units", strlen( str ) +1, str );


  ii = nc_def_var( ncid, "tilt_corr", NC_FLOAT, 0, dims
		   , &ncp->id_tiltc );

  strcpy( str, "Additive tilt correction" );
  ii = nc_put_att_text( ncid, ncp->id_tiltc
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "degrees" );
  ii = nc_put_att_text( ncid, ncp->id_tiltc
			, "units", strlen( str ) +1, str );
# ifdef obsolete


  ii = nc_def_var( ncid, "", NC_FLOAT, 0, dims
		   , &ncp->id_ );

  strcpy( str, "Additive correction" );
  ii = nc_put_att_text( ncid, ncp->id_
			, "long_name", strlen( str ) +1, str );
  strcpy( str, "" );
  ii = nc_put_att_text( ncid, ncp->id_
			, "units", strlen( str ) +1, str );
# endif

  return( ii );
}

/* c------------------------------------------------------------------------ */

void 
ncp_stuff_ray (struct dd_general_info *dgi, struct ncdf_production *ncp)
{
  struct dds_structs *dds=dgi->dds;
  struct radar_d *radd=dds->radd;
  struct cell_d *celv=dds->celvc;
  struct sweepinfo_d *swib=dds->swib;
  struct ray_i *ryib=dds->ryib;
  struct platform_i *asib=dds->asib;
  struct correction_d *cfac=dds->cfac;

  int ii, jj, kk, ll, mm, nn, mark, ndx, fld_ndx;
  int rndx;
  size_t start[4], count[4];
  float f;
  double d;
  time_t tt;
  short *ss;
  int nvarid;
  char *aa, str[256];
  nc_type nct;
  float bv;
  float f_az = (float)dd_azimuth_angle(dgi);
  float f_el = (float)dd_elevation_angle(dgi);
  int ncid;


  if( dgi->new_sweep ) {
    ncdf_finish( ncp );
    ncdf_init_sweep( dgi, ncp );
  }
  rndx = ncp->ray_count++;

  ncid = ncp->ncid;
  ii = nc_inq( ncid, &jj, &nvarid, &ll, &mm );

  start[0] = rndx;
  start[1] = 0;
  count[0] = 1;
  count[1] = ncp->num_cells;

  d = dgi->time - ncp->base_time;
  ii = nc_put_vara_double( ncid, ncp->id_tmoffs, start, count, &d );

				/*
  // azimuth and elevation
				 */
  ii = nc_put_vara_float( ncid, ncp->id_az, start, count, &f_az );
  ii = nc_put_vara_float( ncid, ncp->id_el, start, count, &f_el );

  f = dgi->clip_gate * (celv->dist_cells[1] - celv->dist_cells[0])
    -celv->dist_cells[0];
  ii = nc_put_vara_float( ncid, ncp->id_clip, start, count, &f );
  

  if( radd->radar_type != GROUND ) {
				/*  */
    d = asib->latitude;
    ii = nc_put_vara_double( ncid, ncp->id_lat, start, count
			    , &d );
    d = asib->longitude;
    ii = nc_put_vara_double( ncid, ncp->id_lon, start, count
			    , &d );
    d = asib->altitude_msl * 1000.;
    ii = nc_put_vara_double( ncid, ncp->id_alt, start, count
			    , &d );
    d = asib->altitude_agl * 1000.;
    ii = nc_put_vara_double( ncid, ncp->id_agl, start, count
			    , &d );
    f = (float)dd_rotation_angle(dgi);
    f = asib->rotation_angle;
    ii = nc_put_vara_float( ncid, ncp->id_rotang, start, count, &f );
    f = asib->tilt;
    ii = nc_put_vara_float( ncid, ncp->id_tilt, start, count, &f );
    f = asib->heading;
    ii = nc_put_vara_float( ncid, ncp->id_hdg, start, count, &f );
    f = asib->roll;
    ii = nc_put_vara_float( ncid, ncp->id_roll, start, count, &f );
    f = asib->pitch;
    ii = nc_put_vara_float( ncid, ncp->id_pitch, start, count, &f );
    f = asib->drift_angle;
    ii = nc_put_vara_float( ncid, ncp->id_drift, start, count, &f );

    ii = nc_put_vara_float( ncid, ncp->id_ewv, start, count
			    , &asib->ew_velocity );
    ii = nc_put_vara_float( ncid, ncp->id_nsv, start, count
			    , &asib->ns_velocity );
    ii = nc_put_vara_float( ncid, ncp->id_vspd, start, count
			    , &asib->vert_velocity );
    ii = nc_put_vara_float( ncid, ncp->id_nsw, start, count
			    , &asib->ns_horiz_wind );
    ii = nc_put_vara_float( ncid, ncp->id_eww, start, count
			    , &asib->ew_horiz_wind );
    ii = nc_put_vara_float( ncid, ncp->id_vwspd, start, count
			    , &asib->vert_wind );

    ii = nc_put_vara_float( ncid, ncp->id_hchng, start, count
			    , &asib->heading_change );
    ii = nc_put_vara_float( ncid, ncp->id_pchng, start, count
			    , &asib->pitch_change );
    ii = nc_put_vara_float( ncid, ncp->id_tsrt, start, count
			    , &ryib->true_scan_rate );

    f = (float)dd_ac_vel( dgi );
    ii = nc_put_vara_float( ncid, ncp->id_acvc, start, count, &f );

  }
				/*
  // end platform dependent stuff
				 */

  


  /* do the data fields */

  for( ndx = 0; ndx < num_vars; ndx++ ) {
    fld_ndx = dd_indices[ndx];
    ss = (short *)dds->qdat_ptrs[fld_ndx];
    ii = nc_put_vara_short( ncid, nc_var_id[ndx], start, count, ss );
  }
				/*
  ii = nc_sync( ncid );
				 */
}
/* c------------------------------------------------------------------------ */


/* c------------------------------------------------------------------------ */

# endif
