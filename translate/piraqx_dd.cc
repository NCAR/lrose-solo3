/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

#include <LittleEndian.hh>
#include <stdlib.h>
#include <string.h>
#include <dd_math.h>
#include <piraqx.h>
#include "input_limits.hh"
#include "dd_stats.h"
#include "dd_catalog.hh"
#include "generic_radar_info.h"
#include "sigm_dd.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "dorade_share.hh"
#include "gneric_dd.hh"
#include "dd_io_mgmt.hh"
#include "piraqx_dd.hh"
#include "piraq_dd.hh"
#include "dd_crackers.hh"

#define DATA_SMHVSIM    13 /* 2000 DOW4 copolar matrix for simultaneous H-V (no iq average) */


# ifndef M_PI
#define M_E             2.7182818284590452354
#define M_LOG2E         1.4426950408889634074
#define M_LOG10E        0.43429448190325182765
#define M_LN2           0.69314718055994530942
#define M_LN10          2.30258509299404568402
#define M_PI            3.14159265358979323846
#define M_PI_2          1.57079632679489661923
#define M_PI_4          0.78539816339744830962
#define M_1_PI          0.31830988618379067154
#define M_2_PI          0.63661977236758134308
#define M_2_SQRTPI      1.12837916709551257390
#define M_SQRT2         1.41421356237309504880
#define M_SQRT1_2       0.70710678118654752440
# endif

/* c------------------------------------------------------------------------ */

# define PX2(x) (sig_swap2((&x)))
# define PX4(x) (pc_swap4((&x)))
# define PX4F(x) (pc_swap4f((&x)))

# define         PIRAQ_IO_INDEX 3
# define         SWEEP_QUE_SIZE 64

/* options flags
 */
# define              SCAN_LIST 0x1
# define          SIMPLE_SWEEPS 0x2
# define           FORCE_POLYPP 0x4
# define            FORCE_REV_1 0x8
# define        SPARC_ALIGNMENT 0x10
# define              SPOL_FLAG 0x20
# define             NOSE_RADAR 0x40
# define           MERGE_HRD_AC 0x80
# define       DUMP_TIME_SERIES 0x100
# define         RANGE0_DEFAULT 0x200
# define              BIST_FLAG 0x400
# define        FLIP_VELOCITIES 0x800
# define      	    FULL_MATRIX 0x1000
# define      	    FORCE_POL12 0x2000
# define      	     FORCE_RHIS 0x4000
# define      	     FORCE_PPIS 0x8000
# define      	  HEADER_SUBSEC 0x10000
# define         SPOL_HORZ_FLAG 0x20000
# define               DOW_FLAG 0x40000
# define             GUIFU_FLAG 0x80000
# define     IGNORE_TRANSITIONS 0x100000
# define       HZO_IMPROVE_FLAG 0x200000
# define         DZ_TO_DBZ_FLAG 0x400000
# define              RABID_DOW 0x800000
# define      PIRAQX_TRUST_TIME 0x1000000
# define   PIRAQX_100X_NANOSECS 0x2000000
# define        NEW_SWEEP_AT_AZ 0x4000000

static PIRAQX *dwlx=NULL, *dwlx0 = NULL, *dwlx_swap;
static void *scrs=NULL;

static struct rename_str *top_ren=NULL;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats=NULL;
static struct input_read_info *irq;
static char preamble[24], postamble[64];
static struct d_limits *d_limits=NULL;
static struct generic_radar_info *gri;
static struct piraq_useful_items *pui=NULL;
static struct solo_list_mgmt *slm;
static struct platform_i *asib;
static int view_char=0, view_char_count=200;

static double Default_Range0 = 150;
static double Improve_rconst_corr = 0;
static char *current_file_name;
static double twoPI=2.0*M_PI;
static u_int32_t P960501=0;
static u_int32_t P970101=0;
static u_int32_t P970127=0;
static u_int32_t P970129=0;
static u_int32_t P980201=0;
static float h_rconst=0, v_rconst=0;

static double min_az_diff = .3;
static double min_el_diff = .2;
static double rhi_az_tol = 1.5;
static double ppi_el_tol = .77;

static int ve1_ndx = 0;
static int ve2_ndx = 0;
static int sw1_ndx = 0;
static int sw2_ndx = 0;
static int ve_ndx = 0;
static int dbm_ndx = 0;
static int ncp_ndx = 0;
static int sw_ndx = 0;
static int dz_ndx = 0;
static int dzc_ndx = 0;
static int zdr_ndx = 0;
static int phi_ndx = 0;
static int rhohv_ndx = 0;
static int kdp_ndx = 0;
static int ldr_ndx = 0;
static int dbmv_ndx = 0;
static int dbmx_ndx = 0;
static int hrho_ndx = 0;
static int hang_ndx = 0;
static int vrho_ndx = 0;
static int vang_ndx = 0;
static int ldrv_ndx = 0;
static int iq_norm_ndx = 0;
static int iq_ang_ndx = 0;
static int dbzv_ndx = 0;
static char *acmrg=NULL;

static FILE * ts_fid = NULL;
static int sizeof_ts_fi = 0;

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

void 
piraqx_dd_conv (int interactive_mode)
{
    static int count=0, count_break=99, nok_count=0;
    int mark, nn;
    float *fp=NULL;

    if(!count) {
       piraqx_ini();
       piraqx_reset();
    }
    if(interactive_mode) {
	dd_intset();
	piraqx_positioning();
    }

    for(;;) {
	if(++count >= count_break) {
	    mark = 0;
	}
	if(difs->stop_flag &&
	   (difs->abrupt_start_stop || pui->new_sweep)) {
	    printf("\nBreak on stop time!\n");
	    break;
	}

	if(piraqx_select_ray()) {

	    if(difs->max_beams)
		  if(++pui->ray_count > difs->max_beams ) {
		      printf("\nBreak on ray count!\n");
		      break;
		  }
	    if(pui->sweep_count_flag) {
		pui->sweep_count_flag = NO;
		if(difs->max_sweeps) {
		    if(++pui->sweep_count > difs->max_sweeps ) {
			printf("\nBreak on sweep count!\n");
			break;
		    }
		}
		if(pui->vol_count_flag) {
		    pui->vol_count_flag = NO;
		    if(difs->max_vols) {
			if(++pui->vol_count > difs->max_vols ) {
			    printf("\nBreak on volume count!\n");
			    break;
			}
		    }
		}
		if( gri->ignore_this_sweep &&
		   pui->swp_que->count != pui->mark_selected_swp_count +1 )
		  {		/* it no longer refers to the previous scan */
		     gri->ignore_this_sweep = NO;
		  }
		pui->mark_selected_swp_count = pui->swp_que->count;
	    } /* sweep count flag */

	    if(!difs->catalog_only)
  		  productsx();

	    dd_stuff_ray();	/* pass it off to dorade code */
	}
	else {
	    if(!(nok_count++ % 1000)) {
		piraqx_print_stat_line(count);
	    }
	}
	pui->new_sweep = NO;
	pui->new_vol = NO;

	if((nn = piraqx_next_ray()) < 1) {
	    printf("Break on input status: %d\n", nn);
	    break;
	}
    }
    piraqx_reset();
}
/* c------------------------------------------------------------------------ */

void piraqx_ini (void)
{
    int ii, jj, nn, nt;
    CHAR_PTR aa, bb;
    char str[256], *sptrs[32], *sptrs2[32];
    double d;
    DD_TIME dts;
    float f;
    struct piraq_ray_que *rq, *last_rq;
    struct piraq_swp_que *sq, *last_sq;
    struct solo_list_mgmt *slm;

    /* c...ini() */


    scrs = (void *)malloc(256);

    dd_clear_dts(&dts);

    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();
    gri = return_gri_ptr();
    irq = dd_return_ios(PIRAQ_IO_INDEX, PIRAQX_FMT);
    irq->min_block_size = PIRAQX_MINHEADERLEN;

    pui = (struct piraq_useful_items *)malloc(sizeof(struct piraq_useful_items));
    memset(pui, 0, sizeof(struct piraq_useful_items));

    pui->check_ugly = NO;
    pui->prev_noise = 10.e22;
    pui->new_sweep_at_az = -999.;


    for(ii=0; ii < SWEEP_QUE_SIZE; ii++) {
	sq = (struct piraq_swp_que *)malloc(sizeof(struct piraq_swp_que));
	memset(sq, 0, sizeof(struct piraq_swp_que));
	if(!ii) {
	    pui->swp_que = sq;
	}
	else {
	    sq->last = pui->swp_que->last;
	    pui->swp_que->last->next = sq;
	}
	sq->next = pui->swp_que;
	pui->swp_que->last = sq;
	last_sq = sq;
	sq->swpang = EMPTY_FLAG;
    }

    dwlx_swap = (PIRAQX *)malloc(PIRAQX_MAXHEADERLEN);
    memset(dwlx_swap, 0, PIRAQX_MAXHEADERLEN);

    if(aa=get_tagged_string("PROJECT_NAME")) {
	strcpy(gri->project_name, aa);
    }
    else {
	strcpy(gri->project_name, "UNKNOWN");
    }
    if(aa=get_tagged_string("SITE_NAME")) {
	strcpy(gri->site_name, aa);
    }
    else {
	strcpy(gri->site_name, "UNKNOWN");
    }
    pui->latitude = pui->longitude = pui->altitude = EMPTY_FLAG;

    if(aa=get_tagged_string("RADAR_LATITUDE")) {
	pui->latitude =
	      d = atof(aa);
    }
    if(aa=get_tagged_string("RADAR_LONGITUDE")) {
	pui->longitude =
	      d = atof(aa);
    }
    if(aa=get_tagged_string("RADAR_ALTITUDE")) {
	pui->altitude =
	      d = atof(aa);
    }
    if(aa=get_tagged_string("RCONST_CORRECTION")) {
	pui->rconst_correction = atof(aa);
    }
    if(aa=get_tagged_string("ZDR_BIAS")) {
	pui->zdr_offset_corr = atof(aa);
    }
    if(aa=get_tagged_string("LDR_BIAS")) {
	pui->ldr_offset_corr = atof(aa);
    }
    if(aa=get_tagged_string("RANGE_CORRECTION")) {
	pui->range_correction = atof(aa);
    }
    if(aa=get_tagged_string("TIME_CORRECTION")) {
	pui->time_correction = atof(aa);
    }

    if(aa=get_tagged_string("MIN_AZ_DIFF")) {
        d = atof(aa);
        if(d > .01) min_az_diff = d;
        printf("Min. AZ diff: %.2f\n", min_az_diff);
    }

    if(aa=get_tagged_string("MIN_EL_DIFF")) {
        d = atof(aa);
        if(d > .01) min_el_diff = d;
    }

    if(aa=get_tagged_string("PPI_EL_TOL")) {
        d = atof(aa);
        if(d > .01) ppi_el_tol = d;
    }

    if(aa=get_tagged_string("RHI_AZ_TOL")) {
        d = atof(aa);
        if(d > .001) rhi_az_tol = d;
    }

    if(aa=get_tagged_string("NUM_AZ_DIFFS_AVGD")) {
       ii = atoi(aa);
       if(ii > 1) pui->num_az_diffs_avgd = ii;
    }

    if(aa=get_tagged_string("NUM_EL_DIFFS_AVGD")) {
       ii = atoi(aa);
       if(ii > 1) pui->num_el_diffs_avgd = ii;
    }

    if(aa=get_tagged_string("NUM_SHORT_AVG")) {
       ii = atoi(aa);
       if(ii > 1) pui->short_rq_len = ii;
    }

    if(aa=get_tagged_string("RENAME")) {
	piraq_name_aliases(aa, &top_ren);
    }
    if(aa=get_tagged_string("NEW_SWEEP_AT_AZ")) { 
        pui->new_sweep_at_az = atof(aa); 
    }

    if((aa=get_tagged_string("OPTIONS"))) {
	if(strstr(aa, "RABID"))
	  {
	     pui->options |= RABID_DOW;
	     dts.year = 2003;
	     dts.month = 5;
	     dts.day = 15;
	     d = d_time_stamp(&dts);
	     ii = (int)(d/86400);
	     printf ("yy:%d mo:%2d dd:%2d => unix_day:%d\n"
		     , dts.year, dts.month, dts.day, ii);
	  }
	if(strstr(aa, "RANGE0"))
	  { pui->options |= RANGE0_DEFAULT; }
	if(strstr(aa, "FLIP_VEL")) /* FLIP_VELOCITIES */
	  { pui->options |= FLIP_VELOCITIES; }
	if(strstr(aa, "FULL_MAT")) /* FULL_MATRIX */
	  { pui->options |= FULL_MATRIX; }
	if(strstr(aa, "SPOL_HO")) /* SPOL_HORZ_FLAG */
	  { pui->options |= SPOL_HORZ_FLAG; }
	if(strstr(aa, "DOW_F")) /* DOW_FLAG */
	  { pui->options |= DOW_FLAG; }
	if(strstr(aa, "DZ_TO_DBZ")) /* DZ_TO_DBZ_FLAG */
	  { pui->options |= DZ_TO_DBZ_FLAG; }
	if(strstr(aa, "IGNORE_TRANS")) /*  */
	  { pui->options |= IGNORE_TRANSITIONS; }
	if(strstr(aa, "PIRAQX_TRUST_TIME"))
	  { pui->options |= PIRAQX_TRUST_TIME; }
	if(strstr(aa, "PIRAQX_100X_NANOSECS"))
      { pui->options |= PIRAQX_100X_NANOSECS; }
    }

    nn = dd_min_rays_per_sweep();
    if(nn < 3) { nn = 3; }	/* make it at least 4 */
    pui->min_rays_per_sweep = nn++;

    for(ii=0; ii < nn; ii++) {
	rq = (struct piraq_ray_que *)malloc(sizeof(struct piraq_ray_que));
	memset(rq, 0, sizeof(struct piraq_ray_que));
	if(!ii) {
	    pui->ray_que = rq;
	}
	else {
	    rq->last = pui->ray_que->last;
	    pui->ray_que->last->next = rq;
	}
	rq->next = pui->ray_que;
	pui->ray_que->last = rq;
	last_rq = rq;
	rq->scan_num = -1;
	rq->vol_num = -1;
    }
    piraqx_reset();

    // Initialize running sums we might use
    pui->az_diff_rs = init_run_sum( pui->num_az_diffs_avgd
                    , pui->short_rq_len );
    pui->el_diff_rs = init_run_sum( pui->num_el_diffs_avgd
                    , pui->short_rq_len );
    pui->az_rs = init_run_sum( pui->num_az_diffs_avgd
                    , pui->short_rq_len );
    pui->el_rs = init_run_sum( pui->num_el_diffs_avgd
                    , pui->short_rq_len );

    
    if((aa=get_tagged_string("TAPE_DIR"))) {
      slm = solo_malloc_list_mgmt(256);
      nt = generic_sweepfiles( aa, slm, (char *)"", (char *)".tape", 0 );
      if ( nt < 1 ) {		/* Josh's CDs */
	 nt = generic_sweepfiles( aa, slm, (char *)"", (char *)"x", 0 );
      }
      bb = dd_est_src_dev_names_list((char *)"PRQ", slm, aa );
    }
    else if((aa=get_tagged_string("SOURCE_DEV"))) {
	bb = dd_establish_source_dev_names((char *)"PRQ", aa);
    }
    if( aa && bb ) {
      current_file_name = dd_next_source_dev_name((char *)"PRQ");
      dd_input_read_open(irq, current_file_name );
    }

    nn = piraqx_next_ray();
}
/* c------------------------------------------------------------------------ */

int 
piraqx_next_block (void)
{
  /* returns a pointer to the next block of piraq data
   * which in the case of CD-ROMs may block several rays in a record
   * otherwise it corresponds to a physical record on the tape
   */
  static int count=0, bp=298, illegal_header_count=0;
  int32_t keep_offset;
  int err_count = 0;
  int ii, mm, nn, mark, eof_count=0, ugly;
  int32_t recordlen=0;
  int size=-1, ugly_record_count=0;
  int dwel_record, rhdr_record;
  char *aa, *bb, *cc = NULL;
  char mess[256], *last_top;

  for(;;) {

    if(!count++) {
      mark = 0;
    }
    if(count >= bp) {
      mark = 0;
    }

    if(irq->top->bytes_left < irq->min_block_size) {
      if(irq->io_type == BINARY_IO && irq->top->bytes_left > 0) {
	keep_offset = irq->top->offset;
	dd_io_reset_offset(irq, irq->top->offset
			   + irq->top->sizeof_read
			   - irq->top->bytes_left);
	if(keep_offset == irq->current_offset) {
	  return(-1);
	}
	irq->top->bytes_left = 0;
      }
      dd_logical_read(irq, FORWARD);

      if(irq->top->read_state < 0) {
	if( ++err_count < 4 ) {
	  printf("Last read: %d errno: %d\n"
		 , irq->top->read_state, err_count);
	  irq->top->bytes_left = 0;
	  continue;
	}
	nn = irq->top->read_state;
	dd_input_read_close(irq);
	/*
	 * see if there is another file to read
	 */
	if(aa = dd_next_source_dev_name((char *)"PRQ")) {
	  current_file_name = aa;
	  if((ii = dd_input_read_open(irq, aa)) <= 0) {
	    return(size);
	  }
	  eof_count = 0;
	  err_count = 0;
	  continue;
	}
	else {
	  return(size);
	}
      }
      else if(irq->top->eof_flag) {
	err_count = 0;
	eof_count++;
	dd_stats->file_count++;
	printf( "EOF number: %d at %.2f MB\n"
		, dd_stats->file_count
		, dd_stats->MB);

	irq->top->bytes_left = 0;
	if(eof_count > 3) {
	  nn = irq->top->read_state;
	  dd_input_read_close(irq);
	  /*
	   * see if there is another file to read
	   */
	  if(aa = dd_next_source_dev_name((char *)"PRQ")) {
	    current_file_name = aa;
	    if((ii = dd_input_read_open(irq, aa)) <= 0) {
	      return(size);
	    }
	  }
	  else {
	    return(size);
	  }
	  err_count = 0;
	  eof_count = 0;
	  continue;
	}
	continue;
      }
      else {
	dd_stats->rec_count++;
	dd_stats->MB += BYTES_TO_MB(irq->top->sizeof_read);
	eof_count = 0;
	err_count = 0;
      }
    }
    aa = irq->top->at;
    recordlen = px_recordlen((PIRAQX*)aa);
    if (px_one((PIRAQX*)aa) != 1)
//	    recordlen = PX4(recordlen);  //JUl 26, 2011
	    recordlen = PX4(*(fourB *)&recordlen);  //JUl 26, 2011
	    
    char *desc = px_desc((PIRAQX*)aa);
    dwel_record = !strncmp("DWLX", desc, 4) || !strncmp("DWEL", desc, 4);

    if (!(dwel_record)) {
      if((illegal_header_count++ % 3) == 0 ) {
	sprintf(mess,
		"Illegal header id: %2x %2x %2x %2x %2x %2x rlen: %d  sizeof_read:  %d %d"
		, (int)aa[0] & 0xff
		, (int)aa[1] & 0xff
		, (int)aa[2] & 0xff
		, (int)aa[3] & 0xff
		, (int)aa[4] & 0xff
		, (int)aa[5] & 0xff
		, recordlen
		, irq->top->sizeof_read
		, illegal_header_count
		);
	dd_append_cat_comment(mess);
	printf("%s\n", mess);
      }
      if(((illegal_header_count-1) % 30) == 0 ) {
	sprintf(mess,  "%s %s"
		, current_file_name
		, dts_print(d_unstamp_time(gri->dts))
		);
	dd_append_cat_comment(mess);
	printf("%s\n", mess);
      }
    }

    if (! dwel_record) {
      if(irq->io_type != BINARY_IO) {
	irq->top->bytes_left = 0; /* just read another record */
	continue;
      }
      if(1) {
	dd_input_read_close(irq);
	if(aa = dd_next_source_dev_name((char *)"PRQ")) {
	  if((ii = dd_input_read_open(irq, aa)) <= 0) {
	    return(size);
	  }
	}
	else {
	  return(size);
	}
	irq->top->bytes_left = 0;
	continue;
      }
      /* otherwise try to reposition after the current bad block
       * perhaps by searching for DWEL or RHDR
       * but not for now
       */
      dd_io_reset_offset(irq, irq->top->offset
			 + irq->top->sizeof_read
			 - irq->top->bytes_left
			 + recordlen);
      irq->top->bytes_left = 0;
      continue;
    }
    if(irq->io_type == BINARY_IO) {
      if(recordlen > irq->top->bytes_left) {
	keep_offset = irq->top->offset;
	dd_io_reset_offset(irq, irq->top->offset
			   + irq->top->sizeof_read
			   - irq->top->bytes_left);
	if(keep_offset == irq->current_offset) {
	  return(-1);
	}
	irq->top->bytes_left = 0;
      }
      else {
	break;
      }
    }
    else {
      ugly = (recordlen - irq->top->bytes_left > 1) || recordlen < 1;

      if(pui->check_ugly && dwel_record && ugly) {
	++ugly_record_count;
	printf("Ugly record count: %d  rlen: %d  left: %d\n"
	       , ugly_record_count, px_recordlen(dwlx)
	       , irq->top->bytes_left);
	irq->top->bytes_left = 0;
	if(ugly_record_count > 5) {
	  return(-1);
	}
	continue;
      }
      ugly_record_count = 0;
      break;		/* for non binary io */
    }
  }
  size = recordlen;

  return(size);
}

/* c------------------------------------------------------------------------ */

int piraqx_next_ray (void)
{
    static int whats_left=0;
    static bool prev_transition = false;
    struct io_packet *dp, *dpx;
    int ii, kk, nn, eof_count = 0, mark, gotta_header=NO, looping, size;
    int err_count = 0;
    int ugly_record_count=0, delay = 0;
    float f, r, gs, fxd;
    double d, d2, diff, sd, sum, sumsq, avg, d_nn, ratio;
    char *aa;
    struct piraq_ray_que *rq;

    pui->ignore_this_ray = NO;

    for(;;) {
	if(dd_control_c()) {
	    dd_reset_control_c();
	    return(-1);
	}
	if((size = piraqx_next_block()) < irq->min_block_size) {
	   return(-1);
	}
	aa = irq->top->at;
	break;
    }
    rq = pui->ray_que = pui->ray_que->next;

    dd_stats->ray_count++;
    pui->ray_count++;
    piraqx_map_hdr(aa, gotta_header);

    /* c...next_ray */
    if( fabs( rq->az_diff ) < 11. ) {
        pui->az_diff_sum += rq->az_diff;
        pui->az_diff_run_sum =
                running_sum( pui->az_diff_rs, rq->az_diff );
    }

    if( fabs( rq->el_diff ) < 11. ) {
        pui->el_diff_sum += rq->el_diff;
        pui->el_diff_run_sum =
                running_sum( pui->el_diff_rs, rq->el_diff );
    }
    
    pui->az_run_sum = running_sum( pui->az_rs, rq->az );
    pui->el_run_sum = running_sum( pui->el_rs, rq->el );

    ++pui->sweep_ray_num;
    rq->ray_num = pui->sweep_ray_num;

    dp = dd_return_next_packet(irq);
    dp->len = px_recordlen(dwlx);
    irq->top->at += px_recordlen(dwlx);
    irq->top->bytes_left -= px_recordlen(dwlx);
    pui->count_rays++;

	if( rq->vol_num != rq->last->vol_num) {
	    pui->possible_new_vol = YES;
	}
	if( rq->num_gates != rq->last->num_gates ) {
	    pui->possible_new_vol = YES;
	}
	if( rq->scan_type != rq->last->scan_type ) {
	    pui->possible_new_vol = YES;
	}
	if(rq->scan_num != rq->last->scan_num) {
	    pui->possible_new_sweep = YES;
       }
//    if (px_transition(dwlx) && ! prev_transition) {
//        printf("Entered transition at ray %d, time %.2f\n", rq->ray_num, rq->time);
//    }
//    if (! px_transition(dwlx) && prev_transition) {
//        printf("Left transition at ray %d, time %.2f\n", rq->ray_num, rq->time);
//    }
	prev_transition = px_transition(dwlx);

    if( pui->sweep_ray_num >= dd_max_rays_per_sweep())
	{ pui->new_sweep = YES; }
    if(piraqx_isa_new_sweep())
	{ pui->new_sweep = YES; }

    /* 
     * If requested, start a new PPI or SUR sweep whenever the
     * azimuth crosses new_sweep_at_az.
     */
    if (pui->new_sweep_at_az >= 0.0) {
        int scanmode = pui->swp_que->scan_mode;
        float prevaz = pui->swp_que->last->swpang;
        float thisaz = pui->swp_que->swpang;

        if ((scanmode == PPI || scanmode == SUR) &&
            in_sector(pui->new_sweep_at_az, prevaz, thisaz)) {
            pui->new_sweep = YES;
        }
    }

    /* new sweep? */


    if((pui->new_sweep || pui->new_vol ||
			pui->possible_new_sweep)) {

	/* c...mark */
	/*
	 * see if the last sweep should be saved
	 * remember you can set MIN_RAYS_PER_SWEEP
	 */
       if (pui->options & RABID_DOW) {
	  printf ("D:%d ", pui->unix_day);
       }
       
       if( gri->scan_mode == RHI) {
	    if(fabs(pui->el_diff_sum) < 4.) { /* RHI can't be less than 4 deg. */
//	    printf("Ignoring RHI < 4 degrees @ time %.2f\n", gri->time);
		gri->ignore_this_sweep = YES;
	    }
	 }
	else if(fabs(pui->az_diff_sum) < 7.) {
	   /* ppi can't be less than 7 deg. */
//        printf("Ignoring PPI < 7 degrees\n");
	   gri->ignore_this_sweep = YES;
	}
	pui->possible_new_sweep = NO;
	dd_stats->sweep_count++;
	pui->sweep_count_flag = pui->new_sweep = YES;
	gri->sweep_num++;
	if(pui->options & SPOL_FLAG || pui->options & DOW_FLAG ) {
	    gri->scan_mode = px_scan_type(dwlx);
	}
	pui->sweep_ray_num = 1;
	pui->transition_count = 0;
	pui->max_az_diff = pui->max_el_diff = 0;
	pui->el_diff_sum = pui->az_diff_sum = 0;
	
    reset_running_sum( pui->az_diff_rs );
    running_sum( pui->az_diff_rs, rq->az_diff );

    reset_running_sum( pui->el_diff_rs );
    running_sum( pui->el_diff_rs, rq->el_diff );

    reset_running_sum( pui->az_rs );
    running_sum( pui->az_rs, rq->az );

    reset_running_sum( pui->el_rs );
    running_sum( pui->el_rs, rq->el );    

	pui->swp_que->swpang_diff =
	      pui->swp_que->last->swpang != EMPTY_FLAG
		    ? angdiff(pui->swp_que->last->swpang, pui->swp_que->swpang)
			  : 0;
	/* difference is between the previous two sweeps;
	 * not this sweep and the last sweep
	 */
	pui->swpang_diff_sum += pui->swp_que->swpang_diff;

	pui->swp_que = pui->swp_que->next;
	pui->swp_que->count = dd_stats->sweep_count;
	pui->swp_que->rcvr_pulsewidth = px_rcvr_pulsewidth(dwlx);
	pui->swp_que->scan_mode = gri->scan_mode;
	pui->swp_que->swpang = gri->fixed;
	pui->swp_que->prf = gri->PRF;

	if(pui->options & SPOL_FLAG && pui->possible_new_vol)
	    { pui->new_vol = YES; }

	if(piraqx_isa_new_vol() || pui->new_vol) {
	    pui->possible_new_vol = NO;
	    pui->vol_count_flag = pui->new_vol = YES;
	    pui->swpang_diff_sum = 0;
	    pui->vol_sweep_count = 0;
	    gri->vol_num++;
	    dd_stats->vol_count++;
	    pui->swp_que->new_vol = YES;
	    pui->vol_start = pui->swp_que;
	    r = gri->range_b1;
	    gs = gri->bin_spacing;
	    for(ii=0; ii < gri->num_bins; ii++ ) {
		gri->range_value[ii] = r;
		r += gs;
	    }
	}
	pui->vol_sweep_count++;
	pui->last_scan_mode = gri->scan_mode;

    } /* end isa new sweep */


    if(pui->runaway) {
	return(-1);
    }

    gri->fixed = px_fxd_angle(dwlx);
    // If the fixed angle in the data is bad, just use the last az or el. 
    if (fabs(gri->fixed) > 1000) {
        gri->fixed = (gri->scan_mode == RHI) ? px_az(dwlx) : px_el(dwlx);
    }
    gri->azimuth = px_az(dwlx);
    gri->elevation = px_el(dwlx);

    /*
     * calculate antenna movement per ray
     */
    if (!(pui->options & RABID_DOW)) {
      if(fabs(d = pui->ray_que->az_diff) < 5.)
        gri->azimuth = FMOD360(gri->azimuth - .5 * d);

      if(fabs(d = pui->ray_que->el_diff) < 5.)
        gri->elevation = FMOD360(gri->elevation - .5 * d);
    }

    if(gri->scan_mode == RHI) {
	gri->rotation_angle = gri->corrected_rotation_angle = gri->elevation;
    }
    else {
	gri->rotation_angle = gri->corrected_rotation_angle = gri->azimuth;
    }
    gri->tilt = gri->elevation;

    gri->source_sweep_num = px_transition(dwlx) ? 1 : 0;
    gri->source_sweep_num += 10 * (px_scan_num(dwlx) % 10);
    gri->source_sweep_num += 100 * (px_vol_num(dwlx) % 10);

    pui->skipped_backwards = NO;

    mark = 0;
    return(1);
}
/* c------------------------------------------------------------------------ */

void piraqx_map_hdr (char *aa, int gotta_header)
{
    static int count=0;
    int jj, kk, nn, mark, new_vol = NO;
    double d, subsec;
    struct piraq_ray_que *rq=pui->ray_que;
    int tdiff;
    int fmt;
    float prt;
    char message[256], str[256], tstr[32], tstr2[32];
    uint4 secs_mask = 0x7fffffff;
    uint4 rev, unswapped_rev;
    uint4 headerLen;
    uint8 temp1;
    struct timeval tv;

    /*
#if SYSTEM_CLOCK_INT == 48000000
#define    COUNTFREQ    6000000
#else
#define    COUNTFREQ    8000000
#endif
We're using 48000000 so use 6000000.
     */
#define    COUNTFREQ    6000000

    count++;

    dwlx0 = dwlx = (PIRAQX *)aa;
    rev = px_rev(dwlx0);

    if (px_one(dwlx0) != 1) {
       /*
       	* Unswap the rev number so we know the size of the header...
	*/
       unswapped_rev = rev;
       swack_long ((char*)&unswapped_rev, (char*)&rev, 1);

       headerLen = (rev == 1) ? sizeof(PIRAQX_REV1) : sizeof(PIRAQX_REV2);

       swack_long (aa, (char *)dwlx_swap, headerLen / sizeof (uint4));
       swack_double ((char *)&dwlx0->pulse_num
		     , (char *)&dwlx_swap->pulse_num, 1);
       dwlx = dwlx_swap;
    }
    pui->raw_data = aa + px_byte_offset_to_data(dwlx);

    gri->gpptr4 = (void *)dwlx;
    gri->sizeof_gpptr4 = (rev == 1) ? sizeof(PIRAQX_REV1) : sizeof(PIRAQX_REV2);
    gri->transition = (px_transition(dwlx)) ? IN_TRANSITION : NORMAL;
    gri->num_bins = (px_gates(dwlx));
    gri->num_samples = (px_hits(dwlx));
    gri->pulse_width = (px_rcvr_pulsewidth(dwlx));
    gri->range_b1 = px_meters_to_first_gate(dwlx);
    gri->bin_spacing = px_gate_spacing_meters(dwlx)[0];
    gri->range_b1 += pui->range_correction;
    strcpy (gri->radar_name, px_radar_name(dwlx));

    gri->dts->year = 1970;
    gri->dts->month = gri->dts->day = 0;

    /*
     * Deal with times.  Only if the PIRAQX_TRUST_TIME option is set do we
     * accept the time in the header. Otherwise we calculate time from pulse
     * number in the header.
     */
    if (pui->options & PIRAQX_TRUST_TIME) {
	    /*
    	     * Even if we're otherwise trusting time from the file,
    	     * some files have nanoseconds off by a factor of 100.  Fix
    	     * that if requested.
    	     */
	    if (pui->options & PIRAQX_100X_NANOSECS) {
	    	    dwlx->nanosecs *= 100;
	    }
    } else {
	    /*
	     * By default, PIRAQX data has only 1-second precision time in the
	     * header (i.e., secs is good, but nanosecs is a useless value).
	     * Hence, we translate the pulse_num from the header into a time
	     * with good subsecond precision and write that calculated time
	     * back into the header.
	     */

	    /* klooge! (This is a leftover from Dick.  Not clear why this is
	     * here...  5/4/2009 CB)*/
	    dwlx->secs = dwlx->secs & secs_mask;

	    /*
	     * Calculate average PRT, so our pulse number to time calculation
	     * below works properly.
	     */
	    fmt = px_dataformat(dwlx);
	    if (fmt == DATA_DUALPP || fmt == DATA_DUALPPFLOAT) {
		    prt = 0.5 * (px_prt(dwlx)[0] + px_prt(dwlx)[1]);
	    } else {
		    prt = px_prt(dwlx)[0];
	    }
	    /*
	     * Turn pulse number into clock counts (at COUNTFREQ) since the
	     * epoch, then calculate seconds and nanoseconds since the epoch
	     * and overwrite the old values in the header.
	     */
	    temp1 = px_pulse_num(dwlx) * (uint8)(prt * (float)COUNTFREQ + 0.5);
	    dwlx->secs = temp1 / COUNTFREQ;
	    dwlx->nanosecs = ((uint8)1000000000 *
		(temp1 % ((uint8)COUNTFREQ))) / (uint8)COUNTFREQ;
    }

    pui->unix_day = px_secs(dwlx)/86400;

    rq->ptime = px_secs(dwlx);
    rq->subsec = px_nanosecs(dwlx);

    pui->time =  px_secs(dwlx) + (double)(px_nanosecs(dwlx)) * 1.0e-9;
    pui->time += pui->time_correction;
    gri->dts->time_stamp = gri->time = rq->time = pui->time;

    gri->altitude = pui->altitude != EMPTY_FLAG
	  ? pui->altitude : (px_radar_altitude(dwlx));
    gri->latitude = pui->latitude != EMPTY_FLAG
	  ? pui->latitude : (px_radar_latitude(dwlx));
    gri->longitude = pui->longitude != EMPTY_FLAG
	  ? pui->longitude : (px_radar_longitude(dwlx));

    gri->clutter_filter_val = px_clutter_type(dwlx)[0];
    /*
    gri->clutter_start = px_clutter_start(dwlx)[0];
    gri->clutter_end = px_clutter_end(dwlx)[0];
     */
    gri->ipps[0] = px_prt(dwlx)[0] * 1000.; /* convert to milliseconds */
    gri->PRF = px_prt(dwlx)[0] ? 1./px_prt(dwlx)[0] : EMPTY_FLAG;

    new_vol = pui->last_dataformat != px_dataformat(dwlx) ||
        pui->last_gates != px_gates(dwlx) ||
        pui->last_rcvr_pulsewidth != px_rcvr_pulsewidth(dwlx) ||
        pui->last_prt != px_prt(dwlx)[0];

    if (new_vol) {
       gri->binary_format = DD_16_BITS;
       gri->missing_data_flag = EMPTY_FLAG;
       gri->source_format = PIRAQX_FMT;
       gri->compression_scheme = NO_COMPRESSION;
       gri->missing_data_flag = EMPTY_FLAG;

       piraqx_name_replace(gri->radar_name, top_ren);

       gri->num_freq = 1;
       gri->num_ipps = 1;
       gri->wavelength = px_frequency(dwlx) ? SPEED_OF_LIGHT/px_frequency(dwlx) : 0;
       gri->freq[0] = px_frequency(dwlx) * 1.e-9; /* GHz */

       gri->pulse_width = px_xmit_pulsewidth(dwlx);
       gri->h_beamwidth = px_H_beam_width(dwlx);
       gri->v_beamwidth = px_V_beam_width(dwlx);
# ifdef obsolete
       gri->peak_power = px_peak_power(dwlx);
# endif
       gri->radar_constant = px_rconst(dwlx);
       gri->rcvr_gain = px_receiver_gain(dwlx);
       gri->ant_gain = px_antenna_gain(dwlx);
       gri->noise_power = px_noise_power(dwlx);

       gri->scan_mode = PPI;
       gri->radar_type = GROUND;
       gri->sweep_speed = EMPTY_FLAG;
       gri->rcv_bandwidth = -999.;	/* MHz */
       gri->dd_radar_num = dd_assign_radar_num(gri->radar_name);
       gri->source_vol_num = ++pui->vol_num;

       switch((int)px_polarization(dwlx)) {
	case 'E':
	  gri->polarization = ELLIPTICAL;
	  break;
	case 'V':
	  gri->polarization = VERTICAL;
	  break;
	case 'C':
	  gri->polarization = CIRCULAR_RIGHT;
	  break;
	default:
	  gri->polarization = HORIZONTAL;
	  break;
       }


       pui->last_dataformat = px_dataformat(dwlx);
       pui->last_gates = px_gates(dwlx);
       pui->last_rcvr_pulsewidth = px_rcvr_pulsewidth(dwlx);
       pui->last_prt = px_prt(dwlx)[0];
       if(gri->wavelength > 0 && gri->PRF > 0)
           gri->nyquist_vel = gri->wavelength * gri->PRF * .25;
       pui->new_vol = pui->new_sweep = YES;
       piraqx_fields();
       if (acmrg) {
	  nn = 20;
	  str_terminate (str, gri->project_name, nn);
	  if (strlen (str) + strlen (acmrg) < nn ) {
	     strcat (str, acmrg);
	  }
	  else {
	     jj = nn-strlen (acmrg)-1;
	     strcat (str+jj, acmrg);
	  }
	  strcpy (gri->project_name, str);
       }
    }
    rq->hits = px_hits(dwlx);
    rq->prt = px_prt(dwlx)[0];
    rq->scan_num = px_scan_num(dwlx);
    rq->vol_num = px_vol_num(dwlx);
    rq->num_gates = px_gates(dwlx);
    rq->scan_type = px_scan_type(dwlx);

    rq->az = gri->azimuth;
    rq->el = gri->elevation;
    rq->rcvr_pulsewidth = px_rcvr_pulsewidth(dwlx);;

    rq->az_diff =
	  angdiff(rq->last->az, rq->az);

    rq->el_diff =
	  angdiff(rq->last->el, rq->el);

    rq->time_diff = rq->time - rq->last->time;

    if(fabs(rq->az_diff) > fabs(pui->max_az_diff)) {
	pui->max_az_diff = rq->az_diff;
    }
    if(fabs(rq->el_diff) > fabs(pui->max_el_diff)) {
	pui->max_el_diff = rq->el_diff;
    }

   mark = 0;
}
/* c------------------------------------------------------------------------ */

int 
piraqx_isa_new_vol (void)
{
    struct piraq_ray_que *rq=pui->ray_que;

       if(rq->vol_num != rq->last->vol_num) {
	  return(YES);
       }
       return(NO);

}
/* c------------------------------------------------------------------------ */

int 
piraqx_isa_new_sweep (void)
{
    /* c...mark */
    struct piraq_ray_que *rq=pui->ray_que;

	rq = pui->ray_que;

	if(px_transition(dwlx)) {
	    pui->transition_count++;
	}
	else {
	    pui->transition_count = 0;
	}
	
	if(rq->scan_num != rq->last->scan_num) {
	    pui->possible_new_sweep = YES;
	    return(YES);
	}

	if(pui->transition_count) { /* we're in a transition */
	    double short_avg_az_diff =
	            short_avg_running_sum( pui->az_diff_rs ); /* NUM_SHORT_AVG */
	    double short_avg_el_diff =
	            short_avg_running_sum( pui->el_diff_rs ); /* NUM_SHORT_AVG */

	    if( gri->scan_mode != RHI && fabs(short_avg_el_diff) > min_el_diff
	            && fabs(short_avg_az_diff) < rhi_az_tol ) {
	        /*
	         * MIN_EL_DIFF and RHI_AZ_TOL
	         *
	         * moving in elevation and "not" moving in azimuth
	         */
	        gri->scan_mode = RHI;
//	        printf("New sweep (RHI_AZ_TOL): [avg el diff (%.2f) > %.2f] and "
//	               "[avg az diff (%.2f) < %.2f] @ %.2f\n", 
//	               fabs(short_avg_el_diff), min_el_diff,
//	               fabs(short_avg_az_diff), rhi_az_tol,
//	               rq->time);
	        return(YES);
	    }

	    if(gri->scan_mode == RHI && fabs(short_avg_az_diff) > min_az_diff
	            && fabs(short_avg_el_diff) < ppi_el_tol ) {
	        /*
	         * using MIN_AZ_DIFF and PPI_EL_TOL
	         *
	         * moving in azimuth and not moving in elevation
	         */
	        gri->scan_mode = PPI;
	        return(YES);
	    }
	}
    
	return(NO);

}
/* c------------------------------------------------------------------------ */

int 
piraqx_select_ray (void)
{
    int ii, jj, ok=YES, mark;
    static int select = 0, reject = 0;
    static int select_trip = 111, reject_trip = 222;


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

    if(difs->beam_skip) {
	if(pui->ray_count % difs->beam_skip){
	    mark = 0;
	    return(NO);
	}
    }

    if(difs->sweep_skip) {
	if(!((gri->sweep_num-1) % difs->sweep_skip)) {
	    mark = 0;
	}
	else
	      ok = NO;
    }

    if(difs->num_fixed_lims) {
	for(ii=0; ii < difs->num_fixed_lims; ii++ ) {
	    if(gri->fixed >= difs->fxd_angs[ii]->lower &&
	       gri->fixed <= difs->fxd_angs[ii]->upper) {
		break;
	    }
	}
	if(ii == difs->num_fixed_lims) {
	    ok = NO;
	}
    }

    if(difs->num_modes) {
	if(difs->modes[gri->scan_mode]) {
	    mark = 0;
	}
	else
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
    if( pui->options & SPOL_FLAG || pui->options & DOW_FLAG ) {
       if(( pui->options & IGNORE_TRANSITIONS ) && px_transition(dwlx) )
	 { ok = NO; }
    }

    return(ok);
}
/* c------------------------------------------------------------------------ */

void 
piraqx_print_stat_line (int count)
{
    DD_TIME *dts=gri->dts;

    dts->time_stamp = gri->time;
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
piraqx_reset (void)
{
    struct piraq_ray_que *rq=pui->ray_que;
    int nn = pui->short_rq_len;

    pui->vol_count_flag = pui->sweep_count_flag =
	  pui->new_sweep = pui->new_vol = YES;
    pui->count_rays = pui->ray_count = pui->sweep_count = pui->vol_count = 0;
    pui->ref_time = pui->trip_time = 0;
    difs->stop_flag = NO;
    /*
     * make sure to start a new sweep and vol
     */
    gri->vol_num++;
    gri->sweep_num++;
    difs->stop_flag = NO;

    pui->az_diff_short_sum =
      pui->el_diff_short_sum =
	pui->az_short_sum =
	  pui->el_short_sum =
	    0;
}
/* c------------------------------------------------------------------------ */

void 
piraqx_name_replace (char *name, struct rename_str *top)
{
    struct rename_str *this_str=top;  //Jul 26, 2011 this keyword issue

    for(; this_str; this_str = this_str->next) {
	if(strstr(name, this_str->old_name)) {
	    strcpy(name, this_str->new_name);
	    break;
	}
    }
}
/* c------------------------------------------------------------------------ */

void 
piraqx_name_aliases (char *aliases, struct rename_str **top)
{
    /* routine to construct the list of aliases */
    struct rename_str *this_str, *last;  //Jul 26, 2011 *this
    char string_space[256], *str_ptrs[32];
    int ii, nt;

    strcpy(string_space, aliases);
    nt = dd_tokens(string_space, str_ptrs);

    for(ii=0; ii+2 < nt; ii+=3){
	this_str = (struct rename_str *)malloc(sizeof(struct rename_str));
	memset(this_str, 0, sizeof(struct rename_str));

	/* get the first name */
	strcpy(this_str->old_name, str_ptrs[ii]);

	/* get the second name */
	strcpy(this_str->new_name, str_ptrs[ii+2]);

	if(!(*top)) {
	    *top = this_str;
	}
	else {
	    last->next = this_str;
	    this_str->last = last;
	}
	(*top)->last = this_str;
	last = this_str;
    }
}
/* c------------------------------------------------------------------------ */

void piraqx_backwards_reset (int modest)
{
    if(!modest) {
	pui->skipped_backwards = YES;
	pui->new_vol = YES;
	pui->count_rays = 0;
	gri->scan_mode = PPI;
	pui->transition_count = 1;
    }
    pui->ray_que->fxdang_diff =
	  pui->ray_que->az_diff =
		pui->ray_que->el_diff =
		      pui->ref_time = 0;
}
/* c------------------------------------------------------------------------ */

void piraqx_fields (void)
{
    int ii, pn=0, nbins;
    float scale = 100., bias = 0, r, gs;
    char *aa;

    nbins = gri->num_bins;
    gri->num_fields_present = 0;

    /*
Dick,

These are the parameter names for S-Pol, with some editing and input
from Vivek.

Bob

                01234567890123456789012345678901234567890123456789
                ^         ^         ^         ^         ^
VE    m/sec     Doppler radial velocity
DM    dBm       Horizontal copolar received power (HH)
NCP  (no units) Normalized coherent power (H)
SW    m/s       Spectrum width of VE
DZ    dBZ       Horizontal copolar reflectivity (HH)
DCZ   dBZ       Coherent reflectivity (HH)
LVDR  dB        Linear depolarization ratio (V tx, H rec)
NIQ             Average magnitude of the backscattered power (HH?)
AIQ   degrees   Average phase of the backscattered power (HH)
                (AIQ is used directly in the Fabry refractive index
                work for the determination of virtual temperature)
CH   (no units) Magnitude of the cross correlation between HH and VH
AH    degrees   Angle of the cross correlation between HH and VH
CV   (no units) Magnitude of the cross correlation between VV and HV
AV    degrees   Angle of the cross correlation between VV and HV
RHOHV (no unit) Correlation coefficient between HH and VV
LDR   dB        Linear depolarization ratio (H tx, V rec)
DL    dBm       Vertical copolar received power (VV)
DX    dBm       Cross-polar (H tx, V rec) received power (used
                primarily to threshold LDR)
ZDR   dB        Differential reflectivity, HH - VV
PHI   degrees   Differential propagation phase between HH and VV
KDP   deg/km    Specific differential propagation phase between HH, VV


VE    Doppler radial velocity
DM    Horizontal copolar received power (HH)
NCP   Normalized coherent power (H)
SW    Spectrum width of VE
DZ    Horizontal copolar reflectivity (HH)
DCZ   Coherent reflectivity (HH)
LVDR  Linear depolariz. ratio (V tx, H rec)
NIQ   Avg magnitude of backscatter power (HH?)
AIQ   Avg phase of backscattered power (HH)
CH    Mag. of cross corr. between HH and VH
AH    Angle of cross corr. between HH and VH
CV    Mag. of cross corr. between VV and HV
AV    Angle of cross corr. between VV and HV
RHOHV Corr. coefficient between HH and VV
LDR   Lin. depolarization ratio (H tx, V rec)
DL    Vertical copolar received power (VV)
DX    Cross-polar (H tx, V rec) received power
ZDR   Differential reflectivity, HH - VV
PHI   Diff. propagation phase btwn HH and VV
KDP   Specific diff. prop. phase btwn HH, VV
      ^         ^         ^         ^         ^

     */



    /* these fields are always present */

    ve_ndx =
	pn = gri->num_fields_present++;
    gri->dd_scale[pn] = scale;
    gri->dd_offset[pn] = bias;
    gri->field_id_num[pn] = VE_ID_NUM;
    strcpy( gri->field_name[pn], "VE" );
    strncpy( gri->field_long_name[pn]
	     , "Doppler radial velocity                                    "
	     , 40);
    strcpy( gri->field_units[pn], "m/s     " );
    gri->actual_num_bins[pn] = nbins;

    dbm_ndx =
	pn = gri->num_fields_present++;
    gri->dd_scale[pn] = scale;
    gri->dd_offset[pn] = bias;
    gri->field_id_num[pn] = DBM_ID_NUM;
    strcpy( gri->field_name[pn], "DM      " );
    strncpy( gri->field_long_name[pn]
	     , "Horizontal copolar received power (HH)                     "
	     , 40);
    strcpy( gri->field_units[pn], "dBm     " );
    gri->actual_num_bins[pn] = nbins;

    ncp_ndx =
	pn = gri->num_fields_present++;
    gri->dd_scale[pn] = scale;
    gri->dd_offset[pn] = bias;
    gri->field_id_num[pn] = NCP_ID_NUM;
    strcpy( gri->field_name[pn], "NCP      " );
    strncpy( gri->field_long_name[pn]
	     , "Normalized Coherent Power (H)                             "
	     , 40);
    strcpy( gri->field_units[pn], "         " );
    gri->actual_num_bins[pn] = nbins;

    sw_ndx =
	pn = gri->num_fields_present++;
    gri->dd_scale[pn] = scale;
    gri->dd_offset[pn] = bias;
    gri->field_id_num[pn] = SW_ID_NUM;
    strcpy( gri->field_name[pn], "SW        " );
    strncpy( gri->field_long_name[pn]
	     , "Spectrum width of VE                                        "
	     , 40);
    strcpy( gri->field_units[pn], "m/s     " );
    gri->actual_num_bins[pn] = nbins;

    dz_ndx =
	pn = gri->num_fields_present++;
    gri->dd_scale[pn] = scale;
    gri->dd_offset[pn] = bias;
    gri->field_id_num[pn] = DZ_ID_NUM;
    if (pui->options & DZ_TO_DBZ_FLAG)
      { strcpy( gri->field_name[pn], "DBZ     " ); }
    else
      { strcpy( gri->field_name[pn], "DZ      " ); }

    strncpy( gri->field_long_name[pn]
	     , "Horizontal copolar reflectivity (HH)                        "
	     , 40);
    strcpy( gri->field_units[pn], "dBz     " );
    gri->actual_num_bins[pn] = nbins;

    dzc_ndx =
	pn = gri->num_fields_present++;
    gri->dd_scale[pn] = scale;
    gri->dd_offset[pn] = bias;
    gri->field_id_num[pn] = DZ_ID_NUM;
    strcpy( gri->field_name[pn], "DCZ      " );
    strncpy( gri->field_long_name[pn]
	     , "Coherent Reflectivity (HH)                                "
	     , 40);
    strcpy( gri->field_units[pn], "dBz     " );
    gri->actual_num_bins[pn] = nbins;


    switch( px_dataformat(dwlx) ) {

    case DATA_DUALPP:
    case DATA_DUALPPFLOAT:

	ve1_ndx =
	    pn = gri->num_fields_present++;
	gri->dd_scale[pn] = scale;
	gri->dd_offset[pn] = bias;
	gri->field_id_num[pn] = VE_ID_NUM;
	strcpy( gri->field_name[pn], "V1" );
	strncpy( gri->field_long_name[pn]
		 , "Doppler radial velocity (PRF1)                          "
		 , 40);
	strcpy( gri->field_units[pn], "m/s     " );
	gri->actual_num_bins[pn] = nbins;

	ve2_ndx =
	    pn = gri->num_fields_present++;
	gri->dd_scale[pn] = scale;
	gri->dd_offset[pn] = bias;
	gri->field_id_num[pn] = VE_ID_NUM;
	strcpy( gri->field_name[pn], "V2" );
	strncpy( gri->field_long_name[pn]
		 , "Doppler radial velocity (PRF2)                          "
		 , 40);
	strcpy( gri->field_units[pn], "m/s     " );
	gri->actual_num_bins[pn] = nbins;

	sw1_ndx =
	    pn = gri->num_fields_present++;
	gri->dd_scale[pn] = scale;
	gri->dd_offset[pn] = bias;
	gri->field_id_num[pn] = SW_ID_NUM;
	strcpy( gri->field_name[pn], "W1" );
	strncpy( gri->field_long_name[pn]
		 , "Spectral Width of VE1                                           "
		 , 40);
	strcpy( gri->field_units[pn], "m/s     " );
	gri->actual_num_bins[pn] = nbins;

	sw2_ndx =
	    pn = gri->num_fields_present++;
	gri->dd_scale[pn] = scale;
	gri->dd_offset[pn] = bias;
	gri->field_id_num[pn] = SW_ID_NUM;
	strcpy( gri->field_name[pn], "W2" );
	strncpy( gri->field_long_name[pn]
		 , "Spectral Width of VE2                                   "
		 , 40);
	strcpy( gri->field_units[pn], "m/s     " );
	gri->actual_num_bins[pn] = nbins;
	break;

    case DATA_POL_PLUS:
    case DATA_MAX_POL:
    case DATA_HVSIMUL:
    case DATA_SHRTPUL:
    case DATA_SMHVSIM:
	if( pui->options & FULL_MATRIX ) {

	    ldrv_ndx = pn = gri->num_fields_present++;
	    gri->dd_scale[pn] = 100.;
	    gri->dd_offset[pn] = 0;
	    gri->field_id_num[pn] = 0;
	    strcpy( gri->field_name[pn], "LVDR    " );
	    strncpy( gri->field_long_name[pn]
		     , "Linear depolariz. ratio (V tx, H rec)             "
		     , 40);
	    strcpy( gri->field_units[pn], "dBm " );
	    gri->actual_num_bins[pn] = nbins;

	    iq_norm_ndx = pn = gri->num_fields_present++;
	    gri->dd_scale[pn] = 100.;
	    gri->dd_offset[pn] = 0;
	    gri->field_id_num[pn] = 0;
	    strcpy( gri->field_name[pn], "NIQ     " );
	    strncpy( gri->field_long_name[pn]
		     , "Avg magnitude of backscatter power (HH)            "
		     , 40);
	    strcpy( gri->field_units[pn], "deg." );
	    gri->actual_num_bins[pn] = nbins;

	    iq_ang_ndx = pn = gri->num_fields_present++;
	    gri->dd_scale[pn] = 100.;
	    gri->dd_offset[pn] = 0;
	    gri->field_id_num[pn] = 0;
	    strcpy( gri->field_name[pn], "AIQ     " );
	    strncpy( gri->field_long_name[pn]
		     , "Avg phase of backscattered power (HH)               "
		     , 40);
	    strcpy( gri->field_units[pn], "deg." );
	    gri->actual_num_bins[pn] = nbins;
	}

    case DATA_POL12:
	if( pui->options & FULL_MATRIX ) {

	    hrho_ndx = pn = gri->num_fields_present++;
	    gri->dd_scale[pn] = 100.;
	    gri->dd_offset[pn] = 0;
	    gri->field_id_num[pn] = 0;
	    strcpy( gri->field_name[pn], "CH      " );
	    strncpy( gri->field_long_name[pn]
		     , "Mag. of cross corr. between HH and VH              "
		     , 40);
	    strcpy( gri->field_units[pn], "    " );
	    gri->actual_num_bins[pn] = nbins;

	    hang_ndx = pn = gri->num_fields_present++;
	    gri->dd_scale[pn] = 100.;
	    gri->dd_offset[pn] = 0;
	    gri->field_id_num[pn] = 0;
	    strcpy( gri->field_name[pn], "AH      " );
	    strncpy( gri->field_long_name[pn]
		     , "Angle of cross corr. between HH and VH              "
		     , 40);
	    strcpy( gri->field_units[pn], "deg." );
	    gri->actual_num_bins[pn] = nbins;

	    vrho_ndx = pn = gri->num_fields_present++;
	    gri->dd_scale[pn] = 100.;
	    gri->dd_offset[pn] = 0;
	    gri->field_id_num[pn] = 0;
	    strcpy( gri->field_name[pn], "CV      " );
	    strncpy( gri->field_long_name[pn]
		     , "Mag. of cross corr. between VV and HV               "
		     , 40);
	    strcpy( gri->field_units[pn], "    " );
	    gri->actual_num_bins[pn] = nbins;

	    vang_ndx = pn = gri->num_fields_present++;
	    gri->dd_scale[pn] = 100.;
	    gri->dd_offset[pn] = 0;
	    gri->field_id_num[pn] = 0;
	    strcpy( gri->field_name[pn], "AV      " );
	    strncpy( gri->field_long_name[pn]
		     , "Angle of cross corr. between VV and HV             "
		     , 40);
	    strcpy( gri->field_units[pn], "deg." );
	    gri->actual_num_bins[pn] = nbins;
	}

    case DATA_POL3:

	rhohv_ndx =
	    pn = gri->num_fields_present++;
	gri->dd_scale[pn] = 1000.;
	gri->dd_offset[pn] = 0;
	gri->field_id_num[pn] = 0;
	strcpy( gri->field_name[pn], "RHOHV    " );
	strncpy( gri->field_long_name[pn]
		 , "Corr. coefficient between HH and VV                     "
		 , 40);
	strcpy( gri->field_units[pn], "       " );
	gri->actual_num_bins[pn] = nbins;

	dbmv_ndx =
	    pn = gri->num_fields_present++;
	gri->dd_scale[pn] = 100.;
	gri->dd_offset[pn] = 0;
	gri->field_id_num[pn] = 0;
	strcpy( gri->field_name[pn], "DL      " );
	strncpy( gri->field_long_name[pn]
		 , "Vertical copolar received power (VV)                   "
		 , 40);
	strcpy( gri->field_units[pn], "dBm " );
	gri->actual_num_bins[pn] = nbins;

	if( px_dataformat(dwlx) != DATA_SMHVSIM ) {
	  ldr_ndx =
	    pn = gri->num_fields_present++;
	  gri->dd_scale[pn] = 100.;
	  gri->dd_offset[pn] = 0;
	  gri->field_id_num[pn] = 0;
	  strcpy( gri->field_name[pn], "LDR     " );
	  strncpy( gri->field_long_name[pn]
		   , "Lin. depolarization ratio (H tx, V rec)                "
		   , 40);
	  strcpy( gri->field_units[pn], "dBm " );
	  gri->actual_num_bins[pn] = nbins;

	  dbmx_ndx =
	    pn = gri->num_fields_present++;
	  gri->dd_scale[pn] = 100.;
	  gri->dd_offset[pn] = 0;
	  gri->field_id_num[pn] = 0;
	  strcpy( gri->field_name[pn], "DX      " );
	  strncpy( gri->field_long_name[pn]
		   , "Cross-polar (H tx, V rec) received power                "
		   , 40);
	  strcpy( gri->field_units[pn], "dBm " );
	  gri->actual_num_bins[pn] = nbins;
	}

    case DATA_POL1:

	zdr_ndx =
	    pn = gri->num_fields_present++;
	gri->dd_scale[pn] = scale;
	gri->dd_offset[pn] = bias;
	gri->field_id_num[pn] = 0;
	strcpy( gri->field_name[pn], "ZDR      " );
	strncpy( gri->field_long_name[pn]
		 , "Differential reflectivity, HH - VV                     "
		 , 40);
	strcpy( gri->field_units[pn], "dBm     " );
	gri->actual_num_bins[pn] = nbins;

	phi_ndx =
	    pn = gri->num_fields_present++;
	gri->dd_scale[pn] = scale;
	gri->dd_offset[pn] = bias;
	gri->field_id_num[pn] = 0;
	strcpy( gri->field_name[pn], "PHI      " );
	strncpy( gri->field_long_name[pn]
		 , "Diff. propagation phase btwn HH and VV                        "
		 , 40);
	strcpy( gri->field_units[pn], "deg.     " );
	gri->actual_num_bins[pn] = nbins;
	break;

    default:
	break;
    }
}

/* c------------------------------------------------------------------------ */

static void productsx (void)
{
   int mark;

   switch(px_dataformat(dwlx))
     {
      case DATA_ABPDATA:
	simplepp2();
	/*
	simplepp();
	newsimplepp();
	 */
	break;

      case DATA_SIMPLEPP16:
	newsimplepp();
	break;

      case DATA_DUALPPFLOAT:
	dualprtfloat();
	break;

      default:
	mark = 0;
	fprintf(stderr, "productsx() is not implemented for PIRAQX format %d!\n",
		px_dataformat(dwlx));
	break;
     }
   if (px_rev(dwlx) == 1) {
     dwlx->h_rconst = h_rconst;
     dwlx->v_rconst = v_rconst;
   }
}
# define LIGHT_SPEED SPEED_OF_LIGHT

/* c------------------------------------------------------------------------ */

// create 6 scaled products from the simplepp moments
static void simplepp(void)
{
   unsigned int  i;
   float        *aptr,*pptr;
   double       a,b,p,cp,pcorrect;
   double       noise,velconst;
   double       dbm,widthconst,range,rconst,width,r12;

   short *velp=gri->scaled_data[0];
   short *dbmp=gri->scaled_data[1];
   short *ncpp=gri->scaled_data[2];
   short *swp=gri->scaled_data[3];
   short *dbzp=gri->scaled_data[4];
   short *dbzcp=gri->scaled_data[5];
   float f, scale=100., bias=0;

   h_rconst = rconst =
     px_rconst(dwlx) - 20.0 * log10(px_xmit_pulsewidth(dwlx) / px_rcvr_pulsewidth(dwlx));
   noise = (px_noise_power(dwlx) > -10.0) ? 0.0 : 0.0;
   velconst = LIGHT_SPEED / (2.0 * px_frequency(dwlx) * 2.0 * M_PI * px_prt(dwlx)[0]);
   pcorrect = px_data_sys_sat(dwlx)
	    - 20.0 * log10(0x1000000 * px_rcvr_pulsewidth(dwlx) / 1.25E-7)
	    - 10.0 * log10((double)px_hits(dwlx))
	    - px_receiver_gain(dwlx);
   widthconst = (LIGHT_SPEED / px_frequency(dwlx)) / px_prt(dwlx)[0] / (2.0 * sqrt(2.0) * M_PI);

   if(0)  velconst = -velconst;   /* // fix later for velsign  */
     /*  */
   aptr = (float *)pui->raw_data;
   range = 0.0;

   for(i=0; i<px_gates(dwlx); i++)
      {
      a = *aptr++;
      b = *aptr++;
      p = *aptr++;

      if(i)     range = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * px_rcvr_pulsewidth(dwlx));

      /* // compute floating point, scaled, scientific products  */
      f = velconst * atan2(b,a); /*  // velocity in m/s  */
      *velp++ = (short)DD_SCALE(f, scale, bias);

      f = dbm = 10.0 * log10(fabs(p)) + pcorrect;      /* // power in dBm  */
      *dbmp++ = (short)DD_SCALE(f, scale, bias);

      f = (cp = sqrt(r12 = a*a+b*b))/p; /* // NCP no units  */
      *ncpp++ = (short)DD_SCALE(f, scale, bias);

      if((width = log(fabs((p-noise)/cp))) < 0.0) width = 0.0001;
      f = sqrt(width) * widthconst;  /* // s.w. in m/s  */
      *swp++ = (short)DD_SCALE(f, scale, bias);

      f = dbm + rconst + range; /* // in dBZ  */
      *dbzp++ = (short)DD_SCALE(f, scale, bias);

      f = 10.0 * log10(fabs(cp)) + pcorrect + rconst + range; /* // in dBZ  */
      *dbzcp++ = (short)DD_SCALE(f, scale, bias);

   }
   /*
      if (numProducts == 6) {continue;}
      *(pptr +  6) = 0.0;
      *(pptr +  7) = 0.0;
      *(pptr +  8) = 0.0;
      *(pptr +  9) = 0.0;
      *(pptr + 10) = 0.0;
      *(pptr + 11) = 0.0;
      *(pptr + 12) = 0.0;
      *(pptr + 13) = 0.0;
      *(pptr + 14) = 0.0;
      *(pptr + 15) = 0.0;
      }
   return px_gates(dwlx) *  numProducts * sizeof(float) ;
    */
}
/* c------------------------------------------------------------------------ */

// create 6 scaled products from the simplepp moments
int 
simplepp2 (void)
   {
   unsigned int  i;
   float        *aptr,*pptr;
   double       a,b,p,cp,pcorrect;
   double       noise,velconst;
   double       dbm,widthconst,range,rconst,width,r12;
   int numProducts = 6;

   short *velp=gri->scaled_data[0];
   short *dbmp=gri->scaled_data[1];
   short *ncpp=gri->scaled_data[2];
   short *swp=gri->scaled_data[3];
   short *dbzp=gri->scaled_data[4];
   short *dbzcp=gri->scaled_data[5];
   float f, scale=100., bias=0;

   h_rconst = rconst =
     px_rconst(dwlx) - 20.0 * log10(px_xmit_pulsewidth(dwlx) / px_rcvr_pulsewidth(dwlx));
   noise = (px_noise_power(dwlx) > -10.0) ? 0.0 : 0.0;
   velconst = LIGHT_SPEED / (2.0 * px_frequency(dwlx) * 2.0 * M_PI * px_prt(dwlx)[0]);
   pcorrect = px_data_sys_sat(dwlx) - px_receiver_gain(dwlx);
   widthconst = (LIGHT_SPEED / px_frequency(dwlx)) / px_prt(dwlx)[0] / (2.0 * sqrt(2.0) * M_PI);

   if(0)  velconst = -velconst;   // fix later for velsign

   aptr = (float *)pui->raw_data;
   range = 0.0;

   for(i=0; i<px_gates(dwlx); i++,pptr += numProducts) // 6 (was 16) products
      {
      a = *aptr++;
      b = *aptr++;
      p = *aptr++;

      if(i)     range = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * px_rcvr_pulsewidth(dwlx));

      // compute floating point, scaled, scientific products
      f = velconst * atan2(b,a);    /*  // velocity in m/s  */
      *velp++ = (short)DD_SCALE(f, scale, bias);

      f = dbm = 10.0 * log10(fabs(p)) + pcorrect; /* // power in dBm  */
      *dbmp++ = (short)DD_SCALE(f, scale, bias);

      f = (cp = sqrt(r12 = a*a+b*b))/p;   /* // NCP no units  */
      *ncpp++ = (short)DD_SCALE(f, scale, bias);

      if((width = log(fabs((p-noise)/cp))) < 0.0) width = 0.0001;
      f = sqrt(width) * widthconst;  /* // s.w. in m/s  */
      *swp++ = (short)DD_SCALE(f, scale, bias);

      f = dbm + rconst + range;     /* // in dBZ  */
      *dbzp++ = (short)DD_SCALE(f, scale, bias);

      f = 10.0 * log10(fabs(cp)) + pcorrect + rconst + range;  /* // in dBZ  */
      *dbzcp++ = (short)DD_SCALE(f, scale, bias);
     }
   /*
   return px_gates(dwlx) *  numProducts * sizeof(float) ;
    */
   }


/* c------------------------------------------------------------------------ */
/*
// compute products from encoded A,B,P
// We use a 16 bit format in S-Pol and the DOWs. A, B, P
// are represented by two byte quantities. A and B are
// the two cartesian components of a complex number. In
// the 16 bit format, these this complex number is
// described in polar form. Therefore the three values are:

// 1) Log magnitude of A + jB  (0.004 dB/cnt)
// 2) Angle of A + jB (65536 partitions of a circle)
// 3) Log magnitude of P (0.004 dB/cnt)
 */

/* c------------------------------------------------------------------------ */
size_t newsimplepp()
   {
   unsigned int  i;
   short        *aptr;
   double       cp,v,p,pcorrect;
   double       velconst,dbm,widthconst,range,rconst,scale2db,scale2ln;

   short *velp=gri->scaled_data[0];
   short *dbmp=gri->scaled_data[1];
   short *ncpp=gri->scaled_data[2];
   short *swp=gri->scaled_data[3];
   short *dbzp=gri->scaled_data[4];
   short *dbzcp=gri->scaled_data[5];
   float f, scale=100., bias=0;

   scale2ln = 0.004 * log(10.0) / 10.0;  // from counts to natural log
   scale2db = 0.004 / scale2ln;         // from natural log to 10log10()
# define LIGHT_SPEED SPEED_OF_LIGHT
   velconst = LIGHT_SPEED / (2.0 * px_frequency(dwlx) * 2.0 * fabs(px_prt(dwlx)[0]) * 32768.0);
   h_rconst = rconst =
     px_rconst(dwlx) - 20.0 * log10(px_xmit_pulsewidth(dwlx) / px_rcvr_pulsewidth(dwlx));
   pcorrect = px_data_sys_sat(dwlx)
	    - 20.0 * log10(0x10000)
	    - px_receiver_gain(dwlx);
   // NOTE: 0x10000 is just the standard offset for all systems
   widthconst = (LIGHT_SPEED / px_frequency(dwlx)) / px_prt(dwlx)[0] / (2.0 * sqrt(2.0) * M_PI);

 # ifdef obsolete
   aptr = (short *)dwell->abp;
# endif

   aptr = (short *)pui->raw_data;
   range = 0.0;

   for(i=0; i<px_gates(dwlx); i++) {
      if(hostIsLittleEndian()) {
	 cp = (*aptr++);	/* // 0.004 dB / bit */
	 v = (*aptr++);	/* // nyquist = 65536 = +/- 32768 */
	 p = (*aptr++);	/* // 0.004 dB / bit  */
      }
      else {
//Jul 26, 2011 start
/*	 cp = PX2(*aptr++);
	 v = PX2(*aptr++);
	 p = PX2(*aptr++);
*/
	 cp = PX2(*(twob*)aptr++);
	 v = PX2(*(twob*)aptr++);
	 p = PX2(*(twob*)aptr++);
//Jul 26, 2011 end	 
      }
      if(i) range = 20.0 * log10(i * 0.0005 * LIGHT_SPEED * px_rcvr_pulsewidth(dwlx));

      // compute floating point, scaled, scientific products
	f = velconst * v;                 // velocity in m/s
	  *velp++ = (short)DD_SCALE(f, scale, bias);
      f = dbm = 0.004 * p + pcorrect;      // power in dBm
	*dbmp++ = (short)DD_SCALE(f, scale, bias);
      f = exp(scale2ln*(cp - p));                // NCP no units
	*ncpp++ = (short)DD_SCALE(f, scale, bias);
      f = sqrt(scale2ln * fabs(p-cp)) * widthconst;  // s.w. in m/s
	*swp++ = (short)DD_SCALE(f, scale, bias);
      f = dbm + rconst + range;                          // in dBZ
	*dbzp++ = (short)DD_SCALE(f, scale, bias);
      f = 0.004 * cp + pcorrect + rconst + range;  // Coherent DBZ (in dBZ)
	*dbzcp++ = (short)DD_SCALE(f, scale, bias);

      // pptr += 10;
   }
   return px_gates(dwlx) *  6 * sizeof(float) ;
   }
/* c------------------------------------------------------------------------ */

void xnewsimplepp (void)
{
   /* 16-bit integers instead of floats
    */
    int mark;
    int  i;
    short        *aptr, itemp;
    double       cp,v,p,pcorrect;
    double       logstuff,noise,velconst;
    double       dbm,widthconst,range,rconst,width;
    double       scale2ln,scale2db;
    double       pcorr_const;

    short *velp=gri->scaled_data[0];
    short *dbmp=gri->scaled_data[1];
    short *ncpp=gri->scaled_data[2];
    short *swp=gri->scaled_data[3];
    short *dbzp=gri->scaled_data[4];
    short *dbzcp=gri->scaled_data[5];
    float f, scale=100., bias=0;
    /*  */

   scale2ln = 0.004 * log(10.0) / 10.0;  // from counts to natural log
   scale2db = 0.004 / scale2ln;         // from natural log to 10log10()
   velconst = SPEED_OF_LIGHT / (2.0 * px_frequency(dwlx) * 2.0 * fabs(px_prt(dwlx)[0]) * 32768.0);
   h_rconst = rconst =
     px_rconst(dwlx) - 20.0 * log10(px_xmit_pulsewidth(dwlx) / px_rcvr_pulsewidth(dwlx));
   pcorrect = px_data_sys_sat(dwlx)
	    - 20.0 * log10(0x10000)
	    - px_receiver_gain(dwlx);

    /* // NOTE: 0x10000 is just the standard offset for all systems  */
   widthconst = (SPEED_OF_LIGHT / px_frequency(dwlx)) / px_prt(dwlx)[0] / (2.0 * sqrt(2.0) * M_PI);

# ifdef obsolete
    aptr = dwell->abp;
# endif

    aptr = (short *)pui->raw_data;
    range = 0.0;

    for(i=0; i < px_gates(dwlx); i++) {
       if(hostIsLittleEndian()) {
	  cp = (*aptr++);	/* // 0.004 dB / bit */
	  v = (*aptr++);	/* // nyquist = 65536 = +/- 32768 */
	  p = (*aptr++);	/* // 0.004 dB / bit  */
       }
       else {
//Jul 26, 2011 start
/*	  cp = PX2(*aptr++);
	  v = PX2(*aptr++);
	  p = PX2(*aptr++);
*/
	  cp = PX2(*(twob *)aptr++);
	  v = PX2(*(twob *)aptr++);
	  p = PX2(*(twob *)aptr++);
//Jul 26, 2011 end	  
       }
       if(i) range = 20.0 * log10(i * 0.0005 * SPEED_OF_LIGHT
				  * px_rcvr_pulsewidth(dwlx));

	/* compute floating point, scaled, scientific products
	 */
	f = velconst * v; /* velocity in m/s */
	*velp++ = (short)DD_SCALE(f, scale, bias);

	f = dbm = 0.004 * p + pcorrect; /* power in dBm */
	*dbmp++ = (short)DD_SCALE(f, scale, bias);

	f = exp(scale2ln * (cp - p)); /* NCP no units */
	*ncpp++ = (short)DD_SCALE(f, scale, bias);

        f = sqrt( scale2ln * fabs( cp - p )) * widthconst; /* s.w. in m/s */
	*swp++ = (short)DD_SCALE(f, scale, bias);

	f = dbm + rconst + range; /* in dBZ */
	*dbzp++ = (short)DD_SCALE(f, scale, bias);

	f = 0.004 * cp + pcorrect + rconst + range; /* Coherent DBZ in dBZ */
	*dbzcp++ = (short)DD_SCALE(f, scale, bias);
    }
}

/* c------------------------------------------------------------------------ */
void 
dualprtfloat (void) {
    int  i;
    float        *abpptr_prt1, *abpptr_prt2;
    double       a1, b1, p1, a2, b2, p2, biga, bigb;
    double       cp1, cp2, cp, vel1, vel2, vel, p, ncorrect, pcorrect;
    double       ncp;
    double       velconst, velconst1, velconst2, dbm;
    double       widthconst1, widthconst2, widthconst, range, rconst;

    short *velp=gri->scaled_data[0];
    short *dbmp=gri->scaled_data[1];
    short *ncpp=gri->scaled_data[2];
    short *swp=gri->scaled_data[3];
    short *dbzp=gri->scaled_data[4];
    short *dbzcp=gri->scaled_data[5];
    short *v1p=gri->scaled_data[6];
    short *v2p=gri->scaled_data[7];
    short *w1p=gri->scaled_data[8];
    short *w2p=gri->scaled_data[9];
    float f, scale=100., bias=0;

    velconst = SPEED_OF_LIGHT /
	    (2.0 * px_frequency(dwlx) * 2.0 * M_PI * fabs(px_prt(dwlx)[0] - px_prt(dwlx)[1]));
    velconst1 = SPEED_OF_LIGHT /
	    (2.0 * px_frequency(dwlx) * 2.0 * M_PI * px_prt(dwlx)[0]);
    velconst2 = SPEED_OF_LIGHT /
	    (2.0 * px_frequency(dwlx) * 2.0 * M_PI * px_prt(dwlx)[1]);
    /*
     * Radar constant.  NOTE: 0x10000 is just the standard
     * offset for all systems.
     */
    rconst = px_rconst(dwlx) - 20.0 * log10(px_xmit_pulsewidth(dwlx) /
	    px_rcvr_pulsewidth(dwlx));
    /*
     * This correction factor comes from Mitch. I don't know its derivation...
     */
#define ADFULLSCALE	8192
    ncorrect = px_data_sys_sat(dwlx) -														// Correct
	    20.0 * log10((double)((ADFULLSCALE << 14)/65536)*(px_rcvr_pulsewidth(dwlx)*1.0e7 + 0.5)) -
	    20.0 * log10((double)0x10000) - px_receiver_gain(dwlx);
    /*
     * Power correction
     */
    pcorrect = ncorrect - 10.0*log10((double)px_hits(dwlx)/2.0);
    /*
     * Combined spectrum width constant.
     */
    widthconst = (SPEED_OF_LIGHT / px_frequency(dwlx)) /
	     (0.5 * (px_prt(dwlx)[0] + px_prt(dwlx)[1])) /
	     (2.0 * sqrtf(2.0) * M_PI);
    widthconst1 = (SPEED_OF_LIGHT / px_frequency(dwlx)) /
	     px_prt(dwlx)[0] / (2.0 * sqrtf(2.0) * M_PI);
    widthconst2 = (SPEED_OF_LIGHT / px_frequency(dwlx)) /
	     px_prt(dwlx)[1] / (2.0 * sqrtf(2.0) * M_PI);

    abpptr_prt1 = (float *)pui->raw_data;
    abpptr_prt2 = (float *)pui->raw_data +  3 * px_gates(dwlx);
    range = 0.0;

    for(i = 0; i < px_gates(dwlx); i++) {
	if( hostIsLittleEndian() ) {
	    a1 = *abpptr_prt1++;
	    b1 = *abpptr_prt1++;
	    p1 = *abpptr_prt1++;
	    a2 = *abpptr_prt2++;
	    b2 = *abpptr_prt2++;
	    p2 = *abpptr_prt2++;
	}
	else {
//Jul 26, 2011 start
/*	    a1 = PX4F(*abpptr_prt1++);
	    b1 = PX4F(*abpptr_prt1++);
	    p1 = PX4F(*abpptr_prt1++);
	    a2 = PX4F(*abpptr_prt2++);
	    b2 = PX4F(*abpptr_prt2++);
	    p2 = PX4F(*abpptr_prt2++);
*/
	    a1 = PX4F(*(fourB *)abpptr_prt1++);
	    b1 = PX4F(*(fourB *)abpptr_prt1++);
	    p1 = PX4F(*(fourB *)abpptr_prt1++);
	    a2 = PX4F(*(fourB *)abpptr_prt2++);
	    b2 = PX4F(*(fourB *)abpptr_prt2++);
	    p2 = PX4F(*(fourB *)abpptr_prt2++);
//Jul 26, 2011 end	    
	}

	vel1 = velconst1 * atan2(b1, a1);
	vel2 = velconst2 * atan2(b2, a2);

	// Unfold velocity.  This is just a complex multiply:
	//               ___________
	// (a1 + b1*i) * (a2 + b2*i) = (a1 + b1*i) * (a2 - b2*i)
	//                           = a1*a2 + b1*b2 + (-a1*b2 + a2*b1)i
//	biga = a1 * a2 + b1 * b2;
//	bigb = a2 * b1 - a1 * b2;

	// 4/28/09 cb: Looks like we really need to conjugate the first
	// component and not the second to get the right sign on the velocity:
	//   ___________
	//   (a1 + b1*i) * (a2 + b2*i) = (a1*a2 + b1*b2) + (a1*b2 - a2*b1)i
	biga = a1 * a2 + b1 * b2;
	bigb = a1 * b2 - a2 * b1;
	vel = velconst * atan2(bigb, biga);        /* velocity in m/s */

	cp1 = hypot(a1, b1);
	cp2 = hypot(a2, b2);
	cp = 0.5 * (cp1 + cp2);
	p = 0.5 * (p1 + p2);
	ncp = cp / p;

	range = (i == 0) ?
	    0.0 : 20.0 * log10(i * 0.0005 * SPEED_OF_LIGHT * px_rcvr_pulsewidth(dwlx));

	/* compute floating point, scaled, scientific products */
	*velp++ = (short)DD_SCALE(vel, scale, bias);

	dbm = 10.0 * log10(p) + pcorrect;      /* power in dBm */
	*dbmp++ = (short)DD_SCALE(dbm, scale, bias);

	*ncpp++ = (short)DD_SCALE(ncp, scale, bias); /* NCP no units */

	f = sqrtf(log(p / cp)) * widthconst;       /* combined s.w. */
	*swp++ = (short)DD_SCALE(f, scale, bias);

	f = dbm + rconst + range;            /* in dBZ */
	*dbzp++ = (short)DD_SCALE(f, scale, bias);

	f = 10.0 * log10(cp) + pcorrect + rconst + range;  /* in dBZ */
	*dbzcp++ = (short)DD_SCALE(f, scale, bias);

	*v1p++ = (short)DD_SCALE(vel1, scale, bias);

	*v2p++ = (short)DD_SCALE(vel2, scale, bias);

	f = sqrtf(log(p1 / cp1)) * widthconst1;
	*w1p++ = (short)DD_SCALE(f, scale, bias);

	f = sqrtf(log(p2 / cp2)) * widthconst1;
	*w2p++ = (short)DD_SCALE(f, scale, bias);
    }
}
/* c------------------------------------------------------------------------ */

long long 
pc_swap8 (		/* swap real*8 */
    char *ov
)
{
    union {
	long long newval;
	unsigned char nv[8];
    }u;

    u.nv[7] = ov[0];
    u.nv[6] = ov[1];
    u.nv[5] = ov[2];
    u.nv[4] = ov[3];
    u.nv[3] = ov[4];
    u.nv[2] = ov[5];
    u.nv[1] = ov[6];
    u.nv[0] = ov[7];

    return(u.newval);
}
/* c------------------------------------------------------------------------ */

void 
piraqx_positioning (void)
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
    slm = solo_malloc_list_mgmt(99);


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
13 = Time correction\n\
14 = Sweep time tolerance\n\
15 = Rotation angle tolerance\n\
Option = "
	   );
    nn = getreply(str, sizeof(str));
    if( cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 16 ) {
	if(ival == -2) exit(1);
	printf( "\nIllegal Option!\n" );
	goto menu2;
    }
    pui->new_sweep = NO;
    pui->new_vol = NO;

    if(ival == -1) {
	solo_unmalloc_list_mgmt(slm);
	return;
    }
    else if(ival < 0)
	  exit(0);
    else if(ival == 0) {
	printf("Type number of rays to skip:");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > 20 * K64) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	if(ival > 0) {
	    for(ii=0; ii < ival; ii++) {
		pui->new_sweep = NO;
		pui->new_vol = NO;
		if((jj=piraqx_next_ray()) < 0)
		      break;
	    }
	}
	else if(ival < 0) {
	    /* figure out the number of rays we can really back up
	     */
	    nn = -ival > irq->packet_count ? irq->packet_count-1 : -ival;
	    dp0 = dp = dd_return_current_packet(irq);
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
		kk = ((dp0->ib->b_num - dp->ib->b_num + irq->que_count)
		      % irq->que_count) +1;
		dd_skip_recs(irq, BACKWARD, kk);
		irq->top->at = dp->at;
		irq->top->bytes_left = dp->bytes_left;
	    }
	    if(ii < -ival) {
		/* did not achive goal */
		printf("Unable to back up %d rays. Backed up %d rays\n"
		       , -ival, ii);
		printf("Use record positioning to back up further\n");
	    }
	    piraqx_backwards_reset(0);
	}
	piraqx_reset();
	piraqx_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 1) {
	if((nn = piraqx_inventory(irq)) == -2)
	      exit(0);
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
	if(direction == BACKWARD)
	      piraqx_backwards_reset(0);
	dd_stats_reset();
	piraqx_reset();
	piraqx_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 3) {
	dd_rewind_dev(irq);
	dd_stats_reset();
	piraqx_backwards_reset(0);
	piraqx_reset();
	piraqx_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 4) {
	if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
	    printf("\n");
	    if(view_char < 0) {
		view_char = view_char ? -view_char -2 : 0;
		Xctypeu16((unsigned char *)irq->top->buf, view_char, view_char_count);  //Jul 26, 2011
	    }
	    else {
		ctypeu16((unsigned char *)irq->top->buf, view_char, view_char_count);  //Jul 26, 2011
	    }
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
    else if(ival == 7) {
	printf("Type record skip # or hit <return> to read next rec: ");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > 20 * K64) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	direction = ival >= 0 ? FORWARD : BACKWARD;
	dd_skip_recs(irq, direction, ival > 0 ? ival : -ival);
	if(direction == BACKWARD)
	      piraqx_backwards_reset(0);
	piraqx_reset();
	piraqx_next_ray();
	nn = irq->top->sizeof_read;
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
	    if((nn = piraqx_next_ray()) <= 0 || gri->time >= dtarget ||
	       dd_control_c()) {
		break;
	    }
	    if(!(mm % 1000)) {
		gri_interest(gri, 1, preamble, postamble);
		printf("%s\n", dd_stats_sprintf());
	    }
	    pui->new_vol = pui->new_sweep = NO;
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
	    if((nn = piraqx_next_ray()) <= 0 || in_sector(gri->fixed, fx1, fx2)
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
	piraqx_print_headers(slm);
    }
    else if(ival == 12) {
	piraqx_dump_this_ray(slm);
    }
    else if(ival == 13) {
	printf("Current time correction: %.3f secs. New correction: "
	       , pui->time_correction);
	nn = getreply(str, sizeof(str));
	if((mm=cdcode(str, nn, &ival, &val)) == 1 && !ival) {
	    /* assume they have hit a <return>
	     */
	}
	else {
	    pui->time_correction = val;
	}
    }
# ifdef obsolete
    else if(ival == 14) {
	printf("Current sweep time tolerance: %.3f secs. New tolerance: "
	       , sweep_time_tol);
	nn = getreply(str, sizeof(str));
	if((mm=cdcode(str, nn, &ival, &val)) == 1 && !ival) {
	    /* assume they have hit a <return>
	     */
	}
	else {
	    sweep_time_tol = val;
	}
    }
    else if(ival == 15) {
	printf("Current rot. ang. tolerance: %.3f deg. New tolerance: "
	       , rotang_tol);
	nn = getreply(str, sizeof(str));
	if((mm=cdcode(str, nn, &ival, &val)) == 1 && !ival) {
	    /* assume they have hit a <return>
	     */
	}
	else {
	    rotang_tol = val;
	}
    }
# endif

    preamble[0] = '\0';

    goto menu2;
}
/* c------------------------------------------------------------------------ */

int 
piraqx_inventory (struct input_read_info *irq)
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
	    pui->new_sweep = NO;
	    pui->new_vol = NO;
	    if((nn=piraqx_next_ray()) < 1) {
		break;
	    }
	    sprintf(preamble, "%2d", irq->top->b_num);
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
piraqx_dump_this_ray (struct solo_list_mgmt *slm)
{
    int gg, ii, nf, nn, fw=10;
    char *aa, *bb, *cc, str[128], tmp_str[16];
    float unscale[8], bias[8], *fp=NULL;

    aa = str;
    bb = tmp_str;
    productsx();
    solo_reset_list_entries(slm);

    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of data fields");
    solo_add_list_entry(slm, aa);

    sprintf(aa, "name ");
    nf = gri->num_fields_present > 8 ? 8 : gri->num_fields_present;
    cc = aa+strlen(aa);

    for(ii=0; ii < nf; ii++) {
	str_terminate(bb, gri->field_name[ii], 8);
	memset(cc, ' ', fw);
	strncpy(cc+fw-strlen(bb), bb, strlen(bb));
	cc += fw;
	unscale[ii] = gri->dd_scale[ii] ? 1./gri->dd_scale[ii] : 1.;
	bias[ii] = gri->dd_offset[ii];
    }
    *cc = '\0';
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
piraqx_print_headers (struct solo_list_mgmt *slm)
{
    solo_reset_list_entries(slm);
    piraqx_print_hdr(slm);
    slm_print_list(slm);
}
/* c------------------------------------------------------------------------ */

void 
piraqx_print_hdr (struct solo_list_mgmt *slm)
{
    int ii, jj, kk, nn;
    char *aa, str[256], tmp[256];

    DD_TIME dts;
    double d;
    PIRAQX_REV1 *dwlx_r1 = (PIRAQX_REV1*)dwlx;
    PIRAQX_REV2 *dwlx_r2 = (PIRAQX_REV2*)dwlx;


# ifdef obsolete
    sprintf(aa, " %e", px_(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, " %d", px_(dwlx)  );
    solo_add_list_entry(slm, aa);
# endif

    aa = str;

    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of piraqx ray header");
    solo_add_list_entry(slm, aa);

    sprintf(aa, "desc[4]         %s", str_terminate(tmp,px_desc(dwlx),4));
    solo_add_list_entry(slm, aa);
    sprintf(aa, "recordlen       %d", px_recordlen(dwlx)      );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "channel         %d", px_channel(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "rev             %d", px_rev(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "one             %d", px_one(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "byte_offset_to_data %d", px_byte_offset_to_data(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "dataformat      %d", px_dataformat(dwlx)  );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "typeof_compression %d", px_typeof_compression(dwlx)  );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "pulse_num       %X", px_pulse_num(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "beam_num        %X", px_beam_num(dwlx)  );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "gates           %d", px_gates(dwlx)          );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "start_gate      %d", px_start_gate(dwlx)          );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "hits            %d", px_hits(dwlx)           );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ctrlflags       %d", px_ctrlflags(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "bytespergate    %d", px_bytespergate(dwlx)  );
    solo_add_list_entry(slm, aa);


    sprintf(aa, "rcvr_pulsewidth %e", px_rcvr_pulsewidth(dwlx));
    solo_add_list_entry(slm, aa);
    sprintf(aa, "prt[4]          %e %e %e %e", px_prt(dwlx)[0] , px_prt(dwlx)[1]
	    , px_prt(dwlx)[2] , px_prt(dwlx)[3]            );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "meters_to_first_gate %e", px_meters_to_first_gate(dwlx) );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "num_segments    %d", px_num_segments(dwlx)  );
    solo_add_list_entry(slm, aa);

    strcpy (aa, "gate_spacing_meters[]");
    for (ii=0; ii < px_num_segments(dwlx); ii++) {
       sprintf (aa+strlen(aa), " %.2f", px_gate_spacing_meters(dwlx)[ii]);
    }
    solo_add_list_entry(slm, aa);

    strcpy (aa, "gates_in_segment[]");
    for (ii=0; ii < px_num_segments(dwlx); ii++) {
       sprintf (aa+strlen(aa), "%4d ", px_gates_in_segment(dwlx)[ii]);
    }
    solo_add_list_entry(slm, aa);

    nn = PX_NUM_CLUTTER_REGIONS;

    strcpy (aa, "clutter_start[]");
    for (ii=0; ii < nn; ii++) {
       sprintf (aa+strlen(aa), "%4d ", px_clutter_start(dwlx)[ii]);
    }
    solo_add_list_entry(slm, aa);

    strcpy (aa, "clutter_end[]  ");
    for (ii=0; ii < nn; ii++) {
       sprintf (aa+strlen(aa), "%4d ", px_clutter_end(dwlx)[ii]);
    }
    solo_add_list_entry(slm, aa);

    strcpy (aa, "clutter_type[] ");
    for (ii=0; ii < nn; ii++) {
       sprintf (aa+strlen(aa), "%4d ", px_clutter_type(dwlx)[ii]);
    }
    solo_add_list_entry(slm, aa);

    dts.time_stamp = pui->time;
    sprintf(aa, "secs            %X  %s", px_secs(dwlx)
	    , dts_print(d_unstamp_time(&dts))           );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "nanosecs        %d", px_nanosecs(dwlx)      );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "az              %e", px_az(dwlx)             );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "el              %e", px_el(dwlx)             );

    solo_add_list_entry(slm, aa);
    sprintf(aa, "radar_longitude %e", px_radar_longitude(dwlx));
    solo_add_list_entry(slm, aa);
    sprintf(aa, "radar_latitude  %e", px_radar_latitude(dwlx));
    solo_add_list_entry(slm, aa);
    sprintf(aa, "radar_altitude  %e", px_radar_altitude(dwlx) );
    solo_add_list_entry(slm, aa);
    strcpy (aa, "gps_datum         ");
    str_terminate (aa+strlen(aa), px_gps_datum(dwlx), PX_MAX_GPS_DATUM);
    solo_add_list_entry(slm, aa);

    sprintf(aa, "ts_start_gate   %d", px_ts_start_gate(dwlx)     );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ts_end_gate     %d", px_ts_end_gate(dwlx)     );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "ew_velocity     %e", px_ew_velocity(dwlx)    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ns_velocity     %e", px_ns_velocity(dwlx)    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "vert_velocity   %e", px_vert_velocity(dwlx)  );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "fxd_angle       %e", px_fxd_angle(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "true_scan_rate  %e", px_true_scan_rate(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "scan_type       %d", px_scan_type(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "scan_num        %d", px_scan_num(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "vol_num         %d", px_vol_num(dwlx)  );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "transition      %d", px_transition(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "xmit_power      %f", px_xmit_power(dwlx)  );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "yaw             %f", px_yaw(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "pitch           %f", px_pitch(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "roll            %f", px_roll(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "track           %f", px_track(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "gate0mag        %f", px_gate0mag(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "dacv            %f", px_dacv(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "packetflag      %d", px_packetflag(dwlx)  );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "year            %d", px_year(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "julian_day      %d", px_julian_day(dwlx)  );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "radar_name[16]  %s", str_terminate(tmp,px_radar_name(dwlx)
						    ,PX_MAX_RADAR_NAME));
    solo_add_list_entry(slm, aa);
    sprintf(aa, "channel_name[16] %s", str_terminate(tmp,px_channel_name(dwlx)
						    ,PX_MAX_CHANNEL_NAME));
    solo_add_list_entry(slm, aa);
    sprintf(aa, "project_name[16] %s", str_terminate(tmp,px_project_name(dwlx)
						    ,PX_MAX_PROJECT_NAME));
    solo_add_list_entry(slm, aa);
    sprintf(aa, "operator_name[16] %s", str_terminate(tmp,px_operator_name(dwlx)
						    ,PX_MAX_OPERATOR_NAME));
    solo_add_list_entry(slm, aa);
    sprintf(aa, "site_name[16]   %s", str_terminate(tmp,px_site_name(dwlx)
						    ,PX_MAX_SITE_NAME));
    solo_add_list_entry(slm, aa);

    sprintf(aa, "polarization    %d", px_polarization(dwlx)    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "test_pulse_pwr  %e", px_test_pulse_pwr(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "test_pulse_frq  %e", px_test_pulse_frq(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "frequency       %e", px_frequency(dwlx)       );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "noise_figure    %e", px_noise_figure(dwlx)    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "noise_power     %e", px_noise_power(dwlx)     );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "receiver_gain   %e", px_receiver_gain(dwlx)   );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "E_plane_angle   %e", px_E_plane_angle(dwlx)   );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "H_plane_angle   %e", px_H_plane_angle(dwlx)   );
    solo_add_list_entry(slm, aa);


    sprintf(aa, "data_sys_sat    %e", px_data_sys_sat(dwlx)    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "antenna_gain    %e", px_antenna_gain(dwlx)    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "H_beam_width %e", px_H_beam_width(dwlx) );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "V_beam_width %e", px_V_beam_width(dwlx) );

    solo_add_list_entry(slm, aa);
    sprintf(aa, "xmit_pulsewidth %e", px_xmit_pulsewidth(dwlx) );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "rconst          %e", px_rconst(dwlx)          );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "phaseoffset     %e", px_phaseoffset(dwlx)  );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "zdr_fudge_factor %e", px_zdr_fudge_factor(dwlx)  );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "mismatch_loss    %e", px_mismatch_loss(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "rcvr_const       %e", px_rcvr_const(dwlx)  );
    solo_add_list_entry(slm, aa);

    sprintf(aa, "test_pulse_rngs_km[2] %e %e"
	    , px_test_pulse_rngs_km(dwlx)[0] , px_test_pulse_rngs_km(dwlx)[1] );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "antenna_rotation_angle %e", px_antenna_rotation_angle(dwlx)  );
    solo_add_list_entry(slm, aa);

    strcpy (aa, "comment: ");
    str_terminate (aa+strlen(aa), px_comment(dwlx), PX_SZ_COMMENT);
    solo_add_list_entry(slm, aa);

    sprintf(aa, "i_norm           %e", px_i_norm(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "q_norm           %e", px_q_norm(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "i_compand        %e", px_i_compand(dwlx)  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "q_compand        %e", px_q_compand(dwlx)  );
    solo_add_list_entry(slm, aa);

    strcpy (aa, "transform_matrix[i][j][2] ");
    solo_add_list_entry(slm, aa);

    for (ii=0; ii < 2; ii++) {
       for (jj=0; jj < 2; jj++) {
	   if (px_rev(dwlx) == 1)
	   {
	     sprintf (aa, "  i:%d j:%d  %16e  %16e", ii, jj
		      , dwlx_r1->transform_matrix[ii][jj][0]
		      , dwlx_r1->transform_matrix[ii][jj][1]);
	   } else {
	     sprintf (aa, "  i:%d j:%d  %16e  %16e", ii, jj
		      , dwlx_r2->transform_matrix[ii][jj][0]
		      , dwlx_r2->transform_matrix[ii][jj][1]);
	   }
	   solo_add_list_entry(slm, aa);
       }
    }
    sprintf(aa, "stokes[4] %e %e %e %e", px_stokes(dwlx)[0] , px_stokes(dwlx)[1]
	    , px_stokes(dwlx)[2] , px_stokes(dwlx)[3]  );
    solo_add_list_entry(slm, aa);
    strcpy (aa, "spare[20]");
    solo_add_list_entry(slm, aa);


}

# ifdef notyet
/* c------------------------------------------------------------------------ */

int 
piraqx_ts_stats (void)
{
    /* for a given prt accumulate statistics and print out periodically */


    static double sumhh = 0, sumvv = 0, sumhv;
    static double sumSqhh = 0, sumSqvv = 0, sumSqhv;


    static int ray_count=0;
    static int total_hits;
    static float prev_prt = 0;

    double xx, yy;
    int ii, jj, kk, mm, nn, new_prt = NO, len, N = 0, num_fields;
    int print_stats = NO, sizeof_gate;
    float *fp, *fpx, f;
    double I, Q, mean, sdev;
    char message[256], *aa;

    switch(px_dataformat(dwlx)) {
    case DATA_POL12:
	num_fields = 12;
	break;
    case DATA_POL3:
	num_fields = 8;
	break;
    case DATA_POL1:
	num_fields = 6;
	break;
    case DATA_POLYPP:
	num_fields = 5;
	break;
    default:
	num_fields = 3;
	break;
    }

    sizeof_gate = num_fields * sizeof( short );
    len = px_recordlen(dwlx) - sizeof( LeHEADERV ) - ( px_gates(dwlx) * sizeof_gate );
    kk = len/sizeof(float);

    if( kk < px_hits(dwlx) ) {
	if( sizeof_ts_fi > 0 ) {
	    fclose( ts_fid );
	    sizeof_ts_fi = 0;
	}
	return;
    }

    if( !ray_count++ ) {
	aa = message;
	sprintf( aa, "/scr/hotlips/oye/spol/ts.txt" );
	/* open the ascii file */
	ts_fid = fopen( message, "w" );
    }

    if( fabs( (double)( px_prt(dwlx)[0] - prev_prt ) ) > .001 ) {
	new_prt = YES;
    }
    if( new_prt || (ray_count > 1 && ray_count < 22) || !( ray_count & 100 )) {
	/*
	 * print statistics
	 */
	if( N > 0 ) {
	    aa = message;
	    sprintf( aa, "RC:%d total: %d prt: %.3f\n", ray_count, N, prev_prt );

	    mean = sumhh/(float)N;
	    sdev = sqrt((double)((N*sumSqhh - sumhh*sumhh)/(N*(N-1))));
	    sprintf( aa, "hh:mean=%.3f sdev=%.3f \n", mean, sdev );

	    mean = sumvv/(float)N;
	    sdev = sqrt((double)((N*sumSqvv - sumvv*sumvv)/(N*(N-1))));
	    sprintf( aa + strlen(aa), "vv:mean=%.3f sdev=%.3f \n", mean, sdev );

	    mean = sumvv/(float)N;
	    sdev = sqrt((double)((N*sumSqvv - sumvv*sumvv)/(N*(N-1))));
	    sprintf( aa + strlen(aa), "hv:mean=%.3f sdev=%.3f \n", mean, sdev );
	}
    }

    if( new_prt ) {
	prev_prt = px_prt(dwlx)[0];
	sumhh = sumvv = sumhv = 0;
	sumSqhh = sumSqvv = sumSqhv = 0;
	N = 0;
    }

    fpx = (float *)( pui->raw_data + px_gates(dwlx) * num_fields * sizeof(short) );
    N += px_hits(dwlx);
    aa = message;
    sprintf( aa
	     , "beam: %d tsgate: %d hits: %d prt: %.4f az: %.2f el: %.2f %d \n"
	     , ray_count, px_tsgate(dwlx), px_hits(dwlx), px_prt(dwlx)[0], px_az(dwlx), px_el(dwlx)
	     , len );
    fputs( message, ts_fid );
    sizeof_ts_fi += sizeof( message );

    for( ii = 0; ii < px_hits(dwlx) >> 1; ii++ ) {

	aa = message;

	I = PX4F( *fpx++ );
	Q = PX4F( *fpx++ );
	sprintf( aa, " %4d %10.2f %10.2f ", ii, I, Q );
	xx = sqrt( I*I + Q*Q );
	sumhh += xx;
	sumSqhh += xx*xx;

	I = PX4F( *fpx++ );
	Q = PX4F( *fpx++ );
	aa += strlen(aa);
	sprintf( aa, " %10.2f %10.2f ", I, Q );
	xx = sqrt( I*I + Q*Q );
	sumvv += xx;
	sumSqvv += xx*xx;

# ifdef obsolete
	I = PX4F( *fpx++ );
	Q = PX4F( *fpx++ );
	aa += strlen(aa);
	sprintf( aa, " %10.2f %10.2f ", I, Q );
	xx = sqrt( I*I + Q*Q );
	sumhv += xx;
	sumSqhv += xx*xx;
# endif
	aa += strlen(aa);
	strcat( aa, "\n" );
	fputs( message, ts_fid );
	sizeof_ts_fi += sizeof( message );
    }
}

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */
# endif
