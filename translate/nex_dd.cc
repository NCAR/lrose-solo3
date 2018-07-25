/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

# define FRANKS_STUFFoff

/*
 * This file contains routines:
 * 
 * nex_dd_conv
 * 
 * nex_dump_this_ray
 * nexx_ini
 * nex_inventory
 * nex_nab_data
 * nex_next_ray
 * nex_new_parms
 * nex_positioning
 * nex_print_dhed
 * nex_print_headers
 * nex_print_nmh
 * nex_print_nvst
 * nex_print_stat_line
 * nex_reset
 * nex_select_ray
 * nex_start_stop_chars
 * nexrad_time_stamp
 * 
 */


#include <LittleEndian.hh>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <dd_math.h>
#include <dd_defines.h>
#include "dorade_share.hh"
#include "dd_stats.h"
#include "nex_tables.h"
#include "nex_dd.hh"
#include "dda_common.hh"
#include "gneric_dd.hh"
#include "dd_io_mgmt.hh"
#include "dd_crackers.hh"

static struct nexrad_id_rec nir;
static struct nexrad_vol_scan_title nvst;
static struct nexrad_message_header *nmh, *xnmh;
static struct digital_radar_data_header *drdh, *xdrdh;
static struct nexrad_extended_header *nxh, *xnxh;
static struct rda_status_info *rda_stat, *xrda_stat;

static struct dd_input_filters *difs;
static struct dd_stats *dd_stats;
static struct generic_radar_info *gri;
static struct nexrad_useful_items *nui;
static int nex_unamb_rngs[5][8];	/* 8 ranges, 5 deltas */
static struct nexrad_VCP_header *top_nvh=NULL, *current_nvh;
static int current_delta;
static struct nexrad_site_stuff *nss;
static struct input_read_info *iri;
static int view_char=0, view_char_count=200;
static char preamble[24], postamble[64];
static struct solo_list_mgmt *slm = NULL;
static char *field_names[] = {(char *)"DZ", (char *)"VE", (char *)"SW",
			      (char *)"", (char *)"", (char *)""};
static char *long_field_names[] = {
  (char *)"Reflectivity factor", (char *)"Velocity", (char *)"Spectral Width",
  (char *)"", (char *)"", (char *)""};
static char *field_units[] = {(char *)"DBZ", (char *)"m/s", (char *)"m/s",
			      (char *)"", (char *)"", (char *)""};
static char nexrad_dir[128];
static char *current_file_name;

static char *pbuf=NULL, **pbuf_ptrs=NULL;
static int pbuf_max=1200;
static int dbg_count = 0;

/* c------------------------------------------------------------------------ */

double nex_conc_to_fp (float *ival)
{
    int ii, jj;
    double d=0, zsign =  *ival < 0 ? -1. : 1., zwhole;
    int mexp = (((int32_t)*ival & 0x7f000000) >> 24) - 64;
    int mant =   (int32_t)*ival & 0x00ffffff;
    int precision;

    if((precision = mexp > 5 ? 0 : 6 - mexp)) {
	ii = (~((~0) << precision * 4) & mant);
	jj = mant >> (24 -(mexp * 4));
	d = (double)jj + (double)ii/pow((double)16., (double)precision);
    }
    else {
	d = (double)mant * pow((double)16., (double)(mexp -6));
    }
    return(d);
}
/* c------------------------------------------------------------------------ */

void nex_dd_conv (int interactive_mode)
{
    /* main loop for converting 88D data
     */
    int i, n, mark;
    static int count=0, nok_count;

    if(!count) {
	nexx_ini();
    }

    if(interactive_mode) {
	dd_intset();
	nex_positioning();
    }
    nex_reset();

    for(;;) {
	count++;
	if(difs->stop_flag &&
	   (difs->abrupt_start_stop || nui->new_sweep)) {
	    printf("\nBreak on stop time!\n");
	    break;
	}

	if(nex_select_ray()) {

	    if(difs->max_beams)
		  if(++nui->ray_count > difs->max_beams ) {
		      printf("\nBreak on ray count!\n");
		      break;
		  }
	    if(nui->sweep_count_flag) {
		nui->sweep_count_flag = NO;
		if(difs->max_sweeps) {
		    if(++nui->sweep_count > difs->max_sweeps ) {
			printf("\nBreak on sweep count!\n");
			break;
		    }
		}
		if(nui->vol_count_flag) {
		    nui->vol_count_flag = NO;
		    if(difs->max_vols) {
			if(++nui->vol_count > difs->max_vols ) {
			    printf("\nBreak on volume count!\n");
			    break;
			}
		    }
		}
	    }
	    if(!difs->catalog_only)
		  nex_nab_data();
	    
	    dd_stuff_ray();	/* pass it off to dorade code */
	}
	else {
	    if(!(nok_count++ % 1000)) {
		nex_print_stat_line(count);
	    }
	}
	nui->new_sweep = NO;
	nui->new_vol = NO;

	if((n = nex_next_ray()) < 1) {
	    printf("Break on input status: %d\n", n);
	    break;
	}
    }
    nex_print_stat_line(count);
    return;
}
/* c------------------------------------------------------------------------ */

void nex_dump_this_ray (struct solo_list_mgmt *slm)
{
    int gg, ii;
    char *aa, str[128];
    float unscale[7], bias[7];

    aa = str;
    nex_nab_data();
    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of data fields");
    solo_add_list_entry(slm, aa);

    sprintf(aa, "name ");

    for(ii=0; ii < 3; ii++) {
	sprintf(aa+strlen(aa), "        %s", gri->field_name[ii]);
	unscale[ii] = gri->dd_scale[ii] ? 1./gri->dd_scale[ii] : 1.;
	bias[ii] = gri->dd_offset[ii];
    }
    solo_add_list_entry(slm, aa);

    for(gg=0; gg < gri->num_bins; gg++) {
	sprintf(aa, "%4d)", gg);

	for(ii=0; ii < 3; ii++) {
	    sprintf(aa+strlen(aa), "%10.2f",
		    DD_UNSCALE(*(gri->scaled_data[ii]+gg), unscale[ii]
			       , bias[ii]));
	}
	solo_add_list_entry(slm, aa);
    }

    slm_print_list(slm);
}
/* c------------------------------------------------------------------------ */

void 
nexx_ini (void)
{
    int ii, jj, kk, nn, nt, mark, ms = EMPTY_FLAG, found=NO, swp_count;
    int latd, latm, lats, lond, lonm, lons;
    static struct nexrad_site_stuff nss2[2];
    struct nexrad_VCP_header *nvh, *last_nvh;
    struct nexrad_VCP_items *nvi;
    char str[256], *line, directory[128];
    char *a, *aa, *bb, *cc, *unk=(char *)"UNKNOWN";
    char string_space[256], *str_ptrs[32];
    double d;
    FILE *fs;

    printf("88D_FORMAT\n");

    dd_min_rays_per_sweep();	/* trigger min rays per sweep */
    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();
    gri = return_gri_ptr();
    nui = (struct nexrad_useful_items *)
	  malloc(sizeof(struct nexrad_useful_items));
    memset((char *)nui, 0, sizeof(struct nexrad_useful_items));
    nui->range_ambiguity_flag = 111.;
    nui->max_gates = NEX_MAX_GATES;

    for(ii=0; ii < NEX_MAX_FIELDS; ii++) {
	nui->nufi[ii].nfh.bytes_in_this_header =
	      sizeof(struct nexrad_field_header);
    }
    if(hostIsLittleEndian()) {
       nmh = xnmh = (struct nexrad_message_header *)
	 malloc(sizeof(struct nexrad_message_header));
       memset(xnmh, 0, sizeof(struct nexrad_message_header));
       drdh = xdrdh = (struct digital_radar_data_header *)
	 malloc(sizeof(struct digital_radar_data_header));
       memset(xdrdh, 0, sizeof(struct digital_radar_data_header));
       nxh = xnxh = (struct nexrad_extended_header *)
	 malloc(sizeof(struct nexrad_extended_header));
       memset(xnxh, 0, sizeof(struct nexrad_extended_header));
       rda_stat = xrda_stat = (struct rda_status_info *)
	 malloc( sizeof( struct rda_status_info ));
       memset( xrda_stat, 0, sizeof( struct rda_status_info ));
    }
    /* c...mark */
    /*
     * open up the input file
     */
    iri = dd_return_ios(0, WSR_88D_FMT);
    iri->min_block_size = NEX_PACKET_SIZE;

    if((aa=get_tagged_string("SOURCE_DEV"))) {
        aa = dd_establish_source_dev_names((char *)"NEX", aa);
	aa = dd_next_source_dev_name((char *)"NEX");
	current_file_name = aa;
	dd_input_read_open(iri, aa);
    }

    nui->new_vol = YES;
    gri->binary_format = DD_16_BITS;
    gri->source_format = WSR_88D_FMT;
    gri->radar_type = DD_RADAR_TYPE_GROUND;
    gri->latitude = ms;
    gri->longitude = ms;
    gri->altitude = ms;
    gri->scan_mode = DD_SCAN_MODE_SUR;
    gri->missing_data_flag = EMPTY_FLAG;
    gri->h_beamwidth = NEX_HORZ_BEAM_WIDTH;
    gri->v_beamwidth = NEX_VERT_BEAM_WIDTH;
    gri->polarization = 0;	/* horizontal polarization */
    gri->freq[0] = ms;
    gri->ipps[0] = ms;

    
    if((a=get_tagged_string("OPTIONS"))) {
	if(strstr(a, "REF_ONLY") || strstr(a, "REFL_ONLY") ||
	   strstr(a, "REFLECTIVITY_ONLY")) {
	    nui->options = NEX_REFL_ONLY;
	}
	else if(strstr(a, "NEW_NCDC")) { /* new NCDC blocking */
	    printf("OPTION: NEW_NCDC\n");
	    nui->options = NEX_NCDC_BLOCKING;
	}
	else if(strstr(a, "VEL_ONLY") || strstr(a, "VELOCITY_ONLY")) {
	    nui->options = NEX_VEL_ONLY;
	}
	if(strstr(a, "RAP_NEX")) {
	    printf("OPTION: RAP_NEX \n");
	    nui->options |= RAP_NEXRAD;
	    nmh = (struct nexrad_message_header *)malloc
	      (sizeof(struct nexrad_message_header));
	    memset(nmh, 0, sizeof(struct nexrad_message_header));
	}
	if(strstr(a, "MAX_RANGE")) {
	    nui->options |= NEX_MAX_RNG;
	    nui->max_gates = NEX_MAX_REF_GATES;
	}
        /* no translation of input byte values */
	if(strstr(a, "RAW_VALUE")) {
	    nui->options |= NEX_RAW_VALS;
	}
	if(bb = strstr(a, "RANGE_AMB")) {
	   /* see if there's a number to use e.g. "RANGE_AMB:123." */
	   for(; *bb && *bb != ' ' && *bb != '\t' && *bb != ':'; bb++);
	   if(*bb == ':') {
	      cc = str;
	      for(++bb; *bb && *bb != ' ' && *bb != '\t'; *cc++ = *bb++);
	      *cc = 0;
	      nui->range_ambiguity_flag = atof(str);
	   }
	   nui->options |= NEX_RNG_AMB;
	}
    }
    if(a=get_tagged_string("RADAR_NAME")) {
	strcpy(gri->radar_name, a);
    }
    else {
	strcpy(gri->radar_name, unk); /* UNKN ie unknown */
    }
    gri->dd_radar_num = dd_assign_radar_num(gri->radar_name);

    if (!slm)
      { slm = solo_malloc_list_mgmt(128); }

    if(a=getenv("NEXRAD_SITE_INFO")) {
      nn = dd_absorb_strings (a, slm);
    }
    else {
      nn = sizeof (nex_sites_ptrs)/sizeof (char *);
      dd_strings_to_slm (nex_sites_ptrs, slm, nn);
    }
    if(!nex_nab_site_info(slm)) {
       exit(1);
    }

    a = (char *)"NEXRAD_";
    nn = strlen(a);
    strcpy(gri->project_name, a);
    if(a=get_tagged_string("PROJECT_NAME")) {
	kk = nn + (int)strlen(a) > 20 ? 20-nn : (int)strlen(a);
	strncat(gri->project_name, a, kk);
	if(nn+kk < 20) gri->project_name[nn+kk] = '\0';
    }
    
    if(a=get_tagged_string("SITE_NAME")) {
	strcpy(gri->site_name, a);
    }
    else {
	bb = gri->site_name;
	strcpy(bb, nss->state);
	strcat(bb, ",");
	str_terminate(bb+strlen(bb), nss->city, 8-strlen(bb));
    }

    nn = sizeof (nex_vcps_ptrs)/sizeof (char *);
    dd_strings_to_slm (nex_vcps_ptrs, slm, nn);

    for (jj =0; jj < slm->num_entries; jj++) {
        line = solo_list_entry (slm, jj);

	if(!strncmp(line, "VCPAT", 5)) {
	    /* this is a new pattern */

	    nvh = (struct nexrad_VCP_header *)
		  malloc(sizeof(struct nexrad_VCP_header));
	    memset(nvh, 0, sizeof(struct nexrad_VCP_header));
	    if(!top_nvh)
		  top_nvh = nvh;
	    else
		  last_nvh->next = nvh;
	    last_nvh = nvh;

	    ii = sscanf(line, "%s%d%d%d%d%d%x"
			, str
			, &nvh->item_1
			, &nvh->pattern_type
			, &nvh->VCP_num
			, &nvh->num_sweeps
			, &nvh->item_5
			, &nvh->pulse_flag
			);
	    /*
	     * lower byte of nvh->pulse_flag == 4 => long_pulse
	     * lower byte of nvh->pulse_flag == 2 => short_pulse
	     *
	     */
	    nvh->swp_info = (struct nexrad_VCP_items *)
		  malloc(nvh->num_sweeps*sizeof(struct nexrad_VCP_items));
	    memset(nvh->swp_info, 0
		   , nvh->num_sweeps*sizeof(struct nexrad_VCP_items));
	    swp_count = 0;
	    continue;
	}
	nvi = nvh->swp_info +swp_count++;

	ii = sscanf(line, "%x%d%d%d%x%x%d%d%x%x%d%d%x%d%d%d%d"
		    , &nvi->fixed_angle
		    , &nvi->wave_type
		    , &nvi->is_prf_num
		    , &nvi->is_pulse_count
		    , &nvi->az_rate
		    , &nvi->item_6
		    , &nvi->id_prf_num
		    , &nvi->id_pulse_count
		    , &nvi->item_9
		    , &nvi->item_10
		    , &nvi->item_11
		    , &nvi->item_12
		    , &nvi->item_13
		    , &nvi->item_14
		    , &nvi->item_15
		    , &nvi->item_16
		    , &nvi->item_17
		    );
	/*
	 * fixed_angle = nvi->fixed_angle*NEX_FIXED_ANGLE;
	 * sweep_rate = nvi->az_rate*NEX_AZ_RATE;
	 * drdh->elev_num-1 => vcp index
	 * drdh->volume_coverage_pattern => which table
	 * is_pulse_count goes into reflectivity
	 * id_pulse_count goes into velocities
	 *
	 *
	 *
	 */
    }

    nn = sizeof (nex_unamb_ranges_ptrs)/sizeof (char *);
    dd_strings_to_slm (nex_unamb_ranges_ptrs, slm, nn);

    for (jj = 0; jj < slm->num_entries; jj++) {
        line = solo_list_entry (slm, jj);

	ii = sscanf(line, "%d%d%d%d%d%d%d%d"
		    , &nex_unamb_rngs[jj][0]
		    , &nex_unamb_rngs[jj][1]
		    , &nex_unamb_rngs[jj][2]
		    , &nex_unamb_rngs[jj][3]
		    , &nex_unamb_rngs[jj][4]
		    , &nex_unamb_rngs[jj][5]
		    , &nex_unamb_rngs[jj][6]
		    , &nex_unamb_rngs[jj][7]
		    );
	jj++;
	/*
	 * PRT is determined by searching this array for a match to the
	 * unambiguous range derived from
	 * drdh->unamb_range_x10 and remembering the row number (delta)
	 * then the nvi->is_prf_num becomes the column index for the true
	 * unambiguous range which is then used to calculate PRT or PRF.
	 * use nvi->id_prf_num for velocity data.
	 */
    }

    nex_next_ray();

    return;
}
/* c------------------------------------------------------------------------ */

int nex_nab_site_info (struct solo_list_mgmt *slm)
{
   int ii, jj, nn, mark, found=NO;
   int latd, latm, lats, lond, lonm, lons;
   static struct nexrad_site_stuff nss2[2];
   char str[256], *line;
   char *aa, *unk=(char *)"UNKNOWN";
   const char *bb, *cc;
   double d;
 
   nss = nss2;

   for (jj = 0; jj < slm->num_entries; jj++) {
      line = solo_list_entry (slm, jj);
     
      ii = sscanf(line, "%d%s%s%s%d%d%d%d%d%d%f%f%f%f"
		  , &nss->site_number
		  , nss->radar_name
		  , nss->city
		  , nss->state
		  , &latd
		  , &latm
		  , &lats
		  , &lond
		  , &lonm
		  , &lons
		  , &nss->altitude
		  , &nss->frequency_mhz
		  , &nss->short_pulse_ns
		  , &nss->long_pulse_ns
		  );
      if (ii < 14)
	{ continue; }
      /*
       * wave_length = 300./nss->frequency_mhz; (could be -999 though)
       * short_pul_dep = nss->short_pulse_ns*rad_const/2000.;
       * long_pul_dep = nss->long_pulse_ns*rad_const/2000.;
       * rad_constant = 299.7930;
       * horz_beam_width = 0.95;
       * vert_beam_width = 0.95;
       * polarization is now horizontal
       *
       */
      if(latd < 0) {latm = -IABS(latm); lats = -IABS(lats);}
      if(lond < 0) {lonm = -IABS(lonm); lons = -IABS(lons);}
      nss->latitude = (double)latd +latm/60. +lats/3600.;
      nss->longitude = (double)lond +lonm/60. +lons/3600.;

      if(!strncmp(nss->radar_name, "UNKN", 4)) {
	 /* keep the info around for the UNKNOWN radar */
	 memcpy(&nss2[1], nss, sizeof(struct nexrad_site_stuff));
      }
      if(!strncmp(nss->radar_name, gri->radar_name, 4)) {
	 found = YES;
	 break;
      }
   }
   if(!found) {
      found = YES;
      nss = &nss2[1];
   }
   printf("Radar name: %s  City: %s, %s\n", nss->radar_name
	  , nss->city, nss->state);

   if(nss->frequency_mhz > 0) {
	gri->wavelength = SPEED_OF_LIGHT/(nss->frequency_mhz*1.e6);
	gri->freq[0] = nss->frequency_mhz*.001;	/* GHz! */
    }


   if(aa=get_tagged_string("RADAR_LATITUDE")) {
	gri->latitude = 
	      d = atof(aa);
    }
    else {
	gri->latitude = nss->latitude;
    }
    d = fabs(gri->latitude -(int)gri->latitude)*60.;
    latm = (int)d;
    d -= (int)d;
    lats = (int)(d*60+.5);
    printf("Latitude: %.4f  %02d %02d %02d\n", gri->latitude
	   , (int)gri->latitude, latm, lats);

    if(aa=get_tagged_string("RADAR_LONGITUDE")) {
	gri->longitude = 
	      d = atof(aa);
    }
    else {
	gri->longitude = nss->longitude;
    }
    d = fabs(gri->longitude -(int)gri->longitude)*60.;
    lonm = (int)d;
    d -= (int)d;
    lons = (int)(d*60+.5);
    printf("Longitude: %.4f  %02d %02d %02d\n", gri->longitude
	   , (int)gri->longitude, lonm, lons);

    if(aa=get_tagged_string("RADAR_ALTITUDE")) {
	gri->altitude = 
	      d = atof(aa);
    }
    else {
	gri->altitude = nss->altitude;
    }
    printf("Altitude: %.1f meters.\n", gri->altitude);

   return(found);
}
/* c------------------------------------------------------------------------ */

int 
nex_inventory (struct input_read_info *iri)
{
    /* the purpose of this routine is to facilitate a more detailed
     * examination of the data
     */
    int ii=0, max=gri_max_lines();
    int nn, ival;
    float val;
    char str[256];
    double d;


    for(;;) {
	for(ii=0; ii < max; ii++) {
	    nui->new_sweep = NO;
	    if((nn=nex_next_ray()) < 1) {
		break;
	    }
	    sprintf(preamble, "%2d", iri->top->b_num);
	    gri_interest(gri, 0, preamble, postamble);
	}
	if(nn < 1)
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

void 
nex_nab_data (void)
{
    /* generate data for all three fields even if they are not present
     */
    static int count=0;
    int g, i, m, n, mark, ginc, gmax;
    unsigned char *c;
    short *rr, *ss, *tt, *lut;

    count++;

    if(nui->ref_ptr) {
	lut = nui->lut[nui->ref_ndx];
	rr = nui->dz_replicate;
	ss = gri->scaled_data[nui->ref_ndx];
	tt = ss +gri->num_bins;
	c = (unsigned char *) nui->ref_ptr;
	m = gri->num_bins;
	n = drdh->ref_num_gates;

        /* don't apply lookup tables if user asked for raw values */
	if(nui->options & NEX_RAW_VALS) {
            for(g=0; g < m; g++, rr++) {
                if(*rr > 0 && *rr < n)
                    *ss++ = *(c+(*rr));
                else
                    *ss++ = EMPTY_FLAG;
            }
        }else {
            for(g=0; g < m; g++, rr++) {
                if(*rr > 0 && *rr < n)
                    *ss++ = *(lut +(*(c+(*rr))));
                else
                    *ss++ = EMPTY_FLAG;
            }
        }
    } else {
	/* if it's not present, fake it!
	 */
	ss = gri->scaled_data[nui->ref_ndx];
	tt = ss + gri->num_bins;
	for(; ss < tt; ss++) {
	    *ss = EMPTY_FLAG;
	}
    }

    if(nui->vel_ptr) {
	lut = nui->lut[nui->vel_ndx];
    	ss = gri->scaled_data[nui->vel_ndx];
	c = (unsigned char *) nui->vel_ptr;
	n = gri->num_bins;

	if(nui->options & NEX_MAX_RNG) {
	   gmax = drdh->vel_num_gates/2;
	   ginc = 2;		/* do every other gate */
	}
	else {
	   gmax = drdh->vel_num_gates;
	   ginc = 1;		/* do every gate */
	}
	for(g=0; g < gmax; g++, c+=ginc, ss++) {
	    *ss = *(lut +(*c));
	}
	for(; g < n; g++) {
	    *ss++ = EMPTY_FLAG;
	}
    }
    else {
	ss = gri->scaled_data[nui->vel_ndx];
	tt = ss + gri->num_bins;
	for(; ss < tt; ss++) {
	    *ss = EMPTY_FLAG;
	}
    }

    if(nui->sw_ptr) {
	lut = nui->lut[nui->sw_ndx];
	ss = gri->scaled_data[nui->sw_ndx];
	c = (unsigned char *) nui->sw_ptr;
	n = gri->num_bins;

	if(nui->options & NEX_MAX_RNG) {
	   gmax = drdh->vel_num_gates/2;
	   ginc = 2;		/* do every other gate */
	}
	else {
	   gmax = drdh->vel_num_gates;
	   ginc = 1;		/* do every gate */
	}
	for(g=0; g < gmax; g++, c+=ginc, ss++) {
	    *ss = *(lut +(*c));
	}

	for(; g < n; g++) {
	    *ss = EMPTY_FLAG;
	}
    }
    else {
	ss = gri->scaled_data[nui->sw_ndx];
	tt = ss +gri->num_bins;
	for(; ss < tt; ss++) {
	    *ss = EMPTY_FLAG;
	}
    }
}
/* c------------------------------------------------------------------------ */

void 
nexx_nab_data (void)
{
    /* generate data for the first three fields even if they are not present
     */
    static int count=0;
    int g, i, m, n, mark, pn;
    unsigned char *c;
    unsigned short *us;
    short *rr, *ss, *tt, *lut, *gt;
    struct nex_unique_field_info *nufi;

    count++;

    for(pn=0; pn < nui->num_fields; pn++) {
	nufi = &nui->nufi[pn];
	if(nufi->data_ptr) {
	    lut = nufi->data_lut;
	    gt = nufi->gate_lut;
	    ss = gri->scaled_data[nufi->gri_ndx];
	    tt = ss +gri->num_bins;
	    m = gri->num_bins;
	    n = nufi->nfh.num_gates;

	    switch(nufi->nfh.bytes_per_gate) {
	    case 1:
		c = (unsigned char *)nufi->data_ptr;
		
		for(g=0; g < m; g++, gt++) {
		    if(*gt >= 0 && *gt < n)
			  *ss++ = *(lut +(*(c+(*gt))));
		    else
			  *ss++ = EMPTY_FLAG;
		}
		break;
	    case 2:
		us = (unsigned short *)nufi->data_ptr;
		
		for(g=0; g < m; g++, gt++) {
		    if(*gt >= 0 && *gt < n)
			  *ss++ = *(lut +(*(us+(*gt))));
		    else
			  *ss++ = EMPTY_FLAG;
		}
		break;
	    default:
		break;
	    }
	}
	else {
	    /* if it's not present, fake it!
	     */
	    ss = gri->scaled_data[nufi->gri_ndx];
	    tt = ss +gri->num_bins;
	    for(; ss < tt; ss++) {
		*ss = EMPTY_FLAG;
	    }
	}
    }

}
/* c------------------------------------------------------------------------ */

void 
nexx_new_parms (void)
{
    int ii, jj, kk, fn, gg, pn, size, ms = EMPTY_FLAG, mark;
    static int count=0, trip=3;
    struct nex_unique_field_info *nufi;
    float f, gz, ginc, rfld, rref, *r, scale, bias, rcp_scale, *fp;
    char *at = (char *)drdh;
    short *ss, *lut;
    

    count++;
    if(count >= trip) {
	mark = 0;
    }
    r = gri->range_value;
    f = gri->range_b1 = drdh->vel_gate1;
    gri->bin_spacing = drdh->vel_gate_width;
    gri->num_bins = MAXCVGATES;
    for(ii=0; ii < gri->num_bins; ii++, f+=gri->bin_spacing) {
	*r++ = f;
    }
    /* c...mark */

    gri->num_fields_present = 0;

    for(pn=0; pn < nui->num_fields; pn++) {
	nufi = &nui->nufi[pn];
	if(!(ss = nufi->gate_lut)) {
	    size = MAXCVGATES*sizeof(short);
	    ss = nufi->gate_lut = (short *)malloc(size);
	    memset(nufi->gate_lut, 0, size);
	}
	rref = gri->range_b1;
	rfld = nufi->nfh.meters_to_gate1;
	ginc = gri->bin_spacing/(float)nufi->nfh.meters_per_gate;
	gz = 0;
	/* set up the gate lut
	 */
	for(gg=0; rref < rfld; gg++) {
	    *ss++ = -1;
	    rref += gri->bin_spacing;
	}
	for(; gg < gri->num_bins; gg++, *(ss++) = (int)gz, gz += ginc) {
	    mark = 0;
	}
	nufi->gri_ndx = pn = gri->num_fields_present++;
	gri->dd_scale[pn] = 100.;
	gri->dd_offset[pn] = 0;
	gri->actual_num_bins[pn] = gri->num_bins;
	if(nufi->nfh.scale_mult) {
	    scale = (float)nufi->nfh.scale/(float)nufi->nfh.scale_mult;
	}
	else {
	    fp = (float *)&nufi->nfh.scale;
	    scale = *fp;
	}
	rcp_scale = scale ? 1./scale : 1;

	if(nufi->nfh.bias_mult) {
	    bias = (float)nufi->nfh.bias/(float)nufi->nfh.bias_mult;
	}
	else {
	    fp = (float *)&nufi->nfh.bias;
	    bias = *fp;
	}
	lut = nufi->data_lut;

	switch(nufi->nfh.bytes_per_gate) {

	case 1:			/* one byte */
	    size = 256*sizeof(short);
	    if(size > nufi->sizeof_data_lut) {
		nufi->sizeof_data_lut = size;
		lut = nufi->data_lut = (short *)malloc(size);
		memset(nufi->data_lut, 0, size);
	    }
	    *lut++ = EMPTY_FLAG;	/* special flag */
	    *lut++ = EMPTY_FLAG;	/* special flag */
	    for(ii=2; ii < 256; ii++) {
		*lut++ = (short)DD_SCALE
		      (NEX_UNSCALE(ii, rcp_scale, bias)
		       , gri->dd_scale[pn], gri->dd_offset[pn]); 
	    }
	    break;

	case 2:			/* two bytes */
	    size = 65536*sizeof(short);
	    if(size > nufi->sizeof_data_lut) {
		nufi->sizeof_data_lut = size;
		lut = nufi->data_lut = (short *)malloc(size);
		memset(nufi->data_lut, 0, size);
	    }
	    *lut++ = EMPTY_FLAG;	/* special flag */
	    *lut++ = EMPTY_FLAG;	/* special flag */
	    for(ii=2; ii < 65536; ii++) {
		*lut++ = (short)DD_SCALE(NEX_UNSCALE(ii, rcp_scale, bias)
				  , gri->dd_scale[pn], gri->dd_offset[pn]); 
	    }
	    break;

	default:
	    break;
	}
	str_terminate(gri->field_name[pn], nufi->nfh.name, 8);

	for(ii=0;; ii++) {
	    if(!strlen(field_names[ii])) {
		break;
	    }
	    if(!strcmp(gri->field_name[pn], field_names[ii])) {
		break;
	    }
	}
	if(strlen(field_names[ii])) {
	    strcpy(gri->field_long_name[pn], long_field_names[ii]);
	    strcpy(gri->field_units[pn], field_units[ii]);
	}
    }

    gri->num_freq = gri->num_ipps = 1;
    gri->wavelength = NEX_NOMINAL_WAVELN;
    gri->sweep_speed = ms;
    /*
     * find the current volume coverage pattern header
     */
    current_nvh = top_nvh;
    for(; current_nvh; current_nvh = current_nvh->next) {
	if(drdh->vol_coverage_pattern == current_nvh->VCP_num)
	      break;
    }

    printf("\n***** VCP number: %d *****\n\n", drdh->vol_coverage_pattern);

    sprintf(postamble, "VCP: %d ", drdh->vol_coverage_pattern);
    if(!current_nvh) current_nvh = top_nvh;
    if((ii = current_nvh->pulse_flag & 0xff) == 2)
	  gri->pulse_width = nss->short_pulse_ns * 1.e-9 *
		0.5 * SPEED_OF_LIGHT; /* to get to meters */
    else
	  gri->pulse_width = nss->long_pulse_ns * 1.e-9 *
		0.5 * SPEED_OF_LIGHT;
    /*
     * find the currrent unambiguous range delta
     */
    kk = drdh->unamb_range_x10/10;
    for(current_delta=0; current_delta < NEX_NUM_DELTAS; current_delta++) {
	for(jj=0; jj < NEX_NUM_UNAMB_RNGS; jj++) {
	    if(kk == nex_unamb_rngs[current_delta][jj])
		  break;
	}
	if(jj != NEX_NUM_UNAMB_RNGS) {
	    break;
	}
    }
    if(current_delta == NEX_NUM_DELTAS) current_delta = 0;
}
/* c------------------------------------------------------------------------ */

void 
nex_new_parms (void)
{
    int ii, jj, kk, fn, gg, ms = EMPTY_FLAG, mark;
    static int count=0, trip=3;
    float f, gz, ginc, rz, rv, *r;
    short *ss, *lut;
    

    count++;
    if(count >= trip) {
	mark = 0;
    }
    gri->num_bins = nui->max_gates;
    r = gri->range_value;
    f = gri->range_b1 = drdh->vel_gate1;
    gri->bin_spacing = drdh->vel_gate_width;

    if(nui->options & NEX_MAX_RNG) {
       gri->bin_spacing = 2 * drdh->vel_gate_width;
       /*
	* double the range (each ref gate replicated 2 times instead of 4)
	* velocities are decimated to every other gate
	*/
    }
    for(ii=0; ii < gri->num_bins; ii++, f+=gri->bin_spacing) {
	*r++ = f;
    }
    /* set up the gate lut for dz */

    /* assume we will have to replicate gates for DZ */
    ss = nui->dz_replicate;
    ginc = gri->bin_spacing/(float)drdh->ref_gate_width;
    rz = (float)drdh->ref_gate1;
    rv = (float)drdh->vel_gate1;
    gz = 0;
    for(gg=0; rv < rz && gg < drdh->vel_num_gates; gg++) {
	*ss++ = -1;
	rv += drdh->vel_gate_width;
    }
    nui->initial_missing_gates = gg;
    
    for(; gg < gri->num_bins; gg++, *ss++=(short)gz, gz+=ginc) {
	mark = 0;
    }


    gri->num_fields_present = 0;
    /*
     * Force it to always produce all three fields even if they
     * contain delete flags
     */

    /* reflectivity
     */
    nui->ref_ndx = fn = gri->num_fields_present++;
    if(nui->options & NEX_RAW_VALS) {
        gri->dd_scale[fn] = 1.0;
    }else {
        gri->dd_scale[fn] = 100.;
    }
    
    gri->dd_offset[fn] = 0;
    gri->actual_num_bins[fn] = gri->num_bins;
    if(!nui->lut[fn])
	  nui->lut[fn] = (short *)malloc(NEX_MAX_GATES*sizeof(short));

    lut = nui->lut[fn];
    *lut++ = EMPTY_FLAG;
    *lut++ = nui->options & NEX_RNG_AMB
      ? (short)DD_SCALE(nui->range_ambiguity_flag
		 , gri->dd_scale[fn], gri->dd_offset[fn]) : EMPTY_FLAG;

    for(ii=0; ii < 254; ii++) {
	*lut++ = (short)DD_SCALE(-32. +.5*ii, gri->dd_scale[fn]
			  , gri->dd_offset[fn]); 
    }
    strcpy(gri->field_name[fn], "DZ");
    strcpy(gri->field_long_name[fn], "Reflectivity factor");
    strcpy(gri->field_units[fn], "DBZ");

    /* velocity
     */
    nui->vel_ndx = fn = gri->num_fields_present++;
    if(nui->options & NEX_RAW_VALS) {
        gri->dd_scale[fn] = 1.0;
    }else {
        gri->dd_scale[fn] = 100.;
    }
    
    gri->dd_offset[fn] = 0;
    gri->actual_num_bins[fn] = gri->num_bins;
    if(!nui->lut[fn])
	  nui->lut[fn] = (short *)malloc(NEX_MAX_GATES*sizeof(short));
    
    lut = nui->lut[fn];
    *lut++ = EMPTY_FLAG;
    *lut++ = EMPTY_FLAG;

    if(drdh->velocity_resolution == ONE_METER_PER_SEC) {
	for(ii=0; ii < 254; ii++) {
	    *lut++ = (short)DD_SCALE(-127. +ii, gri->dd_scale[fn]
			      , gri->dd_offset[fn]);
	}
    }
    else {
	for(ii=0; ii < 254; ii++) {
	    *lut++ = (short)DD_SCALE(-63.5 +.5*ii, gri->dd_scale[fn]
			      , gri->dd_offset[fn]);
	}
    }
    strcpy(gri->field_name[fn], "VE");
    strcpy(gri->field_long_name[fn], "Velocity");
    strcpy(gri->field_units[fn], "m/s");

    /* spectral width
     */
    nui->sw_ndx = fn = gri->num_fields_present++;
    if(nui->options & NEX_RAW_VALS) {
        gri->dd_scale[fn] = 1.0;
    }else {
        gri->dd_scale[fn] = 100.;
    }
    gri->dd_offset[fn] = 0;
    gri->actual_num_bins[fn] = gri->num_bins;
    if(!nui->lut[fn])
	  nui->lut[fn] = (short *)malloc(NEX_MAX_GATES*sizeof(short));
    
    lut = nui->lut[fn];
    *lut++ = EMPTY_FLAG;
    *lut++ = EMPTY_FLAG;

    for(ii=0; ii < 254; ii++) {
	*lut++ = (short)DD_SCALE(-63.5 +.5*ii, gri->dd_scale[fn]
			  , gri->dd_offset[fn]);
    }
    strcpy(gri->field_name[fn], "SW");
    strcpy(gri->field_long_name[fn], "Spectral width");
    strcpy(gri->field_units[fn], "m/s");


    gri->num_freq = gri->num_ipps = 1;
    gri->wavelength = NEX_NOMINAL_WAVELN;
    gri->sweep_speed = ms;
    /*
     * find the current volume coverage pattern header
     */
    current_nvh = top_nvh;
    for(; current_nvh; current_nvh = current_nvh->next) {
	if(drdh->vol_coverage_pattern == current_nvh->VCP_num)
	      break;
    }

    printf("\n***** VCP number: %d *****\n\n", drdh->vol_coverage_pattern);

    sprintf(postamble, "VCP: %d ", drdh->vol_coverage_pattern);
    if(!current_nvh) current_nvh = top_nvh;
    if((ii = current_nvh->pulse_flag & 0xff) == 2)
	  gri->pulse_width = nss->short_pulse_ns * 1.e-9 *
		0.5 * SPEED_OF_LIGHT; /* to get to meters */
    else
	  gri->pulse_width = nss->long_pulse_ns * 1.e-9 *
		0.5 * SPEED_OF_LIGHT;
    /*
     * find the currrent unambiguous range delta
     */
    kk = drdh->unamb_range_x10/10;
    for(current_delta=0; current_delta < NEX_NUM_DELTAS; current_delta++) {
	for(jj=0; jj < NEX_NUM_UNAMB_RNGS; jj++) {
	    if(kk == nex_unamb_rngs[current_delta][jj])
		  break;
	}
	if(jj != NEX_NUM_UNAMB_RNGS) {
	    break;
	}
    }
    if(current_delta == NEX_NUM_DELTAS) current_delta = 0;
}
/* c------------------------------------------------------------------------ */

char *
nex_next_block (void)
{
    /* returns a pointer to the next block of a new NCDC 88D Level 2 file
     * 
     */
    static int count=0, bp=98, tmp_rec = NO, cal_count = 0;
    static char *tmp_at = NULL, *next_packet = NULL, *last_packet;
    int ii, mm, nn, mark, message_type, cal_packets = 36;
    int bytes_left = 0, bytes_used = 0, header_plus = 0;
    int eof_count = 0, err_count = 0, packet_size = NEX_PACKET_SIZE;
    char *at, *aa, *bb, *arch2 = (char *)"ARCHIVE2", *top = NULL;
    char *arch2_2 = (char *)"AR2V0001.";
    struct io_packet *dp;
    struct CTM_info *ctm;
    static struct CTM_info *prior_ctm = 0;
    short zeroes[6] = {0,0,0,0,0,0};


    if( !tmp_at ) {
      tmp_at = (char *)malloc( 3 * NEX_PACKET_SIZE );
    }

    if(++count >= bp) {
	mark = 0;
    }
    dbg_count = count;

    for(;;) {			/* 2432 = 19 * 128 */

        bytes_left = 0;

	if(iri->top->bytes_left < iri->min_block_size) {

	    if(( bytes_left = iri->top->bytes_left ) > 0 ) {
	      memcpy( tmp_at, iri->top->at, bytes_left );
	    }	    
	    dd_logical_read(iri, FORWARD);
	    dd_stats->MB += BYTES_TO_MB(iri->top->sizeof_read);
		
	    if(( nn = iri->top->read_state ) < 0) {
	      iri->top->bytes_left = 0;

	      if( ++err_count < 4 ) {
		printf("Last read: %d errno: %d\n"
		       , iri->top->read_state, err_count);
		dd_stats->rec_count++;  
		continue;
	      }
	      dd_input_read_close(iri);
	      /*
	       * see if there is another file to read
	       */
	      if(aa = dd_next_source_dev_name((char *)"NEX")) {
		current_file_name = aa;
		if((ii = dd_input_read_open(iri, aa)) <= 0) {
		  return(NULL);
		}
		err_count = 0;
		eof_count = 0;
		continue;
	      }
	      return(NULL);

	    } /* if( ( nn = iri->top->read_state ) < 0) */

	    else if(iri->top->eof_flag) {
	      nui->new_vol = nui->vol_count_flag = YES;
	      gri->vol_num++;
	      dd_stats->file_count++;
	      printf("EOF: %d at %.2f MB\n"
		     , dd_stats->file_count, dd_stats->MB);
	      if( ++eof_count > 2 ) {
		iri->top->bytes_left = 0;
		dd_input_read_close(iri);
		/*
		 * see if there is another file to read
		 */
		if(aa = dd_next_source_dev_name((char *)"NEX")) {
		  current_file_name = aa;
		  if((ii = dd_input_read_open(iri, aa)) <= 0) {
		    return(NULL);
		  }
		  err_count = eof_count = 0;
		  continue;
		}
		return(NULL);
	      }	/* if( ++eof_count > 2 ) */

	      continue;
	    } /* else if(iri->top->eof_flag) */

	    eof_count = err_count = 0;
	    dd_stats->rec_count++;  

	 } /* if(iri->top->bytes_left < iri->min_block_size) { */


	if( header_plus > 0 ) {
	  bytes_used = NEX_PACKET_SIZE - header_plus;
	  header_plus = 0;
	}
	else if( bytes_left > 0 ) {
	  at = tmp_at;
	  bytes_used = NEX_PACKET_SIZE - bytes_left;
	  memcpy( at +bytes_left, iri->top->at, 2 * NEX_PACKET_SIZE );
	}
	else {
	  at = iri->top->at;
	  bytes_used = NEX_PACKET_SIZE;
	}
	
	if((strncmp(at, arch2, strlen(arch2)) == 0) || 
	   (strncmp(at, arch2_2,strlen(arch2)) == 0)) { /* start of a volume */

	    nn = sizeof(struct nexrad_vol_scan_title);

	    if( bytes_left > 0 ) {
		if(( header_plus = bytes_left -nn ) > 0 ) {
		    /* we have the header and a partial packet in tmp_at */
		    at = tmp_at + nn;
		}
		else {
		    iri->top->at += nn -bytes_left;
		    iri->top->bytes_left -= nn -bytes_left;
		    header_plus = 0;
		}
	    }
	    else {
		iri->top->at += nn; iri->top->bytes_left -= nn;
	    }
	  continue;
	}

# ifdef obsolete
	if( cal_count ) {
	  if( ++cal_count <= cal_packets ) {
	    continue;
	  }
	  cal_count = 0;
	}
# endif
	ctm = (struct CTM_info *)at;
	if (prior_ctm) {
# ifdef obsolete
	   mm = (int)(.1*NEX_PACKET_SIZE);
	   if (memcmp (ctm, prior_ctm, sizeof (*ctm))) {
	      bytes_used += sizeof (*ctm);
	      ezhxdmp(last_packet, 0, mm);
	      printf ("\n***** Data is garbage at ray %d\n\n", count);
	      ezhxdmp(at, 0, mm);
	      return (NULL);
	   }
# endif
	}
	else {
	   prior_ctm = (struct CTM_info *)malloc (sizeof (*ctm));
	}
	memcpy (prior_ctm, ctm, sizeof (*ctm));
	last_packet = at;
	at += sizeof(struct CTM_info);

	iri->top->at += bytes_used;
	iri->top->bytes_left -= bytes_used;

	if(hostIsLittleEndian()) {
//	  nex_crack_nmh(at, nmh, (int)0);  //Jul 26, 2011
	  nex_crack_nmh(at, (char *)nmh, (int)0);  //Jul 26, 2011
	  at += sizeof(struct nexrad_message_header);
//	  nex_crack_drdh(at, drdh, (int)0);  //Jul 26, 2011
	  nex_crack_drdh(at, (char *)drdh, (int)0);  //Jul 26, 2011
	}
	else {
	  nmh = (struct nexrad_message_header *)at;
	  at += sizeof(struct nexrad_message_header);
	  drdh = (struct digital_radar_data_header *)at;
	}

	dp = dd_return_next_packet(iri);
	dp->len = packet_size;

	message_type = nmh->message_type;

	if( message_type == DIGITAL_RADAR_DATA ) {
	  break;
	}

# ifdef obsolete
	if( message_type == UNDEFINED_CLUTTER_DATA ) {
	  /*
	   * calibration data (message type 15) 
	   * get set to blast thru 36 packets
	   */
	  cal_count = 1;
	}
# endif
	/* not a useful packet */

    } /* for(;1;) */

    return( at );
}
/* c------------------------------------------------------------------------ */

int 
nex_next_ray (void)
{
    int ii, nn, break_out, message_type, outermost_loop = YES;
    int pn, eof_count=0, mark, somethings_changed=NO, sizeof_packet;
    int err_count = 0;
    static int count=0, trip=111;
    static int zeroes[] = { 0,0,0,0 };
    char *at, *aa, *bb;
    struct nexrad_VCP_items *nvi;
    struct io_packet *dp;
    void *vpt;
    /* c...mark */


    if( nui->options & NEX_NCDC_BLOCKING || iri->io_type == BINARY_IO ) {

      if(( at = nex_next_block() ) == NULL )
	{ return( -1 ); }

      outermost_loop = NO;
    }

    for(;outermost_loop;) {	/* find next ray */
	count++;
	if(count >= trip) {
	    mark = 0;
	}
	if(dd_control_c()) {
	    dd_reset_control_c();
	    return(-1);
	}

	if(iri->top->bytes_left < NEX_PACKET_SIZE) {
	    for(;2;) {		/* get to next record */
		if(dd_control_c()) {
		    dd_reset_control_c();
		    return(-1);
		}
		dd_logical_read(iri, FORWARD);
		
		if( ( nn = iri->top->read_state ) < 0) {
		   iri->top->bytes_left = 0;
		   if( ++err_count < 4 ) {
		      printf("Last read: %d errno: %d\n"
			     , iri->top->read_state, err_count);
		      continue;
		   }
		   dd_input_read_close(iri);
		   /*
		    * see if there is another file to read
		    */
		   if(aa = dd_next_source_dev_name((char *)"NEX")) {
		      current_file_name = aa;
		      if((ii = dd_input_read_open(iri, aa)) <= 0) {
			 return(nn);
		      }
		      err_count = 0;
		      eof_count = 0;
		      continue;
		   }
		   return(nn);
		}
		else if(iri->top->eof_flag) {
		    nui->new_vol = nui->vol_count_flag = YES;
		    gri->vol_num++;
		    dd_stats->file_count++;
		    printf("EOF: %d at %.2f MB\n"
			   , dd_stats->file_count, dd_stats->MB);
		    if( ++eof_count > 2 ) {
		       iri->top->bytes_left = 0;
		       dd_input_read_close(iri);
		       /*
			* see if there is another file to read
			*/
		       if(aa = dd_next_source_dev_name((char *)"NEX")) {
			  current_file_name = aa;
			  if((ii = dd_input_read_open(iri, aa)) <= 0) {
			     return(-1);
			  }
			  err_count = eof_count = 0;
			  continue;
		       }
		       return(-1);
		    }
		}
		else if(iri->top->sizeof_read < NEX_PACKET_SIZE) {
		    err_count = 0;
		    eof_count = 0;
		    dd_stats->rec_count++;
		    if(iri->top->sizeof_read == sizeof(struct nexrad_id_rec))
			  memcpy(&nir, iri->top->buf
				 , sizeof(struct nexrad_id_rec));
		    else if(iri->top->sizeof_read ==
			    sizeof(struct nexrad_vol_scan_title))
			  memcpy(&nvst, iri->top->buf
				 , sizeof(struct nexrad_vol_scan_title));
		}
		else if( !memcmp( iri->top->buf, (char *)&zeroes, 16 )) {
		   /* record of all zeroes */
		   iri->top->bytes_left = 0;
		   continue;
		}
		else {
		    eof_count = 0;
		    err_count = 0;
		    dd_stats->rec_count++;
		    dd_stats->MB += BYTES_TO_MB(iri->top->sizeof_read);
		    if(nui->options & RAP_NEXRAD ) {
		       sizeof_packet = 2008;
		       iri->top->bytes_left = 10 * sizeof_packet;		       iri->top->bytes_left = 10 * sizeof_packet;
		       /* record size is really 20480 bytes, but we need
			* to stop processing this physical record after 10 rays
			*/
		    }
		    if(*iri->top->buf == 'A' &&
		       iri->top->sizeof_read > 30000) {
		       aa = iri->top->buf;
		       bb = (char *)"ARCHIVE2";
		       if(strncmp(aa, bb, strlen(bb)) == 0) {
			  aa += strlen(bb);
			  if(strcmp(gri->radar_name, "UNKNOWN") == 0) {
			     str_terminate(gri->radar_name, aa, 4);

			     nn = sizeof (nex_sites_ptrs)/sizeof (char *);
			     dd_strings_to_slm (nex_sites_ptrs, slm, nn);
			     if(!nex_nab_site_info(slm)) {
				exit(1);
			     }
			  }
		       }
		       continue;
		    }
		    break;

		} /* else */

	    } /* for(;2;) */

	} /* if(iri->top->bytes_left < NEX_PACKET_SIZE) { */

	break_out = NO;
	dp = dd_return_next_packet(iri);
	at = dp->at;
	if(nui->options & RAP_NEXRAD ) {
	   if(hostIsLittleEndian()) {
//	      nex_crack_drdh(at, drdh, (int)0);  //Jul 26, 2011
	      nex_crack_drdh(at, (char *)drdh, (int)0);  //Jul 26, 2011
	   }
	   else {
	      drdh = (struct digital_radar_data_header *)at;
	   }
	   message_type = DIGITAL_RADAR_DATA;
	   sizeof_packet = 2008;
	}
	else {
	   at += sizeof(struct CTM_info);
	   if(hostIsLittleEndian()) {
//	      nex_crack_nmh(at, nmh, (int)0);  //Jul 26, 2011
	      nex_crack_nmh(at, (char *)nmh, (int)0);  //Jul 26, 2011
	      at += sizeof(struct nexrad_message_header);
//	      nex_crack_drdh(at, drdh, (int)0);  //Jul 26, 2011
	      nex_crack_drdh(at, (char *)drdh, (int)0);  //Jul 26, 2011
	   }
	   else {
	      nmh = (struct nexrad_message_header *)at;
	      at += sizeof(struct nexrad_message_header);
	      drdh = (struct digital_radar_data_header *)at;
	   }
	   message_type = nmh->message_type;
	   sizeof_packet = NEX_PACKET_SIZE;
	}

	switch(message_type) {

	case SITE_ADAPTATION_DATA:
	case ONLINE_RECEIVER_CAL:
	case SUN_CHECK_CAL_DATA:
	case SUPPLEMENTAL_VOLUME_DATA:
	case SUPPLEMENTAL_SWEEP_DATA:
	case 202:
	    break;

	case DIGITAL_RADAR_DATA:
	    break_out = YES;
	    break;

	case NEXRAD_EXTENDED_FORMAT:
	    nxh = (struct nexrad_extended_header *)
		  (at + drdh->extended_header_ptr);
	    sizeof_packet = nxh->bytes_in_this_packet;
	    break_out = YES;
	    break;
# ifdef obsolete
	case 15:
	    nex_process_cal_info();
	    break;
# endif
	default:
	    break;
	 }
	dp->len = sizeof_packet;
	iri->top->bytes_left -= sizeof_packet;
	iri->top->at += sizeof_packet;
	if(break_out)
	      break;
	if( message_type == RDA_STATUS_DATA ) {
	   if(hostIsLittleEndian()) {
//	      nex_crack_rda(at, rda_stat, (int)0);  //Jul 26, 2011
	      nex_crack_rda(at, (char *)rda_stat, (int)0);  //Jul 26, 2011
	   }
	   else {
# ifdef obsolete
	      memcpy( rda_stat, at, sizeof( struct rda_status_info ));
# else
	      rda_stat = (struct rda_status_info *)at;
# endif
	   }
	   continue;
	}
	iri->top->bytes_left = 0;

    } /* for(;outermost_loop;) */



    dd_stats->ray_count++;
    gri->time = nexrad_time_stamp((int)drdh->julian_date
				  , drdh->milliseconds_past_midnight
				  , gri->dts);
    gri->corrected_rotation_angle = gri->rotation_angle = 
	  gri->azimuth = BA2F(drdh->azimuth);
    gri->elevation = BA2F(drdh->elevation);

    if(drdh->ref_ptr <= 0 ||
       (drdh->ref_ptr > 0 && drdh->ref_ptr == drdh->vel_ptr)) {
	/* funky tapes from Atlanta; do nothing for now! */
	nui->ref_ptr = NULL;
    }
    else {
	nui->ref_ptr = at + drdh->ref_ptr;
    }
    nui->vel_ptr = drdh->vel_ptr > 0 ? at + drdh->vel_ptr : NULL;
    nui->sw_ptr = drdh->sw_ptr > 0 ? at + drdh->sw_ptr : NULL;

    nui->previous_parameters = nui->current_parameters;
    nui->current_parameters = nui->ref_ptr ? NEX_DZ_ID : 0;

    if(nui->vel_ptr)
	  nui->current_parameters |= NEX_VE_ID;
    if(nui->sw_ptr)
	  nui->current_parameters |= NEX_SW_ID;

    if(nui->current_parameters != nui->previous_parameters) {
	nui->new_sweep = YES;
	nex_new_parms();
    }

    if(nui->new_vol || drdh->radial_status == BEGINNING_OF_VOL_SCAN) {
	gri->vol_num++;
	nui->vol_count_flag = nui->sweep_count_flag =
	      nui->new_sweep = YES;
	dd_stats->vol_count++;
    }
    if(drdh->radial_status == START_OF_NEW_ELEVATION) {
	nui->sweep_count_flag = nui->new_sweep = YES;
    }    
    if(nui->new_sweep) {
	/* c...mark */
	gri->source_sweep_num = drdh->elev_num;
	gri->sweep_num++;
	dd_stats->sweep_count++;
	nvi = current_nvh->swp_info +drdh->elev_num-1;
	gri->fixed = nvi->fixed_angle*NEX_FIXED_ANGLE;
	gri->sweep_speed = nvi->az_rate*NEX_AZ_RATE;

	gri->freq[0] = nss->frequency_mhz != -999
	      ? nss->frequency_mhz*1.e6 : -999;
	gri->num_ipps = 0;
//	gri->radar_constant = nex_conc_to_fp(&drdh->sys_gain_cal_const);
	gri->radar_constant = nex_conc_to_fp(&drdh->sys_gain_cal_const);
	gri->ant_gain = .001 * drdh->atmos_atten_factor_x1000;

	if(drdh->ref_ptr > 0 && drdh->ref_ptr == drdh->vel_ptr) {
	    /* funky tapes from Atlanta; do nothing for now! */
	    mark = 0;
	}
	else if(drdh->ref_ptr > 0) {
	    gri->unamb_range =
		  nex_unamb_rngs[current_delta][nvi->is_prf_num-1];
	    gri->num_samples = nvi->is_pulse_count;
	    gri->PRF = gri->unamb_range > 0
		  ? 0.5*SPEED_OF_LIGHT/(gri->unamb_range*1000.) : 0;
	    gri->ipps[gri->num_ipps] = gri->PRF > 0
		  ? 1./gri->PRF : -999;
	    gri->num_ipps++;
	}
	if(drdh->vel_ptr > 0) {
	    gri->unamb_range =
		  nex_unamb_rngs[current_delta][nvi->id_prf_num-1];
	    gri->num_samples = nvi->id_pulse_count;
	    gri->PRF = gri->unamb_range > 0
		  ? 0.5*SPEED_OF_LIGHT/(gri->unamb_range*1000.) : 0;
	    gri->ipps[gri->num_ipps] = gri->PRF > 0
		  ? 1./gri->PRF : -999;
	    gri->num_ipps++;
	}
	if(nui->nyquist_vel <= 0) {
	    if(drdh->nyquist_vel_x100 > 0)
		  gri->nyquist_vel = drdh->nyquist_vel_x100*.01;
	    else
		  gri->nyquist_vel = 0.25*gri->PRF*gri->wavelength;
	}
	mark = 0;
    }
    return(NEX_PACKET_SIZE);
}
/* c------------------------------------------------------------------------ */

int 
nex_process_cal_info (void)
{
    int ii;
    char *at;

    for( ii = 0; ii < 36; ii++, iri->top->at += NEX_PACKET_SIZE ) {
	at = iri->top->at + sizeof(struct CTM_info);
	nmh = (struct nexrad_message_header *)at;
	at += sizeof(struct nexrad_message_header);
    }
}
/* c------------------------------------------------------------------------ */

void 
nex_update_xtnded_headers (void)
{
    int ii, pn, nn, mark, somethings_changed=NO;
    struct nex_unique_field_info *nufi;
    struct nexrad_extended_header *nexh;
    struct nexrad_field_header *nfh;
    char *aa, *bb, *at=(char*)drdh;

    nexh = (struct nexrad_extended_header *)(at +drdh->extended_header_ptr);
    nfh = (struct nexrad_field_header *)(at +nexh->first_field_header_ptr);
    nui->num_fields = nexh->num_fields;

    for(pn=0; pn < nui->num_fields; pn++) {
	nufi = &nui->nufi[pn];

	aa = (char *)nfh;
	bb = (char *)&nufi->nfh;
	nn = sizeof(struct nexrad_field_header);

	for(; nn-- && *aa++ != *bb++;); /* compare */
	if(nn) somethings_changed = YES;
	memcpy(&nufi->nfh, nfh, sizeof(struct nexrad_field_header));

	nfh = (struct nexrad_field_header *)
	      (at +nfh->ptr_to_next_header);
    }
    if(somethings_changed) {
	nexx_new_parms();
	nui->new_sweep = YES;
    }
}
/* c------------------------------------------------------------------------ */

void 
nex_fake_xtnded_headers (void)
{
    /* this routine fakes the first nexrad field header struct for data
     * where these structs are not present
     */
    int ii, nn, mark, somethings_changed=NO;
    struct nexrad_field_header nfh;
    struct nex_unique_field_info *nufi;
    char *aa, *bb;

    nui->num_fields = 3;
    /*
     * check the reflectivity
     */
    nufi = &nui->nufi[0];
    aa = (char *)&nfh; bb = (char *)&nufi->nfh;
    memcpy(aa, bb, sizeof(struct nexrad_field_header));
    strcpy(nfh.name, "DZ");
    nfh.scale_mult = 100;
    nfh.bias_mult = 100;
    if(drdh->ref_ptr <= 0 ||
       (drdh->ref_ptr > 0 && drdh->ref_ptr == drdh->vel_ptr)) {
	/* funky tapes from Atlanta;
	 */
	nfh.data_ptr = 0;
	nufi->data_ptr = NULL;
    }
    else {
	nfh.data_ptr = drdh->ref_ptr;
	nufi->data_ptr = (char *)drdh +drdh->ref_ptr;
    }
    nfh.num_gates = drdh->ref_num_gates;
    nfh.bytes_per_gate = 1;
    nfh.scale = (int32_t)S100(2.);
    nfh.bias = (int32_t)S100(32.);
    nfh.km_unambiguous_range_x100 = drdh->unamb_range_x10 * 10;
    nfh.meters_unambiguous_vel_x100 = drdh->nyquist_vel_x100;
    nfh.meters_to_gate1 = drdh->ref_gate1;
    nfh.meters_per_gate = drdh->ref_gate_width;
    nn = sizeof(struct nexrad_field_header);
    for(; nn-- && *aa++ != *bb++;); /* compare */
    if(nn) somethings_changed = YES;
    memcpy(&nufi->nfh, &nfh, sizeof(struct nexrad_field_header));
    /*
     * check the velocity
     */
    nufi = &nui->nufi[1];
    aa = (char *)&nfh; bb = (char *)&nufi->nfh;
    memcpy(aa, bb, sizeof(struct nexrad_field_header));
    nfh.scale_mult = 100;
    nfh.bias_mult = 100;
    strcpy(nfh.name, "VE");
    nfh.data_ptr = drdh->vel_ptr;
    nufi->data_ptr = drdh->vel_ptr > 0 ? (char *)drdh +drdh->vel_ptr : NULL;
    nfh.num_gates = drdh->vel_num_gates;
    nfh.bytes_per_gate = 1;
    if(drdh->velocity_resolution == ONE_METER_PER_SEC) {
	nfh.scale = (int32_t)S100(1.);
	nfh.bias = (int32_t)S100(127.);
    }
    else {
	nfh.scale = (int32_t)S100(2.);
	nfh.bias = (int32_t)S100(63.5);
    }
    nfh.km_unambiguous_range_x100 = drdh->unamb_range_x10 * 10;
    nfh.meters_unambiguous_vel_x100 = drdh->nyquist_vel_x100;
    nfh.meters_to_gate1 = drdh->vel_gate1;
    nfh.meters_per_gate = drdh->vel_gate_width;
    nn = sizeof(struct nexrad_field_header);
    for(; nn-- && *aa++ != *bb++;); /* compare */
    if(nn) somethings_changed = YES;
    memcpy(&nufi->nfh, &nfh, sizeof(struct nexrad_field_header));
    /*
     * check the spectral width
     */
    nufi = &nui->nufi[2];
    aa = (char *)&nfh; bb = (char *)&nufi->nfh;
    memcpy(aa, bb, sizeof(struct nexrad_field_header));
    nfh.scale_mult = 100;
    nfh.bias_mult = 100;
    strcpy(nfh.name, "SW");
    nfh.data_ptr = drdh->sw_ptr;
    nufi->data_ptr = drdh->sw_ptr > 0 ? (char *)drdh +drdh->sw_ptr : NULL;
    nfh.num_gates = drdh->vel_num_gates;
    nfh.bytes_per_gate = 1;
    nfh.scale = (int32_t)S100(2.);
    nfh.bias = (int32_t)S100(63.5);
    nfh.km_unambiguous_range_x100 = drdh->unamb_range_x10 * 10;
    nfh.meters_unambiguous_vel_x100 = drdh->nyquist_vel_x100;
    nfh.meters_to_gate1 = drdh->vel_gate1;
    nfh.meters_per_gate = drdh->vel_gate_width;
    nn = sizeof(struct nexrad_field_header);
    for(; nn-- && *aa++ != *bb++;); /* compare */
    if(nn) somethings_changed = YES;
    memcpy(&nufi->nfh, &nfh, sizeof(struct nexrad_field_header));

    if(somethings_changed) {
	nexx_new_parms();
	nui->new_sweep = YES;
    }
}
/* c------------------------------------------------------------------------ */

void 
nex_positioning (void)
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
    struct io_packet *dp, *dp0;

    preamble[0] = '\0';
    gri_interest(gri, 1, preamble, postamble);
    if (!slm)
      { slm = solo_malloc_list_mgmt(128); }


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
    nui->new_sweep = NO;
    nui->new_vol = NO;

    if(ival == -1) {
	solo_unmalloc_list_mgmt(slm);
	return;
    }
    else if(ival < 0)
	  exit(0);
    else if(ival == 0) {
	printf("Type number of rays to skip:");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > 65536) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	if(ival > 0) {
	    for(ii=0; ii < ival; ii++) {
		nui->new_sweep = NO;
		nui->new_vol = NO;
		if((jj=nex_next_ray()) < 0)
		      break;
	    }
	}
	else if(ival < 0) {
	    /* figure out the number of rays we can really back up
	     */
	    nn = -ival > iri->packet_count ? iri->packet_count-1 : -ival;
	    dp0 = dp = dd_return_current_packet(iri);
	    for(ii=0; ii < nn; ii++, dp=dp->last) {
		/* see if the sequence num for this packet
		 * agrees with the sequence num for the input buf
		 * if so, the data has not been stomped on
		 */
		if(!dp->ib || dp->seq_num != dp->ib->seq_num)
		      break;
	    }
	    if(ii) {
		dp = dp->next;	/* loop pushed us one too far */
		/*
		 * figure out how many buffers to skip back
		 */
		kk = ((dp0->ib->b_num - dp->ib->b_num + iri->que_count)
		      % iri->que_count) +1;
		dd_skip_recs(iri, BACKWARD, kk);
		iri->top->at = dp->at;
		iri->top->bytes_left = dp->bytes_left;
	    }
	    if(ii < -ival) {
		/* did not achive goal */
		printf("Unable to back up %d rays. Backed up %d rays\n"
		       , -ival, ii);
		printf("Use record positioning to back up further\n");
	    }
	}
	nex_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 1) {
	if((nn = nex_inventory(iri)) == -2)
	      exit(0);
    }
    else if(ival == 2) {
	printf("Type skip: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > 65536) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	if(ival) {
	    direction = ival >= 0 ? FORWARD : BACKWARD;
	    dd_skip_files(iri, direction, ival >= 0 ? ival : -ival);
	}
	nex_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 3) {
	dd_rewind_dev(iri);
	nex_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 4) {
	if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
	    printf("\n");
	    ctypeu16((unsigned char *)iri->top->buf, view_char, view_char_count);  //Jul 26, 2011
	}
    }
    else if(ival == 5) {
	if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
	    printf("\n");
	    ezhxdmp(iri->top->buf, view_char, view_char_count);
	}
    }
    else if(ival == 6) {
	if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
	    printf("\n");
	    ezascdmp(iri->top->buf, view_char, view_char_count);
	}
    }
    else if(ival == 7) {
	printf("Type record skip # or hit <return> to read next rec:");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > 65536) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	direction = ival >= 0 ? FORWARD : BACKWARD;
	dd_skip_recs(iri, direction, ival > 0 ? ival : -ival);
	nex_next_ray();
	nn = iri->top->sizeof_read;
	printf("\n Read %d bytes\n", nn);
	gri_interest(gri, 1, preamble, postamble);
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
	    if((nn = nex_next_ray()) <= 0 || gri->time >= dtarget ||
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
	    if((nn = nex_next_ray()) <= 0 || in_sector(gri->fixed, fx1, fx2)
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
	nex_print_headers(slm);
    }
    else if(ival == 12) {
	nex_dump_this_ray(slm);
    }

    preamble[0] = '\0';

    goto menu2;


}
/* c------------------------------------------------------------------------ */

void 
nex_print_dhed (struct digital_radar_data_header *dh, struct solo_list_mgmt *slm)
{
    int ii;
    char *aa, *bb, str[256];
    DD_TIME dts;
    double d;

    aa = str;

    d = nexrad_time_stamp((int)dh->julian_date, dh->milliseconds_past_midnight
			  , &dts);
    d_unstamp_time(&dts);

    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of digital radar data header");
    solo_add_list_entry(slm, aa);

    sprintf(aa, "milliseconds_past_midnight %d", dh->milliseconds_past_midnight);
    solo_add_list_entry(slm, aa);
    sprintf(aa, "julian_date                %d  %s"
	    , dh->julian_date, dts_print(&dts));               
    solo_add_list_entry(slm, aa);
    sprintf(aa, "unamb_range_x10            %d", dh->unamb_range_x10);           
    solo_add_list_entry(slm, aa);
    sprintf(aa, "azimuth                    %d  %.2f"
	    , dh->azimuth, (float)BA2F(dh->azimuth));                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "radial_num                 %d", dh->radial_num);                
    solo_add_list_entry(slm, aa);
    sprintf(aa, "radial_status              %d", dh->radial_status);             
    solo_add_list_entry(slm, aa);
    sprintf(aa, "elevation                  %d  %.2f"
	    , dh->elevation, (float)BA2F(dh->elevation));                 
    solo_add_list_entry(slm, aa);
    sprintf(aa, "elev_num                   %d", dh->elev_num);                  
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ref_gate1                  %d", dh->ref_gate1);                 
    solo_add_list_entry(slm, aa);
    sprintf(aa, "vel_gate1                  %d", dh->vel_gate1);                 
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ref_gate_width             %d", dh->ref_gate_width);            
    solo_add_list_entry(slm, aa);
    sprintf(aa, "vel_gate_width             %d", dh->vel_gate_width);            
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ref_num_gates              %d", dh->ref_num_gates);             
    solo_add_list_entry(slm, aa);
    sprintf(aa, "vel_num_gates              %d", dh->vel_num_gates);             
    solo_add_list_entry(slm, aa);
    sprintf(aa, "sector_num                 %d", dh->sector_num);                
    solo_add_list_entry(slm, aa);
    d = nex_conc_to_fp(&dh->sys_gain_cal_const);
    sprintf(aa, "sys_gain_cal_const         %f", d);        
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ref_ptr                    %d", dh->ref_ptr);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "vel_ptr                    %d", dh->vel_ptr);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "sw_ptr                     %d", dh->sw_ptr);                    
    solo_add_list_entry(slm, aa);
    sprintf(aa, "velocity_resolution        %d", dh->velocity_resolution);       
    solo_add_list_entry(slm, aa);
    sprintf(aa, "vol_coverage_pattern       %d", dh->vol_coverage_pattern);      
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_38                    %d", dh->VNV1);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_39                    %d", dh->VNV2);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_40                    %d", dh->VNV3);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_41                    %d", dh->VNV4);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ref_data_playback          %d", dh->ref_data_playback);         
    solo_add_list_entry(slm, aa);
    sprintf(aa, "vel_data_playback          %d", dh->vel_data_playback);         
    solo_add_list_entry(slm, aa);
    sprintf(aa, "sw_data_playback           %d", dh->sw_data_playback);          
    solo_add_list_entry(slm, aa);
    sprintf(aa, "nyquist_vel_x100           %d", dh->nyquist_vel_x100);          
    solo_add_list_entry(slm, aa);
    sprintf(aa, "atmos_atten_factor_x1000   %d", dh->atmos_atten_factor_x1000);  
    solo_add_list_entry(slm, aa);
    sprintf(aa, "threshold_parameter        %d", dh->threshold_parameter);       
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_48                    %d", dh->word_48);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_49                    %d", dh->word_49);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_50                    %d", dh->word_50);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_51                    %d", dh->word_51);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_52                    %d", dh->word_52);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_53                    %d", dh->word_53);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_54                    %d", dh->word_54);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_55                    %d", dh->word_55);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_56                    %d", dh->word_56);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_57                    %d", dh->word_57);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_58                    %d", dh->word_58);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_59                    %d", dh->word_59);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_60                    %d", dh->word_60);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_61                    %d", dh->word_61);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_62                    %d", dh->word_62);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_63                    %d", dh->word_63);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "word_64                    %d", dh->extended_header_ptr);                   
    solo_add_list_entry(slm, aa);

    return;
}
/* c------------------------------------------------------------------------ */

void 
nex_print_headers (struct solo_list_mgmt *slm)
{
    int ii, jj;
    char *aa, str[128], bstr[128];


    aa = bstr;
    solo_reset_list_entries(slm);
    solo_add_list_entry(slm, " ");

    sprintf(aa, "filename[8] %s", str_terminate(str, nir.filename, 8));
    solo_add_list_entry(slm, aa);

    nex_print_nvst(&nvst, slm);
    nex_print_nmh(nmh, slm);
    nex_print_dhed(drdh, slm);
    nex_print_rda(rda_stat, slm);

    slm_print_list(slm);
}
/* c------------------------------------------------------------------------ */

void 
nex_print_nmh (struct nexrad_message_header *mh, struct solo_list_mgmt *slm)
{
    int ii;
    char *aa, str[128];

    DD_TIME dts;
    double d;

    d = nexrad_time_stamp((int)mh->julian_date, mh->milliseconds_past_midnight
			  , &dts);
    d_unstamp_time(&dts);
    aa = str;

    solo_add_list_entry(slm, " ");
    // NOTE: Isn't the value of aa undefined here?
    solo_add_list_entry(slm, aa);
    sprintf(aa, "Contents of nexrad message header");
    solo_add_list_entry(slm, aa);

    sprintf(aa, "message_len                %d", mh->message_len);               
    solo_add_list_entry(slm, aa);
    sprintf(aa, "channel_id                 %d", mh->channel_id);              
    solo_add_list_entry(slm, aa);
    sprintf(aa, "message_type               %d", mh->message_type);              
    solo_add_list_entry(slm, aa);
    sprintf(aa, "seq_num                    %d", mh->seq_num);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "julian_date                %d  %s"
	    , mh->julian_date, dts_print(&dts));               
    solo_add_list_entry(slm, aa);
    sprintf(aa, "milliseconds_past_midnight %d"
	    , mh->milliseconds_past_midnight);
    solo_add_list_entry(slm, aa);
    sprintf(aa, "num_message_segs           %d", mh->num_message_segs);          
    solo_add_list_entry(slm, aa);
    sprintf(aa, "message_seg_num            %d", mh->message_seg_num);           
    solo_add_list_entry(slm, aa);

    return;
}
/* c------------------------------------------------------------------------ */

void 
nex_print_rda (struct rda_status_info *rda, struct solo_list_mgmt *slm)
{
   int ii;
   char *aa, str[128];
   if (!rda)
     { return; }

   aa = str;

   solo_add_list_entry(slm, " ");
   // NOTE: Isn't the vaiue of aa here undefined?
   solo_add_list_entry(slm, aa);
   sprintf(aa, "Contents of nexrad rda status info header");
   solo_add_list_entry(slm, aa);

   sprintf( aa, "rda_status         %d", rda->rda_status );       
   solo_add_list_entry( slm, aa);
   sprintf( aa, "oper_status        %d", rda->oper_status );      
   solo_add_list_entry( slm, aa);
   sprintf( aa, "cntrl_status       %d", rda->cntrl_status );     
   solo_add_list_entry( slm, aa);
   sprintf( aa, "aux_pwr_gen_state  %d", rda->aux_pwr_gen_state );
   solo_add_list_entry( slm, aa);
   sprintf( aa, "atp                %d", rda->atp );              
   solo_add_list_entry( slm, aa);
   sprintf( aa, "ref_cal_corr       %d", rda->ref_cal_corr );     
   solo_add_list_entry( slm, aa);
   sprintf( aa, "dte                %d", rda->dte );              
   solo_add_list_entry( slm, aa);
   sprintf( aa, "vcp                %d", rda->vcp );              
   solo_add_list_entry( slm, aa);
   sprintf( aa, "rds_cntl_auth      %d", rda->rds_cntl_auth );    
   solo_add_list_entry( slm, aa);
   sprintf( aa, "intefr_det_rate    %d", rda->intefr_det_rate );  
   solo_add_list_entry( slm, aa);
   sprintf( aa, "op_mode            %d", rda->op_mode );          
   solo_add_list_entry( slm, aa);
   sprintf( aa, "intefr_suppr_unit  %d", rda->intefr_suppr_unit );
   solo_add_list_entry( slm, aa);
   sprintf( aa, "arch2status        %d", rda->arch2status );      
   solo_add_list_entry( slm, aa);
   sprintf( aa, "arch2vols          %d", rda->arch2vols );        
   solo_add_list_entry( slm, aa);
   sprintf( aa, "rda_alarms         %d", rda->rda_alarms );       
   solo_add_list_entry( slm, aa);
   sprintf( aa, "command_ak         %d", rda->command_ak );       
   solo_add_list_entry( slm, aa);
   sprintf( aa, "ch_cntrl_stat      %d", rda->ch_cntrl_stat );    
   solo_add_list_entry( slm, aa);
   sprintf( aa, "spol_blnk_stat     %d", rda->spol_blnk_stat );   
   solo_add_list_entry( slm, aa);
   sprintf( aa, "bypass_map_date    %d", rda->bypass_map_date );  
   solo_add_list_entry( slm, aa);
   sprintf( aa, "bypass_map_time    %d", rda->bypass_map_time );  
   solo_add_list_entry( slm, aa);
   sprintf( aa, "notch_map_date     %d", rda->notch_map_date );   
   solo_add_list_entry( slm, aa);
   sprintf( aa, "notch_map_time     %d", rda->notch_map_time );   
   solo_add_list_entry( slm, aa);
   sprintf( aa, "tps_stat           %d", rda->tps_stat );         
   solo_add_list_entry( slm, aa);
   sprintf( aa, "spare1             %d", rda->spare1 );           
   solo_add_list_entry( slm, aa);
   sprintf( aa, "spare2             %d", rda->spare2 );           
   solo_add_list_entry( slm, aa);
   sprintf( aa, "spare3             %d", rda->spare3 );           
   solo_add_list_entry( slm, aa);

   for( ii=0; ii < 14; ii++ ) {
      sprintf( aa, "alarm_codes[%2d]    %d", ii, rda->alarm_codes[ii] );  
      solo_add_list_entry( slm, aa);
   }


   return;
}
/* c------------------------------------------------------------------------ */

void 
nex_print_nvst (struct nexrad_vol_scan_title *vst, struct solo_list_mgmt *slm)
{
    int ii;
    char *aa, str[128], bstr[128];
    DD_TIME dts;
    double d;

    d = nexrad_time_stamp((int)vst->julian_date
			  , vst->milliseconds_past_midnight, &dts);
    d_unstamp_time(&dts);

    aa = bstr;
    solo_add_list_entry(slm, " ");

    sprintf(aa, "Contents of nexrad vol scan title");
    solo_add_list_entry(slm, aa);

    sprintf(aa, "filename[9]                %s"
	, str_terminate(str, vst->filename, 9));                  
    solo_add_list_entry(slm, aa);
    sprintf(aa, "extension[3]               %s"
	, str_terminate(str, vst->extension, 3));                 
    solo_add_list_entry(slm, aa);
    sprintf(aa, "julian_date                %d  %s"
	    , vst->julian_date, dts_print(&dts));               
    solo_add_list_entry(slm, aa);
    sprintf(aa, "milliseconds_past_midnight %d"
	, vst->milliseconds_past_midnight);
    solo_add_list_entry(slm, aa);
    sprintf(aa, "filler_1                   %d", vst->filler_1);                  
    solo_add_list_entry(slm, aa);

    return;
}
/* c------------------------------------------------------------------------ */

void 
nex_print_stat_line (int count)
{
    DD_TIME *dts=gri->dts;
    
    d_unstamp_time(dts);
    printf(" %5d %3d %7.2f %.2f  %s\n"
	   , count
	   , gri->sweep_num
	   , gri->fixed
	   , dts->time_stamp
	   , dts_print(dts)
	   );
}
/* c------------------------------------------------------------------------ */

void 
nex_reset (void)
{
    nui->vol_count_flag = nui->sweep_count_flag =
	  nui->new_sweep = nui->new_vol = YES;
    nui->ray_count = nui->sweep_count = nui->vol_count = 0;
    /* make sure to start a new sweep and vol
     */
    gri->vol_num++;
    gri->sweep_num++;
    difs->stop_flag = NO;
}
/* c------------------------------------------------------------------------ */

int 
nex_select_ray (void)
{
    int ii, jj, mark;
    int ok=YES;


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

    if((nui->options & NEX_REFL_ONLY) && (!nui->ref_ptr)) {
	ok = NO;
    }
    else if((nui->options & NEX_VEL_ONLY) && (!nui->vel_ptr)) {
	ok = NO;
    }

    return(ok);
}
/* c------------------------------------------------------------------------ */

double 
nexrad_time_stamp (int jday, int ms, DD_TIME *dts)
{
    double d;

    dts->year = 1970;
    dts->month = dts->day = 0;
    dts->day_seconds = ms*.001 +(jday-1)*SECONDS_IN_A_DAY;
    d = d_time_stamp(dts);
    return(d);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

