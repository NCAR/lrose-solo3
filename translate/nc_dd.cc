/* 	$Id$	 */
 
#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <dd_defines.h>
#include <dorade_share.h>
#include <CellVector.h>   /* for MAXGATES */
#include <dd_math.h>
#include "nc_dd.hh"
#include "dorade_share.hh"
#include "input_limits.hh"
#include "dd_stats.h"
#include "generic_radar_info.h"
#include "dda_common.hh"
#include "gneric_dd.hh"
#include "dd_swp_files.hh"
#include "ddb_common.hh"


# define              maxSys 8
# define             namelen 64
# define             maxVars 128
# define             maxSegments 16
# define             maxGates 2048
# define            LAST_RAY -1
# define        READ_FAILURE -2
# define        OPEN_FAILURE -3
# define           REMOVE_LT 1
# define           REMOVE_GT 2
# define    REMOVE_LT_AND_GT 4
# define  PRESERVE_LT_AND_GT 8 
# define       MISSING_VALUE -2.2e-22

# define NC_UNSCALE(x,scale,offset) ((x)*(scale)+(offset))

# ifdef NETCDF
# include <netcdf.h>
static nc_type nc_types[maxVars]; 
# endif

static struct solo_list_mgmt *slm = 0;
static struct solo_list_mgmt *slm2 = 0;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats=NULL;
static struct generic_radar_info *gri;

static char nc_dir[256];

static int ncid;

static int id_lat;
static int id_lon;
static int id_alt;
static int id_agl;
static int id_az;
static int id_el;
static int id_g1;
static int id_gs;
static int id_nyq;
static int id_rconst;
static int id_ra;
static int id_fa;
static int id_ut;
static int id_tmoffs;
static int id_rotang;
static int id_tilt;
static int id_hdg;
static int id_drift;
static int id_roll;
static int id_pitch;
static int id_track;
static int id_nsv;
static int id_ewv;
static int id_nsw;
static int id_eww;
static int id_gspd;
static int id_wspd;
static int id_aspd;
static int id_vspd;
static int id_vwspd;
static int id_wdir;
static int id_hchng;
static int id_rchng;
static int id_pchng;
static int id_acvc;
static int id_rcvrg;
static int id_xmtrg;
static int id_antnag;
static int id_sysg;
static int id_bmwdt;
static int id_plsw;
static int id_bndw;
static int id_pkpwr;
static int id_xmtpwr;
static int id_noispwr;
static int id_tppwr;
static int id_tprng0;
static int id_tprng1;
static int id_prf;
static int id_wvlen;

static int scanMode;
static int maxCells;
static int prevMaxCells;
static int maxRays;
static int numFields;
static int numVars;
static int numDims;
static int numGlobals;
static int nextRayIndex;
static int rayNum;
static int prev_vol_num;
static int vol_num;
static int thrType;
static int thrId;
static int sweepNum;
static int nc_year;
static int nc_month;
static int nc_day;
static int nc_hour;
static int nc_minute;
static int nc_second;
static int nc_millisecond;
static char thrName[24];
static char radarName[24];
static char instrumentType[24];
static char scanModeMne[16];
static char tmp_str[256];
static int archive2 = NO;
static int airborne = NO;

static float missing_value[maxVars];
static int int_missing_value[maxVars];
static int nex_missing_vals[maxVars][2];
static float scale_factor[maxVars]; 
static float add_offset[maxVars];
static int  field_ids[maxVars];

static char * var_names[maxVars];
static char * long_names[maxVars];
static char * units[maxVars];
static int * cell_lut[maxVars];
static int newVol;
static int newSweep;
static int newRay;

static float thrMin;
static float thrMax;
static float * fbuf;
static short * sbuf;

static double base_time;
static double time_offset;
static double nc_azimuth;
static double nc_elevation;
static double nc_fixed_angle;
static double nc_latitude;
static double nc_longitude;
static double nc_altitude_km;

static int r0_gs_present = NO;
static float metersToFirstCell=MISSING_VALUE;
static float metersBetweenCells=0;

static int num_segments[maxVars];
static int cells_in_segment[maxVars][maxSegments];
static float meters_to_first_cell[maxVars][maxSegments];
static float meters_between_cells[maxVars][maxSegments];
static int cell_dim_id[maxVars];

static double eff_unamb_vel;
static double eff_unamb_range;
static double radarConst;

static size_t dimSize[16];
static char dimName[16][16];
static int cellDimFlag[16];
static int numCellDims=0;
static int gsmin_ndx;


/* new stuff!  */

static int numFreqs;
static double freqs[16];
static int numIPPs;
static double ipps[16];
static double PRF=0;
static double waveLength_m = 0;
static float horz_beam_width;
static float vert_beam_width;
static int numSamples;
static float receiver_gain;
static float antenna_gain;
static float system_gain;
static float noise_power;
static float peak_power;
static float recvr_bandwidth;
static float pulse_width;

static double sum_azs;
static double sum_els;
static int options = 0;

# define BISTATIC_FLAG 0x1

/* c------------------------------------------------------------------------ */

void 
nc_dd_conv (int interactive_mode)
{
  /* translate netCDF sweepfiles to DORADE sweepfiles */


  static int count=0;
  int mark, nn, ns, le, rn;
  char *aa, *fn;

  ns = nc_ini();		/* number of sweepfile names now in slm */
  fn = nc_dir + strlen( nc_dir );
    
  for( le = 0; le < ns; le++ ) { /* for each sweepfile */

      printf("\n\nFile %d\n",le);
      
    aa = solo_list_entry( slm, le );
    strcpy( fn, aa );
    nc_access_swpfi( nc_dir );
    sum_azs = sum_els = 0;
      
    for( rn = 0; rn < maxRays; rn++ ) { /* for each ray */
      set_gri_stuff();
      dd_stuff_ray();		/* pass it off to dorade code */
    }
    dd_stats->MB += dd_sizeof_prev_file() * 1.e-6;
  }
} 

/* c------------------------------------------------------------------------ */

int 
nc_ini (void)
{
  int ii, jj, kk, le, ns, nt, tndx;
  int hh, mm, ss, ms=0;
  CHAR_PTR aa, bb, cc, ee, sptrs[32];
  char str[256];
  double dtime;

  prev_vol_num = -999999;
  prevMaxCells = 2048;
  sbuf = (short *)malloc( prevMaxCells * sizeof( short ));
  fbuf = (float *)malloc( prevMaxCells * sizeof( float ));

  for( ii = 0; ii < maxVars; ii++ ) {
    var_names[ii] = 0;
    long_names[ii] = 0;
    units[ii] = 0;
  }

  difs = dd_return_difs_ptr();
  dd_stats = dd_return_stats_ptr();
  gri = return_gri_ptr();

  if( aa = get_tagged_string( "OPTIONS" )) {
    if( strstr( aa, "BISTATIC" ))
      { options |= BISTATIC_FLAG; }
  }

  if( !( aa = get_tagged_string( "NC_DIR" ))) { /* location of sweepfiles */
    printf( "No nc sweepfile directory\n" );
    exit(0);
  }
  slash_path( nc_dir, aa );
  if( !slm ) {
    slm = solo_malloc_list_mgmt(256);
    slm2 = solo_malloc_list_mgmt(256);
  }


  if( !difs->num_time_lims ) {
    ns = generic_sweepfiles( aa, slm, (char *)"ncswp", (char *)"", 0 );
  }
  else {			/* pull out sweepfiles between time limits */
    ns = generic_sweepfiles( aa, slm2, (char *)"ncswp", (char *)"", 0 );
    slm->num_entries = 0;

    for( le = 0; le < ns; le++ ) {
      /* for each sweepfile */

      aa = solo_list_entry( slm2, le );
      dtime = ncswp_time_stamp( aa, gri->dts );

      for(ii=0; ii < difs->num_time_lims; ii++ ) {
	if(dtime >= difs->times[ii]->lower &&
	   dtime < difs->times[ii]->upper)
	  {
	    solo_add_list_entry(slm, aa);
	  }
      }
    
    }
  }

  return ns;
}

/* c------------------------------------------------------------------------ */

double 
ncswp_time_stamp (char *fname, DD_TIME *dts)
{
  int hh, mm, ss, ms=0, nt, tndx;
  char *bb, *sptrs[32], str[256];
  double dtime;

  strcpy( str, fname );
  nt = dd_tokenz( str, sptrs, "_" );
  tndx = ( options & BISTATIC_FLAG ) ? 1 : 2;
    
  bb = sptrs[tndx]+6;
  dts->day = atoi( bb );
  *bb = 0;
  bb -= 2;
  dts->month = atoi( bb );
  *bb = 0;
  dts->year = atoi( sptrs[tndx] );

  bb = sptrs[tndx+1]+6;
  *bb = 0;
  bb -= 2;
  ss = atoi( bb );
  *bb = 0;
  bb -= 2;
  mm = atoi( bb );
  *bb = 0;
  hh = atoi( sptrs[tndx+1] );
  dts->day_seconds = D_SECS( hh, mm, ss, ms );

  dtime = d_time_stamp( dts );
}

/* c------------------------------------------------------------------------ */

int 
nc_access_swpfi (char *full_path_name)
{
  static int swp_count = 0;
  int ii, jj, kk, mark, id, ndims, nvars, ngatts, recdim, cells_did;
  int its_data, nn, *iptr, nc, nc0, gsmid;
  size_t size;
  char *aa, *bb, str[256], att_name[256], att_text[256];
  int dimids[4], nvdims, nvatts, fn;
  float ff, f_vals[16], gsmin;
  double dd, d_vals[16], rr, gs, r0, gs0, rcpGs;
  short ss;
  size_t start[] = { 0,0,0,0 };
  size_t count[] = { 1,1,1,1 };

# ifdef NETCDF

  nc_type dtype;
  nc_type attype;

  if (swp_count++ == 0) {
    for (ii=0; ii < maxVars; cell_lut[ii++] = 0 );
  }


  ii = nc_close( ncid );
  ii = nc_open( full_path_name, NC_NOWRITE, &ncid );

  if( ii != NC_NOERR )
    { printf( "Open failure: %d\n", ii ); return OPEN_FAILURE; }

  nextRayIndex = 0;

  ii = nc_inq( ncid, &ndims, &nvars, &ngatts, &recdim );
  numVars = nvars;
  numDims = ndims;
  numCellDims = 0;
  ii = nc_inq_dim( ncid, recdim, str, &size );
  maxRays = size;

  for( jj = 0; jj < ndims; jj++ ) {
    ii = nc_inq_dim( ncid, jj, str, &size );
    ii = nc_inq_dimname (ncid, jj, dimName[jj]);
    dimSize[jj] = size;
    aa = (char *)"maxcell";
    if (strncasecmp( dimName[jj], aa, strlen(aa)) == 0 && size > 11) {
      cellDimFlag[jj] = YES;
      numCellDims++;
    }
    else
      { cellDimFlag[jj] = NO; }
    aa = (char *)"time";
    if (strncasecmp( dimName[jj], aa, strlen(aa)) == 0)
      { maxRays = size; }
  }

  ii = nc_inq_dimid( ncid, "maxCells", &cells_did );
  ii = nc_inq_dim( ncid, cells_did, str, &size );

  if(size > MAXGATES){
      printf("WARNING: cell dim is %d, setting maxCells to %d \n",size,MAXGATES);
      maxCells = MAXGATES;
  }else{
      maxCells = size;
  }

  archive2 = NO;
  ii = nc_inq_varnatts (ncid, NC_GLOBAL, &numGlobals);

  for (id=0; id < numGlobals; id++) {
    ii = nc_inq_attname (ncid, NC_GLOBAL, id, att_name);
    aa = (char *)"source_format";
    if (strncasecmp (att_name, aa, strlen(aa)) == 0) {
      ii = nc_get_att_text (ncid, NC_GLOBAL, att_name, att_text);
      if (strstr (att_text, "ARCHIVE2"))
	{ archive2 = YES; }
    }
  }
  numFields = 0;
  freqs[0] = 0;
  ipps[0] = 0;
  PRF = 0;
  waveLength_m = 0;
  metersBetweenCells=0;
  gsmin = 2.2e22;
  gsmin_ndx = -1;

  for( id = 0; id < nvars; id++ ) { /* look for needed variables */
    
    ii = nc_inq_var( ncid, id, str, &dtype, &nvdims, dimids, &nvatts );
    nc_types[id] = dtype;
    
    if( !var_names[id] )
      { var_names[id] = (char *)malloc( 24 ); }
    strcpy( var_names[id], str );

    its_data = NO;
    for (kk=0; kk < nvdims; kk++) {
      if (cellDimFlag[dimids[kk]]) {
	its_data = YES;
	cell_dim_id[id] = dimids[kk];
      }
    }

    if( !strcmp( str, "system_name" )) {
      count[1] = dimSize[ dimids[1]];
      ii = nc_get_vara_text( ncid, id, start, count, str );
      str_terminate( radarName, str, sizeof(radarName));
      continue;
    }
    if( !strcmp( str, "Radar_Constant" )) {
      ii = nc_get_var1_float( ncid, id, start, &ff );
      radarConst = ff;
      continue;
    }

    if( !strcmp( str, "Azimuth" ))
      { id_az = id; continue; }
    if( !strcmp( str, "Elevation" ))
      { id_el = id; continue;}
    if( !strcmp( str, "time_offset" ))
      { id_tmoffs = id; continue;}

    if( !strcmp( str, "base_time" )) {
      ii = nc_get_var1_int( ncid, id, 0, &jj );
      base_time = jj;
      continue;
    }
    if( !strcmp( str, "Fixed_Angle" )) {
      ii = nc_get_var1_float( ncid, id, start, &ff );
      nc_fixed_angle = ff;
      continue;
    }
    if( !strcmp( str, "Range_to_First_Cell" )) {
      ii = nc_get_var1_float( ncid, id, start, &ff );
      metersToFirstCell = ff;
      continue;
    }
    if( !strcmp( str, "Cell_Spacing" )) {
      ii = nc_get_var1_float( ncid, id, start, &ff );
      metersBetweenCells = ff;
      continue;
    }
    if( !strcmp( str, "Nyquist_Velocity" )) {
      ii = nc_get_var1_float( ncid, id, start, &ff );
      eff_unamb_vel = ff;
      continue;
    }
    if( !strcmp( str, "Unambiguous_Range" )) {
      ii = nc_get_var1_float( ncid, id, start, &ff );
      eff_unamb_range = ff;
      continue;
    }
    if( !strcmp( str, "Latitude" )) {
      ii = nc_get_var1_double( ncid, id, start, &dd );
      nc_latitude = dd;
      continue;
    }
    if( !strcmp( str, "Longitude" )) {
      ii = nc_get_var1_double( ncid, id, start, &dd );
      nc_longitude = dd;
      continue;
    }
    if( !strcmp( str, "Altitude" )) {
      ii = nc_get_var1_double( ncid, id, start, &dd );
      nc_altitude_km  = dd * .001;
      continue;
    }
    if( !strcmp( str, "Wavelength" )) {
      ii = nc_get_var1_double( ncid, id, start, &waveLength_m );
      continue;
    }
    if( !strcmp( str, "PRF" )) {
      ii = nc_get_var1_double( ncid, id, start, &PRF );
      continue;
    }
    if( !strcmp( str, "bm_width" )) {
      f_vals[1] = 0;
      ii = nc_get_var_float( ncid, id, f_vals );
      horz_beam_width = f_vals[0];
      vert_beam_width = f_vals[1];
      continue;
    }
    if( !strcmp( str, "rcvr_gain" )) {
      ii = nc_get_var_float( ncid, id, f_vals );
      receiver_gain = f_vals[0];
      continue;
    }
    if( !strcmp( str, "ant_gain" )) {
      ii = nc_get_var_float( ncid, id, f_vals );
      antenna_gain = f_vals[0];
      continue;
    }
    if( !strcmp( str, "sys_gain" )) {
      ii = nc_get_var_float( ncid, id, f_vals );
      system_gain = f_vals[0];
      continue;
    }
    if( !strcmp( str, "pulse_width" )) {
      ii = nc_get_var_float( ncid, id, f_vals );
      pulse_width = f_vals[0];
      continue;
    }
    if( !strcmp( str, "band_width" )) {
      ii = nc_get_var_float( ncid, id, f_vals );
      recvr_bandwidth = f_vals[0];
      continue;
    }
    if( !strcmp( str, "peak_pwr" )) {
      ii = nc_get_var_float( ncid, id, f_vals );
      peak_power = f_vals[0];
      continue;
    }
    if( !strcmp( str, "xmtr_pwr" )) {
      ii = nc_get_var_float( ncid, id, f_vals );
      ff = f_vals[0];
      continue;
    }
    if( !strcmp( str, "noise_pwr" )) {
      ii = nc_get_var_float( ncid, id, f_vals );
      noise_power = f_vals[0];
      continue;
    }
    if( !strcmp( str, "tst_pls_pwr" )) {
      ii = nc_get_var_float( ncid, id, f_vals );
      ff = f_vals[0];
      continue;
    }
    if( !strcmp( str, "tst_pls_rng0" )) {
      ii = nc_get_var_float( ncid, id, f_vals );
      ff = f_vals[0];
      continue;
    }
    if( !strcmp( str, "tst_pls_rng1" )) {
      ii = nc_get_var_float( ncid, id, f_vals );
      ff = f_vals[0];
      continue;
    }

# ifdef obsolete
    if( !strcmp( str, "" )) {
      ii = nc_get_var1_float( ncid, id, start, &ff );
    }
# endif
    

    if (its_data) {		/* process the data field */

      field_ids[numFields++] = id;

      for (kk = 0; kk < nvatts; kk++) {
	ii = nc_inq_attname (ncid, id, kk, att_name);

	num_segments[id] = 0;
	aa = (char *)"num_segments";
	if (strncasecmp (att_name, aa, strlen(aa)) == 0) {
	  ii = nc_get_att_int (ncid, id, att_name, &nn);
	  num_segments[id] = nn;
	}
	aa = (char *)"cells_in_segment";
	if (strncasecmp (att_name, aa, strlen(aa)) == 0) {
	  ii = nc_get_att_int (ncid, id, att_name, cells_in_segment[id]);
	}
	aa = (char *)"meters_to_first_cell";
	if (strncasecmp (att_name, aa, strlen(aa)) == 0) {
	  ii = nc_get_att_float (ncid, id, att_name,meters_to_first_cell[id]);
	}
	aa = (char *)"meters_between_cells";
	if (strncasecmp (att_name, aa, strlen(aa)) == 0) {
	  ii = nc_get_att_float (ncid, id, att_name, meters_between_cells[id]);
	  if (meters_between_cells[id][0] < gsmin) {
	    gsmin_ndx = numFields -1;
	    gsmin = meters_between_cells[id][0];
	  }
	}
	if (!cell_lut[id]) {
	  cell_lut[id] = (int *)malloc (maxGates*sizeof(int));
	  memset (cell_lut[id], 0, maxGates*sizeof(int));
	}
      }
      
      if( !long_names[id] )
	{ long_names[id] = (char *)malloc( 80 ); }
	
      ii = nc_inq_att( ncid, id, "long_name", &dtype, &size );
      if( ii == NC_NOERR )
	{ ii = nc_get_att_text( ncid, id, "long_name", long_names[id] ); }
	
      if( !units[id] )
	{ units[id] = (char *)malloc( 24 ); }
	
      ii = nc_inq_att( ncid, id, "units", &dtype, &size );
      if( ii == NC_NOERR ) {
	ii = nc_get_att_text( ncid, id, "units", units[id] );
      }

      ff = MISSING_VALUE;
      ii = nc_inq_att( ncid, id, "scale_factor", &dtype, &size );
      if( ii == NC_NOERR )
	{ ii = nc_get_att_float( ncid, id, "scale_factor", &ff ); }
      scale_factor[id] = ff;

      ff = MISSING_VALUE;
      ii = nc_inq_att( ncid, id, "add_offset", &dtype, &size );
      if( ii == NC_NOERR )
	{ ii = nc_get_att_float( ncid, id, "add_offset", &ff ); }
      add_offset[id] = ff;

      ff = MISSING_VALUE;
      ii = nc_inq_att( ncid, id, "missing_value", &dtype, &size );

      if( ii == NC_NOERR ) {
	if( nc_types[id] == NC_SHORT ) {
	  ii = nc_get_att_short( ncid, id, "missing_value", &ss );
	  int_missing_value[id] = ss;
	  ff = (float)NC_UNSCALE( ss, scale_factor[id]
				  , add_offset[id] );
	}
	else if( nc_types[id] == NC_BYTE ) {
	  ii = nc_get_att_int( ncid, id, "missing_value", nex_missing_vals[id]);
	  int_missing_value[id] = nex_missing_vals[id][0];
	  ff = nex_missing_vals[id][0];
	}
	else {
	  ii = nc_get_att_float( ncid, id, "missing_value", &ff );
	}
      }
      missing_value[id] = ff;

      ii = nc_inq_att( ncid, id, "Frequencies_GHz", &dtype, &size );
      if( ii == NC_NOERR ) {
	ii = nc_get_att_double( ncid, id, "Frequencies_GHz", freqs );
      }
      if( freqs[0] && !waveLength_m ) {
	waveLength_m = SPEED_OF_LIGHT/( freqs[0] * 1.e9 );
      }

      ii = nc_inq_att( ncid, id, "InterPulsePeriods_secs", &dtype, &size );
      if( ii == NC_NOERR ) {
	ii = nc_get_att_double( ncid, id, "InterPulsePeriods_secs", ipps );
      }
      if( ipps[0] && !PRF ) {
	PRF = 1./ipps[0];
      }
    }
    mark = 0;

  } /* // end get variable info */

  if (metersBetweenCells == 0) {
    /*
     * We have individual range info for each field
     * generate cell lookup tables for each field
     * based on the highest resolution field
     */
    id = (numCellDims == 1) ? field_ids[0] : field_ids[gsmin_ndx];
    metersToFirstCell = meters_to_first_cell[id][0];
    metersBetweenCells = meters_between_cells[id][0];
    maxCells = dimSize[cell_dim_id[id]];

    for (ii=0; ii < numFields; ii++) {
      id = field_ids[ii];
      iptr = cell_lut[id];
      if (meters_between_cells[id][0] == gsmin) { /* one to one */
	for (jj=0; jj < maxGates; *iptr++ = jj++);
	continue;
      }
      /* determine which lower resolution cell to using in each
       * high resolution cell
       */
      for (jj=0; jj < maxGates; jj++, *iptr++ = -1);
      gsmid = field_ids[gsmin_ndx];
      rr =  meters_to_first_cell[gsmid][0];
      gs = meters_between_cells[gsmid][0];
      nc = dimSize[cell_dim_id[gsmid]];

      r0 =  meters_to_first_cell[id][0];
      gs0 = meters_between_cells[id][0];
      rcpGs = 1./gs0;
      nc0 = dimSize[cell_dim_id[id]];
      iptr = cell_lut[id];

      for (jj=0; jj < nc; jj++, rr += gs) {
	kk = (int)((rr -r0) * rcpGs +.5);
	if (kk >= 0 && kk < nc0)
	  { iptr[jj] = kk; }
      }
    }
  }

  if( nc_latitude == 0 && nc_longitude == 0 ) {
    nc_latitude = 44.4444;
    nc_longitude = -111.1111;
    nc_altitude_km = .123456;
  }

  ii = nc_inq_att( ncid, NC_GLOBAL, "Instrument_Name", &attype, &size );

  ii = nc_get_att_text( ncid, NC_GLOBAL, "Instrument_Name", radarName );
  ii = nc_get_att_text( ncid, NC_GLOBAL, "Instrument_Type", instrumentType );
  airborne = strstr (instrumentType, "AIR") ? YES : NO;
  ii = nc_get_att_text( ncid, NC_GLOBAL, "Scan_Mode", scanModeMne );
  scanMode = dd_scan_mode( scanModeMne );
  ii = nc_get_att_int( ncid, NC_GLOBAL, "Volume_Number", &vol_num );
  ii = nc_get_att_int( ncid, NC_GLOBAL, "Scan_Number", &sweepNum );
  ii = nc_get_att_int( ncid, NC_GLOBAL, "Num_Samples", &numSamples );
  mark = 0;

  gri->fixed = (float)nc_fixed_angle;
  gri->latitude = (float)nc_latitude;
  gri->longitude = (float)nc_longitude;
  gri->altitude = (float)nc_altitude_km * 1000.;
  gri->altitude_agl = 0;

  gri->source_vol_num = vol_num;
  gri->source_sweep_num = sweepNum;
  gri->radar_type = GROUND;
  gri->scan_mode = scanMode;

  strcpy( gri->radar_name, radarName );
  gri->dd_radar_num = dd_assign_radar_num(gri->radar_name);
  strcpy( gri->project_name, "UNKNOWN" );
  strcpy( gri->site_name, "UNKNOWN" );

  rr = gri->range_b1 = metersToFirstCell;
  gs = gri->bin_spacing = metersBetweenCells;

  gri->num_bins = maxCells;

  for( ii = 0; ii < gri->num_bins; ii++ ) {
    gri->range_value[ii] = (float)rr;
    rr += gs;
  }
  gri->radar_constant = radarConst;
  gri->nyquist_vel = eff_unamb_vel;
  gri->PRF = PRF;
  gri->wavelength  = waveLength_m;
  gri->num_freq = 1;
  gri->freq[0] = freqs[0];
  gri->num_ipps = 1;
  gri->ipps[0] = ipps[0];
  gri->h_beamwidth = horz_beam_width;
  gri->v_beamwidth = vert_beam_width;
  gri->num_samples = numSamples;
  gri->sweep_speed = 0;

  gri->noise_power = noise_power;
  gri->rcvr_gain = receiver_gain;
  gri->peak_power = peak_power;
  gri->ant_gain = antenna_gain;
  gri->polarization = 0;	/* horizontal */
  gri->rcv_bandwidth = recvr_bandwidth;
  gri->pulse_width = pulse_width;
  
  ii = field_ids[0];
//  gri->missing_data_flag = missing_value[ii];
  gri->missing_data_flag = int_missing_value[ii];
  gri->num_fields_present = numFields;

  for( fn = 0; fn < numFields; fn++ ) {
    ii = field_ids[fn];
    gri->int_missing_data_flag[fn] = int_missing_value[ii];
    gri->dd_scale[fn] = 100;
    gri->dd_offset[fn] = 0;
    gri->actual_num_bins[fn] = gri->num_bins;
    strcpy(gri->field_name[fn], var_names[ii] );
    strcpy( gri->field_long_name[fn], long_names[ii] );
    strcpy( gri->field_units[fn], units[ii] );
  }
  gri->binary_format = DD_16_BITS;
  gri->source_format = NETCDF_FMT;
  gri->compression_scheme = NO_COMPRESSION;
  gri->missing_data_flag = EMPTY_FLAG;
  gri->ignore_this_sweep = NO;

  if( prev_vol_num != vol_num ) {
    gri->vol_num++;
  }
  gri->sweep_num++;



# endif 
}

/* c------------------------------------------------------------------------ */

int 
set_gri_stuff (void)
{
# ifdef NETCDF

  int ii, jj, kk, gg, rn, fn, id, mark, msv1, msv2, gsmid, *lut, nc, kkmax = 0;
  size_t start[4], count[4];
  float ff, *fpt;
  double dd;
  struct tm *tm;
  time_t btime;
  short *ss, sms;
  double scalef, aoffs;
  unsigned char *uc;

  if( nextRayIndex >= maxRays )
    { return LAST_RAY; }

  rn = rayNum = nextRayIndex++;
  start[0] = rayNum;

  newVol = prev_vol_num != vol_num;
  newSweep = !rn;
  newRay = YES;

  if( !rn ) {
    prev_vol_num = vol_num;
  }

  ii = nc_get_var1_float( ncid, id_az, start, &ff );
  nc_azimuth = ff;
  ii = nc_get_var1_float( ncid, id_el, start, &ff );
  nc_elevation = ff;
  ii = nc_get_var1_double( ncid, id_tmoffs, start, &time_offset );

  btime = (time_t)( base_time + time_offset );
  tm = gmtime( &btime );
  nc_year = tm->tm_year+1900;
  nc_month = tm->tm_mon+1;
  nc_day = tm->tm_mday;
  nc_hour = tm->tm_hour;
  nc_minute = tm->tm_min;
  nc_second = tm->tm_sec;
  nc_millisecond = (int)((( base_time + time_offset ) -btime ) * 1000. +.5  );

  gri->dts->time_stamp = gri->time = ( base_time + time_offset );
  d_unstamp_time(gri->dts);
  gri->year = gri->dts->year;
  gri->month = gri->dts->month;
  gri->day = gri->dts->day;
  gri->hour = gri->dts->hour;
  gri->minute = gri->dts->minute;
  gri->second = gri->dts->second;
  gri->millisecond = gri->dts->millisecond;

  sum_azs += nc_azimuth;
  sum_els += nc_elevation;
  if( options & BISTATIC_FLAG ) {
    gri->fixed = (scanMode == RHI) ? sum_azs/(rayNum+1) : sum_els/(rayNum+1);
  }
  gri->azimuth = (float)nc_azimuth;
  gri->elevation = (float)nc_elevation;

  gri->rotation_angle = gri->corrected_rotation_angle =
    scanMode == RHI ? gri->elevation : gri->azimuth;
  gri->tilt = scanMode == RHI ? gri->azimuth : gri->elevation;


  start[0] = rayNum;
  start[1] = 0;
  count[0] = 1;


  for( fn = 0; fn < numFields; fn++ ) {

    count[1] = maxCells;
    id = field_ids[fn];
    scalef = scale_factor[id];
    aoffs = add_offset[id];
    ss = gri->scaled_data[fn];
    sms = (short)int_missing_value[id];

    if( nc_types[id] == NC_SHORT ) {
      ii = nc_get_vara_short( ncid, id, start, count, sbuf );
      for( gg = 0; gg < maxCells; gg++, ss++ ) {
	if( sbuf[gg] == sms ) {
	  *ss = sms;
	}
	else {
	  dd = NC_UNSCALE( sbuf[gg], scalef, aoffs );
	  *ss = (short)DD_SCALE( dd, 100., 0 );
	}
      }
    }
    else if( nc_types[id] == NC_BYTE ) {

      if (numCellDims > 1) {
	/* use the cell lookup tables
	 */
	count[1] = dimSize[cell_dim_id[id]];
	uc = (unsigned char*)sbuf;
	ii = nc_get_vara_uchar( ncid, id, start, count, uc);
	msv1 = nex_missing_vals[id][0];
	msv2 = nex_missing_vals[id][1];
	lut = cell_lut[id];
	nc = dimSize[cell_dim_id[field_ids[gsmin_ndx]]];
	  
	for( kkmax=0,gg = 0; gg < nc; gg++, ss++ ) {
	  kk = lut[gg]; if (kk > kkmax) kkmax = kk;
	  if (kk < 0 || uc[kk] == msv1 || uc[kk] == msv2) {
	    *ss = sms;
	    continue;
	  }
	  dd = NC_UNSCALE( uc[kk], scalef, aoffs );
	  *ss = (short)DD_SCALE( dd, 100., 0 );
	}
      }
    }
    else {
      ii = nc_get_vara_float( ncid, id, start, count, fbuf );
      fpt = fbuf;
      for( gg = 0; gg < maxCells; gg++ ) {
	*ss++ = (short)DD_SCALE( *fpt++, 100., 0 );
      }
    }
  }
  mark = 0;
# endif
}
/* c------------------------------------------------------------------------ */

int 
field_index_num (char *name)
{
  int ii, id;

  for( ii = 0; ii < numFields; ii++ ) {
    id = field_ids[ii];
    if( !strcmp( var_names[id], name ))
      { return id; }
  }
  return -1;
}

/* c------------------------------------------------------------------------ */

int
alias_index_num( char * name )
{
  static char * known_aliases [] =
  {
    (char *)"DZ DB DBZ"
    , (char *)"VE VR VF VQ VU"
    , (char *)"RH RX RHO RHOHV"
    , (char *)"PH DP PHI PHIDP"
    , (char *)"ZD DR ZDR"
    , (char *)"LD LC LDR"
   , (char *)"LV LVDR LDRV"
    , (char *)"NC NCP"
    , (char *)"KD KDP NKDP"
    , (char *)"PD PID"
  };

  char str[256], * sptrs[64], *aa, *bb, *cc;
  int ii, jj, kk,mm, nn, nt, ndx;
  int num_alias_sets = sizeof( known_aliases )/sizeof( char * );
		
  if(( mm = strlen( name )) < 1 ) { 
    return -1; 	
  }
	
  for( ii = 0; ii < num_alias_sets; ii++ ) { // for each set of aliases 
    if( !strstr( known_aliases[ii], name ))
      { continue; }		// not even close

    strcpy( str, known_aliases[ii] );
    nt = dd_tokens( str, sptrs );

    for( jj = 0; jj < nt; jj++ ) {
      if( !strcmp( name, sptrs[jj] )) {
	//
	// we have a match; now see if this mapper responds to 
	// one of these aliases
	//
	for( kk = 0; kk < nt; kk++ ) {
	  if(( ndx = field_index_num( sptrs[kk] )) >= 0 )
	    { return ndx; }
	}
	return -1;
      }
    }
  }
  return -1;
}

/* c------------------------------------------------------------------------ */
/* c------------------------------------------------------------------------ */












