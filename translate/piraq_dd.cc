/* 	$Id$	 */
 
#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */


//#include <input_limits.hh>
//#include <generic_radar_info.h>
//#include <Platform.h>
//#include <errno.h>
//#include <piraq/piraqx.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dd_math.h>
#include "dd_stats.h"
#include "piraq_dd.hh"
#include "piraq.hh"
#include "dd_catalog.hh"
#include "dd_io_mgmt.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "dorade_share.hh"
#include "gneric_dd.hh"
#include "gpro_data.hh"
#include "sigm_dd.hh"
#include "stdhrd.hh"
#include "dd_crackers.hh"
 
//# include "piraq.h"

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

# define SX2(x) (sig_swap2((&x)))
# define SX4(x) (pc_swap4((&x)))
# define SX4F(x) (pc_swap4f((&x)))

# define         PIRAQ_IO_INDEX 3
# define         SWEEP_QUE_SIZE 64

/* options flags
 */
# define            WINDOWS_DOW 0x1
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

extern int hostIsLittleEndian();

static RADARV *rhdr=NULL, *xrhdr;
static DWELLV *dwl=NULL;
static HEADERV *hdr=NULL, *xhdr;

static LeRADARV *lrhdr=NULL;
static LeDWELLV *ldwl=NULL;
static LeHEADERV *lhdr=NULL;
static PIRAQX *prqx=NULL;
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
static double max_rhi_diff = 9.0;
static double min_az_diff = .3;
static double min_el_diff = .2;
static double min_fxdang_diff = .15;
static double new_vol_thr = 1.5;
static double rhi_az_tol = 1.5;
static double ppi_el_tol = .77;
# ifdef obsolete
static double sweep_time_tol=2.0;
static double rotang_tol=1.5;
# endif
static double Default_Range0 = 150;
static double Improve_rconst_corr = 0;
static int Improve_range0_offset = 6;
static char *current_file_name;
static double twoPI=2.0*M_PI;
static u_int32_t P960501=0;
static u_int32_t P970101=0;
static u_int32_t P970127=0;
static u_int32_t P970129=0;
static u_int32_t P980201=0;
static float h_rconst=0, v_rconst=0;

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
static struct radar_consts r_consts; 
static char *acmrg=NULL;

FILE * ts_fid = NULL;
int sizeof_ts_fi = 0;


static void 
do_Noise_lut12 (double noise, double scale2ln, float *lut)
{
    int ii;
    double xx;
    
    for(ii = 0 ; ii < K64; ii++ ) {
	xx = exp( (double)((short)ii * scale2ln ));
	*(lut + ii) = ( xx - noise < 0 ) ? SMALL : (float)log( xx - noise );
    }
}
/* c------------------------------------------------------------------------ */

static void 
do_Noise_lut15 (double noise, double scale2ln, float *lut)
{
    int ii;
    double xx;
    
    for(ii = 0 ; ii < K64; ii++ ) {
	xx = exp( (double)((short)ii * scale2ln ));
	*(lut + ii) = ( xx - noise < 0 ) ? (float)log((double)SMALLplus)
	    : (float)log( xx - noise );
    }
}
/* c------------------------------------------------------------------------ */

static void 
do_Noise_lut (double noise, double scale2ln)
{
    int ii;
    double xx;
    
    if( fabs((double)( noise - pui->prev_noise)) > .1 ) {
	 /* create a new noise correction lookup table  */
	if( !pui->Noise_lut ) {
	    pui->Noise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->Noise_lut, 0, K64 * sizeof(float));
	}

	for(ii = 0 ; ii < K64; ii++ ) {
	    xx = exp( (double)((short)ii * scale2ln ));
	    *(pui->Noise_lut + ii) = ( xx - noise > 0 )
		? (float)log( xx - noise ) : -10.e22;
	}
    }
    pui->prev_noise = noise;
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

int 
pui_next_block (void)
{
  /* returns a pointer to the next block of piraq data
   * which in the case of CD-ROMs may block several rays in a record
   * otherwise it corresponds to a physical record on the tape
   */
  static int count=0, bp=298, illegal_header_count=0;
  int32_t keep_offset;
  int err_count = 0;
  int ii, mm, nn, mark, eof_count=0, ugly;
  int unsigned short recordlen=0;
  int size=-1, ugly_record_count=0;
  int dwel_record, rhdr_record;
  char *aa, *bb, *cc = NULL;
  char mess[256], *last_top;
  TOP *gh;
  static TOP *last_gh;
  RADARV *rhdrx;
  HEADERV *hdrx;
  /* c...mark */


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
    gh = (TOP *)aa;
//    recordlen = hostIsLittleEndian() ? gh->recordlen : SX2(gh->recordlen);  //Jul 26, 2011
    recordlen = hostIsLittleEndian() ? gh->recordlen : SX2(*(twob*)gh->recordlen);  //Jul 26, 2011

    dwel_record = !strncmp("DWEL", aa, 4);
    rhdr_record = !strncmp("RHDR", aa, 4);
    rhdrx = (RADARV *)aa;
    hdrx = (HEADERV *)aa;

    if (!(dwel_record || rhdr_record)) {
      if((illegal_header_count++ % 3) == 0 ) {
	sprintf(mess, 
		"Illegal header id: %2x %2x %2x %2x %2x %2x rlen: %d  sizeof_read:  %d %d"
		, (int)gh->desc[0] & 0xff
		, (int)gh->desc[1] & 0xff
		, (int)gh->desc[2] & 0xff
		, (int)gh->desc[3] & 0xff
		, (int)gh->desc[4] & 0xff
		, (int)gh->desc[5] & 0xff
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

    if (dwel_record || rhdr_record) {
      last_gh = gh;
    }
    else {
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
	       , ugly_record_count, hdr->recordlen
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

void piraq_dd_conv (int interactive_mode)
{
    static int count=0, count_break=99, nok_count=0;
    int mark, nn;
    float *fp=NULL;

    if(!count) {
       piraq_ini();
       piraq_reset();
    }
    if(interactive_mode) {
	dd_intset();
	piraq_positioning();
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

	if(piraq_select_ray()) {

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
  		  products((DWELL *)dwl, rhdr, fp);

	    dd_stuff_ray();	/* pass it off to dorade code */
	}
	else {
	    if(!(nok_count++ % 1000)) {
		piraq_print_stat_line(count);
	    }
	}
	pui->new_sweep = NO;
	pui->new_vol = NO;

	if((nn = piraq_next_ray()) < 1) {
	    printf("Break on input status: %d\n", nn);
	    break;
	}
    }
    piraq_reset();
}
/* c------------------------------------------------------------------------ */

int 
piraq_isa_new_vol (void)
{
    int n_new = NO;  //Jul 26, 2011 new issue
    struct piraq_ray_que *rq = pui->ray_que;
    struct piraq_swp_que *sq = pui->swp_que;
    double d, diff;

    if(pui->options & SPOL_FLAG || pui->options & DOW_FLAG ) {
       if(rq->vol_num != rq->last->vol_num) {
	  return(YES);
       }
       return(NO);
    }
    if(sq->rcvr_pulsewidth > 0 && sq->last->rcvr_pulsewidth > 0 &&
       fabs(sq->rcvr_pulsewidth - sq->last->rcvr_pulsewidth) > 1.e-7) {
	return(YES);
    }
    if(fabs(sq->prf - sq->last->prf) > 9) {
	return(YES);
    }
    if(sq->last->scan_mode != gri->scan_mode) {
	return(YES);
    }
    if(fabs(pui->swpang_diff_sum) < min_fxdang_diff) {
	return(NO);
    }
    if(pui->vol_sweep_count < 2) {
	return(NO);
    }
    if(fabs(diff = angdiff(sq->last->swpang, gri->fixed)) >
	min_fxdang_diff) {
	if(diff * pui->swpang_diff_sum < 0) {
	    /* the antenna has reversed its normal increment */
	    return(YES);
	}
    }

    return(n_new);
}
/* c------------------------------------------------------------------------ */

int 
piraq_isa_new_sweep (void)
{
    static int ebug = 0;
    int ii, nn, n_new = NO, az_turnaround, el_turnaround;  //Jul 26, 2011 new issue
    int cond_a, cond_b, cond_c;
    int lag_rays = 2;
    double mrd, d, d2, d3, ang_tol, time_tol, dt, da;
    double daz, avgDaz=0, ddaz, dddaz, del, avgDel=0, ddel, halfMad;
    double sumDaz=0, sumDel=0, short_sum_az_diffs, short_sum_el_diffs;
    double short_avg_az, short_avg_az_diff, short_avg_el, short_avg_el_diff;
    float rotang;
    struct piraq_ray_que *rq;
    DD_TIME dts;
    /* c...mark */

    if( pui->options & BIST_FLAG ) {
	if( pui->sweep_ray_num > 999 ) {
	    return( YES );
	}
	else
	  { return( NO ); }
    }
    if(pui->options & SPOL_FLAG || pui->options & DOW_FLAG) {
	rq = pui->ray_que;

	if(hdr->transition) {
	    pui->transition_count++;
	}
	else {
	    pui->transition_count = 0;
	}
	if(rq->scan_num != rq->last->scan_num) {
	    pui->possible_new_sweep = YES;
	    return(YES);
	}
	return(NO);
    }
    daz = pui->ray_que->az_diff;
    ddaz = pui->ray_que->last->az_diff;
    short_sum_az_diffs = short_running_sum( pui->az_diff_rs ); /* NUM_SHORT_AVG */


    if(pui->options & SIMPLE_SWEEPS) {
	az_turnaround =
	    ( fabs(ddaz) > min_az_diff && fabs(daz) > min_az_diff &&
	      (ddaz + daz) * pui->az_diff_sum < 0 )
	    || short_sum_az_diffs * pui->az_diff_sum < 0;

	if( az_turnaround ) {
	    return(YES);
	}
	if(fabs(daz + pui->az_diff_sum) > 360.) {
	    /* over rotated */
	    return(YES);
	}
	return(NO);
    }

    if(pui->sweep_ray_num < pui->min_rays_per_sweep) {
	return(NO);		/* pass thru the min. num. of rays first */
    }

    /* Here's where the pain starts */

    avgDaz = avg_running_sum( pui->az_diff_rs ); /* NUM_AZ_DIFFS_AVGD */
    avgDel = avg_running_sum( pui->el_diff_rs ); /* NUM_EL_DIFFS_AVGD */
    short_sum_el_diffs =
	short_running_sum( pui->el_diff_rs ); /* NUM_SHORT_AVG */

    del = pui->ray_que->el_diff; /* last diff */
    ddel = pui->ray_que->last->el_diff;	/* second to last diff */

    az_turnaround = short_sum_az_diffs * pui->az_diff_sum < 0;
    el_turnaround = short_sum_el_diffs * pui->el_diff_sum < 0;
    /* pui->el_diff_sum is from start of sweep */


    if(gri->scan_mode == RHI) {
	if(pui->transition_count) {
	    /*
	     * pui->x_az_diff_sum set to 0 at start of each sweep
	     * so it should be 0 the first time through
	     */
	    pui->x_az_diff_sum += pui->ray_que->az_diff;
	    pui->transition_count++;

	    if( fabs(pui->x_az_diff_sum) > max_rhi_diff ) { /* MAX_RHI_DIFF */
		/*
		 * change in az since transition began > next reasonable rhi 
		 * end of RHIs
		 */
		gri->scan_mode = PPI;
		return(NO);
	    }
	    short_avg_az_diff =
		short_avg_running_sum( pui->az_diff_rs ); /* NUM_SHORT_AVG */

	    if(fabs(short_avg_az_diff) > min_az_diff) { /* MIN_AZ_DIFF */
		/* moving in azimuth hopefully to the next rhi */
		return(NO);
	    }
	    short_avg_el =
		short_avg_running_sum( pui->el_diff_rs ); /* NUM_SHORT_AVG */

	    if(fabs(short_avg_el) > min_el_diff) { /* MIN_EL_DIFF */
		/* antenna moving vertically */
		return(YES);
	    }
	    return(NO);
	}

	if( el_turnaround ) {
	    printf( "t8:" );
	    pui->transition_count++;
	    return(NO);
	}
	if((fabs(pui->el_diff_run_sum) < min_el_diff)) { 
	    /*
	     * we are examining the sum of the differences between
	     * ths last NUM_EL_DIFFS_AVGD rays not the whole sweep
	     *
	     * the antenna has stopped moving vertically
	     */
	    printf( "t9:" );
	    pui->transition_count++;
	    return(NO);
	}
	return(NO);
    }


    if(pui->transition_count) {	/* we're in a transition */

	short_avg_az_diff =
	    short_avg_running_sum( pui->az_diff_rs ); /* NUM_SHORT_AVG */
	short_avg_el_diff =
	    short_avg_running_sum( pui->el_diff_rs ); /* NUM_SHORT_AVG */
	pui->transition_count++;

	if( fabs(short_avg_el_diff) > min_el_diff
	    && fabs(short_avg_az_diff) < rhi_az_tol ) {
	    /*
	     * MIN_EL_DIFF and RHI_AZ_TOL
	     *
	     * moving in elevation and "not" moving in azimuth
	     */
	    gri->scan_mode = RHI;
	    pui->lag_rays = lag_rays;
	    return(YES);
	}

	if(fabs(short_avg_az_diff) > min_az_diff
	   && fabs(short_avg_el_diff) < ppi_el_tol ) {
	    /*
	     * using MIN_AZ_DIFF and PPI_EL_TOL
	     *
	     * moving in azimuth and not moving in elevation
	     */
	    gri->scan_mode = PPI;
	    pui->lag_rays = lag_rays;
	    return(YES);
	}
	return(NO);
    }
    /*
     * PPI mode
     * see if we're ready for a change
     */


    if( ebug ) {
	printf( " %.4f %.4f %d %.4f %.4f %d"
		, daz, ddaz, az_turnaround
		, del, ddel, el_turnaround
	    );
    }

    short_avg_el = short_avg_running_sum( pui->el_rs );	/* NUM_SHORT_AVG */

    if( ebug ) {
	d2 = short_running_sum( pui->az_diff_rs );
	printf( " %.4f %.4f %.4f %.4f %.4f"
		, fabs(avgDaz), fabs(avgDel), short_sum_el_diffs, short_avg_el
		, d2
	    );
	printf( "\n" );
    }

    if( fabs(short_sum_el_diffs) < ppi_el_tol ) { /* PPI_EL_TOL */
	/*
	 * antenna is stable for several beams but too far away
	 * from the current fixed angle (MIN_FXD_DIFF)
	 */
	if(FABS(gri->fixed - short_avg_el ) > min_fxdang_diff) {
	    printf( "t0:" );
	    return(YES);
	}
    }
    if( fabs(pui->el_diff_run_sum) > ppi_el_tol ) { /* PPI_EL_TOL */
	/*
	 * moving away from cont. elev.
	 */
	pui->transition_count++;
	printf( "t1:" );
	return(NO);
    }
    if(fabs(avgDaz) < .5 * min_az_diff) { /* MIN_AZ_DIFF */
	/*
	 * slowing down in azimuth
	 */
	pui->transition_count++;
	printf( "t2:" );
	return(NO);
    }
    if( az_turnaround ) {
	/*
	 * antenna has changed direction
	 */
	if( ebug ) {
	    printf( "t3:%.4f %.4f %.4f %.4f\n"
		    , ddaz, daz, short_sum_az_diffs, pui->az_diff_sum);
	}
	else
	    { printf( "t3:" ); }
	pui->transition_count++;
	return(NO);
    }
    if(fabs(daz + pui->az_diff_sum) > 360.) { /* for sweep so far */
	/* over rotated
	 */
	pui->transition_count++;
	printf( "t4:" );
	return(NO);
    }
    return(NO);
}
/* c------------------------------------------------------------------------ */

int 
piraq_select_ray (void)
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
       if(( pui->options & IGNORE_TRANSITIONS ) && hdr->transition )
	 { ok = NO; }
    }

    return(ok);
}
/* c------------------------------------------------------------------------ */

void 
piraq_print_stat_line (int count)
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
piraq_reset (void)
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
# ifdef obsolete
    for(; nn--; rq = rq->last) {
       pui->az_diff_short_sum += rq->az_diff;
       pui->el_diff_short_sum += rq->el_diff;
       pui->az_short_sum += rq->az;
       pui->el_short_sum += rq->el;
    }
    /* set trailing pointer */
    pui->rq_short = rq;

    rq = pui->ray_que;
    nn = pui->num_az_diffs_avgd;
    pui->az_diff_avg_sum = 0;

    for(; nn--; rq = rq->last) {
       pui->az_diff_avg_sum += rq->az_diff;
    }
    /* set trailing pointer */
    pui->rq_az_diffs = rq;

    rq = pui->ray_que;
    nn = pui->num_el_diffs_avgd;
    pui->el_diff_avg_sum  = 0;

    for(; nn--; rq = rq->last) {
       pui->el_diff_avg_sum += rq->el_diff;
    }
    /* set trailing pointer */
    pui->rq_el_diffs = rq;
# endif
}
/* c------------------------------------------------------------------------ */

void piraq_ini (void)
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

    dts.year = 1996;
    dts.month = 5;
    dts.day = 1;
    P960501 = (u_int32_t)d_time_stamp(&dts);

    dd_clear_dts(&dts);
    dts.year = 1997;
    dts.month = 1;
    dts.day = 1;
    P970101 = (u_int32_t)d_time_stamp(&dts);

    dts.year = 1997;
    dts.month = 1;
    dts.day = 27;
    P970127 = (u_int32_t)d_time_stamp(&dts);

    dts.year = 1997;
    dts.month = 1;
    dts.day = 29;
    P970129 = (u_int32_t)d_time_stamp(&dts);

    dts.year = 1998;
    dts.month = 2;
    dts.day = 1;
    P980201 = (u_int32_t)d_time_stamp(&dts);

    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();
    gri = return_gri_ptr();
    irq = dd_return_ios(PIRAQ_IO_INDEX, APIRAQ_FMT);
    irq->min_block_size = sizeof(LeHEADER) + sizeof(LeRADAR);
    irq->min_block_size = sizeof(TOP);
    
    pui = (struct piraq_useful_items *)malloc(sizeof(struct piraq_useful_items));
    memset(pui, 0, sizeof(struct piraq_useful_items));
    pui->num_az_diffs_avgd = 5;
    pui->num_el_diffs_avgd = 5;
    pui->short_rq_len = 5;
    pui->last_dataformat = -1;
    pui->check_ugly = YES;
    pui->check_ugly = NO;
    pui->prev_noise = 10.e22;


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

    rhdr = xrhdr = (RADARV *)malloc(sizeof(RADARV));
    memset(rhdr, 0, sizeof(RADARV));
    strncpy(rhdr->desc, "RHDR", 4);

    hdr = xhdr = (HEADERV *)malloc(sizeof(HEADERV));
    memset(hdr, 0, sizeof(HEADERV));
    strncpy(hdr->desc, "DWEL", 4);

    prqx = (PIRAQX *)malloc(sizeof(PIRAQX));
    memset(prqx, 0, sizeof(PIRAQX));

    asib = (struct platform_i *)malloc(sizeof(struct platform_i));
    memset(asib, 0, sizeof(struct platform_i));

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

    if(aa=get_tagged_string("UNIFORM_CELL_SPACING")) {
	if(sscanf(aa, "%f", &f) > 0) {
	    if(f > 0)
		  pui->uniform_cell_spacing = f;
	}
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
    if(aa=get_tagged_string("IMPROVE_HZO_RCONST_CORR")) {
	Improve_rconst_corr = atof(aa);
    }
    if(aa=get_tagged_string("RCONST_CORRECTION")) {
	pui->rconst_correction = atof(aa);
    }
    printf( "rconst correction: %.3f\n", pui->rconst_correction);

    if(aa=get_tagged_string("ZDR_BIAS")) {
	pui->zdr_offset_corr = atof(aa);
    }
    printf( "zdr_offset_corr: %.3f\n", pui->zdr_offset_corr);

    if(aa=get_tagged_string("LDR_BIAS")) {
	pui->ldr_offset_corr = atof(aa);
    }
    printf( "ldr_offset_corr: %.3f\n", pui->ldr_offset_corr);

    if(aa=get_tagged_string("RANGE_CORRECTION")) {
	pui->range_correction = atof(aa);
    }
    printf( "Range correction: %.3f meters\n", pui->range_correction);

    if(aa=get_tagged_string("TIME_CORRECTION")) {
	pui->time_correction = atof(aa);
    }
    printf( "Time correction: %.3f seconds\n", pui->time_correction);

    if(aa=get_tagged_string("MIN_AZ_DIFF")) {
       d = atof(aa);
       if(d > .01) min_az_diff = d;
    }
    printf("Min. AZ diff: %.2f\n", min_az_diff);

    if(aa=get_tagged_string("MIN_EL_DIFF")) {
	d = atof(aa);
	if(d > .01) min_el_diff = d;
    }
    printf("  Min. EL diff: %.2f\n", min_el_diff);

    if(aa=get_tagged_string("MIN_FXD_DIFF")) {
	d = atof(aa);
	if(d > .01) min_fxdang_diff = d;
    }
    printf("  Min. Fixed Angle diff: %.2f\n", min_el_diff);

    if(aa=get_tagged_string("PPI_EL_TOL")) {
	d = atof(aa);
	if(d > .01) ppi_el_tol = d;
    }
    printf("  PPI elevation tolerance: %.2f\n", ppi_el_tol );

    if(aa=get_tagged_string("RHI_AZ_TOL")) {
	d = atof(aa);
	if(d > .01) rhi_az_tol = d;
    }
    printf("  RHI azimuth tolerance: %.2f\n", rhi_az_tol );

    if(aa=get_tagged_string("MAX_RHI_DIFF")) {
	d = atof(aa);
	if(d > .01) max_rhi_diff = d;
    }
    printf("  Max RHI diff: %.2f\n", max_rhi_diff);
	   
    if(aa=get_tagged_string("NUM_AZ_DIFFS_AVGD")) {
       ii = atoi(aa);
       if(ii > 1) pui->num_az_diffs_avgd = ii;
    }
    printf("Num az diffs averaged: %d\n", pui->num_az_diffs_avgd);

    if(aa=get_tagged_string("NUM_EL_DIFFS_AVGD")) {
       ii = atoi(aa);
       if(ii > 1) pui->num_el_diffs_avgd = ii;
    }
    printf("Num el diffs averaged: %d\n", pui->num_el_diffs_avgd);

    if(aa=get_tagged_string("NUM_SHORT_AVG")) {
       ii = atoi(aa);
       if(ii > 1) pui->short_rq_len = ii;
    }
    printf("Short que size: %d\n", pui->short_rq_len);

    if(aa=get_tagged_string("RENAME")) {
	piraq_name_aliases(aa, &top_ren);
    }
    if(aa=get_tagged_string("PIRAQ_FORCE_POLYPP")) {
       pui->options |= FORCE_POLYPP;
    }
    if( get_tagged_string("HRD_ASCII_TAPE") || 
	get_tagged_string("HRD_STD_TAPE") ) {
	pui->options |= MERGE_HRD_AC;
    }
    if((aa=get_tagged_string("OPTIONS"))) {
	if(strstr(aa, "WINDOWS"))
	  { pui->options |= WINDOWS_DOW; }
	if(strstr(aa, "SIMPLE_SW"))
	  { pui->options |= SIMPLE_SWEEPS; }
	if(strstr(aa, "CE_REV_1"))
	  { pui->options |= FORCE_REV_1; }
	if(strstr(aa, "CE_POLYPP"))
	  { pui->options |= FORCE_POLYPP; }
	if(strstr(aa, "CE_POL12"))
	  { pui->options |= FORCE_POL12; }
	if(strstr(aa, "SPARC_A"))
	  { pui->options |= SPARC_ALIGNMENT; }
	if(strstr(aa, "RANGE0"))
	  { pui->options |= RANGE0_DEFAULT; }
	if(strstr(aa, "PIRAQ_T")) /* PIRAQ_TIME_SERIES */
	  { pui->options |= DUMP_TIME_SERIES; }
	if(strstr(aa, "FLIP_VEL")) /* FLIP_VELOCITIES */
	  { pui->options |= FLIP_VELOCITIES; }
	if(strstr(aa, "FULL_MAT")) /* FULL_MATRIX */
	  { pui->options |= FULL_MATRIX; }
	if(strstr(aa, "FORCE_RHIS")) /* FORCE_RHIS */
	  { pui->options |= FORCE_RHIS; }
	if(strstr(aa, "FORCE_PPIS")) /* FORCE_PPIS */
	  { pui->options |= FORCE_PPIS; }
	if(strstr(aa, "HEADER_SUBSEC")) /* HEADER_SUBSEC */
	  { pui->options |= HEADER_SUBSEC; }
	if(strstr(aa, "SPOL_HO")) /* SPOL_HORZ_FLAG */
	  { pui->options |= SPOL_HORZ_FLAG; }
	if(strstr(aa, "BIST_F")) /* BIST_FLAG */
	  { pui->options |= BIST_FLAG; }
	if(strstr(aa, "DOW_F")) /* DOW_FLAG */
	  { pui->options |= DOW_FLAG; }
	if(strstr(aa, "GUIFU_F")) /* GUIFU_FLAG */
	  { pui->options |= GUIFU_FLAG; }
	if(strstr(aa, "DZ_TO_DBZ")) /* DZ_TO_DBZ_FLAG */
	  { pui->options |= DZ_TO_DBZ_FLAG; }
	if(strstr(aa, "IGNORE_TRANS")) /*  */
	  { pui->options |= IGNORE_TRANSITIONS; }
	if(bb = strstr(aa, "HZO_IMP")) {
	  strcpy( str, bb );
	  nt = dd_tokens( str, sptrs );
	  nt = dd_tokenz( str, sptrs, ":" );
	  if( nt > 1 ) {
	    if((ii = sscanf( sptrs[1], "%d", &jj )) == 1 ) {
	      Improve_range0_offset = jj;
	    }
	  }

	  pui->options |= HZO_IMPROVE_FLAG;
	}
# ifdef notyet
	if(strstr(aa, "")) /*  */
	  { pui->options |= ; }
# endif
    }
    if( pui->short_rq_len > pui->num_az_diffs_avgd )
      { pui->short_rq_len = pui->num_az_diffs_avgd; }
    if( pui->short_rq_len > pui->num_el_diffs_avgd )
      { pui->short_rq_len = pui->num_el_diffs_avgd; }

    nn = dd_min_rays_per_sweep();
    if(nn < 3) { nn = 3; }	/* make it at least 4 */
    if(nn < pui->short_rq_len) { nn = pui->short_rq_len; }
    if(nn < pui->num_az_diffs_avgd) { nn = pui->num_az_diffs_avgd; }
    if(nn < pui->num_el_diffs_avgd) { nn = pui->num_el_diffs_avgd; }
    pui->min_rays_per_sweep = nn++;

    pui->rcp_short_len = 1./pui->short_rq_len;
    pui->rcp_num_az_avgs = 1./pui->num_az_diffs_avgd;
    pui->rcp_num_el_avgs = 1./pui->num_el_diffs_avgd;

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
    piraq_reset();

    pui->az_diff_rs = init_run_sum( pui->num_az_diffs_avgd
				    , pui->short_rq_len );
    pui->el_diff_rs = init_run_sum( pui->num_el_diffs_avgd
				    , pui->short_rq_len );
    pui->az_rs = init_run_sum( pui->num_az_diffs_avgd
				    , pui->short_rq_len );
    pui->el_rs = init_run_sum( pui->num_el_diffs_avgd
				    , pui->short_rq_len );

    if(aa=get_tagged_string("VOLUME_HEADER")) {
	/* nab a volume header from another file
	 */
	dd_input_read_open(irq, aa);
	nn = piraq_next_ray();
	dd_input_read_close(irq);
	piraq_backwards_reset(0);
	piraq_reset();
    }

    if((aa=get_tagged_string("TAPE_DIR"))) {
      slm = solo_malloc_list_mgmt(256);
      nt = generic_sweepfiles( aa, slm, (char *)"", (char *)".tape", 0 );
      if ( nt < 1 ) {		/* Josh's CDs */
	 nt = generic_sweepfiles( aa, slm, (char *)"", (char *)"z", 0 );
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

    nn = piraq_next_ray();
}
/* c------------------------------------------------------------------------ */

void 
piraq_name_replace (char *name, struct rename_str *top)
{
    struct rename_str *this_str=top;  //Jul 26, 2011 *this

    for(; this_str; this_str = this_str->next) {
	if(strstr(name, this_str->old_name)) {
	    strcpy(name, this_str->new_name);
	    break;
	}
    }
}
/* c------------------------------------------------------------------------ */

void 
piraq_name_aliases (char *aliases, struct rename_str **top)
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

int piraq_next_ray (void)
{
    static int whats_left=0;
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
	if((size = pui_next_block()) < irq->min_block_size) {
	   return(-1);
	}
	aa = irq->top->at;

	if(strncmp("RHDR", aa, 4) == 0) { /* Header! */
	   lrhdr = (LeRADARV *)aa;
	   piraq_header();
	   gotta_header = YES;
	   if(!(pui->options & SPOL_FLAG)) {
	       pui->new_vol = pui->vol_count_flag = YES;
	       dd_stats->vol_count++;
	   }
	   irq->top->bytes_left -= size;
	   irq->top->at += size;
	   continue;
	}
	else if(pui->rhdr_count < 1) {
	    /* haven't found a radar header yet; no use in going on */
	    irq->top->bytes_left -= size;
	    irq->top->at += size;
	    continue;
	}
	break;
    } 
    rq = pui->ray_que = pui->ray_que->next;
# ifdef obsolete
    pui->rq_short = pui->rq_short->next;
    pui->rq_az_diffs = pui->rq_az_diffs->next;
    pui->rq_el_diffs = pui->rq_el_diffs->next;
# endif

    dd_stats->ray_count++;
    pui->ray_count++;
    piraq_map_hdr(aa, gotta_header);

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

# ifdef obsolete
    pui->az_diff_sq_sum += rq->az_diff * rq->az_diff;
    pui->el_diff_sq_sum += rq->el_diff * rq->el_diff;

    pui->az_short_sum += rq->az;
    pui->el_short_sum += rq->el;
    pui->az_diff_short_sum += rq->az_diff;
    pui->el_diff_short_sum += rq->el_diff;
    pui->az_diff_avg_sum += rq->az_diff;
    pui->el_diff_avg_sum += rq->el_diff;
    
    if(++pui->sweep_ray_num > pui->short_rq_len) {
       pui->az_short_sum -= pui->rq_short->az;
       pui->el_short_sum -= pui->rq_short->el;
       pui->az_diff_short_sum -= pui->rq_short->az_diff;
       pui->el_diff_short_sum -= pui->rq_short->el_diff;
    }
    if(pui->sweep_ray_num > pui->num_az_diffs_avgd) {
       pui->az_diff_avg_sum -= pui->rq_az_diffs->az_diff;
    }
    if(pui->sweep_ray_num > pui->num_el_diffs_avgd) {
       pui->el_diff_avg_sum -= pui->rq_el_diffs->el_diff;
    }
# else
    ++pui->sweep_ray_num;

# endif    
    rq->ray_num = pui->sweep_ray_num;

    dp = dd_return_next_packet(irq);
    dp->len = hdr->recordlen;
    irq->top->at += hdr->recordlen;
    irq->top->bytes_left -= hdr->recordlen;
    pui->count_rays++;
    
    if(pui->options & SPOL_FLAG) {
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
    }
    else if( pui->sweep_ray_num >= dd_max_rays_per_sweep()) 
	{ pui->new_sweep = YES; }
    else if(piraq_isa_new_sweep())
	{ pui->new_sweep = YES; }


    /* new sweep? */


    if((pui->new_sweep || pui->new_vol ||
			pui->possible_new_sweep)) {

	/* c...mark */
	/*
	 * see if the last sweep should be saved
	 * remember you can set MIN_RAYS_PER_SWEEP
	 */
	if( pui->swp_que->scan_mode == RHI) {
	    if(fabs(pui->el_diff_sum) < 4.) { /* RHI can't be less than 4 deg. */
		gri->ignore_this_sweep = YES;
	    }
	    sum = pui->el_diff_sum;
	    sumsq = pui->el_diff_sq_sum;
	 }
	else if(fabs(pui->az_diff_sum) < 7.) {
	   /* ppi can't be less than 7 deg. */
	   gri->ignore_this_sweep = YES;
	}
	pui->possible_new_sweep = NO;
	dd_stats->sweep_count++;
	pui->sweep_count_flag = pui->new_sweep = YES;
	gri->sweep_num++;
	if(pui->options & SPOL_FLAG || pui->options & DOW_FLAG ) {
	    gri->scan_mode = hdr->scan_type;
	}
	pui->sweep_ray_num = 1;
	pui->transition_count = 0;
	pui->max_az_diff = pui->max_el_diff = 0;

	pui->az_diff_sum =
	  pui->el_diff_sum =
	    pui->az_diff_sq_sum =
	      pui->ray_que->el_diff = 
		pui->x_az_diff_sum =
		  pui->fxdang_sum =
		    pui->fxdang_diff_sum =
		      0;
	pui->az_diff_short_sum =
	  pui->az_short_sum =
	    pui->el_diff_short_sum =
	      pui->el_short_sum =
		pui->az_diff_avg_sum =
		  pui->el_diff_avg_sum =
		    0;

	reset_running_sum( pui->az_diff_rs );
	running_sum( pui->az_diff_rs, rq->az_diff );

	reset_running_sum( pui->el_diff_rs );
	running_sum( pui->el_diff_rs, rq->el_diff );

	reset_running_sum( pui->az_rs );
	running_sum( pui->az_rs, rq->az );

	reset_running_sum( pui->el_rs );
	running_sum( pui->el_rs, rq->el );

	pui->az_diff_sum += rq->az_diff;
	pui->el_diff_sum += rq->el_diff;
	
# ifdef obsolete
	pui->az_diff_sq_sum += rq->az_diff * rq->az_diff;
	pui->el_diff_sq_sum += rq->el_diff * rq->el_diff;
	
	pui->az_short_sum += rq->az;
	pui->el_short_sum += rq->el;
	pui->az_diff_short_sum += rq->az_diff;
	pui->el_diff_short_sum += rq->el_diff;
	pui->az_diff_avg_sum += rq->az_diff;
	pui->el_diff_avg_sum += rq->el_diff;
# endif
	
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
	pui->swp_que->rcvr_pulsewidth = hdr->rcvr_pulsewidth;
	pui->swp_que->scan_mode = gri->scan_mode;
	pui->swp_que->swpang = gri->fixed;
	pui->swp_que->prf = gri->PRF;

	if(pui->options & SPOL_FLAG && pui->possible_new_vol)
	    { pui->new_vol = YES; }

	if(piraq_isa_new_vol() || pui->new_vol) {
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
	if(!(pui->options & SPOL_FLAG)) {
	  if(gri->scan_mode == RHI) {
	    printf( "RHI:" );
	  }
	  else {
	    printf( "PPI:" );
	  }
	}

    } /* end isa new sweep */


    if(pui->runaway) {
	return(-1);
    }
    if(!(pui->options & SPOL_FLAG)) {
       pui->ignore_this_ray = pui->transition_count > 0;
    }
    
    if(gri->scan_mode == RHI) {
       pui->ray_que->fxdang = gri->azimuth;
       pui->ray_que->fxdang_diff = pui->ray_que->az_diff;
    }
    else {
	pui->ray_que->fxdang = gri->elevation;
	pui->ray_que->fxdang_diff = pui->ray_que->el_diff;
    }

    fxd = pui->ray_que->fxdang;

    if( pui->sweep_ray_num > 11 ) { /* in case we're hovering at zero */
	diff = fxd -pui->swp_que->swpang;
	if( fabs( diff ) > 180 ) {
	    if( diff < -180. )
		{  fxd += 360.; }
	    else if( diff > 180. )
		{  fxd -= 360.; }
	}
    }
    pui->fxdang_diff_sum += pui->ray_que->fxdang_diff;
    pui->fxdang_sum += fxd;

    if(!pui->transition_count) {
	    pui->swp_que->swpang = 
		  pui->fxdang_sum/(float)pui->sweep_ray_num;
    }


    if(pui->options & SPOL_FLAG) {
	gri->fixed = hdr->fxd_angle * ANGSCALE;
    }
    else {
	gri->fixed = pui->swp_que->swpang;
    }
    /*
     * calculate antenna movement per ray
     */
    if(fabs(d = pui->ray_que->az_diff) < 5.)
	gri->azimuth = FMOD360(gri->azimuth - .5 * d);

    if(pui->options & SIMPLE_SWEEPS) {
       mark = 0;
    }
    else {
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

    if( pui->options & MERGE_HRD_AC ) {
	hrd_merge_std(gri);
    }
    else {
	acmrg = eld_nimbus_fix_asib(gri->dts, asib, pui->options
				 , gri->dd_radar_num);
# ifdef notyet
	gri->altitude = asib->altitude_msl;
	gri->latitude = asib->latitude;
	gri->longitude = asib->longitude;
	gri->vns = asib->ns_velocity;
	gri->vew = asib->ew_velocity;
	gri->vud = asib->vert_velocity;

	gri->heading = asib->heading;
	gri->pitch = asib->pitch;
	gri->roll = asib->roll;
	gri->drift = asib->drift_angle;
# else
	gri->altitude = hdr->radar_altitude;
	gri->latitude = hdr->radar_lattitude;
	gri->longitude = hdr->radar_longitude;

	if( pui->altitude != EMPTY_FLAG ) {
	    gri->altitude = pui->altitude;
	}
	if( pui->latitude != EMPTY_FLAG ) {
	    gri->latitude = pui->latitude;
	}
	if( pui->longitude != EMPTY_FLAG ) {
	    gri->longitude = pui->longitude;
	}
	gri->vns = hdr->ns_velocity;
	gri->vew = hdr->ew_velocity;
	gri->vud = hdr->vert_velocity;

	gri->pitch = hdr->pitch;
	gri->roll = hdr->roll;
# endif
	gri->altitude_agl = asib->altitude_agl;
	gri->drift = asib->drift_angle;
	gri->heading = asib->heading;
	gri->ui = asib->ew_horiz_wind;
	gri->vi = asib->ns_horiz_wind;
	gri->wi = asib->vert_wind;
    }

    if(pui->options & (SPOL_FLAG | DOW_FLAG)) {
       gri->source_sweep_num = hdr->transition ? 1 : 0;
       gri->source_sweep_num += 10 * (hdr->scan_num % 10);
       gri->source_sweep_num += 100 * (hdr->vol_num % 10);
    }
    else {
       gri->source_sweep_num = pui->transition_count > 0 ? 1 : 0;
       gri->source_sweep_num += 10 * (gri->sweep_num % 10);
       gri->source_sweep_num += 100 * (gri->vol_num % 10);
    }
    pui->skipped_backwards = NO;

    mark = 0;
    return(1);
}
/* c------------------------------------------------------------------------ */

void piraq_backwards_reset (int modest)
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

void piraq_map_hdr (char *aa, int gotta_header)
{
    static int count=0;
    int jj, nn, mark, new_vol = NO;
    double d, subsec;
    struct piraq_ray_que *rq=pui->ray_que;
    int sparc_alignment = 0;
    int tdiff;
    char message[256], str[256], tstr[32], tstr2[32];
    
    count++;
    dwl = (DWELLV *)aa;
    lhdr = (LeHEADERV *)aa;
    
# ifdef obsolete
    hdr->recordlen = SX2(lhdr->recordlen);
    hdr->gates = SX2(lhdr->gates);
    hdr->hits = SX2(lhdr->hits);
    hdr->rcvr_pulsewidth = SX4F(lhdr->rcvr_pulsewidth);
    hdr->prt = SX4F(lhdr->prt);
    hdr->delay = SX4F(lhdr->delay);
    hdr->clutterfilter = (lhdr->clutterfilter);
    hdr->timeseries = (lhdr->timeseries);
    hdr->tsgate = SX2(lhdr->tsgate);
    hdr->time = SX4(lhdr->time);
    hdr->subsec = SX2(lhdr->subsec);
    hdr->az = SX4F(lhdr->az);
    hdr->el = SX4F(lhdr->el);
    hdr->radar_longitude = SX4F(lhdr->radar_longitude); 
    hdr->radar_lattitude = SX4F(lhdr->radar_lattitude);
    hdr->radar_altitude = SX4F(lhdr->radar_altitude);
    hdr->ew_velocity = SX4F(lhdr->ew_velocity);
    hdr->ns_velocity = SX4F(lhdr->ns_velocity);
    hdr->vert_velocity = SX4F(lhdr->vert_velocity);
    if(rhdr->rev) {
       hdr->dataformat = lhdr->dataformat;
       hdr->prt2 = SX4F(lhdr->prt2);
    }
    if(rhdr->rev > 1 || hdr->time > P960501) {
       hdr->fxd_angle = SX4F(lhdr->fxd_angle);
       hdr->scan_type = lhdr->scan_type;
       hdr->scan_num = lhdr->scan_num;
       hdr->vol_num = lhdr->vol_num;
       hdr->hxmit_power = SX4F(lhdr->hxmit_power);
       hdr->vxmit_power = SX4F(lhdr->vxmit_power);
    }
# else

   if(!rhdr->rev) {
      hdr->dataformat = 0;
   }
   if(rhdr->rev == 4) {
      sparc_alignment = YES;
   }
   else {
      sparc_alignment = pui->options & SPARC_ALIGNMENT;
   }
//    piraq_crack_header(aa, xhdr, (int)0, sparc_alignment);  //Jul 26, 2011
    piraq_crack_header(aa, (char *)xhdr, (int)0, sparc_alignment);  //Jul 26, 2011

    if (pui->options & WINDOWS_DOW) {
      memcpy (xhdr, aa, sizeof (*xhdr));
      sparc_alignment = NO;
    }

    if(pui->options & SPARC_ALIGNMENT) {
       pui->raw_data = aa + sizeof(HEADERNU);
    }
    else if(pui->options & WINDOWS_DOW) {
       pui->raw_data = aa + sizeof(HEADERV);
    } 
    else {
       pui->raw_data = aa + sizeof(LeHEADERV);
    }
# endif

    /*
     * forced formats perhaps
     */
    if(pui->options & FORCE_POLYPP) hdr->dataformat = DATA_POLYPP;
    if(pui->options & FORCE_POL12) hdr->dataformat = DATA_POL12;

# ifdef obsolete
    memcpy(hdr->reserved, lhdr->reserved, sizeof(hdr->reserved));
# endif    
    gri->gpptr2 = (void *)hdr;
    gri->gpptr4 = (void *)prqx;
    gri->sizeof_gpptr4 = sizeof (*prqx);

    update_prqx (prqx, lrhdr, lhdr);
    gri->transition = (hdr->transition) ? IN_TRANSITION : NORMAL;
    gri->num_bins = (hdr->gates);
    gri->num_samples = (hdr->hits);
    gri->pulse_width = (hdr->rcvr_pulsewidth);
    gri->range_b1 = (hdr->delay) * SPEED_OF_LIGHT;
    gri->range_b1 = 0.0;	/* for magnetron systems per Mitch */
    gri->bin_spacing = pui->uniform_cell_spacing
	  ? pui->uniform_cell_spacing
		: (hdr->rcvr_pulsewidth) * .5 * SPEED_OF_LIGHT;

    if( hdr->dataformat == DATA_SIMPLEPP && pui->options & HZO_IMPROVE_FLAG )
      { gri->range_b1 = 150 - Improve_range0_offset * gri->bin_spacing; }

    else if( pui->options & RANGE0_DEFAULT )
      { gri->range_b1 = Default_Range0; }

    gri->range_b1 += pui->range_correction;
    
    gri->dts->year = 1970;
    gri->dts->month = gri->dts->day = 0;
    rq->ptime = hdr->time;
    rq->subsec = hdr->subsec;

    if((pui->options & HEADER_SUBSEC)) {
       pui->time =  hdr->time + (double)(hdr->subsec) * .0001;
    }
    else {
       /*
	* since the subsecond time is not useful try to calculate
	* a subsecond time
	* all times are integers except pui->time which is a double
	*/
	tdiff = ( hdr->time - pui->trip_time );

	if( tdiff > 0 || tdiff < -1 ) {
	    /* the -1 takes into account the apparent too many
	     * rays in a second as determined below
	     * also should deal with a time glitch
	     */
	    pui->trip_time = hdr->time;
	    pui->count_since_trip = 0;
	    pui->seconds_per_beam = (double)hdr->hits * hdr->prt;
	}
	subsec = (pui->count_since_trip++) * pui->seconds_per_beam;
	pui->time = (double)pui->trip_time + subsec;

	if( subsec >= 1. ) {
	    /* sometimes there are too may rays
	     * before the time changes
	     */
	   pui->trip_time = (u_int32_t) pui->time;
	   pui->count_since_trip = 1;
	}
    }
    pui->time += pui->time_correction;

    if( pui->time < rq->last->time ) {
      gri->dts->time_stamp = rq->last->time;
      strcpy (tstr, dts_print( d_unstamp_time( gri->dts ))); 
      gri->dts->time_stamp = pui->time;
      strcpy (tstr2, dts_print( d_unstamp_time( gri->dts ))); 
# ifdef obsolete
      sprintf( message, "   **** Time glitch at: %s %s\n   %.4f %.4f %.4f %.4f"
	       , tstr, tstr2
	       , rq->last->last->last->time, rq->last->last->time
	       , rq->last->time, pui->time
	       );
# else
      sprintf( message, "   **** Time glitch at: %s %s", tstr, tstr2 );
# endif
       if((pui->time_glitch_count++ % 5 ) == 0 ) {
	 dd_append_cat_comment( message );
	 printf( "%s\n", message );
       }
       if( hdr->subsec >= 10000 )
	 { mark = 0; }
    }
    gri->dts->time_stamp = gri->time = rq->time = pui->time;

    if(pui->options & SPOL_FLAG && pui->time > P970101) {
	pui->check_ugly = NO;
    }
    gri->azimuth = (hdr->az);
    gri->elevation = (hdr->el);

    if( !( pui->options & SPOL_FLAG )) {
      if( hdr->el < -5. || hdr->el > 95. ) {
	gri->elevation = rq->last->el;
      }
    }
    asib->altitude_msl = gri->altitude = pui->altitude != EMPTY_FLAG
	  ? pui->altitude : (hdr->radar_altitude);
    asib->latitude = gri->latitude = pui->latitude != EMPTY_FLAG
	  ? pui->latitude : (hdr->radar_lattitude);
    asib->longitude = gri->longitude = pui->longitude != EMPTY_FLAG
	  ? pui->longitude : (hdr->radar_longitude);
    
    asib->ns_velocity = gri->vns = hdr->ns_velocity;
    asib->ew_velocity = gri->vew = hdr->ew_velocity;
    asib->vert_velocity = gri->vud = hdr->vert_velocity;
    
    asib->heading = hdr->yaw;
    asib->pitch = hdr->pitch;
    asib->roll = hdr->roll;

    gri->clutter_filter_val = hdr->clutterfilter;

    switch((int)hdr->clutterfilter) {
    case 1:
	break;
    default:
	break;
    }
    gri->ipps[0] = hdr->prt * 1000.; /* convert to milliseconds */
    gri->PRF = hdr->prt ? 1./hdr->prt : EMPTY_FLAG;
	
    new_vol = pui->last_dataformat != hdr->dataformat ||
        pui->last_gates != hdr->gates ||
        pui->last_rcvr_pulsewidth != hdr->rcvr_pulsewidth ||
        pui->last_prt != hdr->prt;

    if (new_vol) {
	pui->last_dataformat = hdr->dataformat;
	pui->last_gates = hdr->gates;
	pui->last_rcvr_pulsewidth = hdr->rcvr_pulsewidth;
	pui->last_prt = hdr->prt;
	if(gri->wavelength > 0 && gri->PRF > 0)
	      gri->nyquist_vel = gri->wavelength * gri->PRF * .25;
	pui->new_vol = pui->new_sweep = YES;
	piraq_fields();
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
    rq->hits = hdr->hits;
    rq->prt = hdr->prt;
    rq->scan_num = hdr->scan_num;
    rq->vol_num = hdr->vol_num;
    rq->num_gates = hdr->gates;
    rq->scan_type = hdr->scan_type;

    rq->az = gri->azimuth;
    rq->el = gri->elevation;
    rq->rcvr_pulsewidth = hdr->rcvr_pulsewidth;;

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

void piraq_header (void)
{
    int ii, pn=0, nbins;
    float scale = 100., bias = 0, r, gs;
    char *aa, mess[256];


    if(!(pui->options & SPOL_FLAG)) {
	pui->new_vol = YES;
    }

    pui->rhdr_count++;

    gri->binary_format = DD_16_BITS;
    gri->missing_data_flag = EMPTY_FLAG;
    gri->source_format = APIRAQ_FMT;
    gri->compression_scheme = NO_COMPRESSION;
    gri->missing_data_flag = EMPTY_FLAG;

# ifdef obsolete
    rhdr->recordlen = SX2(lrhdr->recordlen);
    rhdr->rev = SX2(lrhdr->rev);
    rhdr->year = SX2(lrhdr->year);
    memcpy(rhdr->radar_name, lrhdr->radar_name, 8);
    str_terminate(gri->radar_name, rhdr->radar_name, 8);

    if(strstr(gri->radar_name, "SPOL")) {
       pui->options |= SPOL_FLAG;
    }
    piraq_name_replace(gri->radar_name, top_ren);

    rhdr->polarization = (lrhdr->polarization);
    rhdr->test_pulse_pwr = SX4F(lrhdr->test_pulse_pwr);
    rhdr->test_pulse_frq = SX4F(lrhdr->test_pulse_frq);
    rhdr->frequency = SX4F(lrhdr->frequency);
    rhdr->peak_power = SX4F(lrhdr->peak_power);
    rhdr->noise_figure = SX4F(lrhdr->noise_figure);
    rhdr->noise_power = SX4F(lrhdr->noise_power);
    rhdr->receiver_gain = SX4F(lrhdr->receiver_gain);
    rhdr->data_sys_sat = SX4F(lrhdr->data_sys_sat);
    rhdr->antenna_gain = SX4F(lrhdr->antenna_gain);
    rhdr->horz_beam_width = SX4F(lrhdr->horz_beam_width);
    rhdr->vert_beam_width = SX4F(lrhdr->vert_beam_width);
    rhdr->xmit_pulsewidth = SX4F(lrhdr->xmit_pulsewidth);
    rhdr->rconst = SX4F(lrhdr->rconst );
    memcpy(rhdr->text, lrhdr->text, sizeof(rhdr->text));

    if(rhdr->rev) {
	rhdr->phaseoffset = SX4F(lrhdr->phaseoffset);
    }
    if(pui->options & SPOL_FLAG) {
	rhdr->vreceiver_gain = SX4F(lrhdr->vreceiver_gain);
	rhdr->vtest_pulse_pwr = SX4F(lrhdr->vtest_pulse_pwr);
	rhdr->vantenna_gain = SX4F(lrhdr->vantenna_gain);
    }
# else
    ii = 33;
    ii = pui->options & SPARC_ALIGNMENT;

//    piraq_crack_radar (lrhdr, xrhdr, (int)0, ii );  //Jul 26, 2011
    piraq_crack_radar ((char *)lrhdr, (char *)xrhdr, (int)0, ii );  //Jul 26, 2011

    if (pui->options & WINDOWS_DOW) {
      memcpy (xrhdr, lrhdr, sizeof (*xrhdr));
    }
    rhdr = xrhdr;
    if(strstr(rhdr->radar_name, "SPOL")) {
       pui->options |= SPOL_FLAG;
    }
    if(strstr(rhdr->radar_name, "BIST")) {
       pui->options |= BIST_FLAG;
    }
    if(pui->options & FORCE_REV_1) {
       rhdr->rev = 1;
    }
    str_terminate(gri->radar_name, rhdr->radar_name, 8);
    /* remove any blank spaces from the radar name
     */
    aa = gri->radar_name;
    for( ii = 0; *aa && ii < 8; ii++, aa++ ) {
	if( *aa == ' ' )
	    { *aa = '_'; }
    }
    piraq_name_replace(gri->radar_name, top_ren);
# endif
    gri->gpptr1 = (void *)rhdr;
    gri->gpptr3 = (void *)&r_consts;
    gri->num_freq = 1;
    gri->num_ipps = 1;
    gri->wavelength = rhdr->frequency ? SPEED_OF_LIGHT/rhdr->frequency : 0;
    gri->freq[0] = rhdr->frequency * 1.e-9; /* GHz */

    gri->pulse_width = rhdr->xmit_pulsewidth;
    gri->h_beamwidth = rhdr->horz_beam_width;
    gri->v_beamwidth = rhdr->vert_beam_width;

    switch((int)rhdr->polarization) {
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
    gri->radar_constant = rhdr->rconst;
    gri->rcvr_gain = rhdr->receiver_gain;
    gri->ant_gain = rhdr->antenna_gain;
    gri->noise_power = rhdr->noise_power;
    gri->peak_power = rhdr->peak_power;

    gri->scan_mode = PPI;
    if(strncmp(gri->radar_name, "NOSE", 2) == 0 ||
       strncmp(gri->radar_name, "WXR-700C", 3) == 0) {
	gri->radar_type = AIR_NOSE;
    }
    else {
	gri->radar_type = GROUND;
    }
    gri->sweep_speed = EMPTY_FLAG;
    gri->rcv_bandwidth = -999.;	/* MHz */
    gri->dd_radar_num = dd_assign_radar_num(gri->radar_name);
    gri->source_vol_num = ++pui->vol_num;
    if(!(pui->options & SPOL_FLAG)) {
	gri->sweep_num = 0;
    }
    gri->dts->time_stamp = gri->time;
    d_unstamp_time(gri->dts);

    if( pui->rhdr_count < 12 ) {
      strcpy(mess, "Gotta header! ");
      sprintf(mess+strlen(mess)
	      , "%s %s", dts_print(gri->dts), dd_stats_sprintf());
      printf("%s\n", mess);
      dd_append_cat_comment(mess);
    }
}
# ifdef obsolete
# else
/* c------------------------------------------------------------------------ */

void piraq_fields (void)
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


    switch( hdr->dataformat ) {

    case DATA_DUALPP:

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

	if( hdr->dataformat != DATA_SMHVSIM ) {
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
# endif
/* c------------------------------------------------------------------------ */

/* compute 6 radar products aranged as:  */
/* velocity (m/s) */
/* power (dBm) */
/* NCP */
/* spectral width (m/s) */
/* reflectivity (dBZ) */
/* coherent reflectivity (dBZ) */

/* the products fill a floating point array with scientific parameters */
/* in regular units (i.e. m/s, dBm, dBZ, Hz ... ) */


static void simplepp(DWELL *dwell, RADARV *radar, float *prods)      
{
    int mark, gg;
    int  i;
    float        *aptr,*pptr;
    double       a,b,p,cp,pcorrect;
    double       logstuff,noise,velconst;
    double       dbm,widthconst,range,rconst,width;
    double       c1, c2, c3;
    short *velp=gri->scaled_data[0];
    short *dbmp=gri->scaled_data[1];
    short *ncpp=gri->scaled_data[2];
    short *swp=gri->scaled_data[3];
    short *dbzp=gri->scaled_data[4];
    short *dbzcp=gri->scaled_data[5];
    float f, scale=100., bias=0;
    double pcorr_const;
    int flip_vels;

    flip_vels = pui->options & FLIP_VELOCITIES || pui->options & SPOL_FLAG;
    rconst = rhdr->rconst -
	  20.0 * log10(rhdr->xmit_pulsewidth / hdr->rcvr_pulsewidth);
    rconst = rhdr->rconst;

    if( pui->options & HZO_IMPROVE_FLAG ) {
      rconst += Improve_rconst_corr;
    }
    else
      { rconst += pui->rconst_correction;}
    noise = (rhdr->noise_power > -10.0) ? 0.0 : 0.0;
    velconst = C / (2.0 * rhdr->frequency * 2.0 * M_PI * hdr->prt);
    if( flip_vels ) velconst = -velconst;

# ifdef obsolete
    /* SPOL */
    pcorrect = rhdrdata_sys_sat
	- 20.0 * log10(0x1000000 * dwell->header.rcvr_pulsewidth / 1.25E-7) 
	- 10.0 * log10((double)dwell->header.hits)
	- radar->receiver_gain;
    /* WARDS */
    pcorrect = rhdr->data_sys_sat
	- 20.0 * log10(0x2000000 * hdr->rcvr_pulsewidth / 1.25E-7) 
	- 10.0 * log10((double)hdr->hits)
	- rhdr->receiver_gain;
# endif

    pcorr_const = pui->options & SPOL_FLAG ? 0x1000000 : 0x2000000;

    pcorrect = rhdr->data_sys_sat
	  - 20.0 * log10(pcorr_const * hdr->rcvr_pulsewidth / 1.25E-7) 
		- 10.0 * log10((double)hdr->hits)
		      - rhdr->receiver_gain;

    c1 = 20.0 * log10(0x1000000 * hdr->rcvr_pulsewidth / 1.25E-7);
    c2 = 20.0 * log10(0x2000000 * hdr->rcvr_pulsewidth / 1.25E-7);
    c3 = 10.0 * log10((double)hdr->hits);

    widthconst = (C / rhdr->frequency) / hdr->prt
	  / (2.0 * sqrt(2.0) * M_PI);
   
# ifdef obsolete
    aptr = dwell->abp;
# endif

    if( pui->options & SPOL_FLAG ) {
	aptr = (float *)pui->raw_data;
    }
    else {
	/* WARDS and ancient DOW */
	aptr = (float *)((char *)dwell + sizeof(LeHEADER));
    }

    range = 0.0;
    gg = pui->options & HZO_IMPROVE_FLAG ? -Improve_range0_offset : 0;

    for(i=0; i < hdr->gates; i++, gg++) {
       if(hostIsLittleEndian()) {
	  a = (*aptr++);
	  b = (*aptr++);
	  p = (*aptr++);
       }
       else {
//	  a = SX4F(*aptr++);  //Jul 26, 2011
//	  b = SX4F(*aptr++);  //Jul 26, 2011
//	  p = SX4F(*aptr++);  //Jul 26, 2011
	  a = SX4F(*(fourB*)aptr++);  //Jul 26, 2011
	  b = SX4F(*(fourB*)aptr++);  //Jul 26, 2011
	  p = SX4F(*(fourB*)aptr++);  //Jul 26, 2011
       }

       if(gg > 0) range = 20.0 * log10
		    (gg * 0.0005 * C * hdr->rcvr_pulsewidth);

	/* compute floating point, scaled, scientific products
	 */
	f = velconst * atan2(-b,a); /* velocity in m/s */
	*velp++ = (short)DD_SCALE(f, scale, bias);

	f = dbm = 10.0 * log10(fabs(p)) + pcorrect; /* power in dBm */
	*dbmp++ = (short)DD_SCALE(f, scale, bias);

	f = (cp = sqrt(a*a+b*b))/p; /* NCP no units */
	*ncpp++ = (short)DD_SCALE(f, scale, bias);

	if((width = log(fabs((p-noise)/cp))) < 0.0) width = 0.0001;
	f = sqrt(width) * widthconst; /* s.w. in m/s */
	*swp++ = (short)DD_SCALE(f, scale, bias);

	f = dbm + rconst + range; /* in dBZ */
	*dbzp++ = (short)DD_SCALE(f, scale, bias);

	f = 10.0 * log10(fabs(cp)) + pcorrect + rconst + range; /* in dBZ */
	*dbzcp++ = (short)DD_SCALE(f, scale, bias);
    }
}
/* c------------------------------------------------------------------------ */

void simplepp16(DWELL *dwell, RADARV *radar, float *prods)      
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
    int flip_vels;

    flip_vels = pui->options & FLIP_VELOCITIES;

   scale2ln = 0.004 * log(10.0) / 10.0;	/* from counts to natural log */
   scale2db = 0.004 / scale2ln;	/* from natural log to 10log10() */

    rconst = rhdr->rconst -
	  20.0 * log10(rhdr->xmit_pulsewidth / hdr->rcvr_pulsewidth);
    velconst = C / (2.0 * rhdr->frequency * 2.0 * FABS(hdr->prt) * 32768.);

    pcorr_const = pui->options & SPOL_HORZ_FLAG ? 0x1000000/1.25E-7
	: 0x2000000/1.25E-7;

    pcorrect = rhdr->data_sys_sat
      - 20.0 * log10(0x10000)
	- rhdr->receiver_gain;

    widthconst = (C / rhdr->frequency) / hdr->prt
	  / (2.0 * sqrt(2.0) * M_PI);
   
# ifdef obsolete
    aptr = dwell->abp;
    aptr = (short *)((char *)dwell + sizeof(LeHEADERV));
# endif

    aptr = (short *)pui->raw_data;
    range = 0.0;
    for(i=0; i < hdr->gates; i++) {
       if(hostIsLittleEndian()) {
	  cp = (*aptr++);
	  v = (*aptr++);
	  p = (*aptr++);
       }
       else {
//Jul 26, 2011 start
//	  cp = SX2(*aptr++);
//	  v = SX2(*aptr++);
//	  p = SX2(*aptr++);
	  cp = SX2(*(twob*)aptr++);
	  v = SX2(*(twob*)aptr++);
	  p = SX2(*(twob*)aptr++);
//Jul 26, 2011 end
       }
       if(i) range = 20.0 * log10
	      (i * 0.0005 * C * hdr->rcvr_pulsewidth);

	/* compute floating point, scaled, scientific products
	 */
	f = velconst * v; /* velocity in m/s */
	*velp++ = (short)DD_SCALE((flip_vels ? -f : f), scale, bias);

	f = dbm = 0.004 * p + pcorrect; /* power in dBm */
	*dbmp++ = (short)DD_SCALE(f, scale, bias);

	f = exp(scale2ln * (cp - p)); /* NCP no units */
	*ncpp++ = (short)DD_SCALE(f, scale, bias);

        f = sqrt( scale2ln * fabs( cp - p )) * widthconst; /* s.w. in m/s */
	*swp++ = (short)DD_SCALE(f, scale, bias);

	f = dbm + rconst + range; /* in dBZ */
	*dbzp++ = (short)DD_SCALE(f, scale, bias);

	f = 0.004 * cp + pcorrect + rconst + range; /* in dBZ */
	*dbzcp++ = (short)DD_SCALE(f, scale, bias);
    }
}
/* c------------------------------------------------------------------------ */

double 
pc_swap4f (		/* swap real*4 */
    fourB *ov
)
{
    double d;
    union {
	float newval;
	unsigned char nv[4];
    }u;

    u.nv[3] = ov->zero;		/* dec */
    u.nv[2] = ov->one;
    u.nv[1] = ov->two;
    u.nv[0] = ov->three;

    return(d = u.newval);
}
/* c------------------------------------------------------------------------ */

int32_t 
pc_swap4 (		/* swap real*4 */
    fourB *ov
)
{
    double d;
    union {
	int32_t newval;
	unsigned char nv[4];
    }u;

    u.nv[3] = ov->zero;		/* dec */
    u.nv[2] = ov->one;
    u.nv[1] = ov->two;
    u.nv[0] = ov->three;

    return((int32_t)(d = u.newval));
}
/* c------------------------------------------------------------------------ */

void 
piraq_positioning (void)
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
		if((jj=piraq_next_ray()) < 0)
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
	    piraq_backwards_reset(0);
	}
	piraq_reset();
	piraq_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 1) {
	if((nn = piraq_inventory(irq)) == -2)
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
	      piraq_backwards_reset(0);
	dd_stats_reset();
	piraq_reset();
	piraq_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 3) {
	dd_rewind_dev(irq);
	dd_stats_reset();
	piraq_backwards_reset(0);
	piraq_reset();
	piraq_next_ray();
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
	      piraq_backwards_reset(0);
	piraq_reset();
	piraq_next_ray();
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
	    if((nn = piraq_next_ray()) <= 0 || gri->time >= dtarget ||
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
	    if((nn = piraq_next_ray()) <= 0 || in_sector(gri->fixed, fx1, fx2)
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
	piraq_print_headers(slm);
    }
    else if(ival == 12) {
	piraq_dump_this_ray(slm);
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
piraq_inventory (struct input_read_info *irq)
{
    /* the purpose of this routine is to facilitate a more detailed
     * examination of the data
     */
    int ii=0;
    int nn, ival;
    float val;
    char str[256];
    double d;
    int max = 256; //Jun 29, 2011


    for(;;) {
	for(ii=0; ii < max; ii++) {
	    pui->new_sweep = NO;
	    pui->new_vol = NO;
	    if((nn=piraq_next_ray()) < 1) {
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
piraq_dump_this_ray (struct solo_list_mgmt *slm)
{
    int gg, ii, nf, nn, fw=10;
    char *aa, *bb, *cc, str[128], tmp_str[16];
    float unscale[8], bias[8], *fp=NULL;

    aa = str;
    bb = tmp_str;
    products((DWELL *)dwl, rhdr, fp);
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
piraq_print_headers (struct solo_list_mgmt *slm)
{
    solo_reset_list_entries(slm);
    piraq_print_rhdr(rhdr, slm);
    solo_add_list_entry(slm, " ");
    piraq_print_hdr(hdr, slm, rhdr);
    slm_print_list(slm);
}
/* c------------------------------------------------------------------------ */

void 
piraq_print_hdr (HEADERV *hdr, struct solo_list_mgmt *slm, RADARV *rhdr)
{
    int ii;
    char *aa, str[128], tmp[32];
	DD_TIME dts;
    double d;

    aa = str;

    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of piraq ray header");
    solo_add_list_entry(slm, aa);

    sprintf(aa, "desc[4]         %s", str_terminate(tmp,hdr->desc,4));
    solo_add_list_entry(slm, aa);
    sprintf(aa, "recordlen       %d", hdr->recordlen      );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "gates           %d", hdr->gates          );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "hits            %d", hdr->hits           );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "rcvr_pulsewidth %e", hdr->rcvr_pulsewidth);
    solo_add_list_entry(slm, aa);
    sprintf(aa, "prt             %e", hdr->prt            );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "delay           %e", hdr->delay          );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "clutterfilter   %d", hdr->clutterfilter  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "timeseries      %d", hdr->timeseries     );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "tsgate          %d", hdr->tsgate         );
    solo_add_list_entry(slm, aa);
    dts.time_stamp = pui->time;
    sprintf(aa, "time            %d %s", hdr->time
	    , dts_print(d_unstamp_time(&dts))           );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "subsec          %d", hdr->subsec         );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "az              %e", hdr->az             );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "el              %e", hdr->el             );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "radar_longitude %e", hdr->radar_longitude);
    solo_add_list_entry(slm, aa);
    sprintf(aa, "radar_lattitude %e", hdr->radar_lattitude);
    solo_add_list_entry(slm, aa);
    sprintf(aa, "radar_altitude  %e", hdr->radar_altitude );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ew_velocity     %e", hdr->ew_velocity    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ns_velocity     %e", hdr->ns_velocity    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "vert_velocity   %e", hdr->vert_velocity  );
    solo_add_list_entry(slm, aa);
    if(rhdr->rev) {
	sprintf(aa, "dataformat      %d", hdr->dataformat  );
	solo_add_list_entry(slm, aa);
	sprintf(aa, "prt2            %e", hdr->prt2  );
	solo_add_list_entry(slm, aa);
# ifdef obsolete
	sprintf(aa, "reserved[15]    %d", hdr->reserved       );
	solo_add_list_entry(slm, aa);
# endif
    }
    if(pui->options & SPOL_FLAG ||pui->options & DOW_FLAG ) {
	sprintf(aa, "fxd_angle       %e", hdr->fxd_angle  );
	solo_add_list_entry(slm, aa);
	sprintf(aa, "scan_type       %d", hdr->scan_type  );
	solo_add_list_entry(slm, aa);
	sprintf(aa, "scan_num        %d", hdr->scan_num  );
	solo_add_list_entry(slm, aa);
	sprintf(aa, "vol_num         %d", hdr->vol_num  );
	solo_add_list_entry(slm, aa);
	sprintf(aa, "ray_count       %d", hdr->ray_count  );
	solo_add_list_entry(slm, aa);
	sprintf(aa, "transition       %d", hdr->transition  );
	solo_add_list_entry(slm, aa);

	sprintf(aa, "hxmit_power     %f", hdr->hxmit_power  );
	solo_add_list_entry(slm, aa);
	sprintf(aa, "vxmit_power     %f", hdr->vxmit_power  );
	solo_add_list_entry(slm, aa);
    }
    if( gri->radar_type == AIR_NOSE ) {
	sprintf(aa, "yaw             %f", hdr->yaw  );
	solo_add_list_entry(slm, aa);
	sprintf(aa, "pitch           %f", hdr->pitch  );
	solo_add_list_entry(slm, aa);
	sprintf(aa, "roll            %f", hdr->roll  );
	solo_add_list_entry(slm, aa);
    }
}
/* c------------------------------------------------------------------------ */

void 
piraq_print_rhdr (RADARV *rhdr, struct solo_list_mgmt *slm)
{
    int ii, jj, nc, nn;
    char *aa, *bb, *cc, *zz, str[128], tmp[32];

    aa = str;
    solo_add_list_entry(slm, " ");
    sprintf(aa, "Contents of piraq radar header");
    solo_add_list_entry(slm, aa);

    sprintf(aa, "desc[4]         %s", str_terminate(tmp, rhdr->desc, 4));
    solo_add_list_entry(slm, aa);
    sprintf(aa, "recordlen       %d", rhdr->recordlen       );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "rev             %d", rhdr->rev             );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "year            %d", rhdr->year            );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "radar_name[8]   %s", str_terminate(tmp,rhdr->radar_name,8));
    solo_add_list_entry(slm, aa);
    sprintf(aa, "polarization    %d", rhdr->polarization    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "test_pulse_pwr  %e", rhdr->test_pulse_pwr  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "test_pulse_frq  %e", rhdr->test_pulse_frq  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "frequency       %e", rhdr->frequency       );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "peak_power      %e", rhdr->peak_power      );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "noise_figure    %e", rhdr->noise_figure    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "noise_power     %e", rhdr->noise_power     );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "receiver_gain   %e", rhdr->receiver_gain   );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "data_sys_sat    %e", rhdr->data_sys_sat    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "antenna_gain    %e", rhdr->antenna_gain    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "horz_beam_width %e", rhdr->horz_beam_width );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "vert_beam_width %e", rhdr->vert_beam_width );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "xmit_pulsewidth %e", rhdr->xmit_pulsewidth );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "rconst          %e", rhdr->rconst          );
    solo_add_list_entry(slm, aa);
    if(rhdr->rev) {
	sprintf(aa, "phaseoffset     %e", rhdr->phaseoffset  );
	solo_add_list_entry(slm, aa);
	if(pui->options & SPOL_FLAG) {
	   sprintf(aa, "vreceiver_gain  %e", rhdr->vreceiver_gain  );
	   solo_add_list_entry(slm, aa);
	   sprintf(aa, "vtest_pulse_pwr  %e", rhdr->vtest_pulse_pwr  );
	   solo_add_list_entry(slm, aa);
	   sprintf(aa, "vantenna_gain    %e", rhdr->vantenna_gain  );
	   solo_add_list_entry(slm, aa);
	   sprintf(aa, "vnoise_power     %e", rhdr->vnoise_power  );
	   solo_add_list_entry(slm, aa);
	   sprintf(aa, "zdr_fudge_factor %e", rhdr->zdr_fudge_factor  );
	   solo_add_list_entry(slm, aa);
	}
	for(ii=0; ii < ((pui->options & SPOL_FLAG) ? 4 : 9) ; ii++) {
	    sprintf(aa, "misc[%02d]         %e", ii, rhdr->misc[ii]  );
	    solo_add_list_entry(slm, aa);
	}
	sprintf(aa, "text[960]        "                        );
    }
    else {
	sprintf(aa, "text[1000]        "                        );
    }
    solo_add_list_entry(slm, aa);

    bb = rhdr->text;
    zz = bb + strlen(rhdr->text);

    for(cc=aa; bb < zz; cc=aa) {
	for(nn=0; bb < zz && *bb && *bb != '\n'; *cc++ = *bb++, nn++); bb++;
	if(nn) {
	    *cc = '\0';
	    solo_add_list_entry(slm, aa);
	}
    }

# ifdef obsolete
    for(; bb < cc && (*cc == ' ' || *cc == 0); nc--, cc--);

    for(; nc > 0; nc-=76, bb+=76) {
	nn = nc < 76 ? nc : 76;
	memcpy(aa, bb, nn);
	solo_add_list_entry(slm, aa);
    }
# endif
}
/* c------------------------------------------------------------------------ */

static void products(DWELL *dwell, RADARV *radar, float *prods)      
{
  switch(hdr->dataformat)
    {
    case DATA_POLYPP:   polypp(dwell,radar,prods);      break;
    case DATA_DUALPP:   dualprt(dwell,(RADAR *) radar,prods);     break;  //Jul 26, 2011 RADARV and RADAR have the same structure
    case DATA_POL1:
      dualpol1(dwell,radar,prods);
      break;

    case DATA_SMHVSIM:
      smallhvsimul(dwell,(RADAR *)radar,prods);
      break;

    case DATA_POL2:
    case DATA_POL3:
      if( hdr->time > P980201 )
	{ dualpol12(dwell,radar,prods); }
      else
	{ dualpol3(dwell,radar,prods); }
      break;
    case DATA_SIMPLEPP16:
      simplepp16(dwell,radar,prods);
      break;

    case DATA_POL12:
      dualpol12(dwell,radar,prods);
      break;

    case DATA_POL_PLUS:
    case DATA_MAX_POL:
    case DATA_HVSIMUL:
    case DATA_SHRTPUL:
      if( pui->options & GUIFU_FLAG ) {
	//	      fullpolplusGf(dwell,radar,prods);
	/*
	  dualpolplus(dwell,radar,prods);
	*/
      }
      else {
	fullpolplus(dwell,radar,prods);
      }
      break;

    case DATA_SIMPLEPP:   
    default:            simplepp(dwell,radar,prods);    break;
    }
  /*
    r_consts.h_rconst = h_rconst;
    r_consts.v_rconst = v_rconst;
  */
  prqx->h_rconst = h_rconst;
  prqx->v_rconst = v_rconst;
   
}
#define NEW_UNFOLD      
#define NYQ_COUNT  32768.0

/* c------------------------------------------------------------------------ */

void dualprt(DWELL *dwell, RADAR *radar, float *pptr) {
    int  i,j;   
    short        *abpptr;
    double       cp,p,cp1,cp2,p1,p2,v1,v2,vel,v,w1,w2,pcorrect,ratio;
    double       velconst,dbm,widthconst,range,rconst,scale2db,scale2ln;
    double       widthconst2;

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


    scale2ln = 0.004 * log(10.0) / 10.0;  //* from counts to natural log /
    scale2db = 0.004 / scale2ln;         //* from natural log to 10log10() /
#ifndef NEW_UNFOLD
    velconst = C / (2.0 * rhdr->frequency * 2.0 * fabs(hdr->prt - hdr->prt2) * 32768.0);
#endif   
#ifdef NEW_UNFOLD
    velconst = C / (2.0 * rhdr->frequency * 2.0 * fabs(hdr->prt2) * 32768.0);
#endif   
    rconst = rhdr->rconst - 20.0 * log10(rhdr->xmit_pulsewidth / hdr->rcvr_pulsewidth);
    pcorrect = rhdr->data_sys_sat
	- 20.0 * log10(0x10000) 
	- rhdr->receiver_gain;
    //* NOTE: 0x10000 is just the standard offset for all systems /
    widthconst = (C / rhdr->frequency) / hdr->prt / (2.0 * sqrt(2.0) * M_PI);
    widthconst2 = (C / rhdr->frequency) / hdr->prt2 / (2.0 * sqrt(2.0) * M_PI);
    ratio = hdr->prt2 / hdr->prt;

# ifdef obsolete
    abpptr = dwell->abp;
# endif
    abpptr = (short *)pui->raw_data;
    range = 0.0;

    for(i=0; i<hdr->gates; i++) {
	if( hostIsLittleEndian() ) {
	    cp  = *abpptr++;
	    cp1 = cp;
	    v1  = *abpptr++;
	    p   = *abpptr++;
	    p1  = p;
	    cp2 = *abpptr++;
	    cp = cp + log(1 + exp(scale2ln * (cp2 - cp))) / scale2ln;  
	    v2  = *abpptr++;
	    p2  = *abpptr++;
	    p = p + log(1 + exp(scale2ln * (p2 - p))) / scale2ln;  
	}
	else {
//	    cp  = SX2(*abpptr++);  //Jul 26, 2011
	    cp  = SX2(*(twob*)abpptr++);  //Jul 26, 2011
	    cp1 = cp;
//	    v1  = SX2(*abpptr++);  //Jul 26, 2011
//	    p   = SX2(*abpptr++);  //Jul 26, 2011
	    v1  = SX2(*(twob*)abpptr++);  //Jul 26, 2011
	    p   = SX2(*(twob*)abpptr++);  //Jul 26, 2011
	    p1  = p;
//	    cp2 = SX2(*abpptr++);  //Jul 26, 2011
	    cp2 = SX2(*(twob*)abpptr++);  //Jul 26, 2011
	    cp = cp + log(1 + exp(scale2ln * (cp2 - cp))) / scale2ln;  
//	    v2  = SX2(*abpptr++);  //Jul 26, 2011
//	    p2  = SX2(*abpptr++);  //Jul 26, 2011
	    v2  = SX2(*(twob*)abpptr++);  //Jul 26, 2011
	    p2  = SX2(*(twob*)abpptr++);  //Jul 26, 2011
	    p = p + log(1 + exp(scale2ln * (p2 - p))) / scale2ln;  
	}

	if(i)     range = 20.0 * log10(i * 0.0005 * C * hdr->rcvr_pulsewidth);

	//* compute floating point, scaled, scientific products /

#ifndef NEW_UNFOLD
	vel = v1 - v2;
	if(vel < -32768) vel += 65536.0;
	else if(vel >  32767) vel -= 65536.0;
#endif
#ifdef NEW_UNFOLD      
	vel = ratio*v2 - v1;
	if(ratio < 1.3)               //* 5/4 Ratio /
	    {
		if(vel > -8192.0 && vel < 8192.0) 
		    v = (v1 + ratio*v2)/2;
		else if(vel > -24576.0 && vel <= -8192.0)
		    v = (v1 + ratio*v2)/2 + NYQ_COUNT * (1.0 + ratio);
		else if(vel >= 8192.0 && vel < 24576.0)
		    v = (v1 + ratio*v2)/2 - NYQ_COUNT * (1.0 + ratio);
		else if(vel > -40960.0 && vel <= -24576.0)
		    v = (v1 + ratio*v2)/2 + 2.0 * NYQ_COUNT * (1.0 + ratio);
		else if(vel >= 24576.0 && vel < 40960.0)
		    v = (v1 + ratio*v2)/2 - 2.0 * NYQ_COUNT * (1.0 + ratio);
		else if(vel > -57344.0 && vel <= -40960.0)
		    v = (v1 + ratio*v2)/2 - NYQ_COUNT * (2.0 + ratio);
		else if(vel >= 40960.0 && vel < 57344.0)
		    v = (v1 + ratio*v2)/2 + NYQ_COUNT * (2.0 + ratio);
		else if(vel > -73728.0 && vel <= -57344.0)
		    v = (v1 + ratio*v2)/2 - NYQ_COUNT;
		else if(vel >= 57344.0 && vel < 73728.0)
		    v = (v1 + ratio*v2)/2 + NYQ_COUNT;
		else 
		    printf("what the ?%&\n");
	    }
	if(ratio < 1.6 && ratio > 1.4)                //* 3/2 Ratio /
	    {
		if(vel > -16384.0 && vel < 16384.0) 
		    v = (v1 + ratio*v2)/2;
		else if(vel > -49152.0 && vel <= -16384.0)
		    v = (v1 + ratio*v2)/2 + NYQ_COUNT * (1.0 + ratio);
		else if(vel >= 16384.0 && vel < 49152.0)
		    v = (v1 + ratio*v2)/2 - NYQ_COUNT * (1.0 + ratio);
		else if(vel > -81920.0 && vel <= -49152.0)
		    v = (v1 + ratio*v2)/2 - NYQ_COUNT;
		else if(vel >= 49152.0 && vel < 89120.0)
		    v = (v1 + ratio*v2)/2 + NYQ_COUNT;
		else 
		    printf("what the ?%&\n");
	    }
	if(ratio < 1.4 && ratio > 1.3)               //* 4/3 Ratio /
	    {
		if(vel > -10923.0 && vel < 10923.0) 
		    v = (v1 + ratio*v2)/2;
		else if(vel > -32768.0 && vel <= -10923.0)
		    v = (v1 + ratio*v2)/2 + NYQ_COUNT * (1.0 + ratio);
		else if(vel >= 10923.0 && vel < 32768.0)
		    v = (v1 + ratio*v2)/2 - NYQ_COUNT * (1.0 + ratio);
		else if(vel > -54613.0 && vel <= -32768.0)
		    v = (v1 + ratio*v2)/2 - NYQ_COUNT * (2.0 + ratio);
		else if(vel >= 32768.0 && vel < 54613.0)
		    v = (v1 + ratio*v2)/2 + NYQ_COUNT * (2.0 + ratio);
		else if(vel > -76459.0 && vel <= -54613.0)
		    v = (v1 + ratio*v2)/2 - NYQ_COUNT;
		else if(vel >= 54613.0 && vel < 76459.0)
		    v = (v1 + ratio*v2)/2 + NYQ_COUNT;
		else 
		    printf("what the ?%&\n");
	    }
	f = velconst * v;                   //* velocity in m/s /
#endif      
#ifndef NEW_UNFOLD      
	f = velconst * vel;                  //* velocity in m/s /
#endif      
      
	*velp++ = (short)DD_SCALE(f, scale, bias);

	w1 = sqrt(scale2ln * fabs(p1-cp1)) * widthconst2;  //* s.w.long PRT in m/s /
	*w1p++ = (short)DD_SCALE(w1, scale, bias);

# ifdef obsolete
	w2 = sqrt(scale2ln * fabs(p2-cp2)) * widthconst * ratio;  //* s.w. short PRT in m/s /
# else
	w2 = sqrt(scale2ln * fabs(p2-cp2)) * widthconst;  //* s.w. short PRT in m/s /
# endif
	*w2p++ = (short)DD_SCALE(w2, scale, bias);

# ifdef obsolete
	*pptr++ = dbm = 0.004 * p + pcorrect;      //* power in dBm /
	*pptr++ = exp(scale2ln*(cp - p));          //* NCP no units /
	*pptr++ = 0.5 * (w1 + w2);	         //* combined s.w. /
	*pptr++ = dbm + rconst + range;            //* in dBZ /
	*pptr++ = 0.004 * cp + pcorrect + rconst + range;  //* in dBZ /
	*pptr++ = v1 * velconst;                   //* velocity long PRT in m/s /
	*pptr++ = v2 * ratio * velconst;           //* velocity short PRT in m/s /
	*pptr++ = w1;                              //* s.w. long PRT in m/s /
	*pptr++ = w2;				 //* s.w. short PRT in m/s /

	//if(i == 200)
	    // printf("%3d %5.2f %5.2f %5.2f %5.2f %5.2f %5.2f\n",i,pptr[-6],pptr[-5],pptr[-4],pptr[-3],pptr[-2],pptr[-1]);
      
	pptr += 6;
# else
	f = dbm = 0.004 * p + pcorrect;      //* power in dBm /
	*dbmp++ = (short)DD_SCALE(f, scale, bias);
	f = exp(scale2ln*(cp - p));          //* NCP no units /
	*ncpp++ = (short)DD_SCALE(f, scale, bias);
	f = 0.5 * (w1 + w2);	         //* combined s.w. /
	*swp++ = (short)DD_SCALE(f, scale, bias);
	f = dbm + rconst + range;            //* in dBZ /
	*dbzp++ = (short)DD_SCALE(f, scale, bias);
	f = 0.004 * cp + pcorrect + rconst + range;  //* in dBZ /
	*dbzcp++ = (short)DD_SCALE(f, scale, bias);
	f = v1 * velconst;                   //* velocity long PRT in m/s /
	*v1p++ = (short)DD_SCALE(f, scale, bias);
	f = v2 * ratio * velconst;           //* velocity short PRT in m/s /
	*v2p++ = (short)DD_SCALE(f, scale, bias);
# endif
      
    }
}
/* c------------------------------------------------------------------------ */
/* c------------------------------------------------------------------------ */

void polypp(DWELL *dwell, RADARV *radar, float *prods)      
   {
   int  i;   
   float        *aptr,*pptr;
   double       a,b,p,a2,b2,cp,pcorrect,cp2;
   double       logstuff,noise,velconst;
   double       dbm,widthconst,range,rconst,width,r12,r22;
    short *velp=gri->scaled_data[0];
    short *dbmp=gri->scaled_data[1];
    short *ncpp=gri->scaled_data[2];
    short *swp=gri->scaled_data[3];
    short *dbzp=gri->scaled_data[4];
    short *dbzcp=gri->scaled_data[5];
    float f, scale=100., bias=0;

   rconst = radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / hdr->rcvr_pulsewidth);
   noise = (radar->noise_power > -10.0) ? 0.0 : 0.0;
   velconst = C / (2.0 * radar->frequency * 2.0 * M_PI * hdr->prt);
   pcorrect = radar->data_sys_sat
            - 20.0 * log10(0x2000000 * hdr->rcvr_pulsewidth / 1.25E-7) 
            - 10.0 * log10((double)hdr->hits)
            - radar->receiver_gain;
   widthconst = 0.33333333333333333 * (C / radar->frequency) / hdr->prt / (2.0 * sqrt(2.0) * M_PI);
   
# ifdef obsolete
   aptr = dwell->abp;
   pptr = prods;
# else
    aptr = (float *)((char *)dwell + sizeof(LeHEADER));
# endif
   range = 0.0;

   for(i=0; i<hdr->gates; i++)
      {
	 if(hostIsLittleEndian()) {
	    a = (*aptr++);
	    b = (*aptr++);
	    p = (*aptr++);
	    a2 = (*aptr++);
	    b2 = (*aptr++);
	 }
	 else {
//Jul 26, 2011 start
//	    a = SX4F(*aptr++);
//	    b = SX4F(*aptr++);
//	    p = SX4F(*aptr++);
//	    a2 = SX4F(*aptr++);
//	    b2 = SX4F(*aptr++);
	    a = SX4F(*(fourB*)aptr++);
	    b = SX4F(*(fourB*)aptr++);
	    p = SX4F(*(fourB*)aptr++);
	    a2 = SX4F(*(fourB*)aptr++);
	    b2 = SX4F(*(fourB*)aptr++);
//Jul 26, 2011 end	 	
	 }
      
      r12 = a * a + b * b;
      r22 = a2 * a2 + b2 * b2;
      cp = sqrt(r12) * pow(r12/r22,0.1666666666666666);

      if(i)     range = 20.0 * log10(i * 0.0005 * C * hdr->rcvr_pulsewidth);

      /* compute floating point, scaled, scientific products */

      f = velconst * atan2(-b,a);                 /* velocity in m/s */
      *velp++ = (short)DD_SCALE(f, scale, bias);

      f = dbm = 10.0 * log10(fabs(p)) + pcorrect;      /* power in dBm */
      *dbmp++ = (short)DD_SCALE(f, scale, bias);

      f = cp / p;                /* NCP no units */
      *ncpp++ = (short)DD_SCALE(f, scale, bias);

      if((width = 0.5 * log(r12/r22)) < 0.0) width = 0.0001;
      f = sqrt(width) * widthconst;  /* s.w. in m/s */
      *swp++ = (short)DD_SCALE(f, scale, bias);

      f = dbm + rconst + range;                          /* in dBZ */
      *dbzp++ = (short)DD_SCALE(f, scale, bias);

      f = 10.0 * log10(cp) + pcorrect + rconst + range;  /* in dBZ */
      *dbzcp++ = (short)DD_SCALE(f, scale, bias);


      pptr += 2;        /* 8 products total */
      }
   }
# ifdef notyet
/* c------------------------------------------------------------------------ */

void dualprt2(DWELL *dwell, RADARV *radar, float *prods)      
   {
   int  i;   
   float        *aptr,*pptr,*bptr;
   double       a,b,p,a2,b2,cp,pcorrect,biga,bigb;
   double       logstuff,noise,velconst;
   double       dbm,widthconst,range,rconst,width,r12;
 
   rconst = radar->rconst - 20.0 * log10(radar->xmit_pulsewidth / hdr->rcvr_pulsewidth);
   noise = (radar->noise_power > -10.0) ? 0.0 : 0.0;
   velconst = C / (2.0 * radar->frequency * 2.0 * M_PI * fabs(hdr->prt - hdr->prt2));
   pcorrect = radar->data_sys_sat
            - 20.0 * log10(0x2000000 * hdr->rcvr_pulsewidth / 1.25E-7) 
            - 10.0 * log10((double)hdr->hits)
            - radar->receiver_gain;
   widthconst = (C / radar->frequency) / hdr->prt / (2.0 * sqrt(2.0) * M_PI);
   
   aptr = dwell->abp;
   bptr = aptr + 3 * hdr->gates;
   pptr = prods;
   range = 0.0;

   for(i=0; i<hdr->gates; i++)
      {
	 if(hostIsLittleEndian()) {
	    a = *aptr++;
	    b = *aptr++;
	    p = *aptr++;
	    a2 = *bptr++;
	    b2 = *bptr++;
	    p += *bptr++;
	    p /= 2.0;      
	 }
	 else {
	 }

      if(i)     range = 20.0 * log10(i * 0.0005 * C * hdr->rcvr_pulsewidth);

      // compute floating point, scaled, scientific products 
      biga = a * a2 + b * b2;
      bigb = a2 * b - a * b2;
      
      *pptr++ = velconst * atan2(bigb,biga);                 //* velocity in m/s /
      *pptr++ = dbm = 10.0 * log10(fabs(p)) + pcorrect;      //* power in dBm /
      *pptr++ = (cp = sqrt(r12 = a*a+b*b))/p;                //* NCP no units /
      if((width = log(fabs((p-noise)/cp))) < 0.0) width = 0.0001;
      *pptr++ = sqrt(width) * widthconst;  //* s.w. in m/s /
      *pptr++ = dbm + rconst + range;                          //* in dBZ /
      *pptr++ = 10.0 * log10(fabs(cp)) + pcorrect + rconst + range;  //* in dBZ /
      pptr += 2;        //* 8 products total /
      }
   }
# endif
/* c------------------------------------------------------------------------ */

  void dualpol1(DWELL *dwell, RADARV *radar, float *prods)      
{
    int whichv, mark;
    int  i;   
    float        *aptr,*pptr,*bptr;
    double       a,b,p,a2,b2,p2,cp1,cp2,v,dp,pcorrect,biga,bigb;
    double       v1a,v1b,v2a,v2b,cp;
    double       logstuff,noise,velconst;
    double       dbm,widthconst,range,rconst,width,r12;
    short *velp=gri->scaled_data[0];
    short *dbmp=gri->scaled_data[1];
    short *ncpp=gri->scaled_data[2];
    short *swp=gri->scaled_data[3];
    short *dbzp=gri->scaled_data[4];
    short *dbzcp=gri->scaled_data[5];
    short *zdrp=gri->scaled_data[6];
    short *phip=gri->scaled_data[7];
    float f, scale=100., bias=0;


    rconst = radar->rconst - 20.0 * log10(radar->xmit_pulsewidth
					  / hdr->rcvr_pulsewidth);
    noise = (radar->noise_power > -10.0) ? 0.0 : 0.0;
    velconst = C / (2.0 * radar->frequency * 2.0 * M_PI
		    * fabs(hdr->prt - hdr->prt2));
    pcorrect = radar->data_sys_sat
	  - 20.0 * log10(0x2000000 * hdr->rcvr_pulsewidth / 1.25E-7) 
		- 10.0 * log10((double)hdr->hits)
		      - radar->receiver_gain;
    widthconst = (C / radar->frequency) / hdr->prt / (2.0 * sqrt(2.0) * M_PI);
   
# ifdef obsolete
    aptr = dwell->abp;
# endif
    aptr = (float *)((char *)dwell + sizeof(LeHEADER));
    bptr = aptr + 3 * hdr->gates;
    pptr = prods;
    range = 0.0;

    for(i=0; i < hdr->gates; i++) {
       if(hostIsLittleEndian()) {
	  a = (*aptr++);
	  b = (*aptr++);
	  p = (*aptr++);
	  a2 = (*bptr++);
	  b2 = (*bptr++);
	  p2 = (*bptr++);
       }
       else {
//Jul 26, 2011 start
//	  a = SX4F(*aptr++);
//	  b = SX4F(*aptr++);
//	  p = SX4F(*aptr++);
//	  a2 = SX4F(*bptr++);
//	  b2 = SX4F(*bptr++);
//	  p2 = SX4F(*bptr++);
	  a = SX4F(*(fourB*)aptr++);
	  b = SX4F(*(fourB*)aptr++);
	  p = SX4F(*(fourB*)aptr++);
	  a2 = SX4F(*(fourB*)bptr++);
	  b2 = SX4F(*(fourB*)bptr++);
	  p2 = SX4F(*(fourB*)bptr++);
//Jul 26, 2011 end
       }
	if(i)     range = 20.0 * log10(i * 0.0005 * C * hdr->rcvr_pulsewidth);

	/* compute floating point, scaled, scientific products */

	v1a = atan2(b,a) + radar->phaseoffset;
	if(v1a > M_PI) { v1a -= twoPI; }
	else if(v1a < -M_PI) { v1a += twoPI; }

	v2a = atan2(b2,a2) - radar->phaseoffset;
	if(v2a > M_PI) { v2a -= twoPI; }
	else if(v2a < -M_PI) { v2a += twoPI; }
      
	v1b += (v1a < 0.0) ? twoPI : -twoPI;
	v2b += (v2a < 0.0) ? twoPI : -twoPI;

	dp = 0.5 * (v1a - v2a); /* or maybe v1b - v2b */
# ifdef obsolete
	if(dp < (160.0 * M_PI / 180.0) && /* if this doesn't work, */
	   dp > (-20.0 * M_PI / 180.0)) /* then v1b - v2b won't either */
# else
	if(dp < RADIANS(160.0) && dp > RADIANS(-20.0))
	  /* if this doesn't work, then v1b - v2b won't either */
# endif
	      {                               
		  v = 0.5 * (v1a + v2a);
		  if(v > M_PI || v < -M_PI)
			v = 0.5 * (v1b + v2b);
		  whichv = 1;
	      }
	else			/* use the cross terms */   
	      {
		  dp = 0.5 * (v1a - v2b);
		  v = 0.5 * (v1a + v2b);
		  if(v > M_PI || v < -M_PI)
			v = 0.5 * (v1b + v2a);
		  whichv = 2;
	      }

	cp1 = sqrt(a * a + b * b); 
	cp2 = sqrt(a2 * a2 + b2 * b2);
	cp = cp1 + cp2;

	f = velconst * v;	/* velocity in m/s */
	*velp++ = (short)DD_SCALE(f, scale, bias);

	if(FABS(f) > 50. ) {
	    mark = 0;
	}

	f = dbm = 10.0 * log10(fabs(p + p2)) + pcorrect; /* power in dBm */
	*dbmp++ = (short)DD_SCALE(f, scale, bias);

	f = cp / (p + p2); /* NCP no units */
	*ncpp++ = (short)DD_SCALE(f, scale, bias);

	if((width = log(fabs((p-noise)/cp))) < 0.0) width = 0.0001;

	f = sqrt(width) * widthconst; /* s.w. in m/s */
	*swp++ = (short)DD_SCALE(f, scale, bias);

	f = dbm + rconst + range; /* in dBZ */
	*dbzp++ = (short)DD_SCALE(f, scale, bias);

	f = 10.0 * log10(fabs(cp)) + pcorrect + rconst + range; /* in dBZ */
	*dbzcp++ = (short)DD_SCALE(f, scale, bias);

	f = 10.0 * log10(p2/ p); /* Zdr */
	*zdrp++ = (short)DD_SCALE(f, scale, bias);

# ifdef obsolete
	f = dp * 180.0 / M_PI; /* PHI dp */
# else
	f = DEGREES(dp); /* PHI dp */
# endif
	*phip++ = (short)DD_SCALE(f, scale, bias);
    }
}
/* c------------------------------------------------------------------------ */

#define LOG2    0.69314718056

/* works off of log data */
void dualpol3(DWELL *dwell, RADARV *radar, float *prods) {
   int  ii, cnt;   
   short        *aptr, itemp;
   float        *pptr;
   double       a,b,p,a2,b2,p2,cp1,cp2,v,dp,pp,pcorrect,biga,bigb;
   double       v1a,v1b,v2a,v2b,theta,psi_d1,cp,ph_off;
   double       logstuff,noise,velconst,lag2;
   double       dbm,widthconst,range,rconst,width,r12,phv,lncp;
   double       scale2ln,scale2db,zdroffset,ldroffset=0;
   double       d, rng_const, vcnvt, ratio;
   double       thePhaseOffset;

   short *velp=gri->scaled_data[ve_ndx];
   short *dbmp=gri->scaled_data[dbm_ndx];
   short *ncpp=gri->scaled_data[ncp_ndx];
   short *swp=gri->scaled_data[sw_ndx];
   short *dbzp=gri->scaled_data[dz_ndx];
   short *dbzcp=gri->scaled_data[dzc_ndx];
   short *zdrp=gri->scaled_data[zdr_ndx];
   short *phip=gri->scaled_data[phi_ndx];
   short *rhohv=gri->scaled_data[rhohv_ndx];
   short *kdp=gri->scaled_data[kdp_ndx];
   short *ldr=gri->scaled_data[ldr_ndx];
   short *dbmv=gri->scaled_data[dbmv_ndx];
   float f, scale=100., bias=0;
   float * nLut;

   /* c...erik */

   if( pui->options & DUMP_TIME_SERIES ) {
       piraq_ts_stats();
   }

   scale2ln = 0.004 * log(10.0) / 10.0;	/* from counts to natural log */
   scale2db = 0.004 / scale2ln;	/* from natural log to 10log10() */

   rconst = radar->rconst - 20.0 *
     log10(radar->xmit_pulsewidth/ hdr->rcvr_pulsewidth)
       + 10.0 * log10(radar->peak_power/hdr->hxmit_power);
   h_rconst = (float)rconst;

# ifdef obsolete
   noise = (radar->noise_power > -10.0) ? 0.0 : 0.0;
# endif

   velconst = C / (2.0 * radar->frequency * 2.0 * M_PI * fabs(hdr->prt));

   pcorrect = radar->data_sys_sat - 20.0 * log10(0x10000) 
     - radar->receiver_gain + 10.0 * log10(2.0) ;

   /* latest noise power correction 2/28/97 per Mitch */

   noise = (radar->noise_power > -10.0) ? 0.0 :
       exp((double)((radar->noise_power - pcorrect)/scale2db));

   do_Noise_lut( noise, scale2ln );

   nLut = pui->Noise_lut;
   widthconst = (C / radar->frequency) / hdr->prt / (4.0 * sqrt(2.0) * M_PI);

   /* set phase offset to be 10 deg less than system phase */

   ph_off = (-20.0) * M_PI/180.0;

   thePhaseOffset = radar->phaseoffset;

   if(pui->time > P970127 && pui->time < P970129) {
       thePhaseOffset += RADIANS(180.);
   }

   zdroffset = 2.0 * (radar->vantenna_gain - radar->antenna_gain) 
     + radar->vreceiver_gain - radar->receiver_gain
       + 10.0 * log10(hdr->vxmit_power / hdr->hxmit_power);

   rng_const = log10(0.0005 * C * hdr->rcvr_pulsewidth);
   range = 0.0;
   vcnvt = M_PI / 32768.0;

   aptr = (short *)((char *)dwell + sizeof(LeHEADERV));
   aptr = (short *)pui->raw_data;
   
   for(ii=0; ii < hdr->gates; ii++) {
      if(hostIsLittleEndian()) {
# ifdef obsolete
	 cp1  = (*aptr++) * scale2ln;
	 v1a  = (*aptr++) * vcnvt;
	 p    = (*aptr++) * scale2ln;
	 cp2  = (*aptr++) * scale2ln;
	 v2a  = (*aptr++) * vcnvt;
	 p2   = (*aptr++) * scale2ln;
	 lag2 = (*aptr++) * scale2ln;
	 phv  = (*aptr++) * scale2ln;
# else
	 cp1  = (*aptr++) * scale2ln;
	 v1a  = (*aptr++) * vcnvt;
	 p = *(nLut + (unsigned short)(*aptr++));
	 cp2  = (*aptr++) * scale2ln;
	 v2a  = (*aptr++) * vcnvt;
	 p2 = *(nLut + (unsigned short)(*aptr++));
	 lag2 = (*aptr++) * scale2ln;
	 phv = *(nLut + (unsigned short)(*aptr++));
# endif	 
      }
      else {
# ifdef obsolete
	 cp1  = SX2(*aptr++) * scale2ln;
	 v1a  = SX2(*aptr++) * vcnvt;
	 p    = SX2(*aptr++) * scale2ln;
	 cp2  = SX2(*aptr++) * scale2ln;
	 v2a  = SX2(*aptr++) * vcnvt;
	 p2   = SX2(*aptr++) * scale2ln;
	 lag2 = SX2(*aptr++) * scale2ln;
	 phv  = SX2(*aptr++) * scale2ln;
# else
//Jul 26, 2011 start
/*	 cp1  = SX2(*aptr++) * scale2ln;
	 v1a  = SX2(*aptr++) * vcnvt;
	 cnt = SX2(*aptr++);
	 p = *(nLut + (unsigned short)cnt);
	 cp2  = SX2(*aptr++) * scale2ln;
	 v2a  = SX2(*aptr++) * vcnvt;
	 p2 = *(nLut + (unsigned short)SX2(*aptr++));
	 lag2 = SX2(*aptr++) * scale2ln;
	 phv = *(nLut + (unsigned short)SX2(*aptr++));
*/
	 cp1  = SX2(*(twob*)aptr++) * scale2ln;
	 v1a  = SX2(*(twob*)aptr++) * vcnvt;
	 cnt = SX2(*(twob*)aptr++);
	 p = *(nLut + (unsigned short)cnt);
	 cp2  = SX2(*(twob*)aptr++) * scale2ln;
	 v2a  = SX2(*(twob*)aptr++) * vcnvt;
	 p2 = *(nLut + (unsigned short)SX2(*(twob*)aptr++));
	 lag2 = SX2(*(twob*)aptr++) * scale2ln;
	 phv = *(nLut + (unsigned short)SX2(*(twob*)aptr++));
//Jul 26, 2011 end	 
# endif
      }

# ifdef info_only
      /* more new stuff 2//28/97 */
      p   = log( exp(   p ) - noise );
      p2  = log( exp(  p2 ) - noise );
      phv = log( exp( phv ) - noise );
# endif
      /* compute floating point, scaled, scientific products */
      
      v1a -= thePhaseOffset;

      if(v1a > M_PI)
	{ v1a -= twoPI; }
      else if (v1a < -M_PI)
	{ v1a += twoPI; }
      
      v2a += thePhaseOffset;

      if(v2a > M_PI)
	{ v2a -= twoPI; }
      else if (v2a < -M_PI)
	{ v2a += twoPI; }

      theta = (v2a - v1a) * 0.5;

      if (theta > M_PI*0.5)
	{ theta -= M_PI; }
      else if (theta < -M_PI* 0.5)
	{ theta += M_PI; }
      
      if(theta < ph_off)
	{ theta += M_PI; }

      dp = theta;
      v = v2a - dp;

      if (v < -M_PI)
	{ v += twoPI; }
      else if (v > M_PI)
	{ v -= twoPI; }

# ifdef info_only
      /* original code */
      cp = cp1 + mlog(1 + exp(cp2 - cp1));
      pp =  p  + mlog(1 + exp( p2 -  p ));
      *(pptr + 0) = velconst * v; /* velocity in m/s */
      *(pptr + 1) = dbm = p2 * scale2db + pcorrect; /* power in dBm */
      *(pptr + 2) = exp(lncp = (lag2 - pp));
      *(pptr + 3) = widthconst * sqrt(-lncp); /* s.w. in m/s */
      *(pptr + 4) = dbm + rconst + range; /* in dBZ */
      *(pptr + 5) = cp2 * scale2db + pcorrect + rconst + range;	/* in dBZ */
      *(pptr + 6) = (p2 - p) * scale2db + zdroffset; /* Zdr */
      *(pptr + 7) = dp * 180.0 / M_PI; /* PHI dp */
      *(pptr + 8) = exp(cp - (LOG2 + 0.5 * (p + p2) + 0.25 * lncp)); /* RHOhv */
      *(pptr + 9) = 0.0;	/* place holder for KDP */
      //if(!i)  printf("%8.4e  %8.4e  %8.4e  %8.4e  %8.4e  %8.4e\n",cp,pp,p,p2,lncp,lag2);
# endif

      cp = cp1 + LOGN(1 + exp(cp2 - cp1));
      pp =  p  + LOGN(1 + exp( p2 -  p ));

      f = velconst * v;		/* velocity in m/s */
      *velp++ = (short)DD_SCALE(f, scale, bias);

      f = dbm = p2 * scale2db + pcorrect; /* power in dBm */
      *dbmp++ = (short)DD_SCALE(f, scale, bias);

      lncp = (lag2 - pp);
      f = exp(lncp);
      *ncpp++ = (short)DD_SCALE(f, scale, bias);

      f = widthconst * sqrt(-lncp); /* s.w. in m/s */
      *swp++ = (short)DD_SCALE(f, scale, bias);

      f = dbm + rconst + range;	/* in dBZ */
      *dbzp++ = (short)DD_SCALE(f, scale, bias);

      f = cp2 * scale2db + pcorrect + rconst + range; /* in dBZ */
      *dbzcp++ = (short)DD_SCALE(f, scale, bias);

      f = (p2 - p) * scale2db + zdroffset + pui->zdr_offset_corr; /* Zdr */
      *zdrp++ = (short)DD_SCALE(f, scale, bias);

      f = DEGREES(dp);		/* PHI dp */
      *phip++ = (short)DD_SCALE(f, scale, bias);

      f = exp(cp - (LOG2 + 0.5 * (p + p2) + 0.25 * lncp)); /* RHOhv */
      *rhohv++ = (short)DD_SCALE(f, gri->dd_scale[rhohv_ndx]
			  , gri->dd_offset[rhohv_ndx]);

      f = (phv - p2) * scale2db + ldroffset + pui->ldr_offset_corr; /* LDR */
      *ldr++ = (short)DD_SCALE(f, scale, bias);

# ifdef obsolete
      f = p * scale2db + pcorrect +
	  (radar->receiver_gain - radar->vreceiver_gain); /* co-pole */
# else
      f = phv * scale2db + pcorrect; /* cross-pole */
# endif
      *dbmv++ = (short)DD_SCALE(f, scale, bias); /* vertical power "DL" */

      range = 20.0 * (LOG10(ii+1) + rng_const);

   }
}
/* c------------------------------------------------------------------------ */

/* works off of log data */
void dualpol12(DWELL *dwellness, RADARV *radar, float *prods) {

   int  ii, gg, cnt, num_gates;   
   short        *aptr, itemp;
   float        *pptr;

   short *velp=gri->scaled_data[ve_ndx];
   short *dbmp=gri->scaled_data[dbm_ndx];
   short *ncpp=gri->scaled_data[ncp_ndx];
   short *swp=gri->scaled_data[sw_ndx];
   short *dbzp=gri->scaled_data[dz_ndx];
   short *dbzcp=gri->scaled_data[dzc_ndx];
   short *zdrp=gri->scaled_data[zdr_ndx];
   short *phip=gri->scaled_data[phi_ndx];
   short *rhohv=gri->scaled_data[rhohv_ndx];
   short *kdp=gri->scaled_data[kdp_ndx];
   short *ldr=gri->scaled_data[ldr_ndx];
   short *dbmv=gri->scaled_data[dbmv_ndx];
   short *dbmx=gri->scaled_data[dbmx_ndx];
   short *hrho;
   short *hang;
   short *vrho;
   short *vang;
   float f, scale=100., bias=0;
   float * hLut;
   float * vLut;
   float * cLut;

   HEADERV *dwel = hdr;
   short pv, ph, hv;

   /* c...pol12 */

    double       h_channel_radar_constant,v_channel_radar_constant;
    double       angle_to_velocity_scale_factor;
    double       horiz_offset_to_dBm,verti_offset_to_dBm;
    double       cp1,v1a,lnpv,cp2,v2a,lnph,lnhv,lag2;
    double       ph_off,temp,theta,dp,lncp,v;
    double       horiz_dBm_at_coupler,verti_dBm_at_coupler;
    double       horiz_coherent_dBm_at_coupler;
    double       cross_dBm_at_coupler,laghh_dBm_at_coupler;
    double       hchan_noise_power,vchan_noise_power,lncoherent;
    double       widthconst,range_correction,coher_noise_power;
    double       h_rho, h_ang, v_rho, v_ang, d;
    double       scale2ln, scale2db, rcpScale2ln;
    static double prev_hchan_noise_power=10.e22;
    static double prev_vchan_noise_power=10.e22;
    static double prev_coher_noise_power=10.e22;
    
    if(( dwel->dataformat == DATA_POL12 ) && pui->options & FULL_MATRIX ) {
	hrho=gri->scaled_data[hrho_ndx];
	hang=gri->scaled_data[hang_ndx];
	vrho=gri->scaled_data[vrho_ndx];
	vang=gri->scaled_data[vang_ndx];
    }
    
    scale2ln = 0.004 * log(10.0) / 10.0;	/* from counts to natural log */
    rcpScale2ln = 1./scale2ln;
    scale2db = 0.004 / scale2ln;	/* from natural log to 10log10() */
    
    h_channel_radar_constant 
	= rhdr->rconst - 20.0 * log10(rhdr->xmit_pulsewidth / dwel->rcvr_pulsewidth)
	+ 10.0 * log10(rhdr->peak_power / dwel->hxmit_power);
    h_rconst = (float)h_channel_radar_constant;

    v_channel_radar_constant 
	= rhdr->rconst - 20.0 * log10(rhdr->xmit_pulsewidth / dwel->rcvr_pulsewidth)
	+ 10.0 * log10(rhdr->peak_power / dwel->vxmit_power)
	+ 2.0 * (rhdr->antenna_gain - rhdr->vantenna_gain);
    v_rconst = (float)v_channel_radar_constant;

    angle_to_velocity_scale_factor
	= C / (2.0 * rhdr->frequency * 2.0 * M_PI * dwel->prt);

    horiz_offset_to_dBm = rhdr->data_sys_sat - 20.0 * log10(0x10000) 
	- rhdr->receiver_gain + 10.0 * log10(2.0);  /* correct for half power measurement */

    verti_offset_to_dBm = rhdr->data_sys_sat - 20.0 * log10(0x10000) 
	- rhdr->vreceiver_gain + 10.0 * log10(2.0); /* correct for half power measurement */

    widthconst = (C / rhdr->frequency) / dwel->prt / (4.0 * sqrt(2.0) * M_PI);

    ph_off = 20.0 * M_PI / 180.0; /* set phase offset to be 20 deg */


    /* these powers reflect the LNA and waveguide performance */
    /* they cannot be broken down into co an cross powers */
    hchan_noise_power = (rhdr->noise_power > -10.0) ? 0.0 : exp((rhdr->noise_power - horiz_offset_to_dBm) / scale2db);
    vchan_noise_power = (rhdr->vnoise_power > -10.0) ? 0.0 : exp((rhdr->vnoise_power - verti_offset_to_dBm) / scale2db);
    coher_noise_power = exp((-129.0 - verti_offset_to_dBm) / scale2db);

    if( fabs((double)( hchan_noise_power - prev_hchan_noise_power)) > .1  ||
	fabs((double)( vchan_noise_power - prev_vchan_noise_power)) > .1  ||
	fabs((double)( coher_noise_power - prev_coher_noise_power)) > .1  ) {
	if( !pui->hNoise_lut ) {
	    pui->hNoise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->hNoise_lut, 0, K64 * sizeof(float));
	    pui->vNoise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->vNoise_lut, 0, K64 * sizeof(float));
	    pui->cNoise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->cNoise_lut, 0, K64 * sizeof(float));
	}
	do_Noise_lut12( hchan_noise_power, scale2ln, pui->hNoise_lut );
	do_Noise_lut12( vchan_noise_power, scale2ln, pui->vNoise_lut );
	do_Noise_lut12( coher_noise_power, scale2ln, pui->cNoise_lut );
	prev_hchan_noise_power = hchan_noise_power;
	prev_vchan_noise_power = vchan_noise_power;
	prev_coher_noise_power = coher_noise_power;
    }
    hLut = pui->hNoise_lut;
    vLut = pui->vNoise_lut;
    cLut = pui->cNoise_lut;

    range_correction = 0.0;
    aptr = (short *)pui->raw_data;
    num_gates = dwel->gates;

    for(gg = 0; gg < num_gates; gg++ ) {
	if(hostIsLittleEndian()) {
	    /* read in the raw data from structure */
	    cp1  = (unsigned int)(*aptr++) * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
	    v1a  = (*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
	    pv = (*aptr++);
	    lnpv = pv * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
	    cp2  = (*aptr++) * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
	    v2a  = (*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
	    ph = (*aptr++);
	    lnph = ph * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
	    lag2 = (*aptr++) * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
	    hv = (*aptr++);
	    lnhv = hv * scale2ln;        /* natural log  of V power on H xmit pulse */
	    if( dwel->dataformat == DATA_POL12 ) {
		h_rho = (*aptr++) * scale2ln;
		h_ang = (*aptr++) * M_PI / 32768.0;
		v_rho = (*aptr++) * scale2ln;
		v_ang = (*aptr++) * M_PI / 32768.0;
	    }
	}
	else {
//Jul 26, 2011 start
//	    cp1  = SX2(*aptr++) * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
//	    v1a  = SX2(*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
//	    pv   = SX2(*aptr++);
//	    lnpv = pv * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
//	    cp2  = SX2(*aptr++) * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
//	    v2a  = SX2(*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
//	    ph   = SX2(*aptr++);
//	    lnph = ph * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
//	    lag2 = SX2(*aptr++) * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
//	    hv   = SX2(*aptr++);
//	    lnhv = hv * scale2ln;        /* natural log  of V power on H xmit pulse */
//	    if( dwel->dataformat == DATA_POL12 ) {
//		h_rho = SX2(*aptr++) * scale2ln;
//		h_ang = SX2(*aptr++) * M_PI / 32768.0;
//		v_rho = SX2(*aptr++) * scale2ln;
//		v_ang = SX2(*aptr++) * M_PI / 32768.0;
	
	    cp1  = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
	    v1a  = SX2(*(twob*)aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
	    pv   = SX2(*(twob*)aptr++);
	    lnpv = pv * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
	    cp2  = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
	    v2a  = SX2(*(twob*)aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
	    ph   = SX2(*(twob*)aptr++);
	    lnph = ph * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
	    lag2 = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
	    hv   = SX2(*(twob*)aptr++);
	    lnhv = hv * scale2ln;        /* natural log  of V power on H xmit pulse */
	    if( dwel->dataformat == DATA_POL12 ) {
		h_rho = SX2(*(twob*)aptr++) * scale2ln;
		h_ang = SX2(*(twob*)aptr++) * M_PI / 32768.0;
		v_rho = SX2(*(twob*)aptr++) * scale2ln;
		v_ang = SX2(*(twob*)aptr++) * M_PI / 32768.0;
//Jul 26, 2011 end		
	    }
	}

	lncoherent = (cp1 + log(1 + exp(cp2 - cp1)) - LOG2);  /* natural log  of coherent power */

	/* NCP */
	/* it is best if this parameter is computed before noise correction */
        lncp = (lag2 - lnpv - log(1 + exp(lnph -  lnpv)));
	d = exp(lncp);
	*ncpp++ = (short)DD_SCALE(d, scale, bias);

	/* h/v cross correlation RHOhv (normalized dB) */
	/* it is best if this parameter is computed before noise correction */
	d = exp(cp1 + log(1 + exp(cp2 - cp1)) - (LOG2 + 0.5 * (lnph + lnpv) + 0.25 * lncp));
	*rhohv++ = (short)DD_SCALE(d, gri->dd_scale[rhohv_ndx]
			    , gri->dd_offset[rhohv_ndx]);

	if( dwel->dataformat == DATA_POL12 && pui->options & FULL_MATRIX ) {
	    /* cross correlation rho horizontal */
	    d = exp( h_rho -0.5 * ( lnhv + lnph ));
	    *hrho++ = (short)DD_SCALE(d, scale, bias);
	    *hang++ = (short)DD_SCALE(DEGREES( h_ang ), scale, bias);
	    
	    
	    /*	// cross correlation rho vertical	 */
	    d = exp( v_rho -0.5 * ( lnhv + lnpv ));
	    *vrho++ = (short)DD_SCALE(d, scale, bias);
	    *vang++ = (short)DD_SCALE(DEGREES( v_ang ), scale, bias);
	}
# ifdef obsolete
	/* subtract raw noise power from the raw log powers */
	temp = exp(lnph) - hchan_noise_power;       lnph = temp < 0.0 ? SMALL : log(temp);   /* corrected h chan power */
	temp = exp(lnpv) - vchan_noise_power;       lnpv = temp < 0.0 ? SMALL : log(temp);   /* corrected v chan power */
	temp = exp(lnhv) - vchan_noise_power;       lnhv = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */
	temp = exp(lncoherent) - coher_noise_power;       lncoherent = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */
# else
	lnph = *( hLut + (unsigned short)ph );
	lnpv = *( vLut + (unsigned short)pv );
	lnhv = *( vLut + (unsigned short)hv );
	cnt = (int)( lncoherent * rcpScale2ln ) & 0xffff; /* 16 unsigned int */
	lncoherent = *(cLut + cnt );
# endif
	
	/* convert the raw log powers to dBm at the test pulse waveguide coupler */
	horiz_dBm_at_coupler          = lnph * scale2db + horiz_offset_to_dBm;    /* HH  power in dBm */
	horiz_coherent_dBm_at_coupler = lncoherent * scale2db + horiz_offset_to_dBm;    /* HH  coherent power in dBm */
	verti_dBm_at_coupler          = lnpv * scale2db + verti_offset_to_dBm;    /* VV  power in dBm */
	cross_dBm_at_coupler          = lnhv * scale2db + horiz_offset_to_dBm;    /* HV  power in dBm (V rcvd on H xmit) */
	laghh_dBm_at_coupler          = lag2 * scale2db + horiz_offset_to_dBm;    /* HH lag2 power in dBm */
      
	/* compute range correction in dB. Skip the first gate */
	if(gg)     range_correction = 20.0 * log10(gg * 0.0005 * C * dwel->rcvr_pulsewidth);
	
	/* subtract out the system phase from v1a */
	v1a -= rhdr->phaseoffset;
	if(v1a < -M_PI)   v1a += 2.0 * M_PI;
	else if(v1a > M_PI)   v1a -= 2.0 * M_PI;
      
	/* add in the system phase to v2a */
	v2a += rhdr->phaseoffset;
	if(v2a < -M_PI)       v2a += 2.0 * M_PI;
	else if(v2a > M_PI)   v2a -= 2.0 * M_PI;
      
	/* compute the total difference */
	theta = v2a - v1a;
	if (theta > M_PI)         theta -= 2.0 * M_PI;
	else if (theta < -M_PI)   theta += 2.0 * M_PI;      
      
	/* figure the differential phase (from - 20 to +160) */
	dp = theta * 0.5;
	if (dp < -ph_off)   dp += M_PI;
	/* note: dp cannot be greater than +160, range is +/- 90 */        

	/* compute the velocity */
	v = v1a + dp;
	if (v < -M_PI)       v += 2.0 * M_PI;
	else if (v > M_PI)   v -= 2.0 * M_PI;
      
	/* velocity in m/s */
	d = v * angle_to_velocity_scale_factor;
	*velp++ = (short)DD_SCALE(d, scale, bias);

	/* horizontal power in dBm at test pulse coupler */
	*dbmp++ = (short)DD_SCALE(horiz_dBm_at_coupler, scale, bias);

	/* this space intentionaly left blank */
	/* NCP is computed above, before noise correction */

	/* spectral width in m/s */
	if( lncp > 0.0)   lncp = 0.0;
	d = widthconst * sqrt(-lncp);
	*swp++ = (short)DD_SCALE(d, scale, bias);

	/* horizontal reflectivity in dBZ */
	d = horiz_dBm_at_coupler + h_channel_radar_constant
	  + range_correction + pui->rconst_correction;
	*dbzp++ = (short)DD_SCALE(d, scale, bias);

	/* horizontal coherent reflectivity in dBZ */
	d = horiz_coherent_dBm_at_coupler + h_channel_radar_constant + range_correction;
	*dbzcp++ = (short)DD_SCALE(d, scale, bias);

	/* differential reflectivity Zdr in dB */
	d = horiz_dBm_at_coupler + h_channel_radar_constant -
	    verti_dBm_at_coupler - v_channel_radar_constant +
	    rhdr->zdr_fudge_factor + pui->zdr_offset_corr;
	*zdrp++ = (short)DD_SCALE(d, scale, bias);

	/* differential phase PHI dp in degrees */
	d = dp * 180.0 / M_PI;
	*phip++ = (short)DD_SCALE(d, scale, bias);

	/* linear depolarization LDR in dB */ 
	d = 
# ifdef obsolete
	    cross_dBm_at_coupler + v_channel_radar_constant - horiz_dBm_at_coupler - h_channel_radar_constant;
# else
	    cross_dBm_at_coupler - horiz_dBm_at_coupler + rhdr->antenna_gain - rhdr->vantenna_gain;
# endif
	*ldr++ = (short)DD_SCALE(d, scale, bias);

	/* this space intentionaly left blank */
	/* RHOhv is computed above, before noise correction */

	/* v power in dBm */ 
	d = verti_dBm_at_coupler;
	*dbmv++ = (short)DD_SCALE(d, scale, bias); /* vertical power "DL" */

	/* V reflectivity in dBZ */
	/*
	*(dbzv0 +gg) = verti_dBm_at_coupler + v_channel_radar_constant + range_correction;
	 */

	/* crosspolar power in dBm */
	d = cross_dBm_at_coupler;
	*dbmx++ = (short)DD_SCALE(d, scale, bias); /* cross pole power "DX" */
    }
}
/* c------------------------------------------------------------------------ */
#define LOG2            0.69314718056
#define STARTGATE       20      /* start gate for power average of all gates */
/* c------------------------------------------------------------------------ */
/***************************************************************************

	   FULL DUAL POLARIZATION MATRIX FROM ALTERNATING HV

		EXPECTS 15 PARAMETERS IN THE ABP ARRAY

special comments:
	1) the H and V gains must be matched with hardware because
	   the DSP's average HH and VV lag2 together.
	2) that means the two gains to adjust in software are copol
	   and cross pol, however they are labeled receiver_gain and
	   vreceiver_gain respectively.

calibration steps:
	1) Set H and V attenuators for 10 - 14 dB noise above floor
	2) Set receiver_gain and vreceiver_gain to the same value
	3) Inject a horizontal and vertical test pulse of known power.
	4) Adjust H and V IF gains for equal power
	5) Adjust the software receiver_gain to get the correct Phv power.
	6) Adjust the hardware copol attenuator to get the correct H power.
	7) Adjust the software vreceiver_gain to get the correct V Power.
****************************************************************************/
/* c------------------------------------------------------------------------ */
void fullpolplus(DWELL *dwellness, RADARV *radar, float *prods) {

    short *velp=gri->scaled_data[ve_ndx];
    short *dbmp=gri->scaled_data[dbm_ndx];
    short *ncpp=gri->scaled_data[ncp_ndx];
    short *swp=gri->scaled_data[sw_ndx];
    short *dbzp=gri->scaled_data[dz_ndx];
    short *dbzcp=gri->scaled_data[dzc_ndx];
    short *zdrp=gri->scaled_data[zdr_ndx];
    short *phip=gri->scaled_data[phi_ndx];
    short *rhohv=gri->scaled_data[rhohv_ndx];
    short *kdp=gri->scaled_data[kdp_ndx];
    short *ldr=gri->scaled_data[ldr_ndx];
    short *dbmv=gri->scaled_data[dbmv_ndx];
    short *dbmx=gri->scaled_data[dbmx_ndx];
    short *hrho;
    short *hang;
    short *vrho;
    short *vang;
    short *ldrv;
    short *iq_norm;
    short *iq_ang;
    float f, scale=100., bias=0;
    float * hLut;
    float * vLut;
    float * cLut;

    HEADERV *dwel = hdr;
    short pv, ph, hv, vh;
    double d, rcpScale2ln;
    int  ii, cnt, num_gates;   
    int          avecount;   
    short        itemp;
    short        *aptr;
    float        *gate0;
    double       scale2ln,scale2db;
    double       h_channel_radar_constant,v_channel_radar_constant;
    double       angle_to_velocity_scale_factor;
    double       horiz_offset_to_dBm,verti_offset_to_dBm;
    double       cp1,v1a,lnpv,cp2,v2a,lnph,lnhv,lag2;
    double       lncrhv,vcrhv,lncrvh,vcrvh;
    double       ph_off,temp,theta,dp,lncp,v;
    double       horiz_dBm_at_coupler,verti_dBm_at_coupler;
    double       horiz_coherent_dBm_at_coupler;
    double       crosshv_dBm_at_coupler,crossvh_dBm_at_coupler,laghh_dBm_at_coupler;
    double       hchan_noise_power,vchan_noise_power,lncoherent;
    double       widthconst,range_correction,coher_noise_power;
    double       linear_h_power,linear_v_power,average_h_power,average_v_power;
    double       lnvh,lniq,phiq;
    double       linear_hvcross_mag,average_real_hvcross,average_imag_hvcross;
    double       linear_vhcross_mag,average_real_vhcross,average_imag_vhcross;

    static double prev_hchan_noise_power=10.e22;
    static double prev_vchan_noise_power=10.e22;
    static double prev_coher_noise_power=10.e22;

    if( pui->options & FULL_MATRIX ) {
	hrho=gri->scaled_data[hrho_ndx];
	hang=gri->scaled_data[hang_ndx];
	vrho=gri->scaled_data[vrho_ndx];
	vang=gri->scaled_data[vang_ndx];
	ldrv=gri->scaled_data[ldrv_ndx];
	iq_norm=gri->scaled_data[iq_norm_ndx];
	iq_ang=gri->scaled_data[iq_ang_ndx];
    }

    /* initialize the things used for h and v power average */
    average_h_power = average_v_power = 0.0;
    /* initialize the things used for hv antenna isolation average */
    average_real_hvcross = average_imag_hvcross = 0.0;
    average_real_vhcross = average_imag_vhcross = 0.0;
    gate0 = prods;
    avecount = 0;

    scale2ln = 0.004 * log(10.0) / 10.0;  /* from counts to natural log */
    scale2db = 0.004 / scale2ln;         /* from natural log to 10log10() */
    rcpScale2ln = 1./scale2ln;

    h_channel_radar_constant 
	= rhdr->rconst - 20.0 * log10(rhdr->xmit_pulsewidth / dwel->rcvr_pulsewidth)
	+ 10.0 * log10(rhdr->peak_power / dwel->hxmit_power);
    h_channel_radar_constant 
	= rhdr->rconst
	+ 10.0 * log10(rhdr->peak_power / dwel->hxmit_power);
    h_rconst = (float)h_channel_radar_constant;

    v_channel_radar_constant 
	= rhdr->rconst - 20.0 * log10(rhdr->xmit_pulsewidth / dwel->rcvr_pulsewidth)
	+ 10.0 * log10(rhdr->peak_power / dwel->vxmit_power)
	+ 2.0 * (rhdr->antenna_gain - rhdr->vantenna_gain);
    v_channel_radar_constant 
	= rhdr->rconst
	+ 10.0 * log10(rhdr->peak_power / dwel->vxmit_power)
	+ 2.0 * (rhdr->antenna_gain - rhdr->vantenna_gain);
    v_rconst = (float)v_channel_radar_constant;

    angle_to_velocity_scale_factor
	= C / (2.0 * rhdr->frequency * 2.0 * M_PI * dwel->prt);

    horiz_offset_to_dBm = rhdr->data_sys_sat - 20.0 * log10(0x10000) 
	- rhdr->receiver_gain + 10.0 * log10(2.0);  /* correct for half power measurement */

    verti_offset_to_dBm = rhdr->data_sys_sat - 20.0 * log10(0x10000) 
	- rhdr->vreceiver_gain + 10.0 * log10(2.0); /* correct for half power measurement */

    widthconst = (C / rhdr->frequency) / dwel->prt / (4.0 * sqrt(2.0) * M_PI);

    ph_off = 20.0 * M_PI / 180.0; /* set phase offset to be 20 deg */

# ifdef obsolete
    aptr = dwel->abp;
# else
    aptr = (short *)pui->raw_data;
    num_gates = dwel->gates;
# endif
    range_correction = 0.0;

	/* these powers reflect the LNA and waveguide performance */
	/* they cannot be broken down into co an cross powers */
    hchan_noise_power = (rhdr->noise_power > -10.0) ? 0.0 : exp((rhdr->noise_power - horiz_offset_to_dBm) / scale2db);
    vchan_noise_power = (rhdr->vnoise_power > -10.0) ? 0.0 : exp((rhdr->vnoise_power - verti_offset_to_dBm) / scale2db);
    coher_noise_power = exp((-129.0 - verti_offset_to_dBm) / scale2db);

    if( fabs((double)( hchan_noise_power - prev_hchan_noise_power)) > .1  ||
	fabs((double)( vchan_noise_power - prev_vchan_noise_power)) > .1  ||
	fabs((double)( coher_noise_power - prev_coher_noise_power)) > .1  ) {
	if( !pui->hNoise_lut ) {
	    pui->hNoise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->hNoise_lut, 0, K64 * sizeof(float));
	    pui->vNoise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->vNoise_lut, 0, K64 * sizeof(float));
	    pui->cNoise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->cNoise_lut, 0, K64 * sizeof(float));
	}
	do_Noise_lut15( hchan_noise_power, scale2ln, pui->hNoise_lut );
	do_Noise_lut15( vchan_noise_power, scale2ln, pui->vNoise_lut );
	do_Noise_lut15( coher_noise_power, scale2ln, pui->cNoise_lut );
	prev_hchan_noise_power = hchan_noise_power;
	prev_vchan_noise_power = vchan_noise_power;
	prev_coher_noise_power = coher_noise_power;
    }
    hLut = pui->hNoise_lut;
    vLut = pui->vNoise_lut;
    cLut = pui->cNoise_lut;

    range_correction = 0.0;
    aptr = (short *)pui->raw_data;
    num_gates = dwel->gates;


   
    for(ii=0; ii < num_gates; ii++) {
# ifdef obsolete
	/* read in the raw data from structure */
	cp1  = *aptr++ * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
	v1a  = *aptr++ * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
	lnpv = *aptr++ * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
	cp2  = *aptr++ * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
	v2a  = *aptr++ * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
	lnph = *aptr++ * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
	lag2 = *aptr++ * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
	lnhv = *aptr++ * scale2ln;        /* natural log  of V power on H xmit pulse */

	lncrhv = *aptr++ * scale2ln;        /* natural log  of cross correlation on H xmit pulse */
	vcrhv  = *aptr++ * M_PI / 32768.0;  /* radian angle of cross correlation on H xmit pulse */

	lncrvh = *aptr++ * scale2ln;        /* natural log  of cross correlation on H xmit pulse */
	vcrvh  = *aptr++ * M_PI / 32768.0;  /* radian angle of cross correlation on H xmit pulse */
      
	/* below follows the "plus" parameters */

	lnvh = *aptr++ * scale2ln;        /* natural log  of H power on V xmit pulse */
	lniq = *aptr++ * scale2ln;        /* natural log  of average I and average Q from H pulses */
	phiq = *aptr++ * M_PI / 32768.0;  /* radian angle of average I and Q */
# endif
	if(hostIsLittleEndian()) {
	    /* read in the raw data from structure */
	    cp1  = (unsigned int)(*aptr++) * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
	    v1a  = (*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
	    pv = (*aptr++);
	    lnpv = pv * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
	    cp2  = (*aptr++) * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
	    v2a  = (*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
	    ph = (*aptr++);
	    lnph = ph * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
	    lag2 = (*aptr++) * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
	    hv = (*aptr++);
	    lnhv = hv * scale2ln;        /* natural log  of V power on H xmit pulse */

	    lncrhv = (*aptr++) * scale2ln; /* natural log of cross correlation on H xmit pulse */
	    vcrhv = (*aptr++) * M_PI / 32768.0;	/* radian angle of cross correlation of H xmit pulse */

	    lncrvh = (*aptr++) * scale2ln; /* natural log of cross correlation on V xmit pulse */
	    vcrvh = (*aptr++) * M_PI / 32768.0;	/* radian angle of cross correlation of V xmit pulse */

	    /* below follows the "plus" parameters */

	    vh = (*aptr++);
	    lnvh = vh * scale2ln;   /* natural log of H power on M xmit pulse */
	    lniq = (*aptr++) * scale2ln;   /* natural log of average I and average Q from H pulses */
	    phiq = (*aptr++) * M_PI / 32768.0;   /* radian angle of average I and Q */
	}
	else {
//Jul 26, 2011 start		
	    /* SX2() does byte swapping */
//	    cp1  = SX2(*aptr++) * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
//	    v1a  = SX2(*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
//	    pv   = SX2(*aptr++);
//	    lnpv = pv * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
//	    cp2  = SX2(*aptr++) * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
//	    v2a  = SX2(*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
//	    ph   = SX2(*aptr++);
//	    lnph = ph * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
//	    lag2 = SX2(*aptr++) * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
//	    hv   = SX2(*aptr++);
//	    lnhv = hv * scale2ln;        /* natural log  of V power on H xmit pulse */

//	    lncrhv = SX2(*aptr++) * scale2ln; /* natural log of cross correlation on H xmit pulse */
//	    vcrhv = SX2(*aptr++) * M_PI / 32768.0; /* radian angle of cross correlation of H xmit pulse */

//	    lncrvh = SX2(*aptr++) * scale2ln; /* natural log of cross correlation on V xmit pulse */
//	    vcrvh = SX2(*aptr++) * M_PI / 32768.0; /* radian angle of cross correlation of V xmit pulse */

	    /* below follows the "plus" parameters */

//	    vh = SX2(*aptr++);
//	    lnvh = vh * scale2ln;   /* natural log of H power on M xmit pulse */
//	    lniq = SX2(*aptr++) * scale2ln;   /* natural log of average I and average Q from H pulses */
//	    phiq = SX2(*aptr++) * M_PI / 32768.0;   /* radian angle of average I and Q */

	    cp1  = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
	    v1a  = SX2(*(twob*)aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
	    pv   = SX2(*(twob*)aptr++);
	    lnpv = pv * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
	    cp2  = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
	    v2a  = SX2(*(twob*)aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
	    ph   = SX2(*(twob*)aptr++);
	    lnph = ph * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
	    lag2 = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
	    hv   = SX2(*(twob*)aptr++);
	    lnhv = hv * scale2ln;        /* natural log  of V power on H xmit pulse */

	    lncrhv = SX2(*(twob*)aptr++) * scale2ln; /* natural log of cross correlation on H xmit pulse */
	    vcrhv = SX2(*(twob*)aptr++) * M_PI / 32768.0; /* radian angle of cross correlation of H xmit pulse */

	    lncrvh = SX2(*(twob*)aptr++) * scale2ln; /* natural log of cross correlation on V xmit pulse */
	    vcrvh = SX2(*(twob*)aptr++) * M_PI / 32768.0; /* radian angle of cross correlation of V xmit pulse */

	    /* below follows the "plus" parameters */

	    vh = SX2(*(twob*)aptr++);
	    lnvh = vh * scale2ln;   /* natural log of H power on M xmit pulse */
	    lniq = SX2(*(twob*)aptr++) * scale2ln;   /* natural log of average I and average Q from H pulses */
	    phiq = SX2(*(twob*)aptr++) * M_PI / 32768.0;   /* radian angle of average I and Q */
//Jul 26, 2011 end	    
	}
	lncoherent = (cp1 + log(1 + exp(cp2 - cp1)) - LOG2);  /* natural log  of coherent power */

	/* NCP */
	/* it is best if this parameter is computed before noise correction */
	lncp = (lag2 - lnpv - log(1 + exp(lnph -  lnpv)));
# ifdef obsolete
	prods[2] = exp(lncp);
# else
	d = exp(lncp);
	*ncpp++ = (short)DD_SCALE(d, scale, bias);
# endif
      
	/* h/v cross correlation RHOhv (normalized dB) */
	/* it is best if this parameter is computed before noise correction */
# ifdef obsolete
	prods[9] = exp(cp1 + log(1 + exp(cp2 - cp1)) - (LOG2 + 0.5 * (lnph + lnpv) + 0.25 * lncp));
# else
	d = exp(cp1 + log(1 + exp(cp2 - cp1)) - (LOG2 + 0.5 * (lnph + lnpv) + 0.25 * lncp));
	*rhohv++ = (short)DD_SCALE(d, gri->dd_scale[rhohv_ndx]
			    , gri->dd_offset[rhohv_ndx]);
# endif
      
	/* subtract raw noise power from the raw log powers */
# ifdef obsolete
	linear_h_power = exp(lnph) - hchan_noise_power;   if(linear_h_power <= 0.0)       linear_h_power = SMALL;       
	lnph = log(linear_h_power);   /* corrected h chan power */
	linear_v_power = exp(lnpv) - vchan_noise_power;   if(linear_v_power <= 0.0)       linear_v_power = SMALL;       
	lnpv = log(linear_v_power);   /* corrected v chan power */
	temp = exp(lnhv) - vchan_noise_power;       lnhv = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */
	temp = exp(lnvh) - hchan_noise_power;       lnvh = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */
	temp = exp(lncoherent) - coher_noise_power;       lncoherent = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */
# else
	/* log(SMALL) correction occurs in do_Noise_lut15() */
	lnph = *( hLut + (unsigned short)ph );
	lnvh = *( hLut + (unsigned short)vh );
	lnpv = *( vLut + (unsigned short)pv );
	lnhv = *( vLut + (unsigned short)hv );
	cnt = (int)( lncoherent * rcpScale2ln ) & 0xffff; /* 16 unsigned int */
	lncoherent = *(cLut + cnt );
# endif

	/* convert the raw log powers to dBm at the test pulse waveguide coupler */
	horiz_dBm_at_coupler          = lnph * scale2db + horiz_offset_to_dBm;    /* HH  power in dBm */
	horiz_coherent_dBm_at_coupler = lncoherent * scale2db + horiz_offset_to_dBm;    /* HH  coherent power in dBm */
	verti_dBm_at_coupler          = lnpv * scale2db + verti_offset_to_dBm;    /* VV  power in dBm */
	crosshv_dBm_at_coupler        = lnhv * scale2db + horiz_offset_to_dBm;    /* HV  power in dBm (H rcvd on V xmit) */
	crossvh_dBm_at_coupler        = lnvh * scale2db + verti_offset_to_dBm;    /* HV  power in dBm (V rcvd on H xmit) */
	laghh_dBm_at_coupler          = lag2 * scale2db + horiz_offset_to_dBm;    /* HH lag2 power in dBm */

# ifdef only_in_real_time
	if(ii >= STARTGATE)        /* only average past startgate */
	    {
		avecount++;
	 
		/* average h and v power for solar cal purposes */
		average_h_power += linear_h_power;
		average_v_power += linear_v_power;
	 
		/* compute the cross correlation thing used to check HV antenna isolation */
		linear_hvcross_mag = exp(lncrhv); /* linear phv magnitude */
		average_real_hvcross += linear_hvcross_mag * cos(vcrhv);
		average_imag_hvcross += linear_hvcross_mag * sin(vcrhv);
	 
		linear_vhcross_mag = exp(lncrvh); /* linear pvh magnitude */
		average_real_vhcross += linear_vhcross_mag * cos(vcrvh);
		average_imag_vhcross += linear_vhcross_mag * sin(vcrvh);
	    }
# endif

	/* compute range correction in dB. Skip the first gate */
	if(ii)     range_correction = 20.0 * log10(ii * 0.0005 * C * dwel->rcvr_pulsewidth);

	/* subtract out the system phase from v1a */
	v1a -= rhdr->phaseoffset;
	if(v1a < -M_PI)   v1a += 2.0 * M_PI;
	else if(v1a > M_PI)   v1a -= 2.0 * M_PI;
      
	/* add in the system phase to v2a */
	v2a += rhdr->phaseoffset;
	if(v2a < -M_PI)       v2a += 2.0 * M_PI;
	else if(v2a > M_PI)   v2a -= 2.0 * M_PI;
      
	/* compute the total difference */
	theta = v2a - v1a;
	if (theta > M_PI)         theta -= 2.0 * M_PI;
	else if (theta < -M_PI)   theta += 2.0 * M_PI;      
      
	/* figure the differential phase (from - 20 to +160) */
	dp = theta * 0.5;
	if (dp < -ph_off)   dp += M_PI;
	/* note: dp cannot be greater than +160, range is +/- 90 */        

	/* compute the velocity */
	v = v1a + dp;
	if (v < -M_PI)       v += 2.0 * M_PI;
	else if (v > M_PI)   v -= 2.0 * M_PI;
      
	/* velocity in m/s */
# ifdef obsolete
	prods[0] = v * angle_to_velocity_scale_factor;
# else
	d = v * angle_to_velocity_scale_factor;
	*velp++ = (short)DD_SCALE(d, scale, bias);
# endif

	if( pui->options & FULL_MATRIX ) {
	    /* cross correlation rho horizontal */
	    d = exp( lncrhv -0.5 * ( lnhv + lnph ));
	    *hrho++ = (short)DD_SCALE(d, scale, bias);
	    *hang++ = (short)DD_SCALE(DEGREES( vcrhv ), scale, bias);
	    
	    
	    /*	// cross correlation rho vertical	 */
	    d = exp( lncrvh -0.5 * ( lnpv + lnvh ));
	    *vrho++ = (short)DD_SCALE(d, scale, bias);
	    *vang++ = (short)DD_SCALE(DEGREES( vcrvh ), scale, bias);
	}

	/* horizontal power in dBm at test pulse coupler */
# ifdef obsolete
	prods[1] = horiz_dBm_at_coupler;
# else
	*dbmp++ = (short)DD_SCALE(horiz_dBm_at_coupler, scale, bias);
# endif

	if( pui->options & FULL_MATRIX ) {
	    /* IQ magnitude and angle */
	    d = lniq * scale2db + horiz_offset_to_dBm; /* magnitude of ave I Q */
	    *iq_norm++ = (short)DD_SCALE(d, scale, bias);
	    *iq_ang++ = (short)DD_SCALE(DEGREES( phiq ), scale, bias);
	}

	/* this space intentionaly left blank */
	/* NCP is computed above, before noise correction */

	/* spectral width in m/s */
	if( lncp > 0.0)   lncp = 0.0;
# ifdef obsolete
	prods[3] = widthconst * sqrt(-lncp);
# else
	d = widthconst * sqrt(-lncp);
	*swp++ = (short)DD_SCALE(d, scale, bias);
# endif

	/* horizontal reflectivity in dBZ */
# ifdef obsolete
	prods[4] = horiz_dBm_at_coupler + h_channel_radar_constant + range_correction;
# else
	d = horiz_dBm_at_coupler + h_channel_radar_constant + range_correction + pui->rconst_correction;
	*dbzp++ = (short)DD_SCALE(d, scale, bias);
# endif

# ifdef obsolete
	/* horizontal coherent reflectivity in dBZ */
	prods[5] = horiz_coherent_dBm_at_coupler + h_channel_radar_constant + range_correction;
# else
	d = horiz_coherent_dBm_at_coupler + h_channel_radar_constant + range_correction;
	*dbzcp++ = (short)DD_SCALE(d, scale, bias);
# endif


	/* differential reflectivity Zdr in dB */
# ifdef obsolete
	prods[6] = horiz_dBm_at_coupler + h_channel_radar_constant - verti_dBm_at_coupler - v_channel_radar_constant + rhdr->zdr_fudge_factor;
# else
	d = horiz_dBm_at_coupler + h_channel_radar_constant - verti_dBm_at_coupler - v_channel_radar_constant
	+ rhdr->zdr_fudge_factor + pui->zdr_offset_corr;
	*zdrp++ = (short)DD_SCALE(d, scale, bias);
# endif

	/* differential phase PHI dp in degrees */
# ifdef obsolete
	prods[7] = dp * 180.0 / M_PI;
# else
	d = dp * 180.0 / M_PI;
	*phip++ = (short)DD_SCALE(d, scale, bias);
# endif

	/* linear depolarization LDR in dB */
	
# ifdef obsolete
	prods[8] = (crosshv_dBm_at_coupler - rhdr->vantenna_gain) - (horiz_dBm_at_coupler - rhdr->antenna_gain); /* from H pulse LDRH */
# else
	d = (crosshv_dBm_at_coupler - rhdr->vantenna_gain) - (horiz_dBm_at_coupler - rhdr->antenna_gain); /* from H pulse LDRH */
	*ldr++ = (short)DD_SCALE(d, scale, bias);
# endif

	if( pui->options & FULL_MATRIX ) {
	    d = (crossvh_dBm_at_coupler - rhdr->antenna_gain) - (verti_dBm_at_coupler - rhdr->vantenna_gain); /* from V pulse LDRV "plus" parameter */
	    *ldrv++ = (short)DD_SCALE(d, scale, bias);	/* Ldr Vertical */
	}
	/* this space intentionaly left blank */
	/* RHOhv is computed above, before noise correction */

	/* v power in dBm */ 
# ifdef obsolete
	prods[10] = verti_dBm_at_coupler;
# else
	*dbmv++ = (short)DD_SCALE(verti_dBm_at_coupler, scale, bias); /* vertical power "DL" */
# endif

	/* V reflectivity in dBZ */
# ifdef obsolete
	prods[11] = verti_dBm_at_coupler + v_channel_radar_constant + range_correction;
# else
	/* not generated by xltrs */
# endif

	/* crosspolar power in dBm */
# ifdef obsolete
	prods[12] = crosshv_dBm_at_coupler;
# else
	d = crosshv_dBm_at_coupler;
	*dbmx++ = (short)DD_SCALE(d, scale, bias); /* cross pole power "DX" */
# endif
      
    }
# ifdef only_in_real_time
    /* now insert the average h and v power into the gate 0 data */
    /* this is only necessary for the realtime system */
    if(avecount == 0) return;
    gate0[ 1] = log(average_h_power / (double)avecount) * scale2db + horiz_offset_to_dBm;    /* average HH  power in dBm */
    gate0[10] = log(average_v_power / (double)avecount) * scale2db + verti_offset_to_dBm;    /* average VV  power in dBm */
    linear_hvcross_mag = average_real_hvcross * average_real_hvcross + average_imag_hvcross * average_imag_hvcross;
    linear_vhcross_mag = average_real_vhcross * average_real_vhcross + average_imag_vhcross * average_imag_vhcross;
    gate0[13] = log(0.5 * (linear_vhcross_mag + linear_hvcross_mag) / (average_h_power * average_h_power)) * scale2db;    /* H signal in V receiver */
    //   gate0[13] = log(0.5 * (linear_vhcross_mag + linear_hvcross_mag) / (average_v_power * average_v_power)) * scale2db;    /* V signal in H receiver */
# endif

}
/* c------------------------------------------------------------------------ */

/* works off of log data */
void dualpolplus(DWELL *dwellness, RADARV *radar, float *prods) {

   int  ii, gg, cnt, num_gates;   
   short        *aptr, itemp;
   float        *pptr;

   short *velp=gri->scaled_data[ve_ndx];
   short *dbmp=gri->scaled_data[dbm_ndx];
   short *ncpp=gri->scaled_data[ncp_ndx];
   short *swp=gri->scaled_data[sw_ndx];
   short *dbzp=gri->scaled_data[dz_ndx];
   short *dbzcp=gri->scaled_data[dzc_ndx];
   short *zdrp=gri->scaled_data[zdr_ndx];
   short *phip=gri->scaled_data[phi_ndx];
   short *rhohv=gri->scaled_data[rhohv_ndx];
   short *kdp=gri->scaled_data[kdp_ndx];
   short *ldr=gri->scaled_data[ldr_ndx];
   short *dbmv=gri->scaled_data[dbmv_ndx];
   short *dbmx=gri->scaled_data[dbmx_ndx];
   short *hrho;
   short *hang;
   short *vrho;
   short *vang;
   short *ldrv;
   short *iq_norm;
   short *iq_ang;
   float f, scale=100., bias=0;
   float * hLut;
   float * vLut;
   float * cLut;

   HEADERV *dwel = hdr;
   short pv, ph, hv, vh;

   /* c...polplus */

    double       h_channel_radar_constant,v_channel_radar_constant;
    double       angle_to_velocity_scale_factor;
    double       horiz_offset_to_dBm,verti_offset_to_dBm;
    double       cp1,v1a,lnpv,cp2,v2a,lnph,lnhv,lag2;
    double       ph_off,temp,theta,dp,lncp,v;
    double       horiz_dBm_at_coupler,verti_dBm_at_coupler;
    double       horiz_coherent_dBm_at_coupler;
    double       cross_dBm_at_coupler,laghh_dBm_at_coupler;
    double       hchan_noise_power,vchan_noise_power,lncoherent;
    double       widthconst,range_correction,coher_noise_power;
    double       h_rho, h_ang, v_rho, v_ang, d;
    double       scale2ln, scale2db, rcpScale2ln;
    double       crosshv_dBm_at_coupler,crossvh_dBm_at_coupler;
    double       lncrhv, vcrhv, lncrvh, vcrvh, lnvh, lniq, phiq;
    static double prev_hchan_noise_power=10.e22;
    static double prev_vchan_noise_power=10.e22;
    static double prev_coher_noise_power=10.e22;
    
    if( pui->options & FULL_MATRIX ) {
	hrho=gri->scaled_data[hrho_ndx];
	hang=gri->scaled_data[hang_ndx];
	vrho=gri->scaled_data[vrho_ndx];
	vang=gri->scaled_data[vang_ndx];
	ldrv=gri->scaled_data[ldrv_ndx];
	iq_norm=gri->scaled_data[iq_norm_ndx];
	iq_ang=gri->scaled_data[iq_ang_ndx];
    }
    scale2ln = 0.004 * log(10.0) / 10.0;	/* from counts to natural log */
    rcpScale2ln = 1./scale2ln;
    scale2db = 0.004 / scale2ln;	/* from natural log to 10log10() */
    
    h_channel_radar_constant 
	= rhdr->rconst - 20.0 * log10(rhdr->xmit_pulsewidth / dwel->rcvr_pulsewidth)
	+ 10.0 * log10(rhdr->peak_power / dwel->hxmit_power);

    v_channel_radar_constant 
	= rhdr->rconst - 20.0 * log10(rhdr->xmit_pulsewidth / dwel->rcvr_pulsewidth)
	+ 10.0 * log10(rhdr->peak_power / dwel->vxmit_power)
	+ 2.0 * (rhdr->antenna_gain - rhdr->vantenna_gain);

    angle_to_velocity_scale_factor
	= C / (2.0 * rhdr->frequency * 2.0 * M_PI * dwel->prt);

    horiz_offset_to_dBm = rhdr->data_sys_sat - 20.0 * log10(0x10000) 
	- rhdr->receiver_gain + 10.0 * log10(2.0);  /* correct for half power measurement */

    verti_offset_to_dBm = rhdr->data_sys_sat - 20.0 * log10(0x10000) 
	- rhdr->vreceiver_gain + 10.0 * log10(2.0); /* correct for half power measurement */

    widthconst = (C / rhdr->frequency) / dwel->prt / (4.0 * sqrt(2.0) * M_PI);

    ph_off = 20.0 * M_PI / 180.0; /* set phase offset to be 20 deg */


    /* these powers reflect the LNA and waveguide performance */
    /* they cannot be broken down into co an cross powers */
    hchan_noise_power = (rhdr->noise_power > -10.0) ? 0.0 : exp((rhdr->noise_power - horiz_offset_to_dBm) / scale2db);
    vchan_noise_power = (rhdr->vnoise_power > -10.0) ? 0.0 : exp((rhdr->vnoise_power - verti_offset_to_dBm) / scale2db);
    coher_noise_power = exp((-129.0 - verti_offset_to_dBm) / scale2db);

    if( fabs((double)( hchan_noise_power - prev_hchan_noise_power)) > .1  ||
	fabs((double)( vchan_noise_power - prev_vchan_noise_power)) > .1  ||
	fabs((double)( coher_noise_power - prev_coher_noise_power)) > .1  ) {
	if( !pui->hNoise_lut ) {
	    pui->hNoise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->hNoise_lut, 0, K64 * sizeof(float));
	    pui->vNoise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->vNoise_lut, 0, K64 * sizeof(float));
	    pui->cNoise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->cNoise_lut, 0, K64 * sizeof(float));
	}
	do_Noise_lut12( hchan_noise_power, scale2ln, pui->hNoise_lut );
	do_Noise_lut12( vchan_noise_power, scale2ln, pui->vNoise_lut );
	do_Noise_lut12( coher_noise_power, scale2ln, pui->cNoise_lut );
	prev_hchan_noise_power = hchan_noise_power;
	prev_vchan_noise_power = vchan_noise_power;
	prev_coher_noise_power = coher_noise_power;
    }
    hLut = pui->hNoise_lut;
    vLut = pui->vNoise_lut;
    cLut = pui->cNoise_lut;

    range_correction = 0.0;
    aptr = (short *)pui->raw_data;
    num_gates = dwel->gates;

    for(gg = 0; gg < num_gates; gg++ ) {
	if(hostIsLittleEndian()) {
	    /* read in the raw data from structure */
	    cp1  = (unsigned int)(*aptr++) * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
	    v1a  = (*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
	    pv = (*aptr++);
	    lnpv = pv * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
	    cp2  = (*aptr++) * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
	    v2a  = (*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
	    ph = (*aptr++);
	    lnph = ph * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
	    lag2 = (*aptr++) * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
	    hv = (*aptr++);
	    lnhv = hv * scale2ln;        /* natural log  of V power on H xmit pulse */

	    lncrhv = (*aptr++) * scale2ln; /* natural log of cross correlation on H xmit pulse */
	    vcrhv = (*aptr++) * M_PI / 32768.0;	/* radian angle of cross correlation of H xmit pulse */
	    lncrvh = (*aptr++) * scale2ln; /* natural log of cross correlation on V xmit pulse */
	    vcrvh = (*aptr++) * M_PI / 32768.0;	/* radian angle of cross correlation of V xmit pulse */

	    vh = (*aptr++);
	    lnvh = vh * scale2ln;   /* natural log of H power on M xmit pulse */
	    lniq = (*aptr++) * scale2ln;   /* natural log of average I and average Q from H pulses */
	    phiq = (*aptr++) * M_PI / 32768.0;   /* radian angle of average I and Q */
	}
	else {
//Jul 26, 2011 start	
//	    cp1  = SX2(*aptr++) * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
//	    v1a  = SX2(*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
//	    pv   = SX2(*aptr++);
//	    lnpv = pv * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
//	    cp2  = SX2(*aptr++) * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
//	    v2a  = SX2(*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
//	    ph   = SX2(*aptr++);
//	    lnph = ph * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
//	    lag2 = SX2(*aptr++) * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
//	    hv   = SX2(*aptr++);
//	    lnhv = hv * scale2ln;        /* natural log  of V power on H xmit pulse */

//	    lncrhv = SX2(*aptr++) * scale2ln; /* natural log of cross correlation on H xmit pulse */
//	    vcrhv = SX2(*aptr++) * M_PI / 32768.0; /* radian angle of cross correlation of H xmit pulse */
//	    lncrvh = SX2(*aptr++) * scale2ln; /* natural log of cross correlation on V xmit pulse */
//	    vcrvh = SX2(*aptr++) * M_PI / 32768.0; /* radian angle of cross correlation of V xmit pulse */

//	    vh = SX2(*aptr++);
//	    lnvh = vh * scale2ln;   /* natural log of H power on M xmit pulse */
//	    lniq = SX2(*aptr++) * scale2ln;   /* natural log of average I and average Q from H pulses */
//	    phiq = SX2(*aptr++) * M_PI / 32768.0;   /* radian angle of average I and Q */
	
	    cp1  = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
	    v1a  = SX2(*(twob*)aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
	    pv   = SX2(*(twob*)aptr++);
	    lnpv = pv * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
	    cp2  = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
	    v2a  = SX2(*(twob*)aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
	    ph   = SX2(*(twob*)aptr++);
	    lnph = ph * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
	    lag2 = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
	    hv   = SX2(*(twob*)aptr++);
	    lnhv = hv * scale2ln;        /* natural log  of V power on H xmit pulse */

	    lncrhv = SX2(*(twob*)aptr++) * scale2ln; /* natural log of cross correlation on H xmit pulse */
	    vcrhv = SX2(*(twob*)aptr++) * M_PI / 32768.0; /* radian angle of cross correlation of H xmit pulse */
	    lncrvh = SX2(*(twob*)aptr++) * scale2ln; /* natural log of cross correlation on V xmit pulse */
	    vcrvh = SX2(*(twob*)aptr++) * M_PI / 32768.0; /* radian angle of cross correlation of V xmit pulse */

	    vh = SX2(*(twob*)aptr++);
	    lnvh = vh * scale2ln;   /* natural log of H power on M xmit pulse */
	    lniq = SX2(*(twob*)aptr++) * scale2ln;   /* natural log of average I and average Q from H pulses */
	    phiq = SX2(*(twob*)aptr++) * M_PI / 32768.0;   /* radian angle of average I and Q */
//Jul 26, 2011 end	    
	}

	lncoherent = (cp1 + log(1 + exp(cp2 - cp1)) - LOG2);  /* natural log  of coherent power */

	/* NCP */
	/* it is best if this parameter is computed before noise correction */
        lncp = (lag2 - lnpv - log(1 + exp(lnph -  lnpv)));
	d = exp(lncp);
	*ncpp++ = (short)DD_SCALE(d, scale, bias);

	/* h/v cross correlation RHOhv (normalized dB) */
	/* it is best if this parameter is computed before noise correction */
	d = exp(cp1 + log(1 + exp(cp2 - cp1)) - (LOG2 + 0.5 * (lnph + lnpv) + 0.25 * lncp));
	*rhohv++ = (short)DD_SCALE(d, gri->dd_scale[rhohv_ndx]
			    , gri->dd_offset[rhohv_ndx]);

	if( pui->options & FULL_MATRIX ) {
	    /* cross correlation rho horizontal */
	    d = exp( lncrhv -0.5 * ( lnhv + lnph ));
	    *hrho++ = (short)DD_SCALE(d, scale, bias);
	    *hang++ = (short)DD_SCALE(DEGREES( vcrhv ), scale, bias);
	    
	    
	    /*	// cross correlation rho vertical	 */
	    d = exp( lncrvh -0.5 * ( lnpv + lnvh ));
	    *vrho++ = (short)DD_SCALE(d, scale, bias);
	    *vang++ = (short)DD_SCALE(DEGREES( vcrvh ), scale, bias);
	}

# ifdef obsolete
	/* subtract raw noise power from the raw log powers */
	temp = exp(lnph) - hchan_noise_power;       lnph = temp < 0.0 ? SMALL : log(temp);   /* corrected h chan power */
	temp = exp(lnpv) - vchan_noise_power;       lnpv = temp < 0.0 ? SMALL : log(temp);   /* corrected v chan power */
	temp = exp(lnhv) - vchan_noise_power;       lnhv = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */
	temp = exp(lncoherent) - coher_noise_power;       lncoherent = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */
# else
	lnph = *( hLut + (unsigned short)ph );
	lnpv = *( vLut + (unsigned short)pv );
	lnhv = *( vLut + (unsigned short)hv );
	cnt = (int)( lncoherent * rcpScale2ln ) & 0xffff; /* 16 unsigned int */
	lncoherent = *(cLut + cnt );
# endif
	
	/* convert the raw log powers to dBm at the test pulse waveguide coupler */
	/* HH  power in dBm */
	horiz_dBm_at_coupler          = lnph * scale2db + horiz_offset_to_dBm;
	/* HH  coherent power in dBm */
	horiz_coherent_dBm_at_coupler = lncoherent * scale2db + horiz_offset_to_dBm;
	/* VV  power in dBm */
	verti_dBm_at_coupler          = lnpv * scale2db + verti_offset_to_dBm;
	/* HV  power in dBm (H rcvd on V xmit) */
	crosshv_dBm_at_coupler        = lnhv * scale2db + horiz_offset_to_dBm;
	/* HH lag2 power in dBm */
	laghh_dBm_at_coupler          = lag2 * scale2db + horiz_offset_to_dBm;
      
	/* compute range correction in dB. Skip the first gate */
	if(gg)     range_correction = 20.0 * log10(gg * 0.0005 * C * dwel->rcvr_pulsewidth);
	
	/* subtract out the system phase from v1a */
	v1a -= rhdr->phaseoffset;
	if(v1a < -M_PI)   v1a += 2.0 * M_PI;
	else if(v1a > M_PI)   v1a -= 2.0 * M_PI;
      
	/* add in the system phase to v2a */
	v2a += rhdr->phaseoffset;
	if(v2a < -M_PI)       v2a += 2.0 * M_PI;
	else if(v2a > M_PI)   v2a -= 2.0 * M_PI;
      
# define mitch_and_eric

	/* compute the total difference */
# ifdef mitch_and_eric
	theta = v2a - v1a;
# else
	theta = (v2a - v1a) * 0.5;
# endif
	if (theta > M_PI)         theta -= 2.0 * M_PI;
	else if (theta < -M_PI)   theta += 2.0 * M_PI;      
      
	/* figure the differential phase (from - 20 to +160) */
# ifdef mitch_and_eric
	dp = theta * 0.5;
	if (dp < -ph_off)   dp += M_PI;
# else
	dp = theta;
# endif
	/* note: dp cannot be greater than +160, range is +/- 90 */        

	/* compute the velocity */
# ifdef mitch_and_eric
	v = v1a + dp;
# else
	v = (v1a + v2a) * 0.5;
# endif
	if (v < -M_PI)       v += 2.0 * M_PI;
	else if (v > M_PI)   v -= 2.0 * M_PI;
      
	/* velocity in m/s */
	d = v * angle_to_velocity_scale_factor;
	*velp++ = (short)DD_SCALE(d, scale, bias);

	/* horizontal power in dBm at test pulse coupler */
	*dbmp++ = (short)DD_SCALE(horiz_dBm_at_coupler, scale, bias);

	/* this space intentionaly left blank */
	/* NCP is computed above, before noise correction */

	/* spectral width in m/s */
	if( lncp > 0.0)   lncp = 0.0;
	d = widthconst * sqrt(-lncp);
	*swp++ = (short)DD_SCALE(d, scale, bias);

	/* horizontal reflectivity in dBZ */
	d = horiz_dBm_at_coupler + h_channel_radar_constant
	  + range_correction + pui->rconst_correction;
	*dbzp++ = (short)DD_SCALE(d, scale, bias);

	/* horizontal coherent reflectivity in dBZ */
	d = horiz_coherent_dBm_at_coupler + h_channel_radar_constant + range_correction;
	*dbzcp++ = (short)DD_SCALE(d, scale, bias);

	/* differential reflectivity Zdr in dB */
	d = horiz_dBm_at_coupler + h_channel_radar_constant -
	    verti_dBm_at_coupler - v_channel_radar_constant +
	    rhdr->zdr_fudge_factor + pui->zdr_offset_corr;
	*zdrp++ = (short)DD_SCALE(d, scale, bias);

	/* differential phase PHI dp in degrees */
	d = dp * 180.0 / M_PI;
	*phip++ = (short)DD_SCALE(d, scale, bias);

	/* linear depolarization LDR in dB */ 
# ifdef obsolete
	d = 
	    cross_dBm_at_coupler + v_channel_radar_constant - horiz_dBm_at_coupler - h_channel_radar_constant;
# else
	d = 
	    (crosshv_dBm_at_coupler - rhdr->vantenna_gain)
	    - (horiz_dBm_at_coupler - rhdr->antenna_gain);
# endif
	*ldr++ = (short)DD_SCALE(d, scale, bias);

	if( pui->options & FULL_MATRIX ) {
	    lnvh = *( hLut + (unsigned short)vh );
	    /* VH  power in dBm (V rcvd on H xmit) */
	    crossvh_dBm_at_coupler = lnvh * scale2db + verti_offset_to_dBm;
	    d = (crossvh_dBm_at_coupler - rhdr->antenna_gain)
		- (verti_dBm_at_coupler - rhdr->vantenna_gain);
	    *ldrv++ = (short)DD_SCALE(d, scale, bias);	/* Ldr Vertical */

	    /* IQ magnitude and angle */
	    d = lniq * 2.0 * scale2db;
	    *iq_norm++ = (short)DD_SCALE(d, scale, bias);
	    *iq_ang++ = (short)DD_SCALE(DEGREES( phiq ), scale, bias);
	}
	/* this space intentionaly left blank */
	/* RHOhv is computed above, before noise correction */

	/* v power in dBm */ 
	d = verti_dBm_at_coupler;
	*dbmv++ = (short)DD_SCALE(d, scale, bias); /* vertical power "DL" */

	/* V reflectivity in dBZ */
	/*
	*(dbzv0 +gg) = verti_dBm_at_coupler + v_channel_radar_constant + range_correction;
	 */

	/* crosspolar power in dBm */
	d = crosshv_dBm_at_coupler;
	*dbmx++ = (short)DD_SCALE(d, scale, bias); /* cross pole power "DX" */
    }
}
/* c------------------------------------------------------------------------ */

/* works off of log data */
void dualpolplusGf(DWELL *dwellness, RADARV *radar, float *prods) {

   int  ii, gg, cnt, num_gates, mark;   
   short        *aptr, itemp;
   float        *pptr;

   short *velp=gri->scaled_data[ve_ndx];
   short *dbmp=gri->scaled_data[dbm_ndx];
   short *ncpp=gri->scaled_data[ncp_ndx];
   short *swp=gri->scaled_data[sw_ndx];
   short *dbzp=gri->scaled_data[dz_ndx];
   short *dbzcp=gri->scaled_data[dzc_ndx];
   short *zdrp=gri->scaled_data[zdr_ndx];
   short *phip=gri->scaled_data[phi_ndx];
   short *rhohv=gri->scaled_data[rhohv_ndx];
   short *kdp=gri->scaled_data[kdp_ndx];
   short *ldr=gri->scaled_data[ldr_ndx];
   short *dbmv=gri->scaled_data[dbmv_ndx];
   short *dbmx=gri->scaled_data[dbmx_ndx];
   short *hrho;
   short *hang;
   short *vrho;
   short *vang;
   short *ldrv;
   short *iq_norm;
   short *iq_ang;
   float f, scale=100., bias=0;
   float * hLut;
   float * vLut;
   float * cLut;

   HEADERV *dwel = hdr;
   short pv, ph, hv, vh;

   /* c...polplus */

    double       h_channel_radar_constant,v_channel_radar_constant;
    double       angle_to_velocity_scale_factor;
    double       horiz_offset_to_dBm,verti_offset_to_dBm;
    double       cp1,v1a,lnpv,cp2,v2a,lnph,lnhv,lag2;
    double       ph_off,temp,theta,dp,lncp,v;
    double       horiz_dBm_at_coupler,verti_dBm_at_coupler;
    double       horiz_coherent_dBm_at_coupler;
    double       cross_dBm_at_coupler,laghh_dBm_at_coupler;
    double       hchan_noise_power,vchan_noise_power,lncoherent;
    double       widthconst,range_correction,coher_noise_power;
    double       h_rho, h_ang, v_rho, v_ang, d;
    double       scale2ln, scale2db, rcpScale2ln;
    double       crosshv_dBm_at_coupler,crossvh_dBm_at_coupler;
    double       lncrhv, vcrhv, lncrvh, vcrvh, lnvh, lniq, phiq;
    static double prev_hchan_noise_power=10.e22;
    static double prev_vchan_noise_power=10.e22;
    static double prev_coher_noise_power=10.e22;
    double dp_prev, dp_prev_prev, zdr_keep, zdr_prev, zdr_prev_prev;
    
    if( pui->options & FULL_MATRIX ) {
	hrho=gri->scaled_data[hrho_ndx];
	hang=gri->scaled_data[hang_ndx];
	vrho=gri->scaled_data[vrho_ndx];
	vang=gri->scaled_data[vang_ndx];
	ldrv=gri->scaled_data[ldrv_ndx];
	iq_norm=gri->scaled_data[iq_norm_ndx];
	iq_ang=gri->scaled_data[iq_ang_ndx];
    }
    scale2ln = 0.004 * log(10.0) / 10.0;	/* from counts to natural log */
    rcpScale2ln = 1./scale2ln;
    scale2db = 0.004 / scale2ln;	/* from natural log to 10log10() */
    
    h_channel_radar_constant 
	= rhdr->rconst - 20.0 * log10(rhdr->xmit_pulsewidth / dwel->rcvr_pulsewidth)
	+ 10.0 * log10(rhdr->peak_power / dwel->hxmit_power);

    v_channel_radar_constant 
	= rhdr->rconst - 20.0 * log10(rhdr->xmit_pulsewidth / dwel->rcvr_pulsewidth)
	+ 10.0 * log10(rhdr->peak_power / dwel->vxmit_power)
	+ 2.0 * (rhdr->antenna_gain - rhdr->vantenna_gain);

    angle_to_velocity_scale_factor
	= C / (2.0 * rhdr->frequency * 2.0 * M_PI * dwel->prt);

    horiz_offset_to_dBm = rhdr->data_sys_sat - 20.0 * log10(0x10000) 
	- rhdr->receiver_gain + 10.0 * log10(2.0);  /* correct for half power measurement */

    verti_offset_to_dBm = rhdr->data_sys_sat - 20.0 * log10(0x10000) 
	- rhdr->vreceiver_gain + 10.0 * log10(2.0); /* correct for half power measurement */

    widthconst = (C / rhdr->frequency) / dwel->prt / (4.0 * sqrt(2.0) * M_PI);

    ph_off = 20.0 * M_PI / 180.0; /* set phase offset to be 20 deg */


    /* these powers reflect the LNA and waveguide performance */
    /* they cannot be broken down into co an cross powers */
    hchan_noise_power = (rhdr->noise_power > -10.0) ? 0.0 : exp((rhdr->noise_power - horiz_offset_to_dBm) / scale2db);
    vchan_noise_power = (rhdr->vnoise_power > -10.0) ? 0.0 : exp((rhdr->vnoise_power - verti_offset_to_dBm) / scale2db);
    coher_noise_power = exp((-129.0 - verti_offset_to_dBm) / scale2db);

    if( fabs((double)( hchan_noise_power - prev_hchan_noise_power)) > .1  ||
	fabs((double)( vchan_noise_power - prev_vchan_noise_power)) > .1  ||
	fabs((double)( coher_noise_power - prev_coher_noise_power)) > .1  ) {
	if( !pui->hNoise_lut ) {
	    pui->hNoise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->hNoise_lut, 0, K64 * sizeof(float));
	    pui->vNoise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->vNoise_lut, 0, K64 * sizeof(float));
	    pui->cNoise_lut = (float *)malloc( K64 * sizeof(float));
	    memset(pui->cNoise_lut, 0, K64 * sizeof(float));
	}
	do_Noise_lut12( hchan_noise_power, scale2ln, pui->hNoise_lut );
	do_Noise_lut12( vchan_noise_power, scale2ln, pui->vNoise_lut );
	do_Noise_lut12( coher_noise_power, scale2ln, pui->cNoise_lut );
	prev_hchan_noise_power = hchan_noise_power;
	prev_vchan_noise_power = vchan_noise_power;
	prev_coher_noise_power = coher_noise_power;
    }
    hLut = pui->hNoise_lut;
    vLut = pui->vNoise_lut;
    cLut = pui->cNoise_lut;

    range_correction = 0.0;
    aptr = (short *)pui->raw_data;
    num_gates = dwel->gates;
    
    for(gg = 0; gg < num_gates; gg++ ) {
	if(hostIsLittleEndian()) {
	    /* read in the raw data from structure */
	    cp1  = (unsigned int)(*aptr++) * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
	    v1a  = (*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
	    pv = (*aptr++);
	    lnpv = pv * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
	    cp2  = (*aptr++) * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
	    v2a  = (*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
	    ph = (*aptr++);
	    lnph = ph * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
	    lag2 = (*aptr++) * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
	    hv = (*aptr++);
	    lnhv = hv * scale2ln;        /* natural log  of V power on H xmit pulse */

	    lncrhv = (*aptr++) * scale2ln; /* natural log of cross correlation on H xmit pulse */
	    vcrhv = (*aptr++) * M_PI / 32768.0;	/* radian angle of cross correlation of H xmit pulse */
	    lncrvh = (*aptr++) * scale2ln; /* natural log of cross correlation on V xmit pulse */
	    vcrvh = (*aptr++) * M_PI / 32768.0;	/* radian angle of cross correlation of V xmit pulse */

	    vh = (*aptr++);
	    lnvh = vh * scale2ln;   /* natural log of H power on M xmit pulse */
	    lniq = (*aptr++) * scale2ln;   /* natural log of average I and average Q from H pulses */
	    phiq = (*aptr++) * M_PI / 32768.0;   /* radian angle of average I and Q */
	}
	else {
//Jul 26, 2011 start
//	    cp1  = SX2(*aptr++) * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
//	    v1a  = SX2(*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
//	    pv   = SX2(*aptr++);
//	    lnpv = pv * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
//	    cp2  = SX2(*aptr++) * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
//	    v2a  = SX2(*aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
//	    ph   = SX2(*aptr++);
//	    lnph = ph * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
//	    lag2 = SX2(*aptr++) * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
//	    hv   = SX2(*aptr++);
//	    lnhv = hv * scale2ln;        /* natural log  of V power on H xmit pulse */

//	    lncrhv = SX2(*aptr++) * scale2ln; /* natural log of cross correlation on H xmit pulse */
//	    vcrhv = SX2(*aptr++) * M_PI / 32768.0; /* radian angle of cross correlation of H xmit pulse */
//	    lncrvh = SX2(*aptr++) * scale2ln; /* natural log of cross correlation on V xmit pulse */
//	    vcrvh = SX2(*aptr++) * M_PI / 32768.0; /* radian angle of cross correlation of V xmit pulse */

//	    vh = SX2(*aptr++);
//	    lnvh = vh * scale2ln;   /* natural log of H power on M xmit pulse */
//	    lniq = SX2(*aptr++) * scale2ln;   /* natural log of average I and average Q from H pulses */
//	    phiq = SX2(*aptr++) * M_PI / 32768.0;   /* radian angle of average I and Q */
		
	    cp1  = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of R(1) from H to V pulse pair */
	    v1a  = SX2(*(twob*)aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from H to V pulse pair */
	    pv   = SX2(*(twob*)aptr++);
	    lnpv = pv * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
	    cp2  = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of R(1) from V to H pulse pair */
	    v2a  = SX2(*(twob*)aptr++) * M_PI / 32768.0;  /* radian angle of R(1) from V to H pulse pair */
	    ph   = SX2(*(twob*)aptr++);
	    lnph = ph * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
	    lag2 = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of R(2) from H to H + R(2) from V to V */
	    hv   = SX2(*(twob*)aptr++);
	    lnhv = hv * scale2ln;        /* natural log  of V power on H xmit pulse */

	    lncrhv = SX2(*(twob*)aptr++) * scale2ln; /* natural log of cross correlation on H xmit pulse */
	    vcrhv = SX2(*(twob*)aptr++) * M_PI / 32768.0; /* radian angle of cross correlation of H xmit pulse */
	    lncrvh = SX2(*(twob*)aptr++) * scale2ln; /* natural log of cross correlation on V xmit pulse */
	    vcrvh = SX2(*(twob*)aptr++) * M_PI / 32768.0; /* radian angle of cross correlation of V xmit pulse */

	    vh = SX2(*(twob*)aptr++);
	    lnvh = vh * scale2ln;   /* natural log of H power on M xmit pulse */
	    lniq = SX2(*(twob*)aptr++) * scale2ln;   /* natural log of average I and average Q from H pulses */
	    phiq = SX2(*(twob*)aptr++) * M_PI / 32768.0;   /* radian angle of average I and Q */
//Jul 26, 2011 end	    
	}

	lncoherent = (cp1 + log(1 + exp(cp2 - cp1)) - LOG2);  /* natural log  of coherent power */

	/* NCP */
	/* it is best if this parameter is computed before noise correction */
        lncp = (lag2 - lnpv - log(1 + exp(lnph -  lnpv)));
	d = exp(lncp);
	*ncpp++ = (short)DD_SCALE(d, scale, bias);

	/* h/v cross correlation RHOhv (normalized dB) */
	/* it is best if this parameter is computed before noise correction */
	d = exp(cp1 + log(1 + exp(cp2 - cp1)) - (LOG2 + 0.5 * (lnph + lnpv) + 0.25 * lncp));
	*rhohv++ = (short)DD_SCALE(d, gri->dd_scale[rhohv_ndx]
			    , gri->dd_offset[rhohv_ndx]);

	if( pui->options & FULL_MATRIX ) {
	    /* cross correlation rho horizontal */
	    d = exp( lncrhv -0.5 * ( lnhv + lnph ));
	    *hrho++ = (short)DD_SCALE(d, scale, bias);
	    *hang++ = (short)DD_SCALE(DEGREES( vcrhv ), scale, bias);
	    
	    
	    /*	// cross correlation rho vertical	 */
	    d = exp( lncrvh -0.5 * ( lnpv + lnvh ));
	    *vrho++ = (short)DD_SCALE(d, scale, bias);
	    *vang++ = (short)DD_SCALE(DEGREES( vcrvh ), scale, bias);
	}

# ifdef obsolete
	/* subtract raw noise power from the raw log powers */
	temp = exp(lnph) - hchan_noise_power;       lnph = temp < 0.0 ? SMALL : log(temp);   /* corrected h chan power */
	temp = exp(lnpv) - vchan_noise_power;       lnpv = temp < 0.0 ? SMALL : log(temp);   /* corrected v chan power */
	temp = exp(lnhv) - vchan_noise_power;       lnhv = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */
	temp = exp(lncoherent) - coher_noise_power;       lncoherent = temp < 0.0 ? SMALL : log(temp);   /* corrected cross power */
# else
	lnph = *( hLut + (unsigned short)ph );
	lnpv = *( vLut + (unsigned short)pv );
	lnhv = *( vLut + (unsigned short)hv );
	cnt = (int)( lncoherent * rcpScale2ln ) & 0xffff; /* 16 unsigned int */
	lncoherent = *(cLut + cnt );
# endif
	
	/* convert the raw log powers to dBm at the test pulse waveguide coupler */
	/* HH  power in dBm */
	horiz_dBm_at_coupler          = lnph * scale2db + horiz_offset_to_dBm;
	/* HH  coherent power in dBm */
	horiz_coherent_dBm_at_coupler = lncoherent * scale2db + horiz_offset_to_dBm;
	/* VV  power in dBm */
	verti_dBm_at_coupler          = lnpv * scale2db + verti_offset_to_dBm;
	/* HV  power in dBm (H rcvd on V xmit) */
	crosshv_dBm_at_coupler        = lnhv * scale2db + horiz_offset_to_dBm;
	/* HH lag2 power in dBm */
	laghh_dBm_at_coupler          = lag2 * scale2db + horiz_offset_to_dBm;
      
	/* compute range correction in dB. Skip the first gate */
	if(gg)     range_correction = 20.0 * log10(gg * 0.0005 * C * dwel->rcvr_pulsewidth);
	
	/* subtract out the system phase from v1a */
	v1a -= rhdr->phaseoffset;
	if(v1a < -M_PI)   v1a += 2.0 * M_PI;
	else if(v1a > M_PI)   v1a -= 2.0 * M_PI;
      
	/* add in the system phase to v2a */
	v2a += rhdr->phaseoffset;
	if(v2a < -M_PI)       v2a += 2.0 * M_PI;
	else if(v2a > M_PI)   v2a -= 2.0 * M_PI;
      
# define not_mitch_and_eric

	/* compute the total difference */
# ifdef mitch_and_eric
	theta = v2a - v1a;
# else
	theta = (v2a - v1a) * 0.5;
# endif
	if (theta > M_PI)         theta -= 2.0 * M_PI;
	else if (theta < -M_PI)   theta += 2.0 * M_PI;      
      
	/* figure the differential phase (from - 20 to +160) */
# ifdef mitch_and_eric
	dp = theta * 0.5;
	if (dp < -ph_off)   dp += M_PI;
# else
	dp = theta;
# endif
	/* note: dp cannot be greater than +160, range is +/- 90 */        

	/* compute the velocity */
# ifdef mitch_and_eric
	v = v1a + dp;
# else
	v = (v1a + v2a) * 0.5;
# endif
	
	/* horizontal power in dBm at test pulse coupler */
	*dbmp++ = (short)DD_SCALE(horiz_dBm_at_coupler, scale, bias);

	/* this space intentionaly left blank */
	/* NCP is computed above, before noise correction */

	/* spectral width in m/s */
	if( lncp > 0.0)   lncp = 0.0;
	d = widthconst * sqrt(-lncp);
	*swp++ = (short)DD_SCALE(d, scale, bias);

	/* horizontal reflectivity in dBZ */
	d = horiz_dBm_at_coupler + h_channel_radar_constant
	  + range_correction + pui->rconst_correction;
	*dbzp++ = (short)DD_SCALE(d, scale, bias);

	/* horizontal coherent reflectivity in dBZ */
	d = horiz_coherent_dBm_at_coupler + h_channel_radar_constant + range_correction;
	*dbzcp++ = (short)DD_SCALE(d, scale, bias);

	/* differential reflectivity Zdr in dB */
	zdr_keep = d = horiz_dBm_at_coupler + h_channel_radar_constant -
	    verti_dBm_at_coupler - v_channel_radar_constant +
	    rhdr->zdr_fudge_factor + pui->zdr_offset_corr;
	*zdrp++ = (short)DD_SCALE(d, scale, bias);

# ifdef mitch_and_eric
# else
	if( gg > 1 &&  dp < -ph_off ) {
	    dp += M_PI;
	    v += M_PI;
	    if( horiz_dBm_at_coupler > -70. &&
		( fabs( zdr_prev - zdr_prev_prev ) > 1.0 ||
		fabs( zdr_keep - zdr_prev ) >  1.0 ||
		fabs( dp_prev - dp_prev_prev ) > M_PI * .5 ||
		fabs( dp - dp_prev ) >  M_PI * .5 )) {
		dp += M_PI;
		v += M_PI;
	    }
	}
	zdr_prev_prev = zdr_prev;
	zdr_prev = zdr_keep;
	dp_prev_prev = dp_prev;
	dp_prev = dp;
# endif
	if (v < -M_PI)       v += 2.0 * M_PI;
	else if (v > M_PI)   v -= 2.0 * M_PI;

	/* velocity in m/s */
	d = v * angle_to_velocity_scale_factor;
	*velp++ = (short)DD_SCALE(d, scale, bias);

	/* differential phase PHI dp in degrees */
	d = dp * 180.0 / M_PI;
	*phip++ = (short)DD_SCALE(d, scale, bias);

	/* linear depolarization LDR in dB */ 
# ifdef obsolete
	d = 
	    cross_dBm_at_coupler + v_channel_radar_constant - horiz_dBm_at_coupler - h_channel_radar_constant;
# else
	d = 
	    (crosshv_dBm_at_coupler - rhdr->vantenna_gain)
	    - (horiz_dBm_at_coupler - rhdr->antenna_gain);
# endif
	*ldr++ = (short)DD_SCALE(d, scale, bias);

	if( pui->options & FULL_MATRIX ) {
	    lnvh = *( hLut + (unsigned short)vh );
	    /* VH  power in dBm (V rcvd on H xmit) */
	    crossvh_dBm_at_coupler = lnvh * scale2db + verti_offset_to_dBm;
	    d = (crossvh_dBm_at_coupler - rhdr->antenna_gain)
		- (verti_dBm_at_coupler - rhdr->vantenna_gain);
	    *ldrv++ = (short)DD_SCALE(d, scale, bias);	/* Ldr Vertical */

	    /* IQ magnitude and angle */
	    d = lniq * 2.0 * scale2db;
	    *iq_norm++ = (short)DD_SCALE(d, scale, bias);
	    *iq_ang++ = (short)DD_SCALE(DEGREES( phiq ), scale, bias);
	}
	/* this space intentionaly left blank */
	/* RHOhv is computed above, before noise correction */

	/* v power in dBm */ 
	d = verti_dBm_at_coupler;
	*dbmv++ = (short)DD_SCALE(d, scale, bias); /* vertical power "DL" */

	/* V reflectivity in dBZ */
	/*
	*(dbzv0 +gg) = verti_dBm_at_coupler + v_channel_radar_constant + range_correction;
	 */

	/* crosspolar power in dBm */
	d = crosshv_dBm_at_coupler;
	*dbmx++ = (short)DD_SCALE(d, scale, bias); /* cross pole power "DX" */
    }
}
/* c------------------------------------------------------------------------ */

int 
piraq_ts_stats (void)
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

    switch(hdr->dataformat) {
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
    len = hdr->recordlen - sizeof( LeHEADERV ) - ( hdr->gates * sizeof_gate );
    kk = len/sizeof(float);

    if( kk < hdr->hits ) {
	if( sizeof_ts_fi > 0 ) {
	    fclose( ts_fid );
	    sizeof_ts_fi = 0;
	}
	return(1);//return;  //Jul 26, 2011
    }

    if( !ray_count++ ) {
	aa = message;
	sprintf( aa, "/scr/hotlips/oye/spol/ts.txt" );
	/* open the ascii file */
	ts_fid = fopen( message, "w" );
    }

    if( fabs( (double)( hdr->prt - prev_prt ) ) > .001 ) {
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
	prev_prt = hdr->prt;
	sumhh = sumvv = sumhv = 0;
	sumSqhh = sumSqvv = sumSqhv = 0;
	N = 0;
    }

    fpx = (float *)( pui->raw_data + hdr->gates * num_fields * sizeof(short) );
    N += hdr->hits;
    aa = message;
    sprintf( aa
	     , "beam: %d tsgate: %d hits: %d prt: %.4f az: %.2f el: %.2f %d \n"
	     , ray_count, hdr->tsgate, hdr->hits, hdr->prt, hdr->az, hdr->el
	     , len );
    fputs( message, ts_fid );
    sizeof_ts_fi += sizeof( message );

    for( ii = 0; ii < hdr->hits >> 1; ii++ ) {
      
	aa = message;

//	I = SX4F( *fpx++ );  //Jul 26, 2011
//	Q = SX4F( *fpx++ );  //Jul 26, 2011
	I = SX4F( *(fourB*)fpx++ );  //Jul 26, 2011
	Q = SX4F( *(fourB*)fpx++ );  //Jul 26, 2011
	sprintf( aa, " %4d %10.2f %10.2f ", ii, I, Q );
	xx = sqrt( I*I + Q*Q );
	sumhh += xx;
	sumSqhh += xx*xx;

//	I = SX4F( *fpx++ );  //Jul 26, 2011
//	Q = SX4F( *fpx++ );  //Jul 26, 2011
	I = SX4F( *(fourB*)fpx++ );  //Jul 26, 2011
	Q = SX4F( *(fourB*)fpx++ );  //Jul 26, 2011
	aa += strlen(aa);
	sprintf( aa, " %10.2f %10.2f ", I, Q );
	xx = sqrt( I*I + Q*Q );
	sumvv += xx;
	sumSqvv += xx*xx;

# ifdef obsolete
	I = SX4F( *fpx++ );
	Q = SX4F( *fpx++ );
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

// c---------------------------------------------------------------------------


# ifdef obsolete
// 
//  piraq.h contains defs for PIRAQX 
//
//  viraq.h contains defs for LeRADARV and LeHEADERV
//

#include <string.h>

#include "piraq/piraq.h"
#include "viraq/viraq.h"
#include "dorade/dd_defines.h"

#include "update_prqx.h"
# endif

// c------------------------------------------------------------------------
// c------------------------------------------------------------------------

short snarf2 (char *bytes) {
    union {
        short newval;
        unsigned char nv[2];
    }u;
    if (hostIsLittleEndian()) {
      u.nv[0] = bytes[0]; u.nv[1] = bytes[1];
    }
    else {
      u.nv[1] = bytes[0]; u.nv[0] = bytes[1];
    }

    return(u.newval);
}

int32_t snarf4 (char *bytes) {
    union {
        int32_t newval;
        unsigned char nv[4];
    }u;
    if (hostIsLittleEndian()) {
      u.nv[0] = bytes[0]; u.nv[1] = bytes[1];
      u.nv[2] = bytes[2]; u.nv[3] = bytes[3];
    }
    else {
      u.nv[3] = bytes[0]; u.nv[2] = bytes[1];
      u.nv[1] = bytes[2]; u.nv[3] = bytes[0];
    }
    return(u.newval);
}

float snarf4f (char *bytes) {
    union {
        float newval;
        unsigned char nv[4];
    }u;
    if (hostIsLittleEndian()) {
      u.nv[0] = bytes[0]; u.nv[1] = bytes[1];
      u.nv[2] = bytes[2]; u.nv[3] = bytes[3];
    }
    else {
      u.nv[3] = bytes[0]; u.nv[2] = bytes[1];
      u.nv[1] = bytes[2]; u.nv[3] = bytes[0];
    }
    return(u.newval);
}

# define PX2(x) (snarf2(((char *)&x)))
# define PX4(x) (snarf4(((char *)&x)))
# define PX4F(x) (snarf4f(((char *)&x)))

// c------------------------------------------------------------------------



void update_prqx (PIRAQX *prqx, LeRADARV *rhdr, LeHEADERV *dwel)
{
  float prt = PX4F(dwel->prt);
  strncpy(prqx->desc, "DWLX", 4);
  prqx->recordlen       = (unsigned short)PX2(dwel->recordlen);
  prqx->channel         = (uint4)EMPTY_FLAG;
  prqx->rev             = PX2(rhdr->rev);
  prqx->one             = 1; /* always set to the value 1 (endian flag) */
  prqx->byte_offset_to_data = (uint4)EMPTY_FLAG;
  prqx->dataformat      = dwel->dataformat;
  prqx->typeof_compression = (uint4)EMPTY_FLAG;
  prqx->pulse_num       = (uint8)EMPTY_FLAG;
  prqx->beam_num        = (uint8)EMPTY_FLAG;

  prqx->gates           =  PX2(dwel->gates);
  prqx->start_gate      = (uint4)EMPTY_FLAG;
  prqx->hits            = PX2(dwel->hits);
  prqx->ctrlflags       = (uint4)EMPTY_FLAG;  /* equivalent to packetflag below?  */
  prqx->bytespergate    = 2; 
  prqx->rcvr_pulsewidth = PX4F(dwel->rcvr_pulsewidth);
  prqx->prt[0]          = PX4F(dwel->prt);
  prqx->prt[1]          = PX4F(dwel->prt2);
  prqx->meters_to_first_gate = 0;

  prqx->num_segments    = 1;
  prqx->gate_spacing_meters[0] =
    PX4F(dwel->rcvr_pulsewidth) * .5 * SPEED_OF_LIGHT;
  prqx->gates_in_segment[0] = PX2(dwel->gates);

  prqx->clutter_start[0] = (uint4)EMPTY_FLAG;
  prqx->clutter_end[0]  = (uint4)EMPTY_FLAG;
  prqx->clutter_type[0] = (uint4)EMPTY_FLAG;
  prqx->secs            = PX4(dwel->time);
  prqx->nanosecs        = (uint4)(PX2(dwel->subsec) * 1.0e-4 * 1.0e9);
  prqx->az              = PX4F(dwel->az);
  prqx->az_off_ref      = EMPTY_FLAG;   /* azimuth offset off reference */
  prqx->el              = PX4F(dwel->el);
  prqx->el_off_ref      = EMPTY_FLAG;   /* elevation offset off reference */

  uint8 iprf = (prqx->prt[0]) ? (uint8)(1./prqx->prt[0]) : 0;
  prqx->pulse_num       = (uint8)(iprf*(uint8)prqx->secs);
  prqx->pulse_num      += (uint8)(iprf*prqx->nanosecs*0.000000001);
  prqx->beam_num        = (iprf) ? (prqx->pulse_num/iprf) : 0;

  prqx->radar_longitude = PX4F(dwel->radar_longitude); 
  prqx->radar_latitude  = PX4F(dwel->radar_lattitude);
  prqx->radar_altitude  = PX4F(dwel->radar_altitude);
  strcpy (prqx->gps_datum, "UNK");
  prqx->ts_start_gate   = PX2(dwel->tsgate);
  prqx->ts_end_gate     = PX2(dwel->tsgate) +1;

  prqx->ew_velocity     = PX4F(dwel->ew_velocity);
  prqx->ns_velocity     = PX4F(dwel->ns_velocity);
  prqx->vert_velocity   = PX4F(dwel->vert_velocity);
  prqx->fxd_angle       = PX4F(dwel->fxd_angle) * ANGSCALE;
  prqx->true_scan_rate  = EMPTY_FLAG;
  prqx->scan_type       = dwel->scan_type;
  prqx->scan_num        = dwel->scan_num;
  prqx->vol_num         = dwel->vol_num;

  prqx->transition      = dwel->transition;
  prqx->xmit_power      = PX4F(dwel->hxmit_power);
  prqx->yaw             = PX4F(dwel->yaw);
  prqx->pitch           = PX4F(dwel->pitch);
  prqx->roll            = PX4F(dwel->roll);
  prqx->track           = EMPTY_FLAG;
  prqx->gate0mag        = EMPTY_FLAG;
  prqx->dacv            = EMPTY_FLAG;
  prqx->packetflag      = (uint4)EMPTY_FLAG;

  prqx->year             = PX2(rhdr->year);
  prqx->julian_day       = (uint4)EMPTY_FLAG;
  memcpy(prqx->radar_name, rhdr->radar_name, 8);
  strcpy(prqx->channel_name, "UNK");
  strcpy(prqx->project_name, "UNK");
  strcpy(prqx->operator_name, "UNK");
  strcpy(prqx->site_name, "UNK");
  
  prqx->polarization     = rhdr->polarization;
  prqx->test_pulse_pwr   = PX4F(rhdr->test_pulse_pwr);
  prqx->test_pulse_frq   = PX4F(rhdr->test_pulse_frq);
  prqx->frequency        = PX4F(rhdr->frequency);

  prqx->noise_figure     = PX4F(rhdr->noise_figure);
  prqx->noise_power      = PX4F(rhdr->noise_power);
  prqx->receiver_gain    = PX4F(rhdr->receiver_gain);
  prqx->E_plane_angle    = EMPTY_FLAG;  /* offsets from normal pointing angle */
  prqx->H_plane_angle    = EMPTY_FLAG;


  prqx->data_sys_sat     = PX4F(rhdr->data_sys_sat);
  prqx->antenna_gain     = PX4F(rhdr->antenna_gain);
  prqx->H_beam_width     = PX4F(rhdr->horz_beam_width);
  prqx->V_beam_width     = PX4F(rhdr->vert_beam_width);

  prqx->xmit_pulsewidth  = PX4F(rhdr->xmit_pulsewidth);
  prqx->rconst           = PX4F(rhdr->rconst);
  prqx->phaseoffset      = PX4F(rhdr->phaseoffset);
  prqx->zdr_fudge_factor = PX4F(rhdr->zdr_fudge_factor);
  prqx->mismatch_loss    = PX4F(rhdr->mismatch_loss);

  prqx->rcvr_const       = EMPTY_FLAG; 
  prqx->test_pulse_rngs_km[0] = EMPTY_FLAG;
  prqx->test_pulse_rngs_km[1] = EMPTY_FLAG;

  strcpy (prqx->comment, "NO COMMENT");

  prqx->i_norm            = EMPTY_FLAG;
  prqx->q_norm            = EMPTY_FLAG;
  prqx->i_compand         = EMPTY_FLAG;
  prqx->q_compand         = EMPTY_FLAG;
  /*
  float4 transform_matrix[2][2][2];
  float4 stokes[4]; 
   */
  prqx->vxmit_power        = PX4F(dwel->vxmit_power);
  prqx->vtest_pulse_pwr    = PX4F(rhdr->vtest_pulse_pwr);
  prqx->vnoise_power       = PX4F(rhdr->vnoise_power);
  prqx->vreceiver_gain     = PX4F(rhdr->vreceiver_gain);
  prqx->vantenna_gain      = PX4F(rhdr->vantenna_gain);
  //  prqx->h_rconst           = this->return_h_rconst ();
  //  prqx->v_rconst           = this->return_v_rconst ();
  prqx->peak_power         = PX4F(rhdr->peak_power);

}


// c---------------------------------------------------------------------------
# define UEMPTY_FLAG 2147483647

void update_prqxx (PIRAQX *prqx, RADARV *rhdr, HEADERV *dwel)
{
# ifdef obsolete
  strncpy(prqx->desc, "DWLX", 4);
  prqx->recordlen       = sizeof (PIRAQX);
  prqx->channel         = UEMPTY_FLAG;
  prqx->rev             = rhdr->rev;
  prqx->one             = 1; /* always set to the value 1 (endian flag) */
  prqx->byte_offset_to_data = UEMPTY_FLAG;
  prqx->dataformat      = dwel->dataformat;
  prqx->typeof_compression = UEMPTY_FLAG;
  prqx->pulse_num       = (uint8)EMPTY_FLAG;
  prqx->pulse_num       = 0x0102030405060708;
  prqx->beam_num        = (uint8)EMPTY_FLAG;

  prqx->gates           = dwel->gates;
  prqx->start_gate      = UEMPTY_FLAG;
  prqx->hits            = dwel->hits;
  prqx->ctrlflags       = UEMPTY_FLAG;  /* equivalent to packetflag below?  */
  prqx->bytespergate    = 2; 
  prqx->rcvr_pulsewidth = dwel->rcvr_pulsewidth;
  prqx->prt[0]          = dwel->prt;
  prqx->prt[1]          = dwel->prt2;
  prqx->meters_to_first_gate = 0;

  prqx->num_segments    = 1;
  prqx->gate_spacing_meters[0] =
    dwel->rcvr_pulsewidth * .5 * SPEED_OF_LIGHT;
  prqx->gates_in_segment[0] = dwel->gates;

  prqx->clutter_start[0] = UEMPTY_FLAG;
  prqx->clutter_end[0]  = UEMPTY_FLAG;
  prqx->clutter_type[0] = UEMPTY_FLAG;
  prqx->secs            = dwel->time;
  prqx->nanosecs        = (uint4)(dwel->subsec * 1.0e-4 * 1.0e9);
  prqx->az              = dwel->az;
  prqx->az_off_ref      = EMPTY_FLAG;   /* azimuth offset off reference */
  prqx->el              = dwel->el;
  prqx->el_off_ref      = EMPTY_FLAG;   /* elevation offset off reference */

  prqx->radar_longitude = dwel->radar_longitude; 
  prqx->radar_latitude = dwel->radar_lattitude;
  prqx->radar_altitude  = dwel->radar_altitude;
  strcpy (prqx->gps_datum, "UNK");
  prqx->ts_start_gate   = dwel->tsgate;
  prqx->ts_end_gate    = dwel->tsgate;

  prqx->ew_velocity     = dwel->ew_velocity;
  prqx->ns_velocity     = dwel->ns_velocity;
  prqx->vert_velocity   = dwel->vert_velocity;
  prqx->fxd_angle       = dwel->fxd_angle * ANGSCALE;
  prqx->true_scan_rate  = EMPTY_FLAG;
  prqx->scan_type       = dwel->scan_type;
  prqx->scan_num        = dwel->scan_num;
  prqx->vol_num         = dwel->vol_num;

  prqx->transition      = dwel->transition;
  prqx->xmit_power      = dwel->hxmit_power;
  prqx->yaw             = dwel->yaw;
  prqx->pitch           = dwel->pitch;
  prqx->roll            = dwel->roll;
  prqx->track           = EMPTY_FLAG;
  prqx->gate0mag        = EMPTY_FLAG;
  prqx->dacv            = EMPTY_FLAG;
  prqx->packetflag      = UEMPTY_FLAG;

  prqx->year             = rhdr->year;
  prqx->julian_day       = UEMPTY_FLAG;
  memcpy(prqx->radar_name, rhdr->radar_name, 8);
  strcpy(prqx->channel_name, "UNK");
  strcpy(prqx->project_name, "UNK");
  strcpy(prqx->operator_name, "UNK");
  strcpy(prqx->site_name, "UNK");
  
  prqx->polarization     = rhdr->polarization;
  prqx->test_pulse_pwr   = rhdr->test_pulse_pwr;
  prqx->test_pulse_frq   = rhdr->test_pulse_frq;
  prqx->frequency        = rhdr->frequency;

  prqx->noise_figure     = rhdr->noise_figure;
  prqx->noise_power      = rhdr->noise_power;
  prqx->receiver_gain    = rhdr->receiver_gain;
  prqx->E_plane_angle    = EMPTY_FLAG;  /* offsets from normal pointing angle */
  prqx->H_plane_angle    = EMPTY_FLAG;


  prqx->data_sys_sat     = rhdr->data_sys_sat;
  prqx->antenna_gain     = rhdr->antenna_gain;
  prqx->H_beam_width     = rhdr->horz_beam_width;
  prqx->V_beam_width     = rhdr->vert_beam_width;

  prqx->xmit_pulsewidth  = rhdr->xmit_pulsewidth;
  prqx->rconst           = rhdr->rconst ;
  prqx->phaseoffset      = rhdr->phaseoffset;
  prqx->zdr_fudge_factor = rhdr->zdr_fudge_factor;
  prqx->mismatch_loss    = rhdr->mismatch_loss;

  prqx->rcvr_const       = EMPTY_FLAG; 
  prqx->test_pulse_rngs_km[0] = EMPTY_FLAG;
  prqx->test_pulse_rngs_km[1] = EMPTY_FLAG;

  strcpy (prqx->comment, "NO COMMENT");

  prqx->i_norm            = EMPTY_FLAG;
  prqx->q_norm            = EMPTY_FLAG;
  prqx->i_compand         = EMPTY_FLAG;
  prqx->q_compand         = EMPTY_FLAG;
  /*
  float4 transform_matrix[2][2][2];
  float4 stokes[4]; 
   */
  prqx->vxmit_power        = dwel->vxmit_power;
  prqx->vtest_pulse_pwr    = rhdr->vtest_pulse_pwr;
  prqx->vnoise_power       = rhdr->vnoise_power;
  prqx->vreceiver_gain     = rhdr->vreceiver_gain;
  prqx->vantenna_gain      = rhdr->vantenna_gain;
  prqx->h_rconst           = h_rconst;
  prqx->v_rconst           = v_rconst;
  prqx->peak_power         = rhdr->peak_power;
# endif
}

// c---------------------------------------------------------------------------
/* c------------------------------------------------------------------------ */
#define STARTGATE_A       5      /* start gate for power average of all gates */
/* c------------------------------------------------------------------------ */

void smallhvsimul(DWELL *dwell, RADAR *radar, float *prods)      
   {
   int          i,avecount,num_gates, mark;   
   short        *aptr;
   float        *gate1;
   double       scale2ln,scale2db;
   double       h_channel_radar_constant,v_channel_radar_constant;
   double       angle_to_velocity_scale_factor;
   double       horiz_offset_to_dBm,verti_offset_to_dBm;
   double       cp1,v1a,lnpv,cp2,v2a,lnph,lnhv,lag2;
   double       lncrhv,vcrhv,lncrvh,vcrvh;
   double       ph_off,temp,theta,dp,lncp,v;
   double       horiz_dBm_at_coupler,verti_dBm_at_coupler;
   double       horiz_coherent_dBm_at_coupler;
   double       crosshv_dBm_at_coupler,crossvh_dBm_at_coupler,laghh_dBm_at_coupler;
   double       hchan_noise_power,vchan_noise_power,lncoherent;
   double       widthconst,range_correction,coher_noise_power;
   double       linear_h_power,linear_v_power,average_h_power,average_v_power;
   double       lnvh,lniq,phiq;
   double       linear_hvcross_mag,average_real_hvcross,average_imag_hvcross;
   double       linear_vhcross_mag,average_real_vhcross,average_imag_vhcross;
   double       d;

    short *velp=gri->scaled_data[ve_ndx];
    short *dbmp=gri->scaled_data[dbm_ndx];
    short *ncpp=gri->scaled_data[ncp_ndx];
    short *swp=gri->scaled_data[sw_ndx];
    short *dbzp=gri->scaled_data[dz_ndx];

    short *dbzcp=gri->scaled_data[dzc_ndx];
    short *zdrp=gri->scaled_data[zdr_ndx];
    short *phip=gri->scaled_data[phi_ndx];
    short *rhohv=gri->scaled_data[rhohv_ndx];
    short *ldr=gri->scaled_data[ldr_ndx];

    short *dbmv=gri->scaled_data[dbmv_ndx];

    float f, scale=100., bias=0;
    HEADERV *dwel = hdr;


   /* initialize the things used for h and v power average */
   average_h_power = average_v_power = 0.0;
   /* initialize the things used for hv antenna isolation average */
   average_real_hvcross = average_imag_hvcross = 0.0;
   average_real_vhcross = average_imag_vhcross = 0.0;
   gate1 = prods + 16;
   avecount = 0;

   scale2ln = 0.004 * log(10.0) / 10.0;  /* from counts to natural log */
   scale2db = 0.004 / scale2ln;         /* from natural log to 10log10() */

   h_channel_radar_constant 
	= rhdr->rconst - 20.0 * log10(rhdr->xmit_pulsewidth / dwel->rcvr_pulsewidth)
	  + 10.0 * log10(rhdr->peak_power / dwel->hxmit_power);

   v_channel_radar_constant 
	= rhdr->rconst - 20.0 * log10(rhdr->xmit_pulsewidth / dwel->rcvr_pulsewidth)
	  + 10.0 * log10(rhdr->peak_power / dwel->vxmit_power)
	  + 2.0 * (rhdr->antenna_gain - rhdr->vantenna_gain);

   angle_to_velocity_scale_factor
	= C / (2.0 * rhdr->frequency * 2.0 * M_PI * dwel->prt);

   horiz_offset_to_dBm = rhdr->data_sys_sat - 20.0 * log10(0x10000) 
			- rhdr->receiver_gain + 10.0 * log10(2.0);  /* correct for half power measurement */

   verti_offset_to_dBm = rhdr->data_sys_sat - 20.0 * log10(0x10000) 
			- rhdr->vreceiver_gain + 10.0 * log10(2.0); /* correct for half power measurement */

   widthconst = (C / rhdr->frequency) / dwel->prt / (4.0 * sqrt(2.0) * M_PI);

   ph_off = 20.0 * M_PI / 180.0; /* set phase offset to be 20 deg */

# ifdef obsolete
    aptr = dwel->abp;
# else
    aptr = (short *)pui->raw_data;
    num_gates = dwel->gates;
# endif
   range_correction = 0.0;

   /* these powers reflect the LNA and waveguide performance */
   /* they cannot be broken down into co an cross powers */
   hchan_noise_power = (rhdr-> noise_power > -10.0) ? 0.0 : exp((rhdr-> noise_power - horiz_offset_to_dBm) / scale2db);
   vchan_noise_power = (rhdr->vnoise_power > -10.0) ? 0.0 : exp((rhdr->vnoise_power - verti_offset_to_dBm) / scale2db);
   coher_noise_power = exp((-129.0 - verti_offset_to_dBm) / scale2db);
				/*
   mark = *aptr;
				 */
   
   for(i=0; i<num_gates; i++,prods+=16)
      {
# ifdef obsolete	
      /* read in the raw data from structure */
      cp1  = *aptr++ * scale2ln;        /* natural log  of |R(1)| from V pulse pair */
      v1a  = *aptr++ * M_PI / 32768.0;  /* radian angle of |R(1)| from V pulse pair */
      lnpv = *aptr++ * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
      cp2  = *aptr++ * scale2ln;        /* natural log  of |R(1)| from H pulse pair */
      v2a  = *aptr++ * M_PI / 32768.0;  /* radian angle of |R(1)| from H pulse pair */
      lnph = *aptr++ * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
      lnvh = *aptr++ * scale2ln;        /* natural log  of |HV*| -- used for Phidp */
      theta = *aptr++ * M_PI / 32768.0; /* radian angle of VH* ; i.e. differential phase */
# endif

      if(hostIsLittleEndian()) {
	cp1  = (*aptr++) * scale2ln;        /* natural log  of |R(1)| from V pulse pair */
	v1a  = (*aptr++) * M_PI / 32768.0;  /* radian angle of |R(1)| from V pulse pair */
	lnpv = (*aptr++) * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
	cp2  = (*aptr++) * scale2ln;        /* natural log  of |R(1)| from H pulse pair */
	v2a  = (*aptr++) * M_PI / 32768.0;  /* radian angle of |R(1)| from H pulse pair */
	lnph = (*aptr++) * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
	lnvh = (*aptr++) * scale2ln;        /* natural log  of |HV*| -- used for Phidp */
	theta = (*aptr++) * M_PI / 32768.0; /* radian angle of VH* ; i.e. differential phase */
      }
      else {
	/* SX2() does byte swapping */
//Jul 26, 2011 start	
//	cp1  = SX2(*aptr++) * scale2ln; /* natural log  of |R(1)| from V pulse pair */
//	v1a  = SX2(*aptr++) * M_PI / 32768.0;
//	lnpv = SX2(*aptr++) * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
//	cp2  = SX2(*aptr++) * scale2ln;        /* natural log  of |R(1)| from H pulse pair */
//	v2a  = SX2(*aptr++) * M_PI / 32768.0;  /* radian angle of |R(1)| from H pulse pair */
//	lnph = SX2(*aptr++) * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
//	lnvh = SX2(*aptr++) * scale2ln;        /* natural log  of |HV*| -- used for Phidp */
//	theta = SX2(*aptr++) * M_PI / 32768.0; /* radian angle of VH* ; i.e. differential phase */

	cp1  = SX2(*(twob*)aptr++) * scale2ln; /* natural log  of |R(1)| from V pulse pair */
	v1a  = SX2(*(twob*)aptr++) * M_PI / 32768.0;
	lnpv = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of V power (from 16 bit scaled log) */
	cp2  = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of |R(1)| from H pulse pair */
	v2a  = SX2(*(twob*)aptr++) * M_PI / 32768.0;  /* radian angle of |R(1)| from H pulse pair */
	lnph = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of H power (from 16 bit scaled log) */
	lnvh = SX2(*(twob*)aptr++) * scale2ln;        /* natural log  of |HV*| -- used for Phidp */
	theta = SX2(*(twob*)aptr++) * M_PI / 32768.0; /* radian angle of VH* ; i.e. differential phase */
//Jul 26, 2011 end
      }

      lncoherent = (cp1 + log(1 + exp(cp2 - cp1)) - LOG2);  /* natural log  of coherent power */

      /* NCP */
      /* it is best if this parameter is computed before noise correction */
     
      lncp =  log(exp(cp1 - lnpv) + exp(cp2 - lnph)) - log(2.0);
//      lncp = cp1 - lnpv;
# ifdef obsolete
      prods[2] = exp(lncp);        /* average of V and H NCP's */
# else
      d = exp(lncp);        /* average of V and H NCP's */
      *ncpp++ = (short)DD_SCALE(d, scale, bias);
# endif

      
# ifdef obsolete
      /* h/v cross correlation RHOhv (normalized dB) */
      /* it is best if this parameter is computed before noise correction */
      prods[9] = exp(lnvh - (log(0.5) + (lnpv + log(1 + exp(lnph - lnpv)))));
# else
      d = exp(lnvh - (log(0.5) + (lnpv + log(1 + exp(lnph - lnpv)))));
      *rhohv++ = (short)DD_SCALE(d, scale, bias);
# endif
      
      /* subtract raw noise power from the raw log powers */
      linear_h_power = exp(lnph) - hchan_noise_power;   if(linear_h_power <= 0.0)       linear_h_power = SMALL;       
      lnph = log(linear_h_power);   /* corrected h chan power */
      linear_v_power = exp(lnpv) - vchan_noise_power;   if(linear_v_power <= 0.0)       linear_v_power = SMALL;       
      lnpv = log(linear_v_power);   /* corrected v chan power */

      /* convert the raw log powers to dBm at the test pulse waveguide coupler */
      horiz_dBm_at_coupler          = lnph * scale2db + horiz_offset_to_dBm;    /* HH  power in dBm */
      horiz_coherent_dBm_at_coupler = lncoherent * scale2db + horiz_offset_to_dBm;    /* HH  coherent power in dBm */
      verti_dBm_at_coupler          = lnpv * scale2db + verti_offset_to_dBm;    /* VV  power in dBm */

# ifdef notyet
      if(i >= STARTGATE_A)        /* only average past startgate */
	 {
	 avecount++;
	 
	 /* average h and v power for solar cal purposes */
	 average_h_power += linear_h_power;
	 average_v_power += linear_v_power;
	 
	 }
# endif

      /* compute range correction in dB. Skip the first gate */
      if(i)     range_correction = 20.0 * log10(i * 0.0005 * C * dwel->rcvr_pulsewidth);

      /* compute velocity by averaging v1a and v2a */
      v = atan2(exp(cp1) * sin(v1a) + exp(cp2) * sin(v2a),exp(cp1) * cos(v1a) + exp(cp2) * cos(v2a));

      /* figure the differential phase (from - 20 to +160) */
      theta -= rhdr->phaseoffset;
      dp = theta;
      if (dp < -ph_off)   dp += M_PI;
      /* note: dp cannot be greater than +160, range is +/- 90 */        

      /* velocity in m/s */
# ifdef obsolete
      prods[0] = v * angle_to_velocity_scale_factor;
# else
      d = v * angle_to_velocity_scale_factor;
      *velp++ = (short)DD_SCALE(d, scale, bias);
# endif

      /* horizontal power in dBm at test pulse coupler */
# ifdef obsolete
      prods[1] = horiz_dBm_at_coupler;
# else
      d = horiz_dBm_at_coupler;
      *dbmp++ = (short)DD_SCALE(d, scale, bias);
# endif

      /* this space intentionaly left blank */
      /* NCP is computed above, before noise correction */

      /* spectral width in m/s */
      if( lncp > 0.0)   lncp = 0.0;
# ifdef obsolete
      prods[3] = widthconst * sqrt(-lncp);
# else
      d = widthconst * sqrt(-lncp);
      *swp++ = (short)DD_SCALE(d, scale, bias);
# endif

      /* horizontal reflectivity in dBZ */
# ifdef obsolete
      prods[4] = horiz_dBm_at_coupler + h_channel_radar_constant
	+ range_correction;
# else
      d = horiz_dBm_at_coupler + h_channel_radar_constant
	+ range_correction;
      *dbzp++ = (short)DD_SCALE(d, scale, bias);
# endif

      /* horizontal coherent reflectivity in dBZ */
# ifdef obsolete
      prods[5] = horiz_coherent_dBm_at_coupler + h_channel_radar_constant
	+ range_correction;
# else
      d = horiz_coherent_dBm_at_coupler + h_channel_radar_constant
	+ range_correction;
      *dbzcp++ = (short)DD_SCALE(d, scale, bias);
# endif

      /* differential reflectivity Zdr in dB */
# ifdef obsolete
      prods[6] = horiz_dBm_at_coupler + h_channel_radar_constant
	- verti_dBm_at_coupler - v_channel_radar_constant
	+ rhdr->zdr_fudge_factor;
# else
      d = horiz_dBm_at_coupler + h_channel_radar_constant
	- verti_dBm_at_coupler - v_channel_radar_constant
	+ rhdr->zdr_fudge_factor;
      *zdrp++ = (short)DD_SCALE(d, scale, bias);
# endif

      /* differential phase PHI dp in degrees */
# ifdef obsolete
      prods[7] = -dp * 180.0 / M_PI;
# else
      d = -dp * 180.0 / M_PI;
      *phip++ = (short)DD_SCALE(d, scale, bias);
# endif

      /* v power in dBm */ 
# ifdef obsolete
      prods[8] = verti_dBm_at_coupler;
# else
      d = verti_dBm_at_coupler;
      *dbmv++ = (short)DD_SCALE(d, scale, bias);
# endif

      /* this space intentionaly left blank */
      /* RHOhv is computed above, before noise correction */

# ifdef obsolete
      /* v power in dBm--repeats [8]? */ 
      prods[10] = verti_dBm_at_coupler;
      
      /* V reflectivity in dBZ */
      prods[11] = verti_dBm_at_coupler + v_channel_radar_constant
	+ range_correction;
      
      /* v power in dBm */ 
      prods[12] = verti_dBm_at_coupler;
# endif
      
      }

# ifdef notyet
   /* now insert the average h and v power into the gate 1 data */
   /* this is only necessary for the realtime system */
   if(avecount == 0) return;
   gate1[ 1] = log(average_h_power / (double)avecount) * scale2db + horiz_offset_to_dBm;    /* average HH  power in dBm */
   gate1[10] = log(average_v_power / (double)avecount) * scale2db + verti_offset_to_dBm;    /* average VV  power in dBm */
# endif

   }
/* c------------------------------------------------------------------------ */

