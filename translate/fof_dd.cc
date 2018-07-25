/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

/*
 * These environment variables need to be set to run these routines
 * The paths should be changed to reflect your environment
 * 
 * setenv FOF_FIELDLIST "/rdss/solo/catalog/fieldlist.job"
 * setenv FOF_CATALOG_HEADER_FILE "/rdss/solo/catalog/header/headerlist.job"
 * setenv FOF_CATALOG_HEADER_DIR "/rdss/solo/catalog/header/"
 * 
 */

/*
 * This file contains routines:
 * 
 * fof_dd_conv
 * 
 * fof_absorb_catalog_header
 * fof_catalog_header
 * fof_catalog_header_stream
 * fof_check_srs_fld
 * fof_crack_vms_date
 * fof_data_field_spair
 * fof_dump_this_ray
 * fof_duplicate_field
 * fof_era
 * fof_establish_data_field
 * fof_find_srs_fld
 * 
 * fof_gen_counts_field
 * fof_gen_dbz_field
 * fof_gen_field
 * fof_gen_ldr_field
 * fof_gen_noiseless_zdr
 * fof_gen_quotient_field
 * fof_gen_snr
 * fof_gen_sw
 * fof_gen_thrd_field
 * fof_gen_unfolded_linear
 * fof_gen_unfolded_xvert
 * 
 * fof_get_field_list_name
 * fof_header_list
 * fof_ini
 * fof_insert_data_field
 * fof_inventory
 * fof_isa_new_sweep
 * fof_isa_new_vol
 * fof_link_cat_info
 * fof_load_field_list
 * fof_luts
 * 
 * fof_malloc_vals
 * fof_MHR_or_cape
 * fof_mist_cp3
 * fof_nab_data
 * fof_new_sweep
 * fof_new_vol
 * fof_next_ray
 * 
 * fof_positioning
 * fof_possible_field
 * fof_print_fieldlist
 * fof_print_headers
 * fof_print_hsk
 * fof_print_stat_line
 * fof_process_cal_info
 * fof_process_field_info
 * fof_process_system_info
 * fof_push_data_field
 * fof_que_data_field
 * fof_que_ray_info
 * 
 * fof_raw_counts_field
 * fof_rayq_info
 * fof_ray_que_spair
 * fof_redo_srsfs
 * fof_remove_data_field
 * fof_reset
 * fof_rng_corr
 * fof_select_ray
 * fof_setup_field
 * 
 * fof_stack_ray_que_spairs
 * fof_stack_sweep_que_spairs
 * fof_sweep_que_spair
 * fof_time
 * 
 * zatt_arg_ptrs
 * xnext_att
 * znext_att
 * 
 */

#include <LittleEndian.hh>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <dd_math.h>
#include "fof_stuff.h"
#include "rsf_house.h"
#include "dd_stats.h"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "gneric_dd.hh"
#include "dd_io_mgmt.hh"
#include "fof_dd.hh"
#include "dorade_share.hh"
#include "dd_crackers.hh"

# define BITS_SET(x,bits) (((bits) & (x)) == (bits) ? 1 : 0)
# define CLEAR_BITS(x,bits) (~(bits) & (x))
# define SET_BITS(x,bits) ((bits) | (x))

# define SMALLOC(st) ((st *)malloc(sizeof(st)))
# define FMALLOC(n) ((float *)malloc(sizeof(float)*(n)))
# define MATCHN(a,b,n) (strncmp((a),(b),(n)) == 0)
# define MATCH2(a,b) (strncmp((a),(b),2) == 0)
# define IPOWR2(x) ((1) << (x))

# define            SQRT8XPI 8.885765876
# define     DEFAULT_SNR_THR -18.0
# define     DEFAULT_NCP_THR 0.15
# define   DEFAULT_MAX_DELTA 22.
# define    DEFAULT_MIN_RAYS 3
# define    DEFAULT_MAX_RAYS 2048
# define                BIT0 1
# define                BIT4 0x10
# define                BIT6 0x40
/*
 * Change flags, flags a change in the item of interest
 */
# define	    TFLAG_ON 0x1
# define	   XSCAN_NUM 0x2
# define	    XVOL_NUM 0x4
# define	    MAX_RAYS 0x8
  
# define		 GAP 0x10
# define	    PLUS_360 0x20
# define	  XDIRECTION 0x40
# define	  XFIELD_MIX 0x80
  
# define     TRANSITION_OVER 0x100
# define	  XSCAN_MODE 0x200
# define	 XGATE_COUNT 0x400
# define       XSAMPLE_COUNT 0x800
  
# define        XFIXED_ANGLE 0x1000
# define      XDUAL_POL_MODE 0x2000
# define       XGATE_SPACING 0x4000
# define                XPRF 0x8000
# define                XATP 0x10000
/*
 * end change flags
 */
# define    FOF_MIN_REC_SIZE 128
# define	 HORZ_LV_CAL 1
# define	 VERT_LV_CAL 2
# define      XBAND_HORZ_CAL 3
# define      XBAND_VERT_CAL 4
# define	  LINEAR_CAL 5

# define    EARLIEST_CP2_NCP 0x1
# define   CP2_MAYPOLE83_NCP 0x2
# define       MAYPOLE84_NCP 0x4
# define    EARLIEST_CP4_NCP 0x8
# define  NCP_AFTER_APRIL_86 0x10
# define     LUTZ89_MHR_FLAG 0x100

# define                 CP2 2
# define                 CP3 3
# define                 CP4 4
# define                 RP3 3
# define                 RP4 4
# define                 RP5 5
# define                 RP6 6
# define                 RP7 7
# define                 MHR 0x4d48
# define                LLAB 0xf2

# define   PRIMARY_LOG_VIDEO 1
# define             RAW_ZDR 2
# define           R_SUB_TAU 4
# define          R_SUB_ZERO 5
# define     VELOCITY_ID_NUM 8
# define       VELOCITY_FLAG 9
# define         R_SUB_CHUCK 12
# define BROKEN_JAWS_VELOCITY_ID_NUM 247

static int desc_offset[16], scale_offset[16];
static struct fof_useful_info *fui=NULL;
static struct fof_ray_que *ray_que_spairs=NULL;
static struct fof_sweep_que *sweep_que_spairs=NULL;
static struct data_field_info *data_field_spairs=NULL;
static int seq_num=0;
static struct generic_radar_info *gri;
static struct dd_general_info *dgi;
static struct radar_d *xradd=NULL;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats;
static struct input_read_info *iri;
static double v_noise_power=0;
static double max_daz=DEFAULT_MAX_DELTA, max_del=DEFAULT_MAX_DELTA;
static int min_rays_per_sweep=DEFAULT_MIN_RAYS;
static int max_rays_per_sweep=DEFAULT_MAX_RAYS;
static int num_source_ids=0;
static int new_vol_flags;
static struct source_field_info *source_fields[256];
static struct source_field_info *sources_top=NULL;
static struct calibration_info *cals_top=NULL;
static struct field_derivation_info *dfields_top=NULL;
static struct fof_system_attributes *systems_top=NULL;

static char *Months[] = { (char *)"000", (char *)"JAN", (char *)"FEB",
			  (char *)"MAR", (char *)"APR", (char *)"MAY",
			  (char *)"JUN", (char *)"JUL", (char *)"AUG",
			  (char *)"SEP", (char *)"OCT", (char *)"NOV",
			  (char *)"DEC", (char *)"013" };
static double header_start=1.e22, header_stop=-1.e22;
static char experiment[16], instrument[16], tzone[8];
static RSF_HOUSE phsk, *xhsk;
static int view_char=0, view_char_count=200;
static char preamble[24], postamble[64];
static struct solo_list_mgmt *slm;

static int new_sweep_hesitation=0;
static float max_fixed_delta=5.;
static int ray_que_count=0;
static int sweep_que_count=0;
static char header_info[88];
static char *final_fields=(char *)"DM VR";


/* c------------------------------------------------------------------------ */

void fof_dd_conv (int interactive_mode)
{
    /* main loop for converting fof data
     */
    int i, n, mark;
    static int count=0, nok_count;
    struct data_field_info dataf;  


    if(!count) {
	fof_ini();
    }

    if(interactive_mode) {
	dd_intset();
	fof_positioning();
    }
    fof_reset();

    for(;;) {
	count++;
	if(difs->stop_flag &&
	   (difs->abrupt_start_stop || fui->new_sweep)) {
	    printf("\nBreak on stop time!\n");
	    break;
	}

	if(fof_select_ray()) {

	    if(difs->max_beams)
		  if(++fui->ray_count > difs->max_beams ) {
		      printf("\nBreak on ray count!\n");
		      break;
		  }
	    if(fui->sweep_count_flag) {
		fui->sweep_count_flag = NO;
		if(difs->max_sweeps)
		      if(++fui->sweep_count > difs->max_sweeps ) {
			  printf("\nBreak on sweep count!\n");
			  break;
		      }
		if(fui->vol_count_flag) {
		    fui->vol_count_flag = NO;
		    if(difs->max_vols)
			  if(++fui->vol_count > difs->max_vols ) {
			      printf("\nBreak on volume count!\n");
			      break;
			  }
		}
	    }
	    if(!difs->catalog_only)
		  fof_nab_data();

	    dd_stuff_ray();	/* pass it off to dorade code */
	}
	else {
	    if(!(nok_count++ % 1000)) {
		fof_print_stat_line(count);
	    }
	}
	fui->new_sweep = NO;
	fui->new_vol = NO;

	if((n = fof_next_ray()) < 1) {
	    printf("Break on input status: %d\n", n);
	    break;
	}
    }
    fof_print_stat_line(count);
    return;
}
/* c------------------------------------------------------------------------ */

void fof_absorb_catalog_header(FILE *fp)
{
    int ii, jj, kk, nn, mark;
    char *a, att[1024], string_space[1024], *str_ptrs[32];
    double d, deg;
    struct calibration_info *ci, *cix;
    struct calibration_pair *cp, *cpx;
    struct fof_system_attributes *fsi, *fsix;

    /* pass through the header file and nab
     * location information
     * instrument information
     * calibration information
     * field information
     * 
     * but destroy existing header info first
     */
    if(ci=cals_top) {
	for(; ci;) {
	    for(cp=ci->top_pair; cp;) {
		cpx = cp; cp = cp->next;
		free(cpx);
	    }
	    if(ci->dbm_vals)
		  free(ci->dbm_vals);
	}
	cix = ci; ci = ci->next;
	free(cix);
    }
    if(fsi=systems_top) {
	for(; fsi;) {
	    fsix = fsi; fsi = fsi->next;
	    free(fsi);
	}
    }


    for(;;) {
	if(!znext_att(fp, att))
	      return;

	if(strncmp(att, "SCAN", 4) == 0) {
	    break;
	}
	else if(strncmp(att, "CALIBRATION", 5) == 0) {
	    fof_process_cal_info(fp);
	}
	else if(strncmp(att, "FIELD", 5) == 0) {
	    fof_process_field_info(fp);
	}
	else if(strncmp(att, "SYSTEM", 5) == 0) {
	    fof_process_system_info(fp);
	}
	else if(strncmp(att, "LATITUDE", 7) == 0) {
	    deg = 0;
	}
	else if(strncmp(att, "EDUTITAL", 7) == 0) { /* finished latitude */
	    gri->latitude = deg;
	}
	else if(strncmp(att, "LONGITUDE", 7) == 0) {
	    deg = 0;
	}
	else if(strncmp(att, "EDUTIGNOL", 7) == 0) { /* finished longitude */
	    gri->longitude = deg;
	}
	else if(strncmp(att, "DEG.", 4) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    deg += atof(string_space);
	}
	else if(strncmp(att, "MIN.", 4) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    deg += atof(string_space)/60.;
	}
	else if(strncmp(att, "SEC.", 3) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    deg += atof(string_space)/3600.;
	}
	else if(strncmp(att, "ALTITUDE", 5) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    gri->altitude_agl = gri->altitude = atof(string_space);
	}
	else if(strncmp(att, "TZONE", 5) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    strcpy(gri->time_zone, string_space);
	}
	else if(strncmp(att, "SITE", 4) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    nn = strlen(string_space) > sizeof(gri->site_name)
		  ? sizeof(gri->site_name) : strlen(string_space);
	    strncpy(gri->site_name, string_space, nn);
	}
	else if(strncmp(att, "EXPERIMENT", 5) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    nn = strlen(string_space) > sizeof(gri->project_name)
		  ? sizeof(gri->project_name) : strlen(string_space);
	    strncpy(gri->project_name, string_space, nn);
	}
    }

    fof_link_cat_info();
}
/* c------------------------------------------------------------------------ */

struct source_field_info *
fof_check_srs_fld (char *field_name, struct field_derivation_info **derfx)
{
    /* 
     * 
     */
    struct source_field_info *srsf=NULL;
    struct field_derivation_info *derf;

    for(derf=dfields_top; derf; derf = derf->next) {
	if(!MATCH2(field_name, derf->field_name)) {
	    continue;
	}
	if(source_fields[derf->id_num]->present) {
	    srsf = source_fields[derf->id_num];
	    break;
	}
    }
    *derfx = derf;
    return(srsf);
}
/* c------------------------------------------------------------------------ */

void fof_catalog_header (void)
{
    char *a, hfile[64], str[256], tm[24];
    char *dir=(char *)"/data/dmg/catalog/header/", *hlj=(char *)"headerlist.job";
    double ztime;
    RSF_HOUSE *hsk = (RSF_HOUSE *)fui->ray_que->at;
    FILE *fp;
    DD_TIME dts;
    double d;


    /* construct the radar name
     */
    if(hsk->rs_id == MHR) {	/* Mile High Radar */
	sprintf(fui->radar_name, "MHR-RP%d", hsk->ds_id);
    }
    else {
	sprintf(fui->radar_name, "CP%d-RP%d", hsk->rs_id, hsk->ds_id);
    }
    strcpy(gri->radar_name, fui->radar_name);
    /* c...mark
     * establish the dorade general info struct
     */
    gri->dd_radar_num = dd_assign_radar_num(gri->radar_name);
    dgi = dd_get_structs(gri->dd_radar_num);

    fp = fof_catalog_header_stream();

    fof_absorb_catalog_header(fp);
    fclose(fp);
}
/* c------------------------------------------------------------------------ */

FILE *fof_catalog_header_stream()
{
    char *a, *b, *c, hfile[64], str[256], tm[24];
    char *fcat_hdrs=(char *)"/rdss/solo/catalog/header/";
    char *hldj=(char *)"headerlist.job";
    FILE *fp;
    DD_TIME dts;
    double d;

    /* open the headerlist.job file and get the correct header file name
     */
    header_info[0] = '\0';

    if(a=getenv("FOF_CATALOG_HEADER_FILE")) {
	strcpy(str, a);
	printf("Opening %s\n", str);
	
	if(!(fp = fopen(str, "r"))) {
	    printf("Could not open catalog header file %s\n", str);
	    exit(1);
	}
	header_start = -MAX_FLOAT;
	header_stop = MAX_FLOAT;
	return(fp);
    }

# ifdef FOF_CATALOG_HEADER_DIR
    fcat_hdrs = FOF_CATALOG_HEADER_DIR;
# endif
    if ( (a = getenv("FOF_CATALOG_HEADER_DIR")) ){
	fcat_hdrs = a;
    }
    slash_path(str, fcat_hdrs);
    strcat(str, hldj);
    printf("Opening %s\n", str);

    if(!(fp = fopen(str, "r"))) {
	printf("Could not open header list file %s\n", str);
	exit(1);
    }
    fof_header_list(fp, gri->radar_name, fui->ray_que->time, hfile
		    , experiment, instrument, &header_start, &header_stop);
    fclose(fp);

    /*
     * suck in header information
     */
    slash_path(str,fcat_hdrs);
    for(a=experiment; *a; *a = tolower(*a), a++);
    strcat(str, experiment);
    strcat(str, "/");
    for(a=instrument; *a; *a = tolower(*a), a++);
    strcat(str, instrument);
    strcat(str, "/");
    for(a=hfile; *a; *a = tolower(*a), a++);
    strcat(hfile, ".job");
    strcat(str, hfile);

    a = header_info;
    strcpy(a, hfile);
    dts.time_stamp = header_start;
    sprintf(a+strlen(a), " from %s to ", dts_print(d_unstamp_time(&dts)));
    dts.time_stamp = header_stop;
    sprintf(a+strlen(a), "%s", dts_print(d_unstamp_time(&dts)));
    
    printf("Opening %s\n", str);

    if(!(fp = fopen(str, "r"))) {
	printf("Could not open catalog header file %s\n", str);
	exit(1);
    }
    return(fp);
}
/* c------------------------------------------------------------------------ */

double 
fof_crack_vms_date (char **sptrs, int nargs, DD_TIME *dts)
{
    int ii, jj, hh=0, mm=0, ss=0, mark;
    char *a, *b, str[16];
    double d;

    strcpy(str, *sptrs);
    for(a=b=str; *a; a++) if(*a == '-') *a = '\0';

    dts->day = atoi(b);
    b += strlen(b) +1;
    for(ii=1; ii <= 12 && strcmp(Months[ii], b); ii++);
    dts->month = ii;
    b += strlen(b) +1;
    dts->year = 1900 + atoi(b);

    if(nargs > 1) {
	if((jj = sscanf(*(sptrs+1), "%d:%d:%d", &hh, &mm, &ss)) == 3) {
	    mark = 0;
	}
	else if((jj = sscanf(*(sptrs+1), "%d:%d", &hh, &mm)) == 2) {
	    ss = 0;
	}
	else if((jj = sscanf(*(sptrs+1), "%d", &hh)) == 1) {
	    mm = ss = 0;
	}
	else {
	    hh = mm = ss = 0;
	}
    }
    dts->day_seconds = D_SECS(hh, mm, ss, 0);
    return(d_time_stamp(dts));
}
/* c------------------------------------------------------------------------ */

struct data_field_info *
fof_data_field_spair (void)
{
    /* return a datafield struct
     */
    struct data_field_info *this_info=data_field_spairs;  //Jul 26, 2011 *this

    if(!this_info) {
	this_info = data_field_spairs = SMALLOC(struct data_field_info);
	if(!this_info) {
	    printf("Could not allocate another data_field_spair\n");
	    exit(1);
	}
	memset(this_info, 0, sizeof(struct data_field_info));
    }
    data_field_spairs = data_field_spairs->next;
    this_info->missing = EMPTY_FLAG;
    this_info->dd_scale = 100.;
    this_info->raw_counts = NO;
    return(this_info);
}
/* c------------------------------------------------------------------------ */

void 
fof_dump_this_ray (struct solo_list_mgmt *slm)
{
    int gg, ii, jj, kk, nn, nf=gri->num_fields_present;
    float unscale[7], bias[7];
    char *a, *b, *c, str[128];

    if(nf > 7) nf = 7;		/* make it 7 or less */
    a = str;
    solo_reset_list_entries(slm);
    solo_add_list_entry(slm, " ");
    strcpy(a, "     ");

    for(b=a+strlen(a),ii=0; ii < nf; ii++, b+=strlen(b)) {
	sprintf(b, "        %s", gri->field_name[ii]);
	unscale[ii] = gri->dd_scale[ii] ? 1./gri->dd_scale[ii] : 1.;
	bias[ii] = gri->dd_offset[ii];
    }
    solo_add_list_entry(slm, a);
    fof_nab_data();

    for(b=a,gg=0; gg < gri->num_bins; gg++, b = a) {
	sprintf(b, "%4d)", gg);

	for(b+=strlen(b),ii=0; ii < nf; ii++, b+=strlen(b)) {
	    sprintf(b, "%10.2f",
		    DD_UNSCALE(*(gri->scaled_data[ii]+gg), unscale[ii]
			       , bias[ii]));
	}
	solo_add_list_entry(slm, a);
    }
    slm_print_list(slm);
}
/* c------------------------------------------------------------------------ */

struct data_field_info *
fof_duplicate_field (char *field_name)
{
    struct data_field_info *datafx=fui->dataf;

    for(; datafx; datafx = datafx->next) {
	if(MATCH2(datafx->field_name, field_name)) {
	    break;
	}
    }
    return(datafx);
}
/* c------------------------------------------------------------------------ */

void 
fof_era (void)
{
    int ii, era;
    double d, t1, t2;
    DD_TIME dts;
    RSF_HOUSE *hsk=(RSF_HOUSE *)fui->ray_que->at;
    struct fof_ray_que *rq=fui->ray_que;

    dts.day_seconds = 0;
    era = NCP_AFTER_APRIL_86;

    dts.year = 1986; dts.month = 4; dts.day = 1;
    if(rq->time < d_time_stamp(&dts)) /* CP2 & CP4 the same at this point */
	  era = MAYPOLE84_NCP;
    /*
     * CP4 did not change until 1985
     */
    dts.year = 1985; dts.month = 1; dts.day = 1;
    if(hsk->rs_id == CP4 && rq->time < d_time_stamp(&dts))
	  era = EARLIEST_CP4_NCP;			  
							  
    dts.year = 1984; dts.month = 1; dts.day = 1;	  
    if(hsk->rs_id == CP2 && rq->time < d_time_stamp(&dts))
	  era = CP2_MAYPOLE83_NCP;			  
    							  
    dts.year = 1983; dts.month = 5; dts.day = 24;	  
    if(hsk->rs_id == CP2 && rq->time < d_time_stamp(&dts))
	  era = EARLIEST_CP2_NCP;
    
    fui->era = era;

    era = 0;
    dts.time_stamp = rq->time;
    d_unstamp_time(&dts);

    if(hsk->rs_id == MHR && dts.year == 1989 && hsk->hsk_len == 256) {
	/* bit 6 of the MHR flag field is set when the SNR
	 * is above some threshold */
	era = LUTZ89_MHR_FLAG;
    }

    fui->era |= era;

    fui->scan_algorithm = fof_MHR_or_cape;

    if(dts.year > 1990 || hsk->rs_id == MHR) {
	if(hsk->rs_id == CP2)
	      new_sweep_hesitation = 1;
    }
    if(dts.year < 1991 && hsk->rs_id == CP3) {
	fui->scan_algorithm = fof_mist_cp3;
    }
    dts.day_seconds = 0;

    dts.year = 1989; dts.month = 1; dts.day = 1;
    t1 = d_time_stamp(&dts);
    dts.year = 1990; dts.month = 1; dts.day = 1;
    t2 = d_time_stamp(&dts);

    if(hsk->rs_id == MHR && hsk->hsk_len == 256
       && t1 < rq->time < t2) {
    }
}
/* c------------------------------------------------------------------------ */

struct data_field_info *
fof_establish_data_field (char *field_name)
{
    int after=NO;
    struct data_field_info *dataf;

    dataf = fof_data_field_spair();
    dataf->srsf = fof_possible_field(field_name, &dataf->derf);
    if(dataf->derf) {
	dataf->field_name = dataf->derf->field_name;
	dataf->long_field_name = dataf->derf->long_field_name;
    }
    else if(dataf->srsf) {
	dataf->field_name = dataf->srsf->field_name;
	dataf->long_field_name = dataf->srsf->long_field_name;
    }
    fof_setup_field(dataf);
    return(dataf);
}
/* c------------------------------------------------------------------------ */

struct source_field_info *
fof_find_srs_fld (char *field_name)
{
    struct source_field_info *srsf;

    for(srsf=sources_top; srsf; srsf = srsf->next) {
	if(MATCH2(field_name, srsf->field_name)) {
	    break;
	}
    }
    return(srsf);
}
/* c------------------------------------------------------------------------ */

void 
fof_gen_counts_field (struct data_field_info *dataf)
{
    /* generate this field directly from a lookup table
     */
    int gw, mark;
    float *val, *valz;
    unsigned char *gval;

    gval = (unsigned char *)fui->ray_que->data
	  + dataf->srsf->final_offset;
    gw = fui->sizeof_gate_info;
    val = dataf->vals;
    valz = val + fui->ray_que->num_gates;

    for(; val < valz; gval+=gw, val++) {
	*val = *gval;
    }
}
/* c------------------------------------------------------------------------ */

void 
fof_gen_dbz_field (struct data_field_info *dataf)
{
    int gg, gw, ii, ng, mark;
    float *val, *thrf, *sf, thr, mis, *rngc;
    unsigned char *gval;

    ng = fui->ray_que->num_gates;
    thrf = dataf->thr_fld->vals;
    sf = dataf->src_fld->vals;
    val = dataf->vals;
    rngc = dataf->src_fld->srsf->rngcvals;
    thr = dataf->threshold;
    mis = dataf->missing;

    if(dataf->hi_threshold) {	/* don't really do any thresholding */
	for(gg=0; gg < ng; gg++, sf++, thrf++, val++) {
	    *val = *sf + (*rngc);
	}
    }
    else {
	for(gg=0; gg < ng; gg++, sf++, thrf++, val++, rngc++) {
	    *val = *thrf < thr ? mis : *sf + (*rngc);
	}
    }
}
/* c------------------------------------------------------------------------ */

void 
fof_gen_field (struct data_field_info *dataf)
{
    /* generate this field directly from a lookup table
     */
    int gg, ng, gw, ii, mark, *deep6=0;
    float *lut, *val, *valz;
    unsigned char *gval;

    gval = (unsigned char *)fui->ray_que->data
	  + dataf->srsf->final_offset;
    ng = fui->ray_que->num_gates;
    if(ng > 2048) {
	mark = *deep6;
    }
    gw = fui->sizeof_gate_info;
    lut = dataf->lut;
    val = dataf->vals;
    valz = val + fui->ray_que->num_gates;

    for(; val < valz; gval+=gw, val++) {
	*val = *(lut + (*gval));
    }
}
/* c------------------------------------------------------------------------ */

void 
fof_gen_ldr_field (struct data_field_info *dataf)
{
    int gg, gw, ii, ng, mark;
    float *val, hthrp, vthrp, vnoise, ldr_bias, mis, *hpow, *vpow;
    float xh_cutoff, xv_cutoff, hpw, vpw;
    unsigned char *gval;
    double d;

    mis = dataf->missing;
    ng = fui->ray_que->num_gates;

    val = dataf->vals;
    hpow = dataf->src_flda->vals; /* h power */
    vpow = dataf->src_fldb->vals; /* v power */
    hthrp = dataf->src_flda->srsf->cal_info->power_threshold;
    vthrp = dataf->src_fldb->srsf->cal_info->power_threshold;
    vnoise = dataf->src_fldb->srsf->cal_info->noise_power;
    ldr_bias = dataf->src_flda->srsf->sysatts->ldr_bias;
    xh_cutoff = dataf->src_flda->srsf->sysatts->h_cutoff;
    xv_cutoff = dataf->src_flda->srsf->sysatts->v_cutoff;


    for(gg=0; gg < ng; gg++, hpow++, vpow++, val++) {
	*val = *vpow <= vnoise ? mis : *vpow - *hpow + ldr_bias;
    }
}
/* c------------------------------------------------------------------------ */

void 
fof_gen_noiseless_zdr (struct data_field_info *dataf)
{
    int gg, gw, ii, ng, mark;
    float *val, *thrf, *zr, thr, mis, *hpoww, *rcp_zr, zdr_bias, zdr_unfold;
    unsigned char *gval;
    double hpw, hpthrw, vpw, hnoisew, vnoisew, minus90dbm;

    val = dataf->vals;
    ng = fui->ray_que->num_gates;
    zr = dataf->src_fld->vals;	/* ZR */
    hpoww = dataf->src_flda->vals; /* h power in watts */
    rcp_zr = dataf->src_fldb->vals; /* 1/watts */
    mis = dataf->missing;
    zdr_bias = dataf->src_flda->srsf->sysatts->zdr_bias;
    hnoisew = WATTZ(dataf->src_flda->srsf->cal_info->noise_power);
    vnoisew = WATTZ(dataf->src_flda->srsf->cal_info->v_noise_power);
    hpthrw = WATTZ(dataf->src_flda->srsf->cal_info->power_threshold);
    minus90dbm = WATTZ(-90.);
    zdr_unfold = dataf->src_fld->srsf->zdr_scale * 2.;


    for(gg=0; gg < ng; gg++, zr++, hpoww++, rcp_zr++, val++) {
	if((hpw = *hpoww) < hpthrw) {
	    *val = mis;
	}
	else if(hpw > minus90dbm) { /* the noise does not affect the outcome */
	    *val = *zr < -2. ? *zr + zdr_unfold + zdr_bias : *zr + zdr_bias;
	}
	else if((vpw = (hpw + hnoisew) * (*rcp_zr) - vnoisew) <= 0) {
	    *val = mis;		/* cannot derive the vertical power */
	}
	else {
	    *val = W_TO_DBM(hpw/vpw) + zdr_bias;
	}
    }
}
/* c------------------------------------------------------------------------ */

void 
fof_gen_unfolded_linear (struct data_field_info *dataf)
{
    int gg, gw, ii, jj, kk, nn, mark;
    float *lut, *val, *logvideo, saturation, valx;
    unsigned char *lin, *gval;
    double d, zmin;

    gval = (unsigned char *)fui->ray_que->data
	  + dataf->srsf->final_offset;
    gw = fui->sizeof_gate_info;
    nn = fui->ray_que->num_gates;
    lut = dataf->lut;
    lut = dataf->srsf->cal_info->dbm_vals;
    saturation = dataf->srsf->cal_info->saturation_threshold;
    val = dataf->vals;
    logvideo = dataf->src_fld->vals;

    for(gg=0; gg < nn; gg++, gval+=gw, val++, logvideo++) {
	if(*logvideo > saturation) {
	    *val = *logvideo;
	    continue;
	}
	zmin = MAX_FLOAT;
	ii = *gval;
	/*
	 * look at 4 possible values for the linear power
	 */
	for(; ii < 1024; ii+=256) {
	    *val = *(lut + ii);
	    if((d = FABS(*val - *logvideo)) < zmin) {
		zmin = d;
		valx = *val;
	    }
	}
	*val = valx;
    }
}
/* c------------------------------------------------------------------------ */

void 
fof_gen_quotient_field (struct data_field_info *dataf)
{
    /* create a field by dividing one field by another
     * or multiplying the the reciprocal of the denomenater
     */
    int gg, ng, mark;
    float *val, *rcp_denomenater, *numerater;

    ng = fui->ray_que->num_gates;
    numerater = dataf->src_flda->vals;
    rcp_denomenater = dataf->src_fldb->vals;
    val = dataf->vals;

    for(gg=0; gg < ng; gg++, val++, numerater++, rcp_denomenater++) {
	*val = *numerater * (*rcp_denomenater);
    }
}
/* c------------------------------------------------------------------------ */

void 
fof_gen_snr (struct data_field_info *dataf)
{
    int gg, ng;
    float *powr, *val;
    double d, rcp_noise;

    val = dataf->vals;
    ng = fui->ray_que->num_gates;
    powr = dataf->src_fld->vals;
    rcp_noise = 1./WATTZ(dataf->src_fld->srsf->cal_info->noise_power);

    for(gg=0; gg < ng; gg++, powr++, val++) {
	d = WATTZ(*powr) * rcp_noise;
	*val = W_TO_DBM(d);
    }
}
/* c------------------------------------------------------------------------ */

void 
fof_gen_sw (struct data_field_info *dataf)
{
    int gg, ng;
    float *snr, *ncp, *val, mis=EMPTY_FLAG;
    double d, A;

    val = dataf->vals;
    ng = fui->ray_que->num_gates;
    snr = dataf->src_flda->vals;
    ncp = dataf->src_fldb->vals;

    A = dataf->srsf->sysatts->PRF * dataf->srsf->sysatts->wave_length
	  /SQRT8XPI;

    for(gg=0; gg < ng; gg++, snr++, ncp++, val++) {
	if(*ncp <= 0) {
	    *val = mis;
	}
	else {
	    if((d = -LOGN(*ncp + (*ncp)/WATTZ(*snr))) >= 0) {
		*val = A * SQRT(d);
	    }
	    else {
		*val = mis;
	    }
	}
    }
}
/* c------------------------------------------------------------------------ */

void 
fof_gen_thrd_field (struct data_field_info *dataf)
{
    int gg, gw, ii, ng, mark;
    float *val, *thrf, *sf, thr, mis;
    unsigned char *gval;

    ng = fui->ray_que->num_gates;
    thrf = dataf->thr_fld->vals;
    sf = dataf->src_fld->vals;
    val = dataf->vals;
    thr = dataf->threshold;
    mis = dataf->missing;

    if(dataf->hi_threshold) {
	for(gg=0; gg < ng; gg++, sf++, thrf++, val++) {
	    *val = *thrf > thr ? mis : *sf;
	}
    }
    else {
	for(gg=0; gg < ng; gg++, sf++, thrf++, val++) {
	    *val = *thrf < thr ? mis : *sf;
	}
    }
}
/* c------------------------------------------------------------------------ */

void 
fof_gen_unfolded_xvert (struct data_field_info *dataf)
{
    int gg, gw, ii, ng, mark;
    float *val, *lut, hthrp, vthrp, vnoise, ldr_bias, mis, *hpow, *vpow;
    float xh_cutoff, xv_cutoff, hpw, vpw;
    unsigned char *gval;
    double d;

    mis = dataf->missing;
    ng = fui->ray_que->num_gates;
    gval = (unsigned char *)fui->ray_que->data
	  + dataf->srsf->final_offset;
    gw = fui->sizeof_gate_info;

    val = dataf->vals;
    lut = dataf->lut;
    hpow = dataf->src_fld->vals; /* h power */
    xh_cutoff = dataf->srsf->sysatts->h_cutoff;
    xv_cutoff = dataf->srsf->sysatts->v_cutoff;


    for(gg=0; gg < ng; gg++, gval += gw, hpow++, vpow++, val++) {

	if((vpw = *(lut + (*gval))) < xv_cutoff && *hpow > xh_cutoff) {
	    *val = *(lut + (*gval +256));
	}
	else {
	    *val = vpw;
	}
    }
}
/* c------------------------------------------------------------------------ */

char *
fof_get_field_list_name (void)
{
    char *field_list=(char *)"/rdss/solo/catalog/fieldlist.job";
    char *a;

# ifdef FOF_FIELDLIST
    field_list = FOF_FIELDLIST;
# endif    
    if(a=getenv("FOF_FIELDLIST")) {
	field_list = a;
    }
    return(field_list);
}
/* c------------------------------------------------------------------------ */

void fof_header_list(FILE *fp, char *name, double ztime, 
                     char *headername, char *experiment, char *instrument,
					 double *start, double *stop)
{
    int ii, jj, kk, id, nn=0;
    char *a, att[1024], string_space[1024], *str_ptrs[32];
    DD_TIME dts;

    *headername = '\0';
    a = string_space;

    for(;;) {
	if(!znext_att(fp, att))
	      return;

	if(strncmp(att, "EXPERIMENT", 4) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    strcpy(experiment, string_space);
	}
	if(strncmp(att, "INSTRUMENT", 4) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    if(!strstr(name, string_space)) /* match names */
		  continue;
	    strcpy(instrument, string_space);

	    for(;;) {		/* absorb this header info */
		if(!znext_att(fp, att))
		      return;
		kk = zatt_arg_ptrs(att, string_space, str_ptrs);

		if(strncmp(att, "START", 4) == 0) {
		    *start = fof_crack_vms_date(str_ptrs, kk, &dts);
		}
		else if(strncmp(att, "STOP", 4) == 0) {
		    *stop = fof_crack_vms_date(str_ptrs, kk, &dts);
		}
		else if(strncmp(att, "HEADER", 5) == 0) {
		    strcpy(headername, string_space);
		}
		else if(strncmp(att, "QUIT", 4) == 0) {
		    break;
		}
	    }
	    if(ztime >= *start && ztime < *stop)
		  break;
	}
    }
}
/* c------------------------------------------------------------------------ */

void fof_ini (void)
{
    int fn, ii, jj, kk, nn, nt, mark, ok=YES;
    struct fof_sweep_que *sq;
    struct field_derivation_info *derf;
    struct source_field_info *srsf;
    struct final_field_info *finalf;
    struct fof_ray_que *rq;
    char str[256];
    char string_space[256], *str_ptrs[32], *ffn;
    char *a,*unk=(char *)"UNKNOWN", *field_name;
    float f;
    double d;
    FILE *fp;
    
    printf("FOF_FORMAT\n");

    dd_min_rays_per_sweep();	/* trigger min rays per sweep */
    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();
    gri = return_gri_ptr();
    /*
     * open up the input file
     */
    iri = dd_return_ios(0, FOF_FMT);
    nn = 0;

    if((a=get_tagged_string("SOURCE_DEV"))) {
	nn = dd_input_read_open(iri, a);
    }
    if(nn <= 0) {
	printf("SOURCE_DEV missing or incomplete\n");
	exit(1);
    }
    /* padded housekeeping
     */
    phsk.hsk_len = 74;
    phsk.log_rec_len = 576;
    phsk.num_lrecs = 1;
    phsk.lrec_num = 1;
    phsk.num_fields = 3;
    phsk.parm_desc1[0] = 0x1007;
    phsk.parm_desc1[1] = 0x9001;
    phsk.parm_desc1[2] = 0x8008;

    if(hostIsLittleEndian()) {
       xhsk = (RSF_HOUSE *)malloc(sizeof(RSF_HOUSE));
       memset(xhsk, 0, sizeof(RSF_HOUSE));
    }

    fui = SMALLOC(struct fof_useful_info);
    memset(fui, 0, sizeof(struct fof_useful_info));
    /*
     * keep around information on the last 32 sweeps
     */
    for(ii=0; ii < 32; ii++) {
	sq = fof_sweep_que_spair(&fui->sweep_que);
    }
    sq->next = fui->sweep_que;	/* complete the circular list */

    fui->scan_algorithm = fof_MHR_or_cape; /* initial scan delineation algo */
    fui->new_sweep = fui->new_vol = YES;
    fui->ray_que = fof_ray_que_spair();

    for(ii=0,jj=69; ii < 16; ii++) {
	desc_offset[ii] = jj-1;
	jj = (jj == 74) ? 161 : jj+1;
    }
    for(ii=0,jj=79; ii < 16; ii++) {
	scale_offset[ii] = jj-1;
	jj = (jj == 89) ? 212 : jj+2;
    }
    gri->binary_format = DD_16_BITS;
    gri->source_format = FOF_FMT;
    gri->missing_data_flag = EMPTY_FLAG;
    gri->radar_type = DD_RADAR_TYPE_GROUND;

    /* read in the field list stuff
     */
    a = fof_get_field_list_name();
    printf("Opening %s\n", a);
    if(!(fp = fopen(a, "r"))) {
	printf("Could not open field list file %s\n", a);
	exit(1);
    }
    fof_load_field_list(fp);
    fclose(fp);

    if(a=get_tagged_string("TIME_CORRECTION")) {
	ii = sscanf(a, "%f", &f);
	if(ii == 1)
	      fui->time_correction = f;
	printf( "Time correction: %.3f seconds\n", fui->time_correction);
    }
    if(a=get_tagged_string("ASCENDING_ONLY")) {
	fui->ascending_fxd_only = YES;
	printf( "Allow only ASCENDING fixed angles\n");
    }
    if(a=get_tagged_string("FOF_OMIT_TRANSITIONS")) {
	fui->ignore_transitions = YES;
	printf( "Omitting rays with transition flags\n");
    }
    if(a=get_tagged_string("NEW_SWEEP_HESITATION")) {
	ii = new_sweep_hesitation;
	jj = sscanf(a, "%d", &ii);
	if(ii > 0)
	      new_sweep_hesitation = ii;
	printf( "New sweep hesitation: %d rays\n", new_sweep_hesitation);
    }

    new_vol_flags = XFIELD_MIX | XSCAN_MODE 
	  | XDUAL_POL_MODE | XGATE_COUNT | XGATE_SPACING | XPRF;

    if(a = get_tagged_string("NEW_SWEEP_FLAGS")) {
	strcpy(string_space, a);
	nt = dd_tokens(string_space, str_ptrs);

	for(ii=0; ii < nt; ii++) {
	    if(strstr(str_ptrs[ii], "SCAN_N")) {
		fui->new_sweep_flags |= XSCAN_NUM;
	    }
	    if(strstr(str_ptrs[ii], "FIXED")) {
		fui->new_sweep_flags |= XFIXED_ANGLE;
	    }
	}
    }
    else {
	fui->new_sweep_flags = XFIXED_ANGLE | XSCAN_NUM;
    }
    fui->new_sweep_flags |= new_vol_flags;
    
    if(a = get_tagged_string("OUTPUT_FIELDS")) {
	if(strlen(a))
	      final_fields = a;
    }
    strcpy(string_space, final_fields);
    nt = dd_tokens(string_space, str_ptrs);

    for(fn=0; fn < nt; fn++) {
	/*
	 * see if we can find this string in first in the
	 * source fields and then in the derived fields
	 */
	for(derf=NULL,srsf=sources_top; srsf; srsf = srsf->next) {
	    if(MATCH2(str_ptrs[fn], srsf->field_name)) {
		field_name = srsf->field_name;
		break;
	    }
	}
	if(!srsf) {
	   for(derf=NULL,srsf=sources_top; srsf; srsf = srsf->next) {
	      if(MATCH2(str_ptrs[fn], srsf->source_name)) {
		 field_name = srsf->source_name;
		 break;
	      }
	   }
	}
	if(!srsf) {		/* look through the derived fields */
	    for(derf=dfields_top; derf; derf = derf->next) {
		if(MATCH2(str_ptrs[fn], derf->field_name)) {
		    field_name = derf->field_name;
		    break;
		}
	    }
	}
	if(!srsf && !derf) {
	    printf("Cannot produce field %s\n", str_ptrs[fn]);
	    ok = NO;
	    continue;
	}
	fui->num_final_fields++;
	finalf = SMALLOC(struct final_field_info);
	memset(finalf, 0, sizeof(struct final_field_info));
	if(!fui->finalf) {
	    fui->finalf = finalf;
	}
	else {
	    fui->finalf->last->next = finalf;
	    finalf->last = fui->finalf->last;
	}
	fui->finalf->last = finalf;
	strcpy(finalf->field_name, field_name);
	finalf->id_num = srsf ? srsf->id_num : derf->id_num;
	finalf->srsf = srsf;
	finalf->derf = derf;
    }
    if(!ok)
	  exit(1);

    fof_next_ray();
    return;
}
/* c------------------------------------------------------------------------ */

struct data_field_info *
fof_insert_data_field (struct data_field_info *this_info, struct data_field_info *that, int before)
{
    /* insert "this" data field struct befor or after "that" in the list
     */
    struct data_field_info *top=fui->dataf;

    if(!this_info)
	  return(NULL);

    if(!top) {
	fui->dataf = this_info;
	this_info->next = NULL;
	this_info->last = this_info;
	return(this_info);
    }
    if(!that || this_info == that)
	  return(this_info);

    if(before) {
	if(that == top) {
	    this_info->last = top->last;
	    this_info->next = top;
	    fui->dataf = this_info;
	}
	else {
	    this_info->next = that;
	    this_info->last = that->last;
	    that->last->next = this_info;
	    that->last = this_info;
	}
    }
    else {
	/* insert it after that
	 */
	if(that == top->last)
	      top->last = this_info;
	this_info->last = that;
	this_info->next = that->next;
	if(that->next)
	      that->next->last = this_info;
	that->next = this_info;
    }
    return(this_info);
}
/* c------------------------------------------------------------------------ */

int 
fof_inventory (struct input_read_info *iri)
{
    /* the purpose of this routine is to facilitate a more detailed
     * examination of the data
     */
    int ii=0, jj, max=gri_max_lines();
    int nn, ival;
    float val;
    char str[256];
    double d;
    struct io_packet *dp;


    for(;;) {
	for(ii=0; ii < max; ii++) {
	    fui->new_vol = fui->new_sweep = NO;
	    if((nn=fof_next_ray()) < 1) {
		break;
	    }
	    sprintf(preamble, "%2d", iri->top->b_num);
	    sprintf(postamble, " %x", fui->ray_que->flags);
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

int 
fof_isa_new_sweep (struct fof_ray_que *rq)
{
    int ii, jj, mark, its_changed=NO;
    struct fof_sweep_que *sq=fui->sweep_que;
    RSF_HOUSE *hsk=(RSF_HOUSE *)fui->ray_que->at;
    double d;



# ifdef obsolete
    if(rq->prev->flags & TFLAG_ON && !(rq->flags & TFLAG_ON)) {
	its_changed = YES;
	rq->flags |= TRANSITION_OVER;
    }
# endif
    if(rq->prev->vol_num != rq->vol_num) {
	its_changed = YES;
	rq->flags |= XVOL_NUM;
    }
    if(rq->prev->scan_mode != rq->scan_mode) {
	its_changed = YES;
	rq->flags |= XSCAN_MODE;
    }
    if(rq->prev->sweep_num != rq->sweep_num) {
	its_changed = YES;
	rq->flags |= XSCAN_NUM;
    }
    if(FABS(rq->dfx) > .0999) {
	its_changed = YES;
	rq->flags |= XFIXED_ANGLE;
    }
    if(rq->prev->num_gates != rq->num_gates) {
	its_changed = YES;
	rq->flags |= XGATE_COUNT;
    }
    if(rq->prev->gs != rq->gs) {
	its_changed = YES;
	rq->flags |= XGATE_SPACING;
    }
    if(hsk->rs_id != MHR && rq->prev->num_samples != rq->num_samples) {
	its_changed = YES;
	rq->flags |= XSAMPLE_COUNT;
    }
    if(rq->dprf + sq->dprf_sum > 1.) {
	its_changed = YES;
	rq->flags |= XPRF;
    }
    if(rq->datp + sq->datp_sum > 1.) {
	its_changed = YES;
	sq->atp_changed = YES;
	rq->flags |= XATP;
    }
    if(fabs(rq->daz + sq->daz_sum) > 360.) {
	its_changed = YES;
	rq->flags |= PLUS_360;
    }
    if(sq->ray_count >= min_rays_per_sweep
       && FABS(rq->drot) > .2 && FABS(rq->prev->drot) > .2 && 
       (((rq->drot + rq->prev->drot) * sq->drot_sum) < 0)) {
	/* rotation angle changed direction!
	 */
	its_changed = YES;
	rq->flags |= XDIRECTION;
    }
    if(fabs(rq->daz) > max_daz) {
	its_changed = YES;
	rq->flags |= GAP;
    }
    if(fabs(rq->del) > max_del) {
	its_changed = YES;
	rq->flags |= GAP;
    }
    if(sq->ray_count >= max_rays_per_sweep) {
	its_changed = YES;
	rq->flags |= MAX_RAYS;
    }
    /* check the mix and number of fields
     */

    for(ii=0; ii < rq->num_fields; ii++) {
	if(rq->fields_present[ii] != rq->prev->fields_present[ii])
	      break;
    }
    if(rq->num_fields != rq->prev->num_fields || ii != rq->num_fields) {
	its_changed = YES;
	rq->flags |= XFIELD_MIX;
    }
    
    if(hsk->rs_id == CP2 && rq->dual_pol_mode != rq->prev->dual_pol_mode) {
	its_changed = YES;
	rq->flags |= XDUAL_POL_MODE;
    }
    /*
     * now call the appropriate scan delineation algorithm
     */
    (*fui->scan_algorithm)(its_changed, rq); 


    if(fui->transition_count && fui->ignore_transitions) {
	fui->ignore_this_ray = YES;
    }
    return(fui->new_sweep);
}
/* c------------------------------------------------------------------------ */

int 
fof_isa_new_vol (void)
{
    return(fui->new_vol);
}
/* c------------------------------------------------------------------------ */

void 
fof_link_cat_info (void)
{
    struct calibration_info *zcal;
    struct source_field_info *srsf;
    /*
     * link system attributes and calibration info
     * this little exercise guarentees that every source field
     * can get at a noise power.
     * It's necessary because you can't bank on the order of sysatts
     * and calibrations in the catalog file.
     */

    for(zcal=cals_top; zcal; zcal = zcal->next) {
	for(srsf=sources_top; srsf; srsf = srsf->next) {
	    if(srsf->cal_info == zcal) {
		srsf->sysatts->h_noise_power = zcal->noise_power;
		srsf->sysatts->h_noise_power_watts = WATTZ(zcal->noise_power);
		if(v_noise_power) {
		    srsf->sysatts->v_noise_power = v_noise_power;
		    srsf->sysatts->v_noise_power_watts = WATTZ(v_noise_power);
		}
		else {
		    srsf->sysatts->v_noise_power =
			  srsf->sysatts->h_noise_power;
		    srsf->sysatts->v_noise_power_watts =
			  srsf->sysatts->h_noise_power_watts;
		}
	    }
	}
    }
}
/* c------------------------------------------------------------------------ */

void fof_load_field_list(FILE *fp)
{
    int ii, jj, kk, id, nn=0;
    char att[1024], string_space[1024], *str_ptrs[32];
    struct field_derivation_info *derf, *derfx;
    struct source_field_info *srsf;


    for(;;) {
	if(!znext_att(fp, att))
	      break;

	if(strncmp(att, "source", 4) == 0) {
	    if((kk=zatt_arg_ptrs(att, string_space, str_ptrs)) < 5)
		  continue;
	    id = atoi(str_ptrs[2]);
	    if(!(srsf = SMALLOC(struct source_field_info))) {
		printf("Unable to allocate struct source_field_info\n");
		exit(1);
	    }
	    memset(srsf, 0, sizeof(struct field_derivation_info));
	    source_fields[id] = srsf;
	    num_source_ids++;
	    srsf->id_num = id;
	    strcpy(srsf->source_name, str_ptrs[1]);
	    strcpy(srsf->field_name, str_ptrs[0]);
	    strcpy(srsf->long_field_name, str_ptrs[4]+1);
	    srsf->x_band = strstr(srsf->long_field_name, "secondary")
		  ? YES : NO;
	    if(!sources_top) {
		sources_top = srsf;
	    }
	    else {
		srsf->last = sources_top->last;
		sources_top->last->next = srsf;
	    }
	    sources_top->last = srsf;
	}
	else if(strncmp(att, "dfield", 4) == 0) {
	    if((kk=zatt_arg_ptrs(att, string_space, str_ptrs)) < 4)
		  continue;
	    if(!(derf = SMALLOC(struct field_derivation_info))) {
		printf("Unable to allocat struct field_derivation_info\n");
		exit(1);
	    }
	    memset(derf, 0, sizeof(struct field_derivation_info));
	    nn++;
	    if(!dfields_top) {
		dfields_top = derf;
	    }
	    else {
		derf->last = dfields_top->last;
		dfields_top->last->next = derf;
	    }
	    dfields_top->last = derf;
	    strcpy(derf->source_name, str_ptrs[1]);
	    strcpy(derf->field_name, str_ptrs[0]);
	    strcpy(derf->long_field_name, str_ptrs[3]+1);
	}
    }
    /* create links between derived fields and source fields
     */
    if(!num_source_ids)
	  return;
    derf = dfields_top;
    for(; derf; derf = derf->next) { /* for each derived field spec */
	srsf = sources_top;
	for(; srsf; srsf = srsf->next) { /* look through the source fields */
	    if(MATCHN(derf->source_name, srsf->field_name, 2)) {
		derf->id_num = srsf->id_num;
		break;
	    }
	}
    }
    /* sneak in another derived field (NC from FL)
     */
    derf = dfields_top;
    for(derfx=NULL; derf; derf = derf->next) { 
	if(MATCHN(derf->field_name, "NC", 2)) { /* there will be more
						  * than on entry */
	    derfx = derf;
	}
	else if(derfx)
	      break;		/* pointing to last "NC" entry */
    }
    if(!(derf = SMALLOC(struct field_derivation_info))) {
	printf("Unable to allocat last struct field_derivation_info\n");
	exit(1);
    }
    memset(derf, 0, sizeof(struct field_derivation_info));
    memcpy(derf, derfx, sizeof(struct field_derivation_info));
    strcpy(derf->source_name, "FL");
    derf->id_num = 9;
    derf->next = derfx->next;
    derf->last = derfx;
    derfx->next->last = derf;
    derfx->next = derf;
}
/* c------------------------------------------------------------------------ */

void 
fof_luts (struct source_field_info *srsf)
{
    int ii, jj, kk, hbit, mask, r0bit, mark, fbit;
    float f, sc, dbm_scale, scale, bias, *val, *lut, *dbm;
    DD_TIME dts;
    double d, dtime,xp, mantissa;
    struct dual_pol_mode dpm;
    RSF_HOUSE *hsk=(RSF_HOUSE *)fui->ray_que->at;
    struct fof_ray_que *rq=fui->ray_que;



    if(strstr("CP.CX.CY", srsf->source_name))
	  return;

    srsf->prf = rq->prf;
    memcpy(&dpm, &hsk->dual_pol_mode, sizeof(short));

    if(!srsf->lut2) {
	if(!(srsf->lut2 = FMALLOC(256))) {
	    printf("Unable to malloc srsf->lut2\n");
	    exit(1);
	}
	memset(srsf->lut2, 0, 256*sizeof(float));
    }
    val = srsf->lut2;

    if(strstr("CV.CU.CS", srsf->source_name)) {
	/* do old style 1s comp velocities
	 * the last one is from JAWS where the id looks like 247
	 */
	scale = srsf->sysatts->wave_length * srsf->prf * .25/127.;

	if(hsk->rs_id == CP2 && dpm.half_nyq && dpm.dual_polar) {
	    /* cp2 at half Nyquist */
	    scale *= .5;
	}
	for(ii=0; ii < 256; ii++, val++) {
	    *val = (ii > 127 ? ii-255 : ii) * scale;
	}
    }
    else if(MATCH2("CD", srsf->source_name)) {
	/* raw ZDR */
	srsf->zdr_scale = dbm_scale = 3. * EXP2((double)dpm.zdr_scale);
	scale = dbm_scale/127.;

	for(ii=0; ii < 256; ii++, val++) {
	    *val = (ii > 127 ? ii-255 : ii) * scale;
	}
	/* create a second table that will be the denomenater in calculating
	 * ZDR with the noise power removed
	 */
	if(!srsf->lut3) {
	    if(!(srsf->lut3 = FMALLOC(256))) {
		printf("Unable to malloc srsf->lut3 for ZDR\n");
		exit(1);
	    }
	    memset(srsf->lut3, 0, 256*sizeof(float));
	}
	val = srsf->lut3;
	dbm = srsf->lut2;

	for(ii=0; ii < 256; ii++, val++) {
	    if((d = *(dbm + ii)) < -2.)	/* it's folded */
		  d += 2. * dbm_scale;
	    *val = 1./WATTZ(d); /* watts */
	}
    }
    else if(MATCH2("CT", srsf->source_name)) {
	/* r sub tau (coherent power) or the numerater of NCP
	 */
	if(EARLIEST_CP4_NCP SET_IN fui->era) {
	    hbit = 0;
	}
	else if(CP2_MAYPOLE83_NCP SET_IN fui->era) {
	    hbit = 64;
	}
	else if(MAYPOLE84_NCP SET_IN fui->era) {
	    hbit = 0;
	}
	else {
	    hbit = 0;
	}
	for(ii=0; ii < 256; ii++, val++) {
	    mantissa = ii % 64 + hbit;
	    xp = ii/64;
	    *val = mantissa * EXP2(xp);
	}
    }
    else if(MATCH2("CO", srsf->source_name)) {
	/* r sub zero (linear power) as used in the denomenater
	 * of NCP
	 */
	if(EARLIEST_CP4_NCP SET_IN fui->era) {
	    sc = 16.0;
	    mask = 16;
	    r0bit = 17;
	}
	else if(CP2_MAYPOLE83_NCP SET_IN fui->era) {
	    sc = 32.0;
	    mask = 16;
	    r0bit = 17;
	}
	else if(MAYPOLE84_NCP SET_IN fui->era) {
	    sc = 4.0;
	    mask = 64;
	    r0bit = 65;
	}
	else {
	    sc = 2.0;
	    mask = 64;
	    r0bit = 65;
	}
	/* turn this into the reciprocal of the denomenater
	 * in the NCP calculation
	 */
	for(ii=0; ii < 256; ii++, val++) {
	    *val = 1./(sc * ((ii % mask + r0bit)));
	}
    }
    else if(MATCH2("CQ", srsf->source_name)) { /* r sub tau & r sub zero
						   * or r sub CHUCK
						   */
	if(EARLIEST_CP2_NCP SET_IN fui->era) {
	    for(ii=0; ii < 256; ii++, val++) {
		xp = (ii/4) % 4;
		mantissa = ii % 4;
		*val = mantissa * EXP2(xp)/34.;
	    }
	}
	else if(CP2_MAYPOLE83_NCP SET_IN fui->era) {
	    for(ii=0; ii < 256; ii++, val++) {
		xp = ((ii/4) % 4) -2;
		mantissa = ((ii/4) % 4) + 8;
		scale = (float)(((ii/16) % 2) * 8) + 17.;
		*val = mantissa * EXP2(xp)/scale;
	    }
	}
	else {			/* change during maypole 83 */
	    for(ii=0; ii < 256; ii++, val++) {
		xp = ((ii/4) % 4);
		mantissa = ((ii/4) % 4);
		scale = (float)(((ii/16) % 16)) + 17.;
		*val = mantissa * EXP2(xp)/scale;
	    }
	}
    }
    else if(strstr("FL", srsf->source_name)) { 
	for(ii=0; ii < 256; ii++, val++) { /* flag in low order bit */
	    *val = ii & 1;
	}
    }
    else if(strstr("BF", srsf->source_name)) { /* MHR processor */
	/* fake an SNR
	 */
	if(LUTZ89_MHR_FLAG SET_IN fui->era) { /* flagged when good */
	    for(ii=0; ii < 256; ii++, val++) { 
		*val = !(BIT6 SET_IN ii) ? -33. : 33.;
	    }
	}
	else {
	    for(ii=0; ii < 256; ii++, val++) { /* flagged when bad */
		*val = BIT4 SET_IN ii ? -33. : 33.;
	    }
	}

    }
    else if(MATCH2("CW", srsf->source_name)) { /* raw spectral width */
	scale = srsf->sysatts->wave_length * srsf->prf * .25/255.;

	for(ii=0; ii < 256; ii++, val++) {
	    *val = ii*scale;
	}
    }
    else if(*srsf->source_name == 'B' || *srsf->source_name == 'S' ||
	    *srsf->source_name == 'P') { /* MHR and RP7 scaled and biased
					  * fields */
	scale = srsf->scale;
	bias = srsf->bias;

	for(ii=0; ii < 256; ii++, val++) {
	    *val = ii*scale +bias;
	}
    }
    else {
	printf("No lookup table for %s\n", srsf->source_name);
	mark = 0;
    }
    mark = 0;
}
/* c------------------------------------------------------------------------ */

void 
fof_malloc_vals (struct data_field_info *dataf)
{
    if(dataf->num_gates < dataf->srsf->num_gates) {
	dataf->num_gates = dataf->srsf->num_gates;

	if(dataf->vals) free(dataf->vals);

	if(!(dataf->vals = FMALLOC(dataf->num_gates))) {
	    printf("Unable to malloc dataf->vals\n");
	    exit(1);
	}
	memset(dataf->vals, 0, sizeof(float) * dataf->num_gates);
    }
}
/* c------------------------------------------------------------------------------- */

void 
fof_MHR_or_cape (int its_changed, struct fof_ray_que *rq)
{
    double d;

    /*
     * do we really have a new sweep
     */
    if(its_changed || rq->flags & TFLAG_ON
       || fui->presumed_new_vol || fui->presumed_new_sweep) {

	if(rq->flags & XVOL_NUM) {
	    fui->presumed_new_vol = YES;
	}
	/* sometimes the volume number changes prematurely
	 * so we let it go until the transition flag turns off
	 */
	if(fui->presumed_new_vol && rq->flags & TFLAG_ON)
	      fui->presumed_new_sweep = YES;
	
	if(rq->flags & new_vol_flags) {
	    fui->presumed_new_vol = fui->presumed_new_sweep = YES;
	}
	else if(rq->flags & fui->new_sweep_flags) {
	    fui->presumed_new_sweep = YES;
	}
	if(fui->presumed_new_sweep || fui->presumed_new_vol) {
	    fui->transition_count++;
	}
    }

    if(fui->transition_count && !(rq->flags & TFLAG_ON)) {
	/*
	 * don't permit any changes until the transition flag is off
	 * and you don't need to hesitate any longer
	 */
	if(fui->hesitation_count++ >= new_sweep_hesitation) {
	    if(fui->presumed_new_vol && fui->presumed_new_sweep) {
		fui->new_vol = YES;
	    }
	    if(fui->presumed_new_sweep) {
		fui->new_sweep = YES;
	    }
	}
	if(fui->new_sweep || fui->new_vol) {
	    fui->hesitation_count = fui->transition_count = 0;
	    fui->presumed_new_vol = fui->presumed_new_sweep = NO;
	}
    }
}
/* c------------------------------------------------------------------------ */

void 
fof_mist_cp3 (int its_changed, struct fof_ray_que *rq)
{
    double d;


    if(rq->flags & XVOL_NUM) {
	fui->presumed_new_vol = YES;
    }
    if(rq->flags & XFIXED_ANGLE) {
	if(!fui->sweep_que->sweep_num == 1) {
	    d = angdiff(fui->sweep_que->last->fx, fui->sweep_que->fx);
	    if(d * rq->dfx < 0) {
		/* sign change...assume new volume
		 */
		fui->presumed_new_vol = YES;
	    }
	}
	else if(rq->dfx < 0){	/* going back down */
		fui->presumed_new_vol = YES;
	}
    }
    /*           c...mark
     *
     * do we really have a new sweep
     */
    if(its_changed || rq->flags & TFLAG_ON
       || fui->presumed_new_vol|| fui->presumed_new_sweep) {
	/* sometimes the volume number changes prematurely
	 * so we let it go until the transition flag turns off
	 */
	if(fui->presumed_new_vol && rq->flags & TFLAG_ON)
	      fui->presumed_new_sweep = YES;
	
	if(rq->flags & new_vol_flags) {
	    fui->presumed_new_vol = fui->presumed_new_sweep = YES;
	}
	else if(rq->flags & fui->new_sweep_flags) {
	    fui->presumed_new_sweep = YES;
	}
	if(fui->presumed_new_sweep || fui->presumed_new_vol) {
	    fui->transition_count++;
	}
    }

    if(fui->transition_count && !(rq->flags & TFLAG_ON)) {
	/*
	 * don't permit any changes until the transition flag is off
	 * and you don't need to hesitate any longer
	 */
	if(fui->hesitation_count++ >= new_sweep_hesitation) {
	    if(fui->presumed_new_vol && fui->presumed_new_sweep) {
		fui->new_vol = YES;
	    }
	    if(fui->presumed_new_sweep) {
		fui->new_sweep = YES;
	    }
	}
	if(fui->new_sweep || fui->new_vol) {
	    fui->hesitation_count = fui->transition_count = 0;
	    fui->presumed_new_vol = fui->presumed_new_sweep = NO;
	}
    }
}
/* c------------------------------------------------------------------------ */

void 
fof_nab_data (void)
{
    int ii, nn, fn, mark;
    struct data_field_info *dataf=fui->dataf;
    struct final_field_info *finalf=fui->finalf;
    float scale, bias, *ff, *ee, f_empty=EMPTY_FLAG;
    short *ss, *tt;

    /* generate all the source fields
     */
    for(; dataf; dataf = dataf->next) {
	(*dataf->proc_ptr)(dataf); 
    }
    mark = 0;
    /* generate the final scaled fields
     */
    for(fn=0; finalf; finalf = finalf->next, fn++) {
	ss = tt = gri->scaled_data[fn];
	bias = gri->dd_offset[fn];
	scale = gri->dd_scale[fn];
	ff = ee = finalf->dataf->vals;
	nn = gri->actual_num_bins[fn];

	for(ii=0; ii++ < nn; ff++) {
	    *ss++ = *ff == f_empty ? EMPTY_FLAG
		  : (short)DD_SCALE(*ff, scale, bias);
	}
	mark = 0;
    }
}
/* c------------------------------------------------------------------------ */

struct fof_sweep_que *
fof_new_sweep (struct fof_ray_que *rq)
{
    int ii, jj, kk, nn, mark, ok, before=YES, after=NO;
    struct fof_sweep_que *sq, *fsq;
    struct final_field_info *finalf;
    struct data_field_info *dataf;
    struct source_field_info *srsf;
    struct field_derivation_info *derf;
    struct fof_system_attributes *fsi;
    RSF_HOUSE *hsk;
    unsigned short *us;
    char *a, *b, *c, str[88];
    float r, gs;
    double d;

    
    fui->sweep_count_flag = YES;
    gri->sweep_num++;
    dd_stats->sweep_count++;

    if((fui->sweep_que->max_atp - fui->sweep_que->min_atp) > 1.5) {
# ifdef obsolete
	sprintf(str, "ATP Changed!  Average ATP for %d rays in sweep: %.2f"
		, fui->sweep_que->ray_count, fui->sweep_que->atp);
	printf("! %s\n", str);
	dd_append_cat_comment(str);

	sprintf(str, "First ATP: %.2f  Last ATP: %.2f"
		, fui->sweep_que->atp_start, fui->sweep_que->atp_end);
	printf("! %s\n", str);
	dd_append_cat_comment(str);

	sprintf(str, "Min ATP: %.2f at ray: %d %.1f   "
		, fui->sweep_que->min_atp, fui->sweep_que->min_atp_at
		, fui->sweep_que->rotang_min_atp);
	sprintf(str+strlen(str), "Max ATP: %.2f at ray: %d  "
		, fui->sweep_que->max_atp, fui->sweep_que->max_atp_at);
	printf("! %s\n", str);
	dd_append_cat_comment(str);
# endif
	mark = 0;
    }

    sq = fui->sweep_que = fui->sweep_que->next;
    sq->daz_sum = sq->del_sum = sq->dfx_sum = sq->fxd_sum = sq->drot_sum = 0;
    sq->atp_sum = sq->datp_sum = sq->dprf_sum = 0;
    sq->ray_count = 0;
    sq->atp_start = sq->min_atp = sq->max_atp = rq->atp;
    sq->sizeof_gate_info = fui->sizeof_gate_info;
    sq->something_changed_at = 0;
    sq->atp_changed = NO;
    sq->sweep_num = sq->last->sweep_num +1;
    sq->min_atp_at = sq->max_atp_at = 1;
    if(fui->ascending_fxd_only) {
	if(rq->fx >= fui->ref_fixed_angle)
	      fui->ref_fixed_angle = rq->fx; 
    }
    else {
	fui->ref_fixed_angle = rq->fx; 
    }
    /*
     * free up the ray que list two sweeps back
     */
    fof_stack_ray_que_spairs(sq->last->last->first_ray);
    sq->last->last->first_ray = NULL;
    if(sq->last->first_ray)
	  sq->last->first_ray->prev = NULL; /* end the backwards chain */

    sq->first_seq_num = rq->seq_num;

    return(sq);
}
/* c------------------------------------------------------------------------ */

void 
fof_new_vol (void)
{
    fui->sweep_que->sweep_num = 1;
    fui->vol_count_flag = YES;
    fui->ref_fixed_angle = fui->ray_que->fx;
    gri->vol_num++;
    dd_stats->vol_count++;
}
/* c------------------------------------------------------------------------ */

int 
fof_next_ray (void)
{
    int ii, jj, kk, mm, nn, phys_read=NO, mark, loop_count, eof_count=0;
    int ignoring_checks = NO;
    static int count=0, trip=111;
    char *c, *at;
    float f, frcp;
    double d;
    struct io_packet *dp;
    struct fof_ray_que *rq;
    struct fof_sweep_que *fsq;
    static RSF_HOUSE *hsk;
    short nlrc;

    /* c...mark */
    fui->ignore_this_ray = NO;


    for(loop_count=0;; loop_count++) {
	count++;
	if(count >= trip) {
	    mark = 0;
	}
	if(dd_control_c()) {
	    dd_reset_control_c();
	    return(-1);
	}

	if(iri->top->bytes_left < FOF_MIN_REC_SIZE) {
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
		    fui->new_vol = YES;
		    dd_stats->file_count++;
		    printf("EOF: %d at %.2f MB\n"
			   , dd_stats->file_count, dd_stats->MB);
		    if(++eof_count > 3)
			  return((int)0);
		}
		else if(iri->top->sizeof_read < FOF_MIN_REC_SIZE) {
		    eof_count = 0;
		    dd_stats->rec_count++;
		}
		else {
		    eof_count = 0;
		    fui->hsk64_flag =
			  iri->top->sizeof_read == 576*sizeof(short) ? YES : NO;
		    dd_stats->rec_count++;
		    dd_stats->MB += BYTES_TO_MB(iri->top->sizeof_read);
		    break;
		}
	    }
	    hsk = (RSF_HOUSE *)iri->top->at;
	    if(fui->hsk64_flag) {
	       fui->rays_in_rec = phsk.num_lrecs;
	    }
	    else {
	       fui->rays_in_rec = hsk->num_lrecs;
	    }
	    fui->ray_in_rec = 0;
	}
	if(!loop_count) {
	    rq = fof_ray_que_spair();
	    rq->prev = fui->ray_que;
	    fui->ray_que = rq;
	}
	fof_rayq_info(iri->top->at, rq);
	rq->seq_num = seq_num++;
	fui->ray_in_rec++;
	if((fui->ray_in_rec <= fui->rays_in_rec &&
	    rq->sizeof_ray > FOF_MIN_REC_SIZE &&
	    rq->sizeof_ray <= iri->top->bytes_left))
	      {
		  break;
	      }
	iri->top->bytes_left = 0;
    }
    dd_stats->ray_count++;
    dp = dd_return_next_packet(iri);
    dp->len = rq->sizeof_ray;
    /*
     * see if we need a new header
     */
    if(fui->ray_que->time < header_start ||
       fui->ray_que->time >= header_stop)
	  {
	      fof_catalog_header();
	      fof_era();
	  }
    rq->gs = fui->gs0 + ((RSF_HOUSE *)rq->at)->gs;

    if(fui->ray_in_rec == fui->rays_in_rec) {
	iri->top->bytes_left = 0;
    }
    else {
	iri->top->at += rq->sizeof_ray;
	iri->top->bytes_left -= rq->sizeof_ray;
    }
    fof_isa_new_sweep(rq);

    if(difs->max_beams) {
	if(fui->ray_count > 0) {
	    fui->new_vol = fui->new_sweep = NO;
	    fui->sweep_que->something_changed_at = 0;
	    ignoring_checks = YES;
	}
    }

    if((rq->flags & XFIELD_MIX) || (rq->flags & XGATE_COUNT)
       || (rq->flags & XPRF)|| (rq->flags & XGATE_SPACING)) {
	/* field mix has changed */
	if(!ignoring_checks) {
	    fui->sweep_que->something_changed_at = fui->sweep_que->ray_count;
	    fof_redo_srsfs(rq);
	}
    }

    if(fui->new_sweep) {
	fsq = fof_new_sweep(rq);
    }
    else {
	fsq = fui->sweep_que;
	fsq->drot_sum += rq->drot;
	fsq->daz_sum += rq->daz;
	fsq->del_sum += rq->del;
	fsq->dfx_sum += rq->dfx;
    }
    fsq->fxd_sum += rq->fx;
    fsq->atp_sum += rq->atp;
    if(rq->atp < fsq->min_atp) {
	fsq->min_atp = rq->atp;
	fsq->min_atp_at = fsq->ray_count;
	fsq->rotang_min_atp = rq->rotation_angle;
    }
    if(rq->atp > fsq->max_atp) {
	fsq->max_atp = rq->atp;
	fsq->max_atp_at = fsq->ray_count;
    }
    fsq->atp_end = rq->atp;
    fsq->datp_sum += rq->datp;
    fsq->dprf_sum += rq->dprf;
    fsq->ray_count++;
    fsq->fx = fsq->fxd_sum * (frcp=1./(double)fsq->ray_count);
    fsq->atp = fsq->atp_sum * frcp;
    fof_que_ray_info(&fsq->first_ray, rq);

    fui->sweep_que->last_seq_num = seq_num;

    if(fof_isa_new_vol()) {
	fof_new_vol();
    }
    gri->source_sweep_num = (TFLAG_ON & rq->flags) ? 1 : 0;
    gri->source_sweep_num += 10 * (rq->sweep_num % 10);
    gri->source_sweep_num += 100 * (gri->sweep_num % 10);
    gri->source_sweep_num += 1000 * (rq->vol_num % 10);
    gri->source_sweep_num += 10000 * (gri->vol_num % 10);

    return(dp->len);
}
/* c-------------------------------------------------------------------------------- */

void 
fof_positioning (void)
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
    double d, tol,dtarget;
    struct io_packet *dp, *dp0;
    struct input_buf *ib;

    slm = solo_malloc_list_mgmt(87);
    preamble[0] = '\0';
    gri_interest(gri, 1, preamble, postamble);


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
11 = Display data\n\
12 = Display housekeeping\n\
13 = Display catalog header\n\
14 = Display field list\n\
Option = "
	   );
    nn = getreply(str, sizeof(str));
    if( cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 14 ) {
	if(ival == -2) exit(1);
	printf( "\nIllegal Option!\n" );
	goto menu2;
    }
    fui->new_sweep = NO;
    fui->new_vol = NO;

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
		fui->new_sweep = NO;
		fui->new_vol = NO;
		if((jj=fof_next_ray()) < 0)
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
	    fof_reset();
	}
	fof_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 1) {
	if((nn = fof_inventory(iri)) == -2)
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
	dd_stats_reset();
	fof_reset();
	fof_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 3) {
	dd_rewind_dev(iri);
	dd_stats_reset();
	fof_reset();
	fof_next_ray();
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
	printf("Type record skip # or hit <return> to read next rec: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > 65536) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	direction = ival >= 0 ? FORWARD : BACKWARD;
	dd_skip_recs(iri, direction, ival > 0 ? ival : -ival);
	fof_reset();
	fof_next_ray();
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
	    if((nn = fof_next_ray()) <= 0 || gri->time >= dtarget ||
	       dd_control_c()) {
		break;
	    }
	    fui->new_sweep = NO;
	    fui->new_vol = NO;
	    if(!(mm % 1000)) {
		gri_interest(gri, 1, preamble, postamble);
		printf("%s\n", dd_stats_sprintf());
	    }
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
	    if((nn = fof_next_ray()) <= 0 || in_sector(gri->fixed, fx1, fx2)
	       || dd_control_c()) {
		break;
	    }
	    fui->new_sweep = NO;
	    fui->new_vol = NO;
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
	fof_dump_this_ray(slm);
    }
    else if(ival == 12) {
	fof_print_hsk(slm);
    }
    else if(ival == 13) {
	fof_print_headers(slm);
    }
    else if(ival == 14) {
	fof_print_fieldlist(slm);
    }

    preamble[0] = '\0';

    goto menu2;

}
/* c------------------------------------------------------------------------ */

struct source_field_info *
fof_possible_field (char *field_name, struct field_derivation_info **derf)
{
    struct source_field_info *srsf;

    *derf = NULL;
    /*
     * see if it's a derived field
     */
    if((srsf = fof_check_srs_fld(field_name, derf))) 
	  return(srsf);
    /*
     * now look for the source field
     */
    srsf = fof_find_srs_fld(field_name);
    if(srsf)
	  return(srsf->present ? srsf : NULL);

    return(srsf);
}
/* c------------------------------------------------------------------------ */

void 
fof_print_fieldlist (struct solo_list_mgmt *slm)
{
    char *a, *b, att[128];
    FILE *fp;

        /* read in the field list stuff
     */
    a = fof_get_field_list_name();
    printf("Opening %s\n", a);
    if(!(fp = fopen(a, "r"))) {
	printf("Could not open field list file %s\n", a);
	exit(1);
    }
    a = att;
    solo_reset_list_entries(slm);
    solo_add_list_entry(slm, " ");

    for(;;) {
	if(!xnext_att(fp, att))
	      break;
	if(MATCHN(att, "source:", 4) || MATCHN(att, "dfield:", 4))
	      solo_add_list_entry(slm, att);
    }
    fclose(fp);

    slm_print_list(slm);
}
/* c------------------------------------------------------------------------ */

void 
fof_print_headers (struct solo_list_mgmt *slm)
{
    /* print the catalog header
     */
    int ii, jj, kk, nn;
    char *a, *b, att[1024], str[256];
    FILE *fp;

    fp = fof_catalog_header_stream();
    a = att;
    solo_reset_list_entries(slm);
    solo_add_list_entry(slm, " ");
    solo_add_list_entry(slm, header_info);
    solo_add_list_entry(slm, " ");

    for(;;) {
	if(!xnext_att(fp, att))
	      break;
	solo_add_list_entry(slm, a);
    }
    slm_print_list(slm);
}
/* c------------------------------------------------------------------------ */

void 
fof_print_hsk (struct solo_list_mgmt *slm)
{
    int ii;
    char *a, *b, *c, str[256];
    RSF_HOUSE *hsk = (RSF_HOUSE *)fui->ray_que->at;
    SCALING *cal_info;

    a = str;
    solo_reset_list_entries(slm);
    solo_add_list_entry(slm, " ");

    sprintf(a, " rec_num           %6d", hsk->rec_num          );
    solo_add_list_entry(slm, a);
    sprintf(a, " field_tape_seq    %6d", hsk->field_tape_seq   );
    solo_add_list_entry(slm, a);
    sprintf(a, " rec_type          %6d", hsk->rec_type         );
    solo_add_list_entry(slm, a);
    sprintf(a, " year              %6d", hsk->year             );
    solo_add_list_entry(slm, a);
    sprintf(a, " month             %6d", hsk->month            );
    solo_add_list_entry(slm, a);
    sprintf(a, " day               %6d", hsk->day              );
    solo_add_list_entry(slm, a);
    sprintf(a, " hour              %6d", hsk->hour             );
    solo_add_list_entry(slm, a);
    sprintf(a, " minute            %6d", hsk->minute           );
    solo_add_list_entry(slm, a);
    sprintf(a, " second            %6d", hsk->second           );
    solo_add_list_entry(slm, a);
    sprintf(a, " az_xCF            %6d %7.2f"
	    , hsk->az_xCF, BA2F(hsk->az_xCF));
    solo_add_list_entry(slm, a);
    sprintf(a, " el_xCF            %6d %7.2f"
	    , hsk->el_xCF, BA2F(hsk->el_xCF));
    solo_add_list_entry(slm, a);
    sprintf(a, " rhozero1          %6d", hsk->rhozero1         );
    solo_add_list_entry(slm, a);
    sprintf(a, " rhozero2          %6d", hsk->rhozero2         );
    solo_add_list_entry(slm, a);
    sprintf(a, " gs                %6d", hsk->gs               );
    solo_add_list_entry(slm, a);
    sprintf(a, " num_gates         %6d", hsk->num_gates        );
    solo_add_list_entry(slm, a);
    sprintf(a, " samples_per_beam  %6d", hsk->samples_per_beam );
    solo_add_list_entry(slm, a);
    sprintf(a, " test_pulse_level  %6d", hsk->test_pulse_level );
    solo_add_list_entry(slm, a);
    sprintf(a, " atp               %6d", hsk->atp              );
    solo_add_list_entry(slm, a);
    sprintf(a, " pulse_width       %6d", hsk->pulse_width      );
    solo_add_list_entry(slm, a);
    sprintf(a, " prfx10            %6d", hsk->prfx10           );
    solo_add_list_entry(slm, a);
    sprintf(a, " wavelength        %6d", hsk->wavelength       );
    solo_add_list_entry(slm, a);
    sprintf(a, " swp_num           %6d", hsk->swp_num          );
    solo_add_list_entry(slm, a);
    sprintf(a, " sweep_index       %6d", hsk->sweep_index      );
    solo_add_list_entry(slm, a);

    solo_add_list_entry(slm, " ");
    /*
    sprintf(a, " unused1[2]        %6d", hsk->unused1[2]       );
    solo_add_list_entry(slm, a);
     */
    sprintf(a, " scan_mode         %6d", hsk->scan_mode        );
    solo_add_list_entry(slm, a);
    sprintf(a, " cw_az_lim         %6d", hsk->cw_az_lim        );
    solo_add_list_entry(slm, a);
    sprintf(a, " ccw_az_lim        %6d", hsk->ccw_az_lim       );
    solo_add_list_entry(slm, a);
    sprintf(a, " up_elev_lim       %6d", hsk->up_elev_lim      );
    solo_add_list_entry(slm, a);
    sprintf(a, " lo_elev_lim       %6d", hsk->lo_elev_lim      );
    solo_add_list_entry(slm, a);
    sprintf(a, " fixed_ang         %6d %7.2f"
	    , hsk->fixed_ang, BA2F(hsk->fixed_ang) );
    solo_add_list_entry(slm, a);
    sprintf(a, " sig_source        %6d", hsk->sig_source       );
    solo_add_list_entry(slm, a);
    sprintf(a, " coupler_loss      %6d", hsk->coupler_loss     );
    solo_add_list_entry(slm, a);
    sprintf(a, " tp_strt           %6d", hsk->tp_strt          );
    solo_add_list_entry(slm, a);
    sprintf(a, " tp_width          %6d", hsk->tp_width         );
    solo_add_list_entry(slm, a);
    sprintf(a, " pri_co_bl         %6d %7.2f"
	    , hsk->pri_co_bl, BA2F(hsk->pri_co_bl));
    solo_add_list_entry(slm, a);
    sprintf(a, " scnd_co_bl        %6d %7.2f"
	    , hsk->scnd_co_bl, BA2F(hsk->scnd_co_bl));
    solo_add_list_entry(slm, a);
    sprintf(a, " tp_atten          %6d", hsk->tp_atten         );
    solo_add_list_entry(slm, a);
    sprintf(a, " sys_gain          %6d", hsk->sys_gain         );
    solo_add_list_entry(slm, a);
    sprintf(a, " fix_tape          %6d", hsk->fix_tape         );
    solo_add_list_entry(slm, a);
    sprintf(a, " tp_freq_off       %6d", hsk->tp_freq_off      );
    solo_add_list_entry(slm, a);
    sprintf(a, " log_bw            %6d", hsk->log_bw           );
    solo_add_list_entry(slm, a);
    sprintf(a, " lin_bw            %6d", hsk->lin_bw           );
    solo_add_list_entry(slm, a);
    sprintf(a, " ant_bw            %6d", hsk->ant_bw           );
    solo_add_list_entry(slm, a);
    sprintf(a, " ant_scan_rate     %6d", hsk->ant_scan_rate    );
    solo_add_list_entry(slm, a);

    solo_add_list_entry(slm, " ");
    /*
    sprintf(a, " unused2[2]        %6d", hsk->unused2[2]       );
     */
    solo_add_list_entry(slm, a);
    sprintf(a, " vol_num           %6d", hsk->vol_num          );
    solo_add_list_entry(slm, a);
    sprintf(a, " clut_filt         %6d", hsk->clut_filt        );
    solo_add_list_entry(slm, a);
    sprintf(a, " polarization      %6d", hsk->polarization     );
    solo_add_list_entry(slm, a);
    sprintf(a, " prf1              %6d", hsk->prf1             );
    solo_add_list_entry(slm, a);
    sprintf(a, " prf2              %6d", hsk->prf2             );
    solo_add_list_entry(slm, a);
    sprintf(a, " prf3              %6d", hsk->prf3             );
    solo_add_list_entry(slm, a);
    sprintf(a, " prf4              %6d", hsk->prf4             );
    solo_add_list_entry(slm, a);
    sprintf(a, " prf5              %6d", hsk->prf5             );
    solo_add_list_entry(slm, a);

    solo_add_list_entry(slm, " ");
    /*
    sprintf(a, " unused3           %6d", hsk->unused3          );
    solo_add_list_entry(slm, a);
     */
    sprintf(a, " rec_num_d32768    %6d", hsk->rec_num_d32768   );
    solo_add_list_entry(slm, a);
    sprintf(a, " altitude          %6d", hsk->altitude         );
    solo_add_list_entry(slm, a);
    sprintf(a, " latitude          %6d", hsk->latitude         );
    solo_add_list_entry(slm, a);
    sprintf(a, " longitude         %6d", hsk->longitude        );
    solo_add_list_entry(slm, a);
    sprintf(a, " transit           %6d", hsk->transit          );
    solo_add_list_entry(slm, a);
    sprintf(a, " ds_id             %6d", hsk->ds_id            );
    solo_add_list_entry(slm, a);
    sprintf(a, " rs_id             %6d", hsk->rs_id            );
    solo_add_list_entry(slm, a);
    sprintf(a, " proj_num          %6d", hsk->proj_num         );
    solo_add_list_entry(slm, a);
    sprintf(a, " hsk_len           %6d", hsk->hsk_len          );
    solo_add_list_entry(slm, a);
    sprintf(a, " log_rec_len       %6d", hsk->log_rec_len      );
    solo_add_list_entry(slm, a);
    sprintf(a, " num_lrecs         %6d", hsk->num_lrecs        );
    solo_add_list_entry(slm, a);
    sprintf(a, " lrec_num          %6d", hsk->lrec_num         );
    solo_add_list_entry(slm, a);
    sprintf(a, " num_fields        %6d", hsk->num_fields       );
    solo_add_list_entry(slm, a);

    sprintf(a, " parm_desc1[6]");
    solo_add_list_entry(slm, a);

    for(b=a,ii=0; ii < 6; ii++, b+=strlen(b)) {
	sprintf(b, " %4x", hsk->parm_desc1[ii]);
    }
    solo_add_list_entry(slm, a);

    if(hsk->hsk_len <= 74) {
	slm_print_list(slm);
	return;
    }
    sprintf(a, " tp_max            %6d", hsk->tp_max           );
    solo_add_list_entry(slm, a);
    sprintf(a, " tp_min            %6d", hsk->tp_min           );
    solo_add_list_entry(slm, a);
    sprintf(a, " tp_step           %6d", hsk->tp_step          );
    solo_add_list_entry(slm, a);
    sprintf(a, " vol_scan_prg      %6d", hsk->vol_scan_prg     );
    solo_add_list_entry(slm, a);

    sprintf(a, " SCALING cal_info1[6]");
    solo_add_list_entry(slm, a);

    cal_info = hsk->cal_info1;

    for(b=a,ii=0; ii < 6; ii++, b += strlen(b), cal_info++) {
	sprintf(b, "  %4x %4x", cal_info->pscale, cal_info->pbias);
    }
    solo_add_list_entry(slm, a);

    sprintf(a, " rnoise_bdcal      %6d", hsk->rnoise_bdcal      );
    solo_add_list_entry(slm, a);
    sprintf(a, " rsolar            %6d", hsk->rsolar            );
    solo_add_list_entry(slm, a);
    sprintf(a, " rgcc              %6d", hsk->rgcc              );
    solo_add_list_entry(slm, a);
    sprintf(a, " rvtime            %6d", hsk->rvtime            );
    solo_add_list_entry(slm, a);
    sprintf(a, " rcrec             %6d", hsk->rcrec             );
    solo_add_list_entry(slm, a);

    solo_add_list_entry(slm, " ");
    /*
    sprintf(a, " unused4[4]        %6d", hsk->unused4[4]        );
    solo_add_list_entry(slm, a);
     */
    sprintf(a, " live_or_sim       %6d", hsk->live_or_sim       );
    solo_add_list_entry(slm, a);

    if(hsk->hsk_len <= 100) {
	slm_print_list(slm);
	return;
    }
    solo_add_list_entry(slm, " ");
    /*
    sprintf(a, " unused5[60]       %6d", hsk->unused5[60]       );
    solo_add_list_entry(slm, a);
     */
    sprintf(a, " parm_desc2[10]");
    solo_add_list_entry(slm, a);

    for(b=a,ii=0; ii < 10; ii++, b+=strlen(b)) {
	sprintf(b, " %4x", hsk->parm_desc2[ii]);
    }
    solo_add_list_entry(slm, a);

    sprintf(a, " src_test_bus      %6d", hsk->src_test_bus      );
    solo_add_list_entry(slm, a);
    sprintf(a, " add_test_bus      %6d", hsk->add_test_bus      );
    solo_add_list_entry(slm, a);
    sprintf(a, " half_prf          %6d", hsk->half_prf          );
    solo_add_list_entry(slm, a);
    sprintf(a, " ptape_unit        %6d", hsk->ptape_unit        );
    solo_add_list_entry(slm, a);
    sprintf(a, " stape_unit        %6d", hsk->stape_unit        );
    solo_add_list_entry(slm, a);
    sprintf(a, " word_176          %6d", hsk->word_176          );
    solo_add_list_entry(slm, a);
    sprintf(a, " word_177          %6d", hsk->word_177          );
    solo_add_list_entry(slm, a);
    sprintf(a, " word_178          %6d", hsk->word_178          );
    solo_add_list_entry(slm, a);
    sprintf(a, " cal_attn_step     %6d", hsk->cal_attn_step     );
    solo_add_list_entry(slm, a);
    sprintf(a, " cal_freq_step     %6d", hsk->cal_freq_step     );
    solo_add_list_entry(slm, a);
    sprintf(a, " r_sq_offset       %6d", hsk->r_sq_offset       );
    solo_add_list_entry(slm, a);
    sprintf(a, " refl_thres        %6d", hsk->refl_thres        );
    solo_add_list_entry(slm, a);
    sprintf(a, " shifter_cnts      %6d", hsk->shifter_cnts      );
    solo_add_list_entry(slm, a);
    sprintf(a, " attn_setting      %6d", hsk->attn_setting      );
    solo_add_list_entry(slm, a);
    sprintf(a, " swp_center        %6d", hsk->swp_center        );
    solo_add_list_entry(slm, a);
    sprintf(a, " cp2_mode          %6d", hsk->cp2_mode          );
    solo_add_list_entry(slm, a);
    sprintf(a, " non_dual_mode     %6d", hsk->non_dual_mode     );
    solo_add_list_entry(slm, a);
    sprintf(a, " word_188          %6d", hsk->word_188          );
    solo_add_list_entry(slm, a);

    solo_add_list_entry(slm, " ");
    /*
    sprintf(a, " unused6[12]       %6d", hsk->unused6[12]       );
    solo_add_list_entry(slm, a);
     */
    sprintf(a, " wavelength2       %6d", hsk->wavelength2       );
    solo_add_list_entry(slm, a);
    sprintf(a, " atp2              %6d", hsk->atp2              );
    solo_add_list_entry(slm, a);
    sprintf(a, " pulse_width2      %6d", hsk->pulse_width2      );
    solo_add_list_entry(slm, a);
    sprintf(a, " prf_secondary     %6d", hsk->prf_secondary     );
    solo_add_list_entry(slm, a);
    sprintf(a, " sys_gain2         %6d", hsk->sys_gain2         );
    solo_add_list_entry(slm, a);
    sprintf(a, " log_bw2           %6d", hsk->log_bw2           );
    solo_add_list_entry(slm, a);
    sprintf(a, " ant_bw2           %6d", hsk->ant_bw2           );
    solo_add_list_entry(slm, a);
    sprintf(a, " polarization2     %6d", hsk->polarization2     );
    solo_add_list_entry(slm, a);
    /*
    sprintf(a, " unused7[3]        %6d", hsk->unused7[3]        );
    solo_add_list_entry(slm, a);
     */
    solo_add_list_entry(slm, " ");

    sprintf(a, " SCALING cal_info2[10]");
    solo_add_list_entry(slm, a);

    cal_info = hsk->cal_info2;

    for(b=a,ii=0; ii < 7; ii++, b += strlen(b), cal_info++) {
	sprintf(b, "  %4x %4x", cal_info->pscale, cal_info->pbias);
    }
    solo_add_list_entry(slm, a);

    solo_add_list_entry(slm, " ");
    /*
    sprintf(a, " unused8[9]        %6d", hsk->unused8[9]        );
    solo_add_list_entry(slm, a);
     */

    sprintf(a, " aircraft_info[6]");
    solo_add_list_entry(slm, a);

    for(b=a,ii=0; ii < 6; ii++, b+=strlen(b)) {
	sprintf(b, " %5d", hsk->aircraft_info[ii]);
    }
    solo_add_list_entry(slm, a);

    sprintf(a, " dual_pol_mode     %4x", hsk->dual_pol_mode     );
    solo_add_list_entry(slm, a);
    sprintf(a, " dual_pol_switch   %4x", hsk->dual_pol_switch   );
    solo_add_list_entry(slm, a);
    /*
    sprintf(a, " unused9[8]        %6d", hsk->unused9[8]        );
    solo_add_list_entry(slm, a);
     */

    slm_print_list(slm);
    return;
}
/* c------------------------------------------------------------------------ */

void fof_print_stat_line (int a)
{
}
/* c------------------------------------------------------------------------ */

int fof_process_cal_info(FILE *fp)
{
    int ii, jj, kk, limit;
    char *a, att[1024], string_space[1024], *str_ptrs[32];
    double d, A, B, noise;
    struct calibration_info *zcal;
    struct calibration_pair *xcp, *zcp;
    struct source_field_info *srsf;
    float *ff, *dbm;


    if(!(zcal = SMALLOC(struct calibration_info))) {
	printf("Unable to malloc struct calibration_info\n");
	exit(1);
    }
    memset(zcal, 0, sizeof(struct calibration_info));

    if(!cals_top) {
	cals_top = zcal;
    }
    else {
	cals_top->last->next = zcal;
	zcal->last = cals_top->last;
    }
    cals_top->last = zcal;

    for(;;) {
	if(!znext_att(fp, att))
	      return(zcal->num_pairs);

	if(strncmp(att, "PAIR", 4) == 0) {
	    if((kk = zatt_arg_ptrs(att, string_space, str_ptrs)) < 2) {
		continue;
	    }
	    if(!(zcp = SMALLOC(struct calibration_pair))) {
		printf("Unable to malloc struct calibration_pair\n");
		exit(1);
	    }
	    memset(zcp, 0, sizeof(struct calibration_pair));
	    zcp->counts = atoi(str_ptrs[0]);
	    zcp->dbm = atof(str_ptrs[1]);
	    zcal->num_pairs++;

	    if(!zcal->top_pair) {
		zcal->top_pair = zcp;
		zcal->top_pair->last = zcp;
		continue;
	    }
	    /* insert in list in ascending order */
	    for(xcp=zcal->top_pair; xcp; xcp = xcp->next) {
		if(zcp->counts < xcp->counts) { /* insert here */
		    if(xcp == zcal->top_pair) {
			zcp->next = zcal->top_pair;
			zcp->last = zcal->top_pair->last;
			zcal->top_pair->last = zcp;
			zcal->top_pair = zcp;
		    }
		    else {
			zcp->next = xcp;
			zcp->last = xcp->last;
			xcp->last = zcp;
		    }
		    break;
		}
	    }
	    if(!xcp) { /* at the end */
		zcal->top_pair->last->next = zcp;
		zcp->last = zcal->top_pair->last;
		zcal->top_pair->last = zcp;
	    }
	}
	else if(strncmp(att, "NOITARBILAC", 8) == 0) {
	    break;
	}
	else if(strncmp(att, "NAME", 4) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    strcpy(zcal->name, string_space);

	    if(strstr(zcal->name, "VERTICAL X-BAND")) {
		zcal->cal_type = XBAND_VERT_CAL;
	    }
	    else if(strstr(zcal->name, "FOR X-BAND")) {
		zcal->cal_type = XBAND_HORZ_CAL;
	    }
	    else if(strstr(zcal->name, "LINEAR S-BAND")
		    || strstr(zcal->name, "LINEAR C-BAND")) {
		zcal->cal_type = LINEAR_CAL;
	    }
	    else if(strstr(zcal->name, "FOR S-BAND")
		    || strstr(zcal->name, "FOR C-BAND")) {
		zcal->cal_type = HORZ_LV_CAL;
	    }
	}
	else if(strncmp(att, "THRESHOLD COUNTS", 4) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    zcal->threshold_counts = atoi(string_space);
	}
	else if(strstr(att, "NOISE POWER")) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    zcal->noise_power = zcal->v_noise_power = atof(string_space);
	}
	else if(strstr(att, "POWER THRESHOLD")) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    zcal->power_threshold = atof(string_space);
	}
	else if(strstr(att, "R HORIZONTAL SAT")) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    zcal->saturation_threshold = atof(string_space);
	}
    }

    if(!zcal->saturation_threshold) {
	zcal->saturation_threshold = -66.;
    }
    /*
     * link up source fields and calibration info
     */
    for(srsf=sources_top; srsf; srsf = srsf->next) {

	if(MATCHN("XM", srsf->field_name, 2)) {
	    if(zcal->cal_type == XBAND_HORZ_CAL) {
		srsf->cal_info = zcal;
		if(a=get_tagged_string("LDR_MDS")) {
		    zcal->adjusted_mds = atof(a);
		}
		else {
		    zcal->adjusted_mds = zcal->power_threshold +30.;
		}
		break;
	    }
	}
	else if(MATCHN("XL", srsf->field_name, 2)) {
	    if(zcal->cal_type == XBAND_VERT_CAL) {
		srsf->cal_info = zcal;
		break;
	    }
	}
	else if(MATCHN("DM", srsf->field_name, 2)) {
	    if(zcal->cal_type == HORZ_LV_CAL) {
		srsf->cal_info = zcal;
		break;
	    }
	}
	else if(MATCHN("LM", srsf->field_name, 2) ||
	   MATCHN("LX", srsf->field_name, 2)) {
	    if(zcal->cal_type == LINEAR_CAL) {
		srsf->cal_info = zcal;
		break;
	    }
	}
    }
    if(zcal->num_pairs < 2)
	  return(zcal->num_pairs);

    limit = 256;
    if(zcal->cal_type == XBAND_VERT_CAL) limit = 512;
    if(zcal->cal_type == LINEAR_CAL) limit = 1024;

    if(!zcal->dbm_vals) {
	if(!(zcal->dbm_vals = (float *)malloc(limit*sizeof(float)))) {
	    printf("Unalbe to allocate dbm_vals\n");
	    exit(1);
	}
	memset(zcal->dbm_vals, 0, limit*sizeof(float));
    }
    ff = zcal->dbm_vals;
    memset(ff, 0, limit*sizeof(float));

    zcp = zcal->top_pair;

    if(zcp->counts > 0) {	/* need to extrapolate down */
	A = (zcp->next->dbm - zcp->dbm)
	      /(double)(zcp->next->counts - zcp->counts);
	B = zcp->dbm - A*zcp->counts;
	for(ii=0; ii < zcp->counts; ii++) {
	    *(ff+ii) = A*ii +B;
	}
    }

    for(; zcp->next; zcp = zcp->next) { /* fill in the segments */
	A = (zcp->next->dbm - zcp->dbm)
	      /(double)(zcp->next->counts - zcp->counts);
	B = zcp->dbm - A*zcp->counts;
	for(ii=zcp->counts; ii < zcp->next->counts; ii++) {
	    if(ii < 0 || ii > limit) {
		printf("limit %d ii %d\n", limit, ii);
	    }
	    *(ff+ii) = A*ii +B;
	}
	if(!zcp->next->next)
	      break;
    }

    if(zcp->next->counts+1 < limit) {	/* extrapolate up */
	A = (zcp->next->dbm - zcp->dbm)/
	      (double)(zcp->next->counts - zcp->counts);
	B = zcp->dbm - A*zcp->counts;
	for(ii=zcp->next->counts; ii < limit; ii++) {
	    *(ff+ii) = A*ii +B;
	}
    }
    /* create a table of watts
     */
    if(!srsf->lut3) {
	if(!(srsf->lut3 = (float *)malloc(limit*sizeof(float)))) {
	    printf("Unable to allocate srsf->lut3 for watts\n");
	    exit(1);
	}
	memset(srsf->lut3, 0, limit*sizeof(float));
    }
    ff = srsf->lut3;
    dbm = zcal->dbm_vals;

    for(ii=0; ii < limit; ii++, ff++) {
	*ff = WATTZ(*(dbm + ii)); /* watts */
    }
    return(zcal->num_pairs);
}
/* c------------------------------------------------------------------------ */

void fof_process_field_info(FILE *fp)
{
    int ii, jj, kk;
    char *a, att[1024], string_space[1024], *str_ptrs[32];
    double d;
    struct source_field_info *srsf=NULL;
    float *ff;

    for(;;) {
	if(!znext_att(fp, att))
	      return;

	if(strncmp(att, "DLEIF", 5) == 0) {
	    return;
	}
	else if(strncmp(att, "NAME", 4) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);

	    for(srsf=sources_top; srsf; srsf = srsf->next) {
		/* find this field in the source field list
		 */
		if(strncmp(srsf->source_name, string_space, 2) == 0)
		      break;
	    }
	    if(!srsf) {
		printf("Field %s not in field list\n", string_space);
		exit(1);
	    }
	}
	else if(strncmp(att, "MTERM", 4) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    srsf->mterm = atoi(string_space);
	}
	else if(strncmp(att, "NUMBER OF BITS", 4) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    srsf->num_bits = atoi(string_space);
	}
	else if(strncmp(att, "E0", 2) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    srsf->e0 = atof(string_space);
	}
	else if(strncmp(att, "GS0", 3) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    fui->gs0 = srsf->gs0 = atof(string_space);
	}
    }
}
/* c------------------------------------------------------------------------ */

void fof_process_system_info(FILE *fp)
{
    int ii, jj, kk, x_band, mark;
    char *a, att[1024], string_space[1024], *str_ptrs[32];
    double d;
    struct fof_system_attributes *sysatts=systems_top;
    struct source_field_info *srsf;
    float *ff;

    sysatts = SMALLOC(struct fof_system_attributes);
    memset(sysatts, 0, sizeof(struct fof_system_attributes));
    if(!systems_top) {
	systems_top = sysatts;
    }
    else {
	systems_top->last->next = sysatts;
	sysatts->last = systems_top->last;
    }
    systems_top->last = sysatts;
    sysatts->h_ncp_threshold = DEFAULT_NCP_THR;
    sysatts->h_snr_threshold = DEFAULT_SNR_THR;

    for(;;) {
	if(!znext_att(fp, att))
	      return;

	if(strncmp(att, "METSYS", 5) == 0) {
	    break;
	}
	else if(strncmp(att, "AVERAGE TRANSMITTED POWER", 11) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->avg_trans_pwr = atof(string_space);
	}
	else if(strncmp(att, "BEAM WIDTH", 7) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->v_beam_width = sysatts->h_beam_width = atof(string_space);
	}
	else if(strncmp(att, "HORIZONTAL NCP THRESHOLD", 15) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->h_ncp_threshold = atof(string_space);
	}
	else if(strncmp(att, "HORIZONTAL SNR THRESHOLD", 15) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->h_snr_threshold = atof(string_space);
	}
	else if(strncmp(att, "IF FILTER LOSS", 11) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->if_filter_loss = atof(string_space);
	}
	else if(strncmp(att, "INTEGRATION BIAS", 11) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->integration_bias = atof(string_space);
	}
	else if(strncmp(att, "LDR BIAS", 7) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->ldr_bias = atof(string_space);
	}
	else if(strncmp(att, "NAME", 4) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    strcpy(sysatts->name, string_space);
	    x_band = strstr(sysatts->name, "X-BAND") ? YES : NO;
	}
	else if(strncmp(att, "PULSE WIDTH", 7) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->pulse_width = atof(string_space)*1.e-9;
	}
	else if(strncmp(att, "RECEIVED POWER CORRECTION", 17) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->rec_power_corr = atof(string_space);
	}
	else if(strncmp(att, "SYSTEM GAIN", 11) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->system_gain = atof(string_space);
	}
	else if(strncmp(att, "WAVE LENGTH", 7) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->wave_length = atof(string_space) * .01;
	}
	else if(strncmp(att, "XM CUTOFF", 7) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->h_cutoff = atof(string_space);
	}
	else if(strncmp(att, "XL CUTOFF", 7) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->v_cutoff = atof(string_space);
	}
	else if(strncmp(att, "ZDR BIAS", 7) == 0) {
	    kk = zatt_arg_ptrs(att, string_space, str_ptrs);
	    sysatts->zdr_bias = atof(string_space);
	}
    }
    /* link up source fields with the appropriate systems attributes
     */
    if(x_band) {
	for(srsf=sources_top; srsf; srsf = srsf->next) 
	      if(srsf->x_band) {
		  srsf->sysatts = sysatts;
	      }
	/* c...mark
	 * establish a second radar descriptor for x-band
	 * assume that the old code will properly filling the
	 * first (primary) radar descriptor
	 */
	strncpy(dgi->dds->radd->config_name, "SBAND   ", 8);
	xradd = dd_malloc_radd(dgi, (char *)"XBAND   ");
	xradd->freq1 = SPEED_OF_LIGHT/sysatts->wave_length;
	xradd->pulsewidth = sysatts->pulse_width;
	xradd->system_gain = sysatts->system_gain;
	xradd->horz_beam_width = sysatts->h_beam_width;
	xradd->vert_beam_width = sysatts->v_beam_width;
    }
    else {
	for(srsf=sources_top; srsf; srsf = srsf->next) 
	      if(!srsf->x_band) {
		  srsf->sysatts = sysatts;
	      }
    }
}
/* c------------------------------------------------------------------------ */

struct data_field_info *
fof_push_data_field (struct data_field_info *dataf)
{
    int before=YES;
    struct data_field_info *this_info;  //Jul 26, 2011 *this

    return(this_info = fof_insert_data_field(dataf, fui->dataf, before));
}
/* c------------------------------------------------------------------------ */

struct data_field_info *
fof_que_data_field (struct data_field_info *dataf)
{
    int after=NO;
    struct data_field_info *this_info;  //Jul 26, 2011 *this

    return(this_info = fof_insert_data_field
	   (dataf, (fui->dataf ? fui->dataf->last : fui->dataf), after));
}
/* c------------------------------------------------------------------------ */

struct fof_ray_que *
fof_que_ray_info (struct fof_ray_que **top, struct fof_ray_que *this_ray)  //Jul 26, 2011 *this
{
    /* "(*top)->last" should always point to the last item in the queue
     */
    if(*top) {
	(*top)->last->next = this_ray;
	this_ray->last = (*top)->last;
	(*top)->last = this_ray;
    }
    else {
	*top = this_ray;
	this_ray->last = this_ray;
    }
    this_ray->next = NULL;
    return(this_ray);
}  
/* c------------------------------------------------------------------------ */

struct source_field_info *
fof_raw_counts_field (char *field_name)
{
    /* check for fields like "CP.CD.CN.CO.CX.CY.CV.CU.CR.CQ.CW.CM.CL.CS"
     */
    int mark;
    struct source_field_info *srsf=sources_top;

    for(; srsf; srsf = srsf->next) {
	if(*field_name == 'C' && MATCH2(srsf->source_name, field_name)) {
	    break;
	}
    }
    return(srsf);
}
/* c------------------------------------------------------------------------ */

void fof_rayq_info (char *at, struct fof_ray_que *rq)
{
    int ii, jj, kk, mark, *deep6=0;
    unsigned short *us, *ush;
    u_int32_t usl;
    RSF_HOUSE *hsk;
    struct param_desc pd;
    double d;
    

    hsk = (RSF_HOUSE *)at;

    if(hostIsLittleEndian()) {
//       fof_crack_hsk(hsk, xhsk, (int)0);  //Jul 26, 2011
       fof_crack_hsk((char *)hsk, (char *)xhsk, (int)0);  //Jul 26, 2011
       hsk = xhsk;
    }

    if(fui->hsk64_flag) {	/* fake it for the old 64 word housekeeping */
       memcpy(&phsk, hsk,  64 * sizeof(short));
       hsk = &phsk;
       rq->data = at + 64 * sizeof(short);
    }
    else {
	rq->data = at + hsk->hsk_len * sizeof(short);
    }
    rq->at = (char *)hsk;
    ush = (unsigned short *)hsk;
    rq->flags = 0;

    /*
     *
C   CALCULATE PEAK POWER IN DBM
      AVPWR = IHSK( 18 )*.1
      TAU = IHSK( 19 )*1.E-9		
      PRF = IHSK( 20 )*.1		
      PKPWR = -512.
      IF( TAU*PRF .GT. 0 )
     A   PKPWR = AVPWR -10.*ALOG10( TAU*PRF )
     *
     */
    gri->PRF = rq->prf = (float)hsk->prfx10 * .1;
    rq->dprf = rq->prf - rq->prev->prf;
    if(FABS(rq->prf) < 1.) {
	mark = *deep6;
    }
    rq->atp = (float)hsk->atp * .1;
    rq->datp = rq->atp - rq->prev->atp;
    gri->peak_power = rq->atp
	  - 10. * log10((double)(gri->pulse_width * gri->PRF));
    gri->peak_power = WATTZ(gri->peak_power) * .001; /* kw. */
    if(xradd) {
	d = (double)hsk->atp2 * .1
	      -10. * log10((double)(gri->pulse_width * gri->PRF));
	xradd->peak_power = WATTZ(d) * .001;
    }
    gri->num_samples = rq->num_samples = hsk->samples_per_beam;
    gri->bin_spacing = rq->gs = hsk->gs;

    rq->time = fof_time(hsk);
    if(rq->time != fui->trip_time) {
	fui->trip_time = rq->time;
	fui->count_since_trip = 0;
    }
    if(rq->prf > 0) {
	rq->time = fui->trip_time + (fui->count_since_trip++) *
	      (float)rq->num_samples/rq->prf;
    }
    rq->time += fui->time_correction;
    gri->dts->time_stamp = gri->time = rq->time;
    d_unstamp_time(gri->dts);

    rq->delta_time = rq->time - rq->prev->time;

    gri->azimuth = rq->az = BA2F(hsk->az_xCF);
    rq->daz = angdiff(rq->prev->az, rq->az);

    gri->elevation = rq->el = BA2F(hsk->el_xCF);
    rq->del = angdiff(rq->prev->el, rq->el);

    gri->fixed = rq->fx = BA2F(hsk->fixed_ang);
    rq->dfx = angdiff(rq->prev->fx, rq->fx);

    gri->scan_mode = rq->scan_mode = hsk->scan_mode;
    rq->rotation_angle = gri->rotation_angle = gri->corrected_rotation_angle =
	  rq->scan_mode == DD_SCAN_MODE_RHI ? rq->el : rq->az;
    rq->drot = angdiff(rq->prev->rotation_angle, rq->rotation_angle);

    rq->sweep_num = hsk->swp_num;
    
    rq->vol_num = hsk->vol_num;
    gri->num_bins = rq->num_gates = hsk->num_gates;
    rq->num_fields = hsk->num_fields;

    rq->sizeof_ray = hsk->log_rec_len * sizeof(short);

    for(ii=0; ii < rq->num_fields; ii++) {
	if(hostIsLittleEndian()) {
	   usl = *(ush + desc_offset[ii]);
	   usl >>= 8;
	   rq->fields_present[ii] = 16 * (usl & 0xf) + ((usl >> 4) & 0xf);
	}
	else {
	   us = ush + desc_offset[ii];
	   memcpy(&pd, us, sizeof(short)); /* eliminates alignment problems */
	   rq->fields_present[ii] = pd.id0 + 16 * pd.id1;
	}
	if(rq->fields_present[ii] == BROKEN_JAWS_VELOCITY_ID_NUM)
	      rq->fields_present[ii] = VELOCITY_ID_NUM;
    }
    rq->seq_num = seq_num;

    if(hsk->transit) {
	rq->flags |= TFLAG_ON;
    }

    rq->dual_pol_mode = hsk->dual_pol_mode;
    rq->dual_pol_switch = hsk->dual_pol_switch;

    return;
}
/* c------------------------------------------------------------------------ */

struct fof_ray_que *
fof_ray_que_spair (void)
{
    struct fof_ray_que *this_ray=ray_que_spairs;  //Jul 26, 2011 *this

    if(!this_ray) {
	this_ray = ray_que_spairs = SMALLOC(struct fof_ray_que);
	if(!this_ray) {
	    printf("Could not allocate another ray_que_spair: %d %d\n"
		   , ray_que_count, sweep_que_count);
	    exit(1);
	}
	ray_que_count++;
	memset(this_ray, 0, sizeof(struct fof_ray_que));
    }
    ray_que_spairs = ray_que_spairs->next;
    this_ray->next = this_ray->last = this_ray->prev = NULL;

    return(this_ray);
}
/* c------------------------------------------------------------------------ */

void 
fof_redo_srsfs (struct fof_ray_que *rq)
{
    int ii, jj, kk, nn, mark, ok, before=YES, after=NO;
    struct final_field_info *finalf;
    struct data_field_info *dataf;
    struct source_field_info *srsf;
    struct field_derivation_info *derf;
    struct fof_system_attributes *fsi;
    RSF_HOUSE *hsk;
    unsigned short *us;
    char *a, *b, *c;
    float r, gs;
    double d;

    
    /*
     * unmark present the previous mix of fields
     */
    for(ii=0; ii < rq->prev->num_fields; ii++) {
	jj = rq->prev->fields_present[ii];
	if(source_fields[jj])
	      source_fields[jj]->present = NO;
    }
    us = (unsigned short *)rq->at;
    *gri->source_field_mnemonics = '\0';
    fui->x_band = NO;
    /*
     * mark present the current mix of fields
     */
    for(ii=kk=0; ii < rq->num_fields; ii++) {
	if(!(srsf = source_fields[rq->fields_present[ii]])) {
	    printf("Source field: %d is not in the fieldlist %d\n"
		   , rq->fields_present[ii], ii);
	    exit(1);
	}
	strcat(gri->source_field_mnemonics, srsf->source_name);
	strcat(gri->source_field_mnemonics, " ");
	if(srsf->x_band) fui->x_band = YES;
	srsf->scale = *((short *)us + scale_offset[ii]) * .01;
	srsf->bias = *((short *)us + scale_offset[ii] +1) * .01;
	srsf->sysatts->PRF = rq->prf;
	kk += srsf->num_bits;
	srsf->base_offset = srsf->final_offset = (kk-1)/8;
	fof_rng_corr(srsf);
	fof_luts(srsf);
	srsf->present = YES;
    }
    fui->sizeof_gate_info = kk/8;
    fof_release_data_fields(fui->dataf);
    fui->dataf = NULL;

    /* see if the final fields can be generated
     * 
     */
    gri->num_fields_present = 0;
    fui->cannot_generate_fields = NO;

    for(nn=0,ii=0,finalf=fui->finalf; finalf; finalf = finalf->next, ii++) {
	/*
	 * first see if the data field already exists
	 */
	if(dataf=fof_duplicate_field(finalf->field_name)) {
	    finalf->dataf = dataf;
	    finalf->long_field_name = dataf->long_field_name;
	}
	else if(srsf=fof_raw_counts_field(finalf->field_name)){
	    /* this might be a counts field!
	     */
	    finalf->dataf = dataf = fof_data_field_spair();
	    dataf->raw_counts = YES;
	    dataf->srsf = srsf;
	    dataf->field_name = srsf->source_name;
	    dataf->long_field_name = srsf->long_field_name;
	    finalf->long_field_name = dataf->long_field_name;
	    fof_setup_field(dataf);
	}
	else if(srsf=fof_possible_field(finalf->field_name, &derf)) {
	    finalf->dataf = fof_establish_data_field(finalf->field_name);
	    finalf->long_field_name = finalf->dataf->long_field_name;
	}
	else {
        printf("\nCannot produce requested field %s!\n", finalf->field_name);
        /* 
         * Just bail out now, because it isn't clear what all needs to be
         * cleaned up at this point. (8/2010 cb)
         */
        exit(1);
#if 0
	    fui->cannot_generate_fields = YES;
	    mark = 0;
	    return;
#endif
	}
	gri->num_fields_present++;
	gri->actual_num_bins[ii] = gri->num_bins;
	strcpy(gri->field_name[ii], finalf->field_name);
	str_terminate(gri->field_long_name[ii], finalf->long_field_name, 44);
	gri->dd_scale[ii] = finalf->dataf->dd_scale;
	gri->dd_offset[ii] = finalf->dataf->dd_bias;
    }
    hsk = (RSF_HOUSE *)rq->at;
    fui->ref_srsf = srsf = fof_check_srs_fld((char *)"DZ", &derf);
    gri->h_beamwidth = srsf->sysatts->h_beam_width;
    gri->v_beamwidth = srsf->sysatts->v_beam_width;
    gri->polarization = 0;	/* horizontal polarization */
    gri->wavelength = srsf->sysatts->wave_length;
    gri->sweep_speed = BA2F(hsk->ant_scan_rate) * 60./64.;
    gri->radar_constant = srsf->rad_con;
    gri->noise_power = srsf->sysatts->h_noise_power;
    gri->rcvr_gain = srsf->sysatts->system_gain;
    gri->ant_gain = EMPTY_FLAG;
    gri->pulse_width = srsf->sysatts->pulse_width;
    gri->rcv_bandwidth = (float)hsk->log_bw * 1.e-3; /* Hz */
    gri->nyquist_vel = gri->PRF * gri->wavelength * .25;
    gri->unamb_range = .001 * .5 * SPEED_OF_LIGHT/gri->PRF; /* km. */

    for(fsi=systems_top,ii=0; fsi; fsi = fsi->next) {
	gri->num_freq = gri->num_ipps = ++ii;
	gri->freq[ii-1] = (SPEED_OF_LIGHT/fsi->wave_length) * 1.e-9; /* GHz */
	gri->ipps[ii-1] = rq->prf > 0 ? 1./rq->prf : -999;
    }
    /* calculate the final offsets
     */
    for(dataf=fui->dataf; dataf; dataf = dataf->next) {
	kk = fui->ref_srsf->mterm - dataf->srsf->mterm;
	dataf->srsf->final_offset = dataf->srsf->base_offset
	      + kk * fui->sizeof_gate_info;
    }
    r = fui->ref_srsf->r0 * 1000.;
    gs = fui->ref_srsf->gs * 1000.;

    for(ii=0; ii < rq->num_gates; ii++, r+=gs) {
	gri->range_value[ii] = r;
    }
    return;
}
/* c------------------------------------------------------------------------ */

struct data_field_info *
fof_remove_data_field (struct data_field_info *this_info)  //Jul 26, 2011 *this
{
    struct data_field_info *top=fui->dataf;

    if(!this_info)
	  return(NULL);

    if(this_info == top) {		/* on top */
	if(top->next) {
	    top->next->last = top->last;
	}
	fui->dataf = top->next;
    }
    else {
	this_info->last->next = this_info->next;
	if(this_info->next) {
	    this_info->next->last = this_info->last;
	}
	else { /* at the end */
	    top->last = this_info->last;
	}
    }
    return(this_info);
}
/* c------------------------------------------------------------------------ */

void 
fof_reset (void)
{
    fui->vol_count_flag = fui->sweep_count_flag =
	  fui->new_sweep = fui->new_vol = YES;
    fui->ray_count = fui->sweep_count = fui->vol_count = 0;
    fui->trip_time = 0;
    /* make sure to start a new sweep and vol
     */
    gri->vol_num++;
    gri->sweep_num++;
    difs->stop_flag = NO;
}
/* c------------------------------------------------------------------------ */

void 
fof_rng_corr (struct source_field_info *srsf)
{
    /* calculate the range correction array for this field
     */
    int gg, ii, ng;
    double d, r, gs, rad_con, atp;
    struct fof_ray_que *rq=fui->ray_que;
    float *rngcvals, *rmark=NULL;
    struct fof_system_attributes *sysatts=srsf->sysatts;

    
    srsf->gs = gs = (rq->gs + fui->gs0) * .001;
    srsf->r0 = r = gs * (1 + srsf->mterm) + srsf->e0
	  - (.25 * 3.e5 * sysatts->pulse_width);
    srsf->prf = rq->prf;

    if(!strstr("DM.LM.XM.XL.LX", srsf->field_name)) {
	srsf->num_gates = rq->num_gates;
	return;
    }
    srsf->lut = srsf->cal_info->dbm_vals;
    atp = srsf->x_band ? sysatts->avg_trans_pwr : rq->atp;

    srsf->rad_con = rad_con = 
	168.8
        + (10. * LOG10(srsf->prf))
	- (2. * sysatts->system_gain)
	- atp
	+ (10. * LOG10(SQ(sysatts->wave_length)))
	- (10. * LOG10(RADIANS(sysatts->h_beam_width) *
				RADIANS(sysatts->v_beam_width)))
        - (10. * LOG10(.93))
	+ sysatts->if_filter_loss
	+ sysatts->integration_bias
	+ sysatts->rec_power_corr
        ;
    printf("Radar Constant for %s: %.2f  atp: %.2f  prf: %.2f\n"
	   , srsf->sysatts->name, rad_con, atp, srsf->prf);


    if((ng = rq->num_gates) != srsf->num_gates) {
	if(srsf->rngcvals) free(srsf->rngcvals);
	
	if(!(srsf->rngcvals = FMALLOC(rq->num_gates))) {
	    printf("Unable to do srsf->rngcvals = FMALLOC(rq->num_gates)\n");
	    exit(1);
	}
	memset(srsf->rngcvals, 0, sizeof(float)*rq->num_gates);
    }
    srsf->num_gates = ng;
    rngcvals = srsf->rngcvals;

    for(gg=0; gg < ng; gg++, rngcvals++, r+=gs) {
	if(r <= 0) {
	    rmark = rngcvals;
	}
	else
	      *rngcvals = 20. * LOG10(r) + rad_con;
    }
    for(; rmark && rmark >= srsf->rngcvals; rmark--) {
	/* give the predata gates a value */
	*rmark = *(rmark+1);
    }
}
/* c------------------------------------------------------------------------ */

int 
fof_select_ray (void)
{
    int ii, jj, ok, mark;


    ok = !fui->ignore_this_ray;

    if(fui->sweep_que->something_changed_at) /* in transition */
	  ok = NO;

    if(fui->cannot_generate_fields)
	  ok = NO;

    if(fui->ascending_fxd_only && gri->fixed < fui->ref_fixed_angle)
	  ok = NO;

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

    return(ok);
}
/* c------------------------------------------------------------------------ */

void 
fof_release_data_fields (struct data_field_info *this_info)  //Jul 26, 2011 *this
{
    if(!this_info)
	  return;
    this_info->last->next = data_field_spairs;
    data_field_spairs = this_info;
}
/* c------------------------------------------------------------------------ */

void 
fof_setup_field (struct data_field_info *dataf)
{
    /* the purpose here is to setup this field and establish other
     * data fields so this field can be produced
     */
    int ii, mark, before=YES, after=NO;
    struct source_field_info *srsf=dataf->srsf;
    char *field_name=dataf->field_name;
    struct field_derivation_info *derf;
    struct data_field_info *datafx, *datafy;

    fof_malloc_vals(dataf);

    if(strstr("DM.DL.LM.XM.XL", field_name)) {
	/* Calibrated power in dbm fields
	 */
	dataf->lut = srsf->lut;
	dataf->proc_ptr = fof_gen_field;
	fof_push_data_field(dataf); /* "push" puts this field at the
				     * start of the list so it will be
				     * done before you need it for
				     * another field */
    }
    else if(strstr("VR.VQ.VJ.RT.ZR.SR.RS.FL.NR", field_name)) {
	/* Raw velocities and other direct lookup fields
	 */
	dataf->lut = srsf->lut2;
	dataf->proc_ptr = fof_gen_field;
	fof_push_data_field(dataf);
    }
    else if(strstr("WP.ZW", field_name)) {
	/* funkier fields...DM and ZR in watts */
	dataf->lut = srsf->lut3;
	dataf->proc_ptr = fof_gen_field;
	fof_push_data_field(dataf);
    }
    else if(strstr("XZ.XY", field_name)) {
	/* a reflectivity field for x-band */
	if(!(datafx = fof_duplicate_field(dataf->srsf->field_name))) {
	    datafx = fof_establish_data_field(dataf->srsf->field_name);
	}
	dataf->src_fld = dataf->thr_fld = datafx;
	dataf->proc_ptr = fof_gen_dbz_field;
	dataf->threshold = dataf->srsf->cal_info->power_threshold;
	fof_que_data_field(dataf); /* put it at the end of the list
				    * of fields to be processed */
    }
    else if(strstr("DZ.FZ.EZ.GZ", field_name)) {
	/* a reflectivity field...could be MHR, rp7 or the old processors
	 */
	if(MATCH2("DM", dataf->srsf->field_name)) { /* old way */
	    if(!(datafx = fof_duplicate_field(dataf->srsf->field_name))) {
		datafx = fof_establish_data_field(dataf->srsf->field_name);
	    }
	    dataf->src_fld = dataf->thr_fld = datafx;
	    dataf->proc_ptr = fof_gen_dbz_field;
	    dataf->threshold = dataf->srsf->cal_info->power_threshold;
	    if(MATCH2("EZ", field_name))
		  dataf->hi_threshold = YES; /* i.e. don't threshold dBz */
	}
	/* assume source field is "BZ" or "BX" and do it the new
	 * (rp7 or MHR) way...by thresholding on SNR or...
	 */
	else if(strstr("EZ.GZ", field_name)) { /* no thresholding just a lut */
	    dataf->lut = srsf->lut2;
	    dataf->proc_ptr = fof_gen_field;
	}
	else {
	    if(!(datafx = fof_duplicate_field((char *)"BZ"))) {
		datafx = fof_establish_data_field((char *)"BZ");
	    }
	    dataf->src_fld = datafx;

	    if(!(datafx = fof_duplicate_field((char *)"SN"))) {
		datafx = fof_establish_data_field((char *)"SN");
	    }
	    dataf->thr_fld = datafx;
	    dataf->threshold = dataf->srsf->sysatts->h_snr_threshold;
	    dataf->proc_ptr = fof_gen_thrd_field;
	}
	fof_que_data_field(dataf);
    } 
    else if(strstr("LZ", field_name)) {
	/* the linear channel reflectivity field */
	if(!(datafx = fof_duplicate_field((char *)"LU"))) {
	    datafx = fof_establish_data_field((char *)"LU"); /* unfolded linear power */
	}
	dataf->src_fld = dataf->thr_fld = datafx;
	dataf->proc_ptr = fof_gen_dbz_field;
	dataf->threshold = dataf->srsf->cal_info->power_threshold;
	fof_que_data_field(dataf);
    }
    else if(strstr("SN", field_name)) {
	/* the linear channel SNR */
	if(strstr("BS.BF", dataf->srsf->field_name)) {
	    /* newer MHR or rp7 stuff */
	    dataf->lut = srsf->lut2;
	    dataf->proc_ptr = fof_gen_field;
	    fof_push_data_field(dataf);
	}
	else {
	    if(!(datafx = fof_duplicate_field((char *)"LU"))) {
		/* unfolded linear power */
		datafx = fof_establish_data_field((char *)"LU");
	    }
	    dataf->src_fld = datafx;
	    dataf->proc_ptr = fof_gen_snr;
	    fof_que_data_field(dataf);
	}
    }
    else if(strstr("SM", field_name)) {
	/* the log channel SNR */
	if(!(datafx = fof_duplicate_field((char *)"DM"))) {
	    datafx = fof_establish_data_field((char *)"DM");
	}
	dataf->src_fld = datafx;
	dataf->proc_ptr = fof_gen_snr;
	fof_que_data_field(dataf);
    }
    else if(strstr("SW", field_name)) {
	if(MATCH2("BW", dataf->srsf->field_name)) {
	    /* newer MHR or rp7 stuff */
	    dataf->lut = srsf->lut2;
	    dataf->proc_ptr = fof_gen_field;
	    fof_push_data_field(dataf);
	}
	else {
	    if(!(datafx = fof_duplicate_field((char *)"SN"))) {
		/* unfolded linear power */
		datafx = fof_establish_data_field((char *)"SN");
	    }
	    dataf->src_flda = datafx;
	    
	    if(!(datafx = fof_duplicate_field((char *)"NC"))) {
		datafx = fof_establish_data_field((char *)"NC");
	    }
	    dataf->src_fldb = datafx;
	    dataf->proc_ptr = fof_gen_sw;
	    fof_que_data_field(dataf);
	}
    }
    else if(strstr("VE.VT.VF", field_name)) {
	/* velocity thresholded on NCP or spectral width
	 * first generate the velocity field
	 */
	if(!(datafx = fof_duplicate_field(dataf->srsf->field_name))) {
	    datafx = fof_establish_data_field(dataf->srsf->field_name);
	}
	dataf->src_fld = datafx;
	dataf->proc_ptr = fof_gen_thrd_field;
	fof_que_data_field(dataf);
	
	if(srsf = fof_check_srs_fld((char *)"NC", &derf)) { /* NCP! */
	    if(!(datafx = fof_duplicate_field((char *)"NC"))) {
		datafx = fof_establish_data_field((char *)"NC");
	    }
	    dataf->threshold = datafx->srsf->sysatts->h_ncp_threshold;
	    dataf->thr_fld = datafx;
	}
	else if((srsf = fof_find_srs_fld((char *)"SR")) && srsf->present) {
	    /* raw spectral width! */
	    if(!(datafx = fof_duplicate_field((char *)"SR"))) {
		datafx = fof_establish_data_field((char *)"SR");
	    }
	    dataf->threshold = datafx->srsf->sysatts->sw_threshold;
	    dataf->thr_fld = datafx;
	    dataf->hi_threshold = YES;
	}
	else if(srsf = fof_check_srs_fld((char *)"SN", &derf)) { /* SNR! */
	    if(!(datafx = fof_duplicate_field((char *)"SN"))) {
		datafx = fof_establish_data_field((char *)"SN");
	    }
	    dataf->threshold = datafx->srsf->sysatts->h_snr_threshold;
	    dataf->thr_fld = datafx;
	}
    }
    else if(MATCH2("NC", field_name)) {
	/* see how we can generate NCP
	 * put this field at the front since it may be used
	 * to threshold other fields
	 */
	if(MATCH2("BN", dataf->srsf->field_name)) {
	    /* newer MHR or rp7 stuff */
	    dataf->lut = srsf->lut2;
	    dataf->proc_ptr = fof_gen_field;
	    fof_push_data_field(dataf);
	}
	else if(MATCH2("RT", dataf->srsf->field_name)) {
	    /* generate the numerater and denomenater of NCP from
	     * R_SUB_TAU and R_SUB_ZERO
	     */
	    dataf->proc_ptr = fof_gen_quotient_field;
	    if(!(datafx = fof_duplicate_field((char *)"RT"))) {
		datafx = fof_establish_data_field((char *)"RT");
	    }
	    else {		/* move it to the front */
		datafx = fof_remove_data_field(datafx);
		fof_push_data_field(datafx);
	    }
	    fof_insert_data_field(dataf, datafx, after);
	    dataf->src_flda = datafx;
	    /* the denomenater
	     */
	    if(!(datafx = fof_duplicate_field((char *)"NR"))) {
		datafx = fof_establish_data_field((char *)"NR");
	    }
	    dataf->src_fldb = datafx;
	}
	else {			/* simple lookup table */
	    dataf->lut = dataf->srsf->lut2;
	    dataf->proc_ptr = fof_gen_field;
	    fof_push_data_field(dataf);
	}
    }
    else if(MATCH2("LU", field_name)) {	/* unfolded linear power */
	if(!(datafx = fof_duplicate_field((char *)"DM"))) {
	    datafx = fof_establish_data_field((char *)"DM");
	}
	dataf->src_fld = datafx;
	dataf->lut = dataf->srsf->lut;
	dataf->proc_ptr = fof_gen_unfolded_linear;
	fof_que_data_field(dataf);
    }
    else if(MATCH2("LD", field_name)) {
	if(!(datafx = fof_duplicate_field((char *)"XM"))) {
	    datafx = fof_establish_data_field((char *)"XM");
	}
	dataf->proc_ptr = fof_gen_ldr_field;
	dataf->src_flda = datafx;

	if(!(datafx = fof_duplicate_field((char *)"XU"))) {
	    datafx = fof_establish_data_field((char *)"XU");
	}
	dataf->src_fldb = datafx;
	fof_que_data_field(dataf);
    }
    else if(MATCH2("XU", field_name)) {	/* unfolded vertical power */
	if(!(datafx = fof_duplicate_field((char *)"XM"))) {
	    datafx = fof_establish_data_field((char *)"XM");
	}
	dataf->src_fld = datafx;
	dataf->lut = dataf->srsf->lut;
	dataf->proc_ptr = fof_gen_unfolded_xvert;
	fof_que_data_field(dataf);
    }

    else if(MATCH2("ZD", field_name)) {
	/* ZDR with noise removed
	 * first setup for horz power in watts
	 */
	dataf->proc_ptr = fof_gen_noiseless_zdr;

	if(!(datafx = fof_duplicate_field((char *)"ZR"))) { /* raw zdr */
	    datafx = fof_establish_data_field((char *)"ZR");
	}
	dataf->src_fld = datafx;

	if(!(datafx = fof_duplicate_field((char *)"WP"))) {
	    datafx = fof_establish_data_field((char *)"WP");
	}
	dataf->src_flda = datafx;
	/* then setup the reciprocal of ZR in watts
	 */
	if(!(datafx = fof_duplicate_field((char *)"ZW"))) {
	    datafx = fof_establish_data_field((char *)"ZW");
	}
	dataf->src_fldb = datafx;
	fof_que_data_field(dataf);
    }

    else if(MATCH2("VP", field_name)) {
	if(!(datafx = fof_duplicate_field(dataf->srsf->field_name))) {
	    datafx = fof_establish_data_field(dataf->srsf->field_name);
	}
	dataf->src_fld = datafx;

	if(!(datafx = fof_duplicate_field((char *)"DM"))) {
	    datafx = fof_establish_data_field((char *)"DM");
	}
	dataf->thr_fld = datafx;
	dataf->threshold = datafx->srsf->cal_info->power_threshold;
	dataf->proc_ptr = fof_gen_thrd_field;
	fof_que_data_field(dataf);
    }

    else if(dataf->raw_counts) {
	dataf->proc_ptr = fof_gen_counts_field;
	dataf->dd_scale = 1.;
	fof_push_data_field(dataf);
    }

    else if(*field_name == 'B' || *field_name == 'S' || *field_name == 'P') {
	/* MHR and RP7 scaled and biased fields
	 */
	dataf->lut = srsf->lut2;
	dataf->proc_ptr = fof_gen_field;
	fof_push_data_field(dataf); /* put it at the front of the list
				     * of fields to be processed */
    }

    else if(strstr("DY", field_name)) {
	printf("This field \"%s\" cannot be produced yet!", field_name);
	exit(1);
    }
    mark = 0;
}
/* c------------------------------------------------------------------------ */

void 
fof_stack_ray_que_spairs (struct fof_ray_que *this_ray)  //Jul 26, 2011 *this
{
    /* "this->last" should point to the last item in the
     * linked list of items being returned even
     * if there's only one item
     */
    if(!this_ray)
	  return;
    this_ray->last->next = ray_que_spairs;
    ray_que_spairs = this_ray;
}
/* c------------------------------------------------------------------------ */

void 
fof_stack_sweep_que_spairs (struct fof_sweep_que *this_que) //Jul 26, 2011 *this
{
    if(!this_que)
	  return;
    this_que->last->next = sweep_que_spairs;
    sweep_que_spairs = this_que;
}
/* c------------------------------------------------------------------------ */

struct fof_sweep_que *
fof_sweep_que_spair (struct fof_sweep_que **top)
{
    struct fof_sweep_que *this_que=sweep_que_spairs;  //Jul 26, 2011 *this

    if(!this_que) {
	if(!(this_que = SMALLOC(struct fof_sweep_que))) {
	    printf("Could not allocate another sweep_que_spair\n");
	    exit(1);
	}
	sweep_que_count++;
	memset(this_que, 0, sizeof(struct fof_sweep_que));
	sweep_que_spairs = this_que;
    }
    sweep_que_spairs = sweep_que_spairs->next;

    if(*top) {
	(*top)->last->next = this_que;
	this_que->last = (*top)->last;
	(*top)->last = this_que;
    }
    else {
	*top = this_que;
	this_que->last = this_que;
    }
    this_que->next = NULL;
    this_que->first_ray = NULL;
    this_que->daz_sum = this_que->del_sum = this_que->dfx_sum = this_que->fxd_sum = 0;
    this_que->drot_sum = 0;
    this_que->ray_count = 0;
    return(this_que);
}
/* c------------------------------------------------------------------------ */

double fof_time (RSF_HOUSE *at)
{
    RSF_HOUSE *hsk = (RSF_HOUSE *)at;
    DD_TIME dts;
    double d;
    int ii=0;

    dts.day_seconds = D_SECS((int)hsk->hour, (int)hsk->minute
			     , (int)hsk->second, (int)0);
    dts.year = 1900 + hsk->year;
    dts.month = hsk->month;
    dts.day = hsk->day;

    return(d = d_time_stamp(&dts));
}
/* c----------------------------------------------------------------------- */

int zatt_arg_ptrs (char *att, char *string_space, char **str_ptrs)
{
    /* return the number of arguments in the attribute,
     * stuff the arguemnts into the string space
     * and update a list of pointers to the arguments
     */
    int nargs=0;
    char *a=att, *b, *c=string_space, **sp=str_ptrs;

    *c = '\0';
    if(!a || !strlen(a))
	  return(nargs);

    if(!(a=strchr(a, ':')))
	  return(nargs);

    for(; *a++; c++) {		/* for each argument */
	for(b=c; *a && *a != ','; *c++ = *a++); *c = '\0';
	nargs++;
	for(; b < c && *b == ' '; b++);	/* leading blanks removed */
	*sp++ = b;
	for(; b < c && *(c-1) == ' '; *((c--)-1) = '\0'); /* trailing blanks */

	if(!(*a))
	      break;
    }
    return(nargs);
}
/* c------------------------------------------------------------------------ */

int xnext_att(FILE *fp, char *att)		/* get the next attribute */
{
    int ch, nn=0;
    char *b, *c;
    
    for(b=c=att;;) {		/* keep going till we get an attribute */
	while((ch = getc(fp)) != EOF) {
	    if(ch == ' ' && (nn == 0 || *(c-1) == ' '))
		  continue;	/* ignore leading or consecutive blanks */
	    if(strchr(";\n", ch)) { /* just nab one line or
				     * one attribute at a time */
		break;
	    }
	    nn++;
	    *c++ = ch;
	}
	if(nn || ch == EOF)
	      break;
    }
    for(; b < c && *(c-1) == ' '; c--);	/* ignore trailing blanks */
    *c = '\0';
    return(nn);			/* nn == 0 implies no more attributes */
}
/* c------------------------------------------------------------------------ */

int znext_att(FILE *fp, char *att)			/* get the next attribute */
{
    int ch, nn=0;
    char *b, *c;
    
    for(b=c=att;;) {		/* keep going till we get an attribute */
	while((ch = getc(fp)) != EOF) {
	    if(ch == '\n')	/* ignore line feeds */
		  continue;
	    if(ch == ' ' && (nn == 0 || *(c-1) == ' '))
		  continue;	/* ignore leading or consecutive blanks */
	    if(ch == ';')
		  break;
	    nn++;
	    *c++ = ch;
	}
	if(nn || ch == EOF)
	      break;
    }
    for(; b < c && *(c-1) == ' '; c--);	/* ignore trailing blanks */
    *c = '\0';
    return(nn);			/* nn == 0 implies no more attributes */
}
/* c-------------------------------------------------------------------------*/

void 
ctypef (	/* type as floats */
    float *ff,
    int m,
    int n
)
{
    int ii=m, s=0, nn=n;
    float *f = ff+m;
    
    for(; nn-- > 0; ii++, f++) {
	if( s == 0 ) {
	    printf("%5d)", ii);	/* new line label */
	    s = 6;
	}
	printf( "%7.1f", *f);
	s += 7;
	
	if( s >= 76 ) {
	    printf( "\n" );	/* start a new line */
	    s = 0;
	}
    }
    if( s > 0 ) {
	printf( "\n" );
    }
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

