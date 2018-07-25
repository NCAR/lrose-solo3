/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

#include <LittleEndian.hh>
#include <sys/types.h>

# define           BASE_DATA 0x2b00
# define   BASE_DATA_LOW_PRF 0x2b01
# define        TDWR_TAPE_ID 0x2b02

# define TDWR_UNSCALE(val, unscale, excess) ((val)*(unscale)+(excess))

# define   SNR_EXCESS 0
# define  SNR_UNSCALE 0.5
# define    SW_EXCESS 0
# define   SW_UNSCALE 0.25
# define     Z_EXCESS -30
# define    Z_UNSCALE 0.5
# define     V_EXCESS -80
# define    V_UNSCALE 0.25

/* scan strategies */
# define                       MONITOR 1
# define             HAZARDOUS_WEATHER 2
# define        CLEAR_AIR_REFLECTIVITY 3
# define       CLUTTER_SUPPRESSION_MAP 4
# define   CLUTTER_RESIDUE_EDITING_MAP 5
# define                      TDWR_PPI 6
# define                      TDWR_RHI 7
# define                    TDWR_POINT 8
# define                TDWR_SUN_TRACK 9

#define          NORMAL_PRF_RESOLUTION 150
#define             LOW_PRF_RESOLUTION 300

# ifdef obsolete
#define            RANGE_TO_FIRST_GATE 20
# else
#define            RANGE_TO_FIRST_GATE 450
# endif

#define                     BEAM_WIDTH 55
#define                    PULSE_WIDTH 1100
#define                      FREQUENCY 5600 /* megahertz */
#define                  LOW_PRF_GATES 600
#define               NORMAL_PRF_GATES 1988

# define  TDWR_NOMINAL_WAVE_LENGTH .05 /* meters */
# define      TDWR_HORZ_BEAM_WIDTH 1.0
# define      TDWR_VERT_BEAM_WIDTH 1.0
# define TDWR_NOMINAL_GATE_SPACING 150. /* meters */

# define                 VALID_FLAG_CV 0x40
# define  POINT_TARGET_FILTER_FLAG_CTF 0x04


struct tdwr_base_datum {
    unsigned int uz   : 8;
    unsigned int snr  : 8;
    unsigned int urv  : 16;
    unsigned int w    : 8;
    unsigned int xx   : 2;
    unsigned int caf  : 3;
    unsigned int ctf  : 1;
    unsigned int cvf  : 1;
    unsigned int ccv  : 1;
    unsigned int drv  : 16;
};

struct pw0 {
    unsigned int eov  : 1;
    unsigned int sov  : 1;
    int unused        : 6;
    unsigned int scan_strategy : 8;
};

struct pw1 {
    int unused   : 13;
    unsigned int dri  : 1;
    unsigned int sld  : 1;
    unsigned int spd  : 1;
};

struct pw2 {
    unsigned int scan_number : 8;
    unsigned int eoe         : 1;
    unsigned int soe         : 1;
    unsigned int crmn        : 3;
    int reserved             : 6;
    unsigned int of          : 1;
    unsigned int sr          : 1;
    unsigned int da          : 2;
    unsigned int ss          : 1;
    unsigned int ma          : 1;
    unsigned int pr          : 2;
    unsigned int ws          : 1;
    unsigned int le          : 1;
    unsigned int mb          : 1;
    unsigned int gf          : 1;
    unsigned int lp          : 1;
};

struct pw3 {
    unsigned int dwell_id  : 2;
    unsigned int si        : 1;
    unsigned int reserved  : 1;
    unsigned int pulses_per_dwell : 12;
};

struct pw4 {
    unsigned int reserved  : 11;
    unsigned int wi        : 1;
    unsigned int ivs       : 1;
    unsigned int ies       : 1;
    unsigned int vsr       : 1;
    unsigned int esr       : 1;
};


struct cbd_header {
    unsigned short message_id;
    unsigned short sizeof_message;
    unsigned short volume_scan_count;
    unsigned short pw0;
    unsigned short peak_xmittd_pwr_kw;
    unsigned short pw1;
    u_int32_t pw2;
    float current_elevation;
    float angular_scan_rate;
    unsigned short pri_xe6;	/* pulse repetition interval in microsec. */
    unsigned short pw3;
    unsigned short final_range_sample;
    unsigned short range_samples_per_dwell;
    float current_azimuth;
    float total_noise_power;
    u_int32_t time_stamp;
    unsigned short base_data_type;
    unsigned short pw4;
    unsigned short integer_azimuth;
    unsigned short load_shed_final_sample;
};

# define   TDWR_MAX_FIELDS 8

# define        TDWR_Z_LUT 0
# define      TDWR_SNR_LUT 1
# define      TDWR_VEL_LUT 2
# define       TDWR_SW_LUT 3

# define  BASE_RATE_INDEX 0
# define   LOW_RATE_INDEX 1

struct tdwr_for_each_radar {
    int new_vol;
    int radar_num;
    int vol_count;
    int vol_count_flag;
    int vol_num;
    char radar_name[12];
};

struct tdwr_useful_items {
    double sum_delta_elev;
    double sum_elev;
    double time;
    double ang_diff_sum;

    float first_elev;
    float last_elev;
    float nyquist_vel;
    float prev_rot_angle;
    int32_t trip_time;

    int assigned_dd_nums;
    int count_since_trip;
    int low_prf;
    int new_sweep;
    int ray_count;
    int sweep_count;
    int sweep_count_flag;
    int sweep_ray_count;
    int sweep_transition;
    int user_supplied_name;
    int vol_transition;

    short *lut[TDWR_MAX_FIELDS];
    struct tdwr_for_each_radar *tfer;
    struct tdwr_for_each_radar *tfer_list[2];
};

static struct tdwr_useful_items *tdui;
static struct tdwr_base_datum *tbd, *xtbd;
static struct cbd_header *cbdh, *xcbdh;
static struct pw0 pw0;
static struct pw1 pw1;
static struct pw2 pw2;
static struct pw3 pw3;
static struct pw4 pw4;

extern int Sparc_Arch;

/*
 * This file contains routines:
 * 
 * tdwr_dd_conv
 * 
 * tdwr_dump_this_ray
 * tdwr_ini
 * tdwr_inventory
 * tdwr_map_structs
 * tdwr_nab_data
 * tdwr_next_ray
 * tdwr_new_parms
 * tdwr_positioning
 * tdwr_print_cbdh
 * tdwr_print_stat_line
 * tdwr_reset
 * tdwr_select_ray
 * tdwr_setup_fields
 * 
 */


#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <dd_math.h>
#include <dd_defines.h>
#include <iostream>
#include "input_limits.hh"
#include "dd_stats.h"
#include "generic_radar_info.h"
#include "tdwr_dd.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "gneric_dd.hh"
#include "dorade_share.hh"
#include "dd_io_mgmt.hh"
#include "dd_crackers.hh"

static struct dd_input_filters *difs;
static struct dd_stats *dd_stats;
static struct generic_radar_info *gri;
static struct input_read_info *iri;
static int view_char=0, view_char_count=200;
static char preamble[24], postamble[64];
static struct solo_list_mgmt *slm;
static char *pbuf=NULL, **pbuf_ptrs=NULL;
static int pbuf_max=1200;

/* c------------------------------------------------------------------------ */

void 
tdwr_dd_conv (int interactive_mode)
{
    /* main loop for converting 88D data
     */
    int i, n, mark;
    static int count=0, nok_count;

    if(!count) {
	tdwr_ini();
    }

    if(interactive_mode) {
	dd_intset();
	tdwr_positioning();
    }
    tdwr_reset();

    for(;;) {
	count++;
	if(difs->stop_flag &&
	   (difs->abrupt_start_stop || tdui->new_sweep)) {
	    printf("\nBreak on stop time!\n");
	    break;
	}

	if(tdwr_select_ray()) {

	    if(difs->max_beams)
		  if(++tdui->ray_count > difs->max_beams ) {
		      printf("\nBreak on ray count!\n");
		      break;
		  }
	    if(tdui->sweep_count_flag) {
		tdui->sweep_count_flag = NO;
		if(difs->max_sweeps) {
		    if(++tdui->sweep_count > difs->max_sweeps ) {
			printf("\nBreak on sweep count!\n");
			break;
		    }
		}
		if(tdui->tfer->vol_count_flag) {
		    tdui->tfer->vol_count_flag = NO;
		    if(difs->max_vols) {
			if(++tdui->tfer->vol_count > difs->max_vols ) {
			    printf("\nBreak on volume count!\n");
			    break;
			}
		    }
		}
	    }
	    if(!difs->catalog_only)
		  tdwr_nab_data();
	    
	    dd_stuff_ray();	/* pass it off to dorade code */
	}
	else {
	    if(!(nok_count++ % 1000)) {
		tdwr_print_stat_line(count);
	    }
	}
	tdui->new_sweep = NO;
	tdui->tfer->new_vol = NO;

	if((n = tdwr_next_ray()) < 1) {
	    printf("Break on input status: %d\n", n);
	    break;
	}
    }
    tdwr_print_stat_line(count);
    return;
}
/* c------------------------------------------------------------------------ */

void 
tdwr_dump_this_ray (struct solo_list_mgmt *slm)
{
    int gg, ii, nf;
    char *aa, str[128];
    float unscale[7], bias[7];

    nf = gri->num_fields_present <= 7 ? gri->num_fields_present : 7;
    aa = str;
    tdwr_nab_data();
    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of data fields");
    solo_add_list_entry(slm, aa);

    sprintf(aa, "name ");

    for(ii=0; ii < nf; ii++) {
	sprintf(aa+strlen(aa), "        %s", gri->field_name[ii]);
	unscale[ii] = gri->dd_scale[ii] ? 1./gri->dd_scale[ii] : 1.;
	bias[ii] = gri->dd_offset[ii];
    }
    solo_add_list_entry(slm, aa);

    for(gg=0; gg < gri->num_bins; gg++) {
	sprintf(aa, "%4d)", gg);

	for(ii=0; ii < nf; ii++) {
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
tdwr_ini (void)
{
    int fn, ii, jj, kk, nn, mark, ms = EMPTY_FLAG, found=NO, swp_count;
    int latd, latm, lats, lond, lonm, lons;
    char str[256], line[128], directory[128];
    char *a, *aa, *bb, *unk=(char *)"UNKNOWN";
    double d;
    unsigned short *us;
    short *ss;

    printf("TDWR_FORMAT\n");

    dd_min_rays_per_sweep();	/* trigger min rays per sweep */
    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();
    gri = return_gri_ptr();

    if(hostIsLittleEndian()) {
       xtbd = (struct tdwr_base_datum *)malloc(sizeof(struct tdwr_base_datum));
       memset(xtbd, 0, sizeof(struct tdwr_base_datum));
       cbdh = xcbdh = (struct cbd_header *)malloc(sizeof(struct cbd_header));
       memset(xcbdh, 0, sizeof(struct cbd_header));
    }
    tdui = (struct tdwr_useful_items *)
	  malloc(sizeof(struct tdwr_useful_items));
    memset((char *)tdui, 0, sizeof(struct tdwr_useful_items));

    for(ii = 0; ii < 2; ii++) {
	tdui->tfer_list[ii] = (struct tdwr_for_each_radar *)
	      malloc(sizeof(struct tdwr_for_each_radar));
	memset(tdui->tfer_list[ii], 0, sizeof(struct tdwr_for_each_radar));
//	aa = ii == LOW_RATE_INDEX ? "TDWR_LR" : "TDWR";  //Jul 26, 2011
	aa = const_cast<char *>(ii == LOW_RATE_INDEX ? "TDWR_LR" : "TDWR");  //Jul 26, 2011
	strcpy(tdui->tfer_list[ii]->radar_name, aa);
	tdui->tfer_list[ii]->new_vol = YES;
    }

    /* set up lookup tables
     */
    ss = tdui->lut[TDWR_Z_LUT] = (short *)malloc(256 * sizeof(short));
    memset(ss, 0, 256 * sizeof(short));

    for(ii=0; ii < 256; ii++) {
	*ss++ = (short)DD_SCALE(TDWR_UNSCALE(ii, Z_UNSCALE, Z_EXCESS), 100., 0);
    }

    ss = tdui->lut[TDWR_SNR_LUT] = (short *)malloc(256 * sizeof(short));
    memset(ss, 0, 256 * sizeof(short));

    for(ii=0; ii < 256; ii++) {
	*ss++ = (short)DD_SCALE(TDWR_UNSCALE(ii, SNR_UNSCALE, SNR_EXCESS), 100., 0);
    }

    ss = tdui->lut[TDWR_SW_LUT] = (short *)malloc(256 * sizeof(short));
    memset(ss, 0, 256 * sizeof(short));

    for(ii=0; ii < 256; ii++) {
	*ss++ = (short)DD_SCALE(TDWR_UNSCALE(ii, SW_UNSCALE, SW_EXCESS), 100., 0);
    }

    ss = tdui->lut[TDWR_VEL_LUT] = (short *)malloc(65536 * sizeof(short));
    memset(ss, 0, 65536 * sizeof(short));

    for(ii=0; ii < 65536; ii++) {
	*ss++ = (short)DD_SCALE(TDWR_UNSCALE(ii, V_UNSCALE, V_EXCESS), 100., 0);
    }

    /* set up the default data fields
     */
    for(ii=0; ii < SRS_MAX_FIELDS; gri->dd_scale[ii++] = 100.);

    fn = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "DZ");
    strcpy(gri->field_long_name[fn], "Reflectivity factor");
    strcpy(gri->field_units[fn], "DBZ");

    fn = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "SNR");
    strcpy(gri->field_long_name[fn], "Signal to noise ratio");
    strcpy(gri->field_units[fn], "DBM");

    fn = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "VR");
    strcpy(gri->field_long_name[fn], "Raw radial velocity");
    strcpy(gri->field_units[fn], "m/s");

    fn = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "SW");
    strcpy(gri->field_long_name[fn], "Spectral width");
    strcpy(gri->field_units[fn], "m/s");

    fn = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "VE");
    strcpy(gri->field_long_name[fn], "Dealiased radial velocity");
    strcpy(gri->field_units[fn], "m/s");

    fn = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "FL");
    strcpy(gri->field_long_name[fn], "Flag field");
    strcpy(gri->field_units[fn], "???");

    gri->binary_format = DD_16_BITS;
    gri->source_format = TDWR_FMT;
    gri->radar_type = DD_RADAR_TYPE_GROUND;
    gri->latitude = 29.;
    gri->longitude = -81;
    gri->altitude = 33;
    gri->scan_mode = DD_SCAN_MODE_SUR;
    gri->missing_data_flag = EMPTY_FLAG;
    gri->h_beamwidth = gri->v_beamwidth = BEAM_WIDTH * .01;
    gri->polarization = 0;	/* horizontal polarization */

    
    if(aa=get_tagged_string("RADAR_NAME")) {
	bb = str_terminate(tdui->tfer_list[LOW_RATE_INDEX]->radar_name, aa, 8);
	strcpy(tdui->tfer_list[BASE_RATE_INDEX]->radar_name, bb);
	bb += strlen(bb) -2;
	strncpy(bb, "LR", 2);
	tdui->user_supplied_name = YES;
    }

    if(a=get_tagged_string("RADAR_LATITUDE")) {
	gri->latitude = d = atof(a);
    }
    d = fabs(gri->latitude -(int)gri->latitude)*60.;
    latm = (int)d;
    d -= (int)d;
    lats = (int)(d*60+.5);
    printf("Latitude: %.4f  %02d %02d %02d\n", gri->latitude
	   , (int)gri->latitude, latm, lats);

    if(a=get_tagged_string("RADAR_LONGITUDE")) {
	gri->longitude = d = atof(a);
    }
    d = fabs(gri->longitude -(int)gri->longitude)*60.;
    lonm = (int)d;
    d -= (int)d;
    lons = (int)(d*60+.5);
    printf("Longitude: %.4f  %02d %02d %02d\n", gri->longitude
	   , (int)gri->longitude, lonm, lons);

    if(a=get_tagged_string("RADAR_ALTITUDE")) {
	gri->altitude = d = atof(a);
    }
    printf("Altitude: %.1f meters.\n", gri->altitude);
    
    gri->wavelength = SPEED_OF_LIGHT/(FREQUENCY * 1.e6); /* Hertz */
    if(a=get_tagged_string("RADAR_WAVELENGTH")) {
	gri->wavelength = 
	      d = atof(a);
    }
    gri->num_ipps = gri->num_freq = 1;
    gri->freq[0] = (SPEED_OF_LIGHT/gri->wavelength) * 1.e-9; /* GHz */
    printf("TDWR wavelength: %.3f meters\n", gri->wavelength);

    a = (char *)"TDWR_";
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

    /*
     * open up the input file
     */
    iri = dd_return_ios(0, TDWR_FMT);

    if(aa=get_tagged_string("VOLUME_HEADER")) {
	/* nab a volume header from another file
	 */
	dd_input_read_open(iri, aa);
	nn = tdwr_next_ray();
	dd_input_read_close(iri);
    }
    if((a=get_tagged_string("SOURCE_DEV"))) {
	dd_input_read_open(iri, a);
    }

    tdwr_next_ray();
    return;
}
/* c------------------------------------------------------------------------ */

int 
tdwr_inventory (struct input_read_info *iri)
{
    /* the purpose of this routine is to facilitate a more detailed
     * examination of the data
     */
    int ii=0, max=gri_max_lines();
    int nn=2, ival;
    float val;
    char str[256];
    double d;


    for(;;) {
	for(ii=0; ii < max; ii++) {
	    tdui->tfer->new_vol = NO;
	    tdui->new_sweep = NO;

	    if((nn=tdwr_next_ray()) < 1) {
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
tdwr_map_structs (char *buf)
{
   unsigned short us;
   u_int32_t ul;

   cbdh = (struct cbd_header *)buf;
   tbd = (struct tdwr_base_datum *)(buf + sizeof(struct cbd_header));

   if(hostIsLittleEndian()) {
      cbdh = xcbdh;
//      tdwr_crack_cbdh(buf, cbdh, (int)0);  //Jul 26, 2011
      tdwr_crack_cbdh(buf, (char *)cbdh, (int)0);  //Jul 26, 2011
      ul = cbdh->pw0; 
      pw0.scan_strategy = ul & 0xff; ul >>= 14;
      pw0.sov = ul & 1;
      pw0.eov = ul >> 1;

      ul = cbdh->pw1;
      pw1.dri = (ul >> 2) & 1;

      ul = cbdh->pw2;
      pw2.lp = ul & 1; ul >>= 22;
      pw2.soe = ul & 1; ul >>= 1;
      pw2.eoe = ul & 1; ul >>= 1;
      pw2.scan_number = ul & 0xff;

      ul = cbdh->pw3;
      pw3.pulses_per_dwell = ul & 0xfff;
      
      ul = cbdh->pw4;
      pw4.vsr = (ul >> 1) & 1;



   }
   else {
      memcpy(&pw0, &cbdh->pw0, sizeof(cbdh->pw0));
      memcpy(&pw1, &cbdh->pw1, sizeof(cbdh->pw1));
      memcpy(&pw2, &cbdh->pw2, sizeof(cbdh->pw2));
      memcpy(&pw3, &cbdh->pw3, sizeof(cbdh->pw3));
      memcpy(&pw4, &cbdh->pw4, sizeof(cbdh->pw4));
   }
}
/* c------------------------------------------------------------------------ */

void 
tdwr_nab_data (void)
{
    /* generate data for all three fields even if they are not present
     */
    static int count=0;
    int ii, kk, mm, nn, mark, pn=0, slack;
    int frs = cbdh->final_range_sample;
    unsigned char *uc, *ut;
    short *rr, *ss, *tt, *lut;
    struct tdwr_base_datum *bd;

    if(hostIsLittleEndian()) {
       tdwr_nab_dataLE();
       return;
    }

    count++;
    if((slack = cbdh->range_samples_per_dwell - frs) < 0)
	  slack = 0;

    if(cbdh->message_id == BASE_DATA) {
	lut = tdui->lut[TDWR_Z_LUT];
	bd = tbd;
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, bd++) {
	    *ss = *(lut + bd->uz);
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	pn++;
	lut = tdui->lut[TDWR_SNR_LUT];
	bd = tbd;
	ss = gri->scaled_data[pn];
	nn = frs;
	for(nn = frs; nn--; ss++, bd++) {
	    *ss = *(lut + bd->snr);
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	pn++;
	lut = tdui->lut[TDWR_VEL_LUT];
	bd = tbd;
	ss = gri->scaled_data[pn];
	nn = frs;
	for(nn = frs; nn--; ss++, bd++) {
	    *ss = *(lut + bd->urv);
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	pn++;
	lut = tdui->lut[TDWR_SW_LUT];
	bd = tbd;
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, bd++) {
	    *ss = *(lut + bd->w);
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	pn++;
	lut = tdui->lut[TDWR_VEL_LUT];
	bd = tbd;
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, bd++) {
	    *ss = *(lut + bd->drv);
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	/* 
	 * valid flags
	 */
	pn++;
	bd = tbd;
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, bd++) {
	    mm = bd->ccv ? 0x8 : 0;    /* conditioned valid flag */ 
	    if(bd->cvf) mm |= 0x4;     /* conditioned valid velocity */
	    if(bd->ctf) mm |= 0x2;     /* point target filter flag */
	    if(bd->caf % 1) mm |= 0x1; /* dealias algo. failure */
	    *ss = mm * 100;
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
    }
    else {
//	uc = (char *)tbd;  //Jul 26, 2011
	uc = (unsigned char *)tbd;  //Jul 26, 2011

	lut = tdui->lut[TDWR_Z_LUT];
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, uc++) {
	    *ss = *(lut + (*uc));
	}
	if(nn = slack) {
	    uc += slack;
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	pn++;
	lut = tdui->lut[TDWR_SNR_LUT];
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, uc++) {
	    *ss = *(lut + (*uc));
	}
	if(nn = slack) {
	    uc += slack;
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	pn++;
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, uc++) {
	    kk = *uc;
	    mm = kk & VALID_FLAG_CV ? 0x8 : 0;
	    if(kk & POINT_TARGET_FILTER_FLAG_CTF) mm |= 0x2;
	    *ss = mm * 100;
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
    }	
}
/* c------------------------------------------------------------------------ */

void 
tdwr_nab_dataLE (void)
{
    /* generate data for all three fields even if they are not present
     */
    static int count=0;
    int ii, kk, mm, nn, mark, pn=0, slack;
    int frs = cbdh->final_range_sample;
    unsigned char *uc, *ut;
    short *rr, *ss, *tt, *lut;
    unsigned short usi;
    u_int32_t usl;
    char *aa, *bb;

    struct bdb {
       unsigned char uz;
       unsigned char snr;
       unsigned char urv[2];
       unsigned char w;
       unsigned char xx;
       unsigned char drv[2];
    } *bdb;

    aa = (char *)&usi;

    count++;
    if((slack = cbdh->range_samples_per_dwell - frs) < 0)
	  slack = 0;

    if(cbdh->message_id == BASE_DATA) {
	lut = tdui->lut[TDWR_Z_LUT];
	bdb = (struct bdb *)tbd;
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, bdb++) {
	    *ss = *(lut + bdb->uz);
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	pn++;
	lut = tdui->lut[TDWR_SNR_LUT];
	bdb = (struct bdb *)tbd;
	ss = gri->scaled_data[pn];
	nn = frs;
	for(nn = frs; nn--; ss++, bdb++) {
	    *ss = *(lut + bdb->snr);
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	pn++;
	lut = tdui->lut[TDWR_VEL_LUT];
	bdb = (struct bdb *)tbd;
	ss = gri->scaled_data[pn];
	nn = frs;
	for(nn = frs; nn--; ss++, bdb++) {
	   *aa = bdb->urv[1];
	   *(aa+1) = bdb->urv[0];
	   *ss = *(lut + usi);
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	pn++;
	lut = tdui->lut[TDWR_SW_LUT];
	bdb = (struct bdb *)tbd;
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, bdb++) {
	    *ss = *(lut + bdb->w);
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	pn++;
	lut = tdui->lut[TDWR_VEL_LUT];
	bdb = (struct bdb *)tbd;
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, bdb++) {
	   *aa = bdb->drv[1];
	   *(aa+1) = bdb->drv[0];
	   *ss = *(lut + usi);
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	/* 
	 * valid flags
	 */
	pn++;
	bdb = (struct bdb *)tbd;
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, bdb++) {
# ifdef obsolete	   
	   mm = bdb->ccv ? 0x8 : 0;    /* conditioned valid flag */ 
	   if(bd->cvf) mm |= 0x4;     /* conditioned valid velocity */
	   if(bd->ctf) mm |= 0x2;     /* point target filter flag */
	   if(bd->caf % 1) mm |= 0x1; /* dealias algo. failure */
# endif
	   usl = bdb->xx;
	   mm = usl & 1 ? 0x8 : 0;	/* conditioned valid flag */ 
	   if(usl & 0x2) mm |= 0x4;     /* conditioned valid velocity */
	   if(usl & 0x4) mm |= 0x2;     /* point target filter flag */
	   if(usl & 0x8) mm |= 0x1; /* dealias algo. failure */
	   *ss = mm * 100;
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
    }
    else {
//	uc = (char *)tbd;  //Jul 26, 2011
	uc = (unsigned char *)tbd;  //Jul 26, 2011

	lut = tdui->lut[TDWR_Z_LUT];
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, uc++) {
	    *ss = *(lut + (*uc));
	}
	if(nn = slack) {
	    uc += slack;
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	pn++;
	lut = tdui->lut[TDWR_SNR_LUT];
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, uc++) {
	    *ss = *(lut + (*uc));
	}
	if(nn = slack) {
	    uc += slack;
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
	pn++;
	ss = gri->scaled_data[pn];
	for(nn = frs; nn--; ss++, uc++) {
	    kk = *uc;
	    mm = kk & VALID_FLAG_CV ? 0x8 : 0;
	    if(kk & POINT_TARGET_FILTER_FLAG_CTF) mm |= 0x2;
	    *ss = mm * 100;
	}
	if(nn = slack) {
	    for(; nn--; *ss++ = EMPTY_FLAG);
	}
    }	
}
/* c------------------------------------------------------------------------ */

int 
tdwr_next_ray (void)
{
    int ii, nn, break_out, ms=EMPTY_FLAG;
    int pn, eof_count=0, mark, sizeof_packet;
    static int count=0, trip=111;
    struct io_packet *dp;
    char *at, *aa, *bb;
    float f, *r;

    for(;;) {
	count++;
	if(count >= trip) {
	    mark = 0;
	}
	if(dd_control_c()) {
	    dd_reset_control_c();
	    return(-1);
	}
	if(iri->top->bytes_left < 4) {
	    for(;;) {
		if(dd_control_c()) {
		    dd_reset_control_c();
		    return(-1);
		}
		dd_logical_read(iri, FORWARD);
		
		if(iri->top->read_state < 0) {
		    printf("Last read: %d\n", iri->top->read_state);
		    return(iri->top->read_state);
		}
		else if(iri->top->eof_flag) {
		    tdui->tfer_list[LOW_RATE_INDEX]->new_vol = YES;
		    tdui->tfer_list[BASE_RATE_INDEX]->new_vol = YES;
		    dd_stats->file_count++;
		    printf("EOF: %d at %.2f MB\n"
			   , dd_stats->file_count, dd_stats->MB);
		    if(++eof_count > 1)
			  return((int)0);
		}
		else {
		    eof_count = 0;
		    dd_stats->rec_count++;
		    dd_stats->MB += BYTES_TO_MB(iri->top->sizeof_read);
		    break;
		}
	    }
	}
	break_out = NO;
	dp = dd_return_next_packet(iri);
	tdwr_map_structs(dp->at);
	sizeof_packet = cbdh->sizeof_message;
	dp->len = sizeof_packet;

	iri->top->bytes_left -= sizeof_packet;
	iri->top->at += sizeof_packet;
	if(iri->top->bytes_left < sizeof_packet) {
	   iri->top->bytes_left = 0;
	}

	if(cbdh->message_id == TDWR_TAPE_ID && !tdui->user_supplied_name) {
	    /* tape header!
	     */
	    aa = dp->at +4 +8 +8; /* point to the name */
	    bb = tdui->tfer_list[LOW_RATE_INDEX]->radar_name;
	    str_terminate(bb, aa, 8);
	    for(aa=bb; *aa; aa++) { /* replace interior blanks with underscores */
		if(*aa == ' ') *aa = '_';
	    }
	    strcpy(tdui->tfer_list[BASE_RATE_INDEX]->radar_name, bb);
	    /*
	     * now generate the low rate radar name
	     * by just replacing the last two letters by "LR"
	     */
	    bb += strlen(bb) -2;
	    strncpy(bb, "LR", 2);
	    
	    iri->top->bytes_left = 0;
	}
	else if(cbdh->message_id == BASE_DATA ||
		cbdh->message_id == BASE_DATA_LOW_PRF) {

	    if(cbdh->message_id == BASE_DATA_LOW_PRF || pw2.lp) {
		tdui->tfer = tdui->tfer_list[LOW_RATE_INDEX];
		tdui->low_prf = YES;
	    }
	    else {
		tdui->tfer = tdui->tfer_list[BASE_RATE_INDEX];
		tdui->low_prf = NO;
	    }
	    if(pw0.eov) {
		tdui->vol_transition++;
	    }
	    if(pw0.sov) {
		tdui->vol_transition = 0;
		dd_stats->vol_count++;
		tdui->tfer_list[LOW_RATE_INDEX]->new_vol = YES;
		tdui->tfer_list[BASE_RATE_INDEX]->new_vol = YES;
	    }
	    if(pw2.eoe) {
		tdui->sweep_transition++;
	    }
	    if(pw2.soe) {
		tdui->sweep_transition = 0;
		tdui->new_sweep = YES;
	    }
	    if(FABS(tdui->ang_diff_sum) >= 360.) {
		tdui->sweep_transition = 0;
		tdui->new_sweep = YES;
	    }
	    if(!(pw1.dri || pw4.vsr))
		  break_out = YES;
	}
	if(break_out)
	      break;
    }
    dd_stats->ray_count++;
    if(!tdui->assigned_dd_nums) {
	tdui->assigned_dd_nums = YES;
	tdui->tfer_list[BASE_RATE_INDEX]->radar_num = dd_assign_radar_num
	      (tdui->tfer_list[BASE_RATE_INDEX]->radar_name);
	dd_radar_selected(tdui->tfer_list[BASE_RATE_INDEX]->radar_name
			  , tdui->tfer_list[BASE_RATE_INDEX]->radar_num, difs);
	tdui->tfer_list[LOW_RATE_INDEX]->radar_num = dd_assign_radar_num
	      (tdui->tfer_list[LOW_RATE_INDEX]->radar_name);
	dd_radar_selected(tdui->tfer_list[LOW_RATE_INDEX]->radar_name
			  , tdui->tfer_list[LOW_RATE_INDEX]->radar_num, difs);
    }

    if(tdui->trip_time != cbdh->time_stamp) {
	tdui->count_since_trip = 0;
	tdui->trip_time = cbdh->time_stamp;
    }
    gri->dts->time_stamp = 
	  gri->time = tdui->trip_time + tdui->count_since_trip++ *
		pw3.pulses_per_dwell * cbdh->pri_xe6 * 1.e-6;

    gri->corrected_rotation_angle = gri->rotation_angle = 
	  gri->azimuth = cbdh->current_azimuth;
    gri->elevation = cbdh->current_elevation;

    if(tdui->tfer->new_vol) {
	gri->vol_num++;
	tdui->tfer->vol_count_flag =
	      tdui->new_sweep = YES;
    }
    if(tdui->new_sweep) {
	/* c...mark */
	tdwr_setup_fields();
	gri->fixed = tdui->first_elev = tdui->last_elev = gri->elevation;
	tdui->sweep_count_flag = YES;
	tdui->sum_elev = 
	    tdui->sum_delta_elev = tdui->ang_diff_sum = 0;
	tdui->sweep_ray_count = 0;
	gri->dd_radar_num = tdui->tfer->radar_num;
	strcpy(gri->radar_name, tdui->tfer->radar_name);
	gri->source_sweep_num = pw2.scan_number;
	gri->source_vol_num = cbdh->volume_scan_count;
	gri->sweep_num++;
	dd_stats->sweep_count++;
	gri->sweep_speed = cbdh->angular_scan_rate;

	gri->ipps[0] = cbdh->pri_xe6 * 1.e-6; /* seconds */
	gri->unamb_range = .5 * (float)cbdh->pri_xe6 * 1.e-6 *
	      SPEED_OF_LIGHT * .001;
	gri->num_samples = pw3.pulses_per_dwell;
	gri->PRF = 1.e6/(float)cbdh->pri_xe6;
	gri->nyquist_vel = 0.25 * gri->PRF * gri->wavelength;
	gri->peak_power = cbdh->peak_xmittd_pwr_kw;
	gri->noise_power = cbdh->total_noise_power;
	if((gri->num_bins = cbdh->range_samples_per_dwell) > 1500) {
	    gri->num_bins = 1500;
	}
	for(ii=0; ii < gri->num_fields_present;
	    gri->actual_num_bins[ii++] = gri->num_bins);

	r = gri->range_value;
	f = gri->range_b1 = RANGE_TO_FIRST_GATE;
	gri->bin_spacing = NORMAL_PRF_RESOLUTION;
	for(ii=0; ii < gri->num_bins; ii++, f+=gri->bin_spacing) {
	    *r++ = f;
	}

	switch((int)pw0.scan_strategy) {
	case TDWR_RHI:
	    gri->scan_mode = DD_SCAN_MODE_RHI;
	    break;
	case TDWR_POINT:
	    gri->scan_mode = DD_SCAN_MODE_TAR;
	    break;
	case TDWR_SUN_TRACK:
	    gri->scan_mode = DD_SCAN_MODE_MAN;
	    break;
	default:
	    gri->scan_mode =  pw2.lp ? DD_SCAN_MODE_SUR : DD_SCAN_MODE_PPI; /* low prf scan? */
	    break;
	}
	gri->radar_constant = ms;
	gri->ant_gain = ms;
    }
    if(tdui->vol_transition) {
	tdui->vol_transition++;
    }
    if(tdui->sweep_transition) {
	tdui->sweep_transition++;
    }
    tdui->sweep_ray_count++;
    tdui->sum_delta_elev += angdiff(tdui->last_elev, gri->elevation);
    gri->fixed = tdui->first_elev +
	  tdui->sum_delta_elev/(double)tdui->sweep_ray_count;
    tdui->sum_elev += gri->elevation;
# ifdef notyet
    gri->fixed = tdui->sum_elev/(double)tdui->sweep_ray_count;
    gri->fixed = gri->elevation;
# endif
    tdui->last_elev = gri->elevation;

    if(tdui->sweep_ray_count > 1) {
       tdui->ang_diff_sum += angdiff(tdui->prev_rot_angle, gri->azimuth);
    }
    tdui->prev_rot_angle = gri->azimuth;
    return(dp->len);
}
/* c------------------------------------------------------------------------ */

void 
tdwr_positioning (void)
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
    slm = solo_malloc_list_mgmt(87);


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
    tdui->new_sweep = NO;
    tdui->tfer->new_vol = NO;

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
		tdui->new_sweep = NO;
		tdui->tfer->new_vol = NO;
		if((jj=tdwr_next_ray()) < 0)
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
	tdwr_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 1) {
	if((nn = tdwr_inventory(iri)) == -2)
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
	tdwr_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 3) {
	dd_rewind_dev(iri);
	tdwr_reset();
	tdwr_next_ray();
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
	tdwr_next_ray();
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
	    tdui->new_sweep = NO;
	    tdui->tfer->new_vol = NO;
	    if((nn = tdwr_next_ray()) <= 0 || gri->time >= dtarget ||
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
	    tdui->new_sweep = NO;
	    tdui->tfer->new_vol = NO;
	    if((nn = tdwr_next_ray()) <= 0 || in_sector(gri->fixed, fx1, fx2)
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
	tdwr_print_cbdh(slm);
    }
    else if(ival == 12) {
	tdwr_dump_this_ray(slm);
    }
    preamble[0] = '\0';

    goto menu2;


}
/* c------------------------------------------------------------------------ */

void 
tdwr_print_cbdh (struct solo_list_mgmt *slm)
{
    int ii;
    char *aa, str[128];

    aa = str;
    solo_reset_list_entries(slm);

    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of ray header");
    solo_add_list_entry(slm, aa);

    sprintf(aa, "message_id              %4x", cbdh->message_id);             
    solo_add_list_entry(slm, aa);
    sprintf(aa, "sizeof_message          %d", cbdh->sizeof_message);          
    solo_add_list_entry(slm, aa);
    sprintf(aa, "volume_scan_count       %d", cbdh->volume_scan_count);       
    solo_add_list_entry(slm, aa);
    sprintf(aa, "pw0                     %04x", cbdh->pw0);  		      
    solo_add_list_entry(slm, aa);
    sprintf(aa, "peak_xmittd_pwr_kw      %d", cbdh->peak_xmittd_pwr_kw);      
    solo_add_list_entry(slm, aa);
    sprintf(aa, "pw1                     %04x", cbdh->pw1);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "pw2                     %08x", cbdh->pw2);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "current_elevation       %.2f", cbdh->current_elevation);     
    solo_add_list_entry(slm, aa);
    sprintf(aa, "angular_scan_rate       %.2f", cbdh->angular_scan_rate);     
    solo_add_list_entry(slm, aa);
    sprintf(aa, "pri_xe6                 %d", cbdh->pri_xe6);                 
    solo_add_list_entry(slm, aa);
    sprintf(aa, "pw3                     %04x", cbdh->pw3);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "final_range_sample      %d", cbdh->final_range_sample);      
    solo_add_list_entry(slm, aa);
    sprintf(aa, "range_samples_per_dwell %d", cbdh->range_samples_per_dwell); 
    solo_add_list_entry(slm, aa);
    sprintf(aa, "current_azimuth         %.2f", cbdh->current_azimuth);       
    solo_add_list_entry(slm, aa);
    sprintf(aa, "total_noise_power       %f", cbdh->total_noise_power);       
    solo_add_list_entry(slm, aa);
    sprintf(aa, "time_stamp              %d %s", cbdh->time_stamp	      
	    , dts_print(d_unstamp_time(gri->dts)));          		      
    solo_add_list_entry(slm, aa);
    sprintf(aa, "base_data_type          %d", cbdh->base_data_type);          
    solo_add_list_entry(slm, aa);
    sprintf(aa, "pw4                     %04x", cbdh->pw4);                   
    solo_add_list_entry(slm, aa);
    sprintf(aa, "integer_azimuth         %d", cbdh->integer_azimuth);         
    solo_add_list_entry(slm, aa);
    sprintf(aa, "load_shed_final_sample  %d", cbdh->load_shed_final_sample);  
    solo_add_list_entry(slm, aa);

    slm_print_list(slm);

    return;
}
/* c------------------------------------------------------------------------ */

void 
tdwr_print_stat_line (int count)
{
    DD_TIME *dts=gri->dts;
    
    d_unstamp_time(dts);
    printf(" %5d %3d %7.2f %.2f  %s %s\n"
	   , count
	   , gri->sweep_num
	   , gri->fixed
	   , dts->time_stamp
	   , dts_print(dts)
	   , gri->radar_name
	   );
}
/* c------------------------------------------------------------------------ */

void 
tdwr_reset (void)
{
    int ii;

    for(ii=0; ii < 2; ii++) {
	tdui->tfer_list[ii]->new_vol = 
	      tdui->tfer_list[ii]->vol_count_flag = YES;
	tdui->tfer_list[ii]->vol_count = 0;
    }
    tdui->sweep_count_flag = tdui->new_sweep = YES;
    tdui->ray_count = tdui->sweep_count = 0;
    tdui->sum_delta_elev = tdui->ang_diff_sum = tdui->sum_elev = 0;

    /* make sure to start a new sweep and vol
     */
    gri->vol_num++;
    gri->sweep_num++;
    difs->stop_flag = NO;
}
/* c------------------------------------------------------------------------ */

int 
tdwr_select_ray (void)
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

    if(!difs->radar_selected[gri->dd_radar_num]) 
	  ok = NO;

    return(ok);
}
/* c------------------------------------------------------------------------ */

void 
tdwr_setup_fields (void)
{
    int fn, kk, mm;

    for(fn=0; fn < SRS_MAX_FIELDS; gri->dd_scale[fn++] = 100.);
    gri->num_fields_present = 0;

    fn = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "DZ");
    strcpy(gri->field_long_name[fn], "Reflectivity factor");
    strcpy(gri->field_units[fn], "DBZ");
    
    fn = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "SNR");
    strcpy(gri->field_long_name[fn], "Signal to noise ratio");
    strcpy(gri->field_units[fn], "DBM");
    
    if(cbdh->message_id == BASE_DATA) {
	fn = gri->num_fields_present++;
	strcpy(gri->field_name[fn], "VR");
	strcpy(gri->field_long_name[fn], "Raw radial velocity");
	strcpy(gri->field_units[fn], "m/s");
	
	fn = gri->num_fields_present++;
	strcpy(gri->field_name[fn], "SW");
	strcpy(gri->field_long_name[fn], "Spectral width");
	strcpy(gri->field_units[fn], "m/s");
	
	fn = gri->num_fields_present++;
	strcpy(gri->field_name[fn], "VE");
	strcpy(gri->field_long_name[fn], "Dealiased radial velocity");
	strcpy(gri->field_units[fn], "m/s");
    }
    fn = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "FL");
    strcpy(gri->field_long_name[fn], "Flag field");
    strcpy(gri->field_units[fn], "???");
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

