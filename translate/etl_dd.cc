/* 	$Id$	 */
 
#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */

#include <LittleEndian.hh>
#include <string.h>
#include <stdlib.h>
#include <dd_math.h>
#include <dd_time.h>
#include <ETL.h>
#include "dd_stats.h"
#include "etl_dd.hh"
#include "dda_common.hh"
#include "gneric_dd.hh"
#include "dd_io_mgmt.hh"
#include "dorade_share.hh"
#include "dd_catalog.hh"

static struct etl_useful_info *etui=NULL;

/* 2 micron structs */
static struct moment_header_struct *mom_ptr, *mom_ptrx;
static struct raw_header_struct    *raw_ptr, *raw_ptrx;
static struct anc_header_struct    *anc_ptr, *anc_ptrx;
static struct cov_header_struct    *cov_ptr, *cov_ptrx;
static struct file_header_struct   *fhed_ptr, *fhed_ptrx;

/* MCAWS structs */
static struct moment_header_struct_mcws *mc_mom_ptr, *mc_mom_ptrx;
static struct raw_header_struct_mcws    *mc_raw_ptr, *mc_raw_ptrx;
static struct anc_header_struct_mcws    *mc_anc_ptr, *mc_anc_ptrx;
static struct cov_header_struct_mcws    *mc_cov_ptr, *mc_cov_ptrx;

static double min_az_diff=.3, min_el_diff=.15;
static double max_az_diff=999.;
static double min_rhi_angle_swept=1.5;
static double min_ppi_angle_swept=3.;

static struct dd_input_filters *difs;
static struct dd_stats *dd_stats=NULL;
static struct input_read_info *irq;
static char preamble[24], postamble[64];
static struct d_limits *d_limits=NULL;
static struct generic_radar_info *gri;
static struct solo_list_mgmt *slm;
static int view_char=0, view_char_count=200;
static int ms=EMPTY_FLAG;
static char *scr_buf;
static int etlDebug=NO;
static double Oct_1_1996=0;

static char *current_file_name;

/* c------------------------------------------------------------------------ */

void etl_dd_conv (int interactive_mode)
{
   static int count=0, count_break=99, nok_count=0;
   int mark, nn;
   float *fp=NULL;

    if(!count)
	  etl_ini();

    if(interactive_mode) {
	dd_intset();
	etl_positioning();
    }
    etl_reset();

    for(;;) {
	if(++count >= count_break) {
	    mark = 0;
	}
	if(difs->stop_flag &&
	   (difs->abrupt_start_stop || etui->new_sweep)) {
	    printf("\nBreak on stop time!\n");
	    break;
	}

	if(etl_select_ray()) {

	    if(difs->max_beams)
		  if(++etui->ray_count > difs->max_beams ) {
		      printf("\nBreak on ray count!\n");
		      break;
		  }
	    if(etui->sweep_count_flag) {
		etui->sweep_count_flag = NO;
		if(difs->max_sweeps) {
		    if(++etui->sweep_count > difs->max_sweeps ) {
			printf("\nBreak on sweep count!\n");
			break;
		    }
		}
		if(etui->vol_count_flag) {
		    etui->vol_count_flag = NO;
		    if(difs->max_vols) {
			if(++etui->vol_count > difs->max_vols ) {
			    printf("\nBreak on volume count!\n");
			    break;
			}
		    }
		}
	    }
	    if(!difs->catalog_only)
  		  etl_nab_data();
	    
	    dd_stuff_ray();	/* pass it off to dorade code */
	}
	else {
	    if(!(nok_count++ % 1000)) {
		etl_print_stat_line(count);
	    }
	}
	etui->new_sweep = NO;
	etui->new_vol = NO;

	if((nn = etl_next_ray()) < 1) {
	   printf("Break on input status: %d\n", nn);
	   break;
	}
     }
   etl_reset();
}
/* c------------------------------------------------------------------------ */

void etl_ini (void)
{
   int ii, jj, nn, nt, ms=EMPTY_FLAG;
   CHAR_PTR aa, bb;
   char string_space[256], str_ptrs[32];
   struct mc_lidar_info *mli;
   double d;
   DD_TIME dts;
   float f;
   struct etl_ray_que *rq, *last_rq;
   struct etl_swp_que *sq, *last_sq;

   
   dts.year = 1996;
   dts.month = 10;
   dts.day = 1;
   dts.day_seconds = 0;
   d_unstamp_time(&dts);
   Oct_1_1996 = dts.time_stamp;

   difs = dd_return_difs_ptr();
   dd_stats = dd_return_stats_ptr();
   gri = return_gri_ptr();
   irq = dd_return_ios(ETL_IO_INDEX, ETL2M_FMT);
   irq->min_block_size = 4;

   mom_ptr = mom_ptrx = (struct moment_header_struct *)
     malloc(sizeof(struct moment_header_struct));
   memset(mom_ptr, 0, sizeof(struct moment_header_struct));
   raw_ptr = raw_ptrx = (struct raw_header_struct *)
     malloc(sizeof(struct raw_header_struct));
   memset(raw_ptr, 0, sizeof(struct raw_header_struct));
   anc_ptr = anc_ptrx = (struct anc_header_struct *)
     malloc(sizeof(struct anc_header_struct));
   memset(anc_ptr, 0, sizeof(struct anc_header_struct));
   cov_ptr = cov_ptrx = (struct cov_header_struct *)
     malloc(sizeof(struct cov_header_struct));
   memset(cov_ptr, 0, sizeof(struct cov_header_struct));
   
   fhed_ptr = fhed_ptrx = (struct file_header_struct *)
     malloc(sizeof(struct file_header_struct));
   memset(fhed_ptr, 0, sizeof(struct file_header_struct));
   
   mc_mom_ptr = mc_mom_ptrx = (struct moment_header_struct_mcws *)
     malloc(sizeof(struct moment_header_struct_mcws));
   memset(mc_mom_ptr, 0, sizeof(struct moment_header_struct_mcws));
   mc_raw_ptr = mc_raw_ptrx = (struct raw_header_struct_mcws *)
     malloc(sizeof(struct raw_header_struct_mcws));
   memset(mc_raw_ptr, 0, sizeof(struct raw_header_struct_mcws));
   mc_anc_ptr = mc_anc_ptrx = (struct anc_header_struct_mcws *)
     malloc(sizeof(struct anc_header_struct_mcws));
   memset(mc_anc_ptr, 0, sizeof(struct anc_header_struct_mcws));
   mc_cov_ptr = mc_cov_ptrx = (struct cov_header_struct_mcws *)
     malloc(sizeof(struct cov_header_struct_mcws));
   memset(mc_cov_ptr, 0, sizeof(struct cov_header_struct_mcws));

   scr_buf = (char *)malloc(K64);
   memset(scr_buf, 0, K64);
   
   etui = (struct etl_useful_info *)
     malloc(sizeof(struct etl_useful_info));
   memset(etui, 0, sizeof(struct etl_useful_info));
   etui->sweep_time_limit = 180; /* seconds */
   etui->new_sweep = etui->new_vol = YES;
   etui->lidar_number = _2MICRON;

   etui->mom_data = (char *)malloc(K64/2);
   memset(etui->mom_data, 0, K64/2);
   etui->cov_data = (char *)malloc(K64);
   memset(etui->cov_data, 0, K64);
   etui->raw_data = (char *)malloc(K64);
   memset(etui->raw_data, 0, K64);
  
   for(ii=0; ii < MAX_MC_LIDARS; ii++) {
      etui->mc_lidars[ii] = mli =
	(struct mc_lidar_info *)malloc(sizeof(struct mc_lidar_info));
      memset(mli, 0, sizeof(struct mc_lidar_info));
   }
   strcpy(etui->mc_lidars[MC_FORE]->lidar_name, MC_FORE_NAME);
   strcpy(etui->mc_lidars[MC_AFT]->lidar_name, MC_AFT_NAME);
   strcpy(etui->mc_lidars[MC_FXD]->lidar_name, MC_FXD_NAME);

    for(ii=0; ii < RAY_QUE_SIZE; ii++) {
	rq = (struct etl_ray_que *)malloc(sizeof(struct etl_ray_que));
	memset(rq, 0, sizeof(struct etl_ray_que));
	if(!ii) {
	    etui->ray_que = rq;
	}
	else {
	    rq->last = etui->ray_que->last;
	    etui->ray_que->last->next = rq;
	}
	rq->next = etui->ray_que;
	etui->ray_que->last = rq;
	last_rq = rq;
	rq->scan_num = -1;
	rq->vol_num = -1;
    }

    for(ii=0; ii < SWEEP_QUE_SIZE; ii++) {
	sq = (struct etl_swp_que *)malloc(sizeof(struct etl_swp_que));
	memset(sq, 0, sizeof(struct etl_swp_que));
	if(!ii) {
	    etui->swp_que = sq;
	}
	else {
	    sq->last = etui->swp_que->last;
	    etui->swp_que->last->next = sq;
	}
	sq->next = etui->swp_que;
	etui->swp_que->last = sq;
	last_sq = sq;
	sq->swpang = EMPTY_FLAG;
    }

    gri->binary_format = DD_16_BITS;
    gri->source_format = ETL2M_FMT;
    gri->radar_type = GROUND;
    gri->latitude = ms;
    gri->longitude = ms;
    gri->altitude = ms;
    gri->missing_data_flag = EMPTY_FLAG;
    gri->h_beamwidth = ms;
    gri->v_beamwidth = ms;
    gri->polarization = 0;	/* horizontal polarization */
    gri->freq[0] = ms;
    gri->ipps[0] = ms;

    if((aa=get_tagged_string("OPTIONS"))) {
       if(strstr(aa, "DEBUG")) {
	  etlDebug = YES;
       }
    }
   
   if(aa=get_tagged_string("LIDAR_SWEEP_TIME_LIMIT")) {
      if((d = atof(aa)) > 0) {
	 etui->sweep_time_limit = d;
      }
      etui->fake_sweep_nums = YES;
   }
   printf( "Sweep_time_limit: %.3f seconds\n", etui->sweep_time_limit);
   
    if(aa=get_tagged_string("MAX_AZ_DIFF")) {
       d = atof(aa);
       if(d > .01) max_az_diff = d;
    }
    printf("Max. AZ diff: %.2f", max_az_diff);

    if(aa=get_tagged_string("MIN_AZ_DIFF")) {
       d = atof(aa);
       if(d > .01) min_az_diff = d;
    }
    printf("Min. AZ diff: %.2f", min_az_diff);

    if(aa=get_tagged_string("MIN_EL_DIFF")) {
	d = atof(aa);
	if(d > .01) min_el_diff = d;
    }
    printf("  Min. EL diff: %.2f", min_el_diff);
   printf("\n");
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
    if(aa=get_tagged_string("RADAR_NAME")) {
	strcpy(gri->radar_name, aa);
    }
    else {
	strcpy(gri->radar_name, "UNKNOWN"); 
    }

    if(aa=get_tagged_string("TIME_CORRECTION")) {
	etui->time_correction = atof(aa);
	printf( "Time correction: %.3f seconds\n", etui->time_correction);
    }


   if(aa=get_tagged_string("VOLUME_HEADER")) {
      /* nab a volume header from another file
       */
      dd_input_read_open(irq, aa);
      nn = etl_next_ray();
      dd_input_read_close(irq);
   }
   if((aa=get_tagged_string("SOURCE_DEV"))) {
      aa = dd_establish_source_dev_names((char *)"ETL", aa);
      aa = dd_next_source_dev_name((char *)"ETL");
      current_file_name = aa;
      dd_input_read_open(irq, aa);
   }
   
   nn = etl_next_ray();
}
/* c------------------------------------------------------------------------ */

int etl_next_ray (void)
{
   static int whats_left=0;
   struct io_packet *dp, *dpx;
   int ii, kk, nn, eof_count, mark, gotta_header=NO, looping, size = 0;
   int ugly_record_count=0;
   float f;
   double d, sd, sum, sumsq, avg, d_nn, ratio;
   char *aa, *ee;
   
   int ms=EMPTY_FLAG;
   struct etl_ray_que *rq;
   struct etl_swp_que *sq;
   float r, gs;
   double da, dda;
   /* c...mark */
   
   etui->ignore_this_ray = NO;
   
   for(;;) {
      if(dd_control_c()) {
	 dd_reset_control_c();
	 return(-1);
      }
      if(etui->lidar_number == _2MICRON) {
	 if((size = etl_next_ray_2micron()) == REALLY_MCAWS) {
	    continue;
	 }
	 if(size < 1) {
	    return(-1);
	 }
      }
      else if((size = etl_next_ray_mcaws()) < 1) {
	 return(-1);
      }
      break;
   }
   rq = etui->ray_que = etui->ray_que->next;
   
   rq->time = gri->time;
   
   if(gri->time > etui->ref_time) {
      etui->ref_time = gri->time;
      etui->count_since_trip = 1;
   }
   else {
      etui->seconds_per_beam = gri->PRF > 0
	? gri->num_samples/gri->PRF : 1;
      gri->time += (etui->count_since_trip++) * etui->seconds_per_beam;
   }
   gri->dts->time_stamp = gri->time;
   
   rq->az = gri->azimuth;
   rq->el = gri->elevation;
   rq->az_diff = angdiff(rq->last->az, rq->az);
   rq->el_diff = angdiff(rq->last->el, rq->el);
   rq->time_diff = rq->time - rq->last->time;

   switch(etui->lidar_number) {
    case _2MICRON:
      break;
    case MCAWS:
      rq->etl_beam_index = mc_mom_ptr->beam_loc.beam_index;
      break;
   }
   
   
   sq = etui->swp_que;
   if(etui->sweep_ray_num > 5) {
      if(sq->scan_mode == PPI) {
	 da = rq->az_diff;
	 dda = rq->last->az_diff;

	 if(fabs(dda) < max_az_diff && fabs(da) < max_az_diff) {

	     if(fabs(dda) > min_az_diff && fabs(da) > min_az_diff &&
		(dda + da) * etui->az_diff_sum < 0) {
		 /* antenna has changed direction
		  */
		 etui->new_sweep = YES;
	     }
	     if(fabs(etui->az_diff_sum >= 360.)) {
		 etui->new_sweep = YES;
	     }
	 }
      }
      else if(sq->scan_mode == RHI) {
	 da = rq->el_diff;
	 dda = rq->last->el_diff;
	 if(fabs(dda) > min_el_diff && fabs(da) > min_el_diff &&
	    (dda + da) * etui->el_diff_sum < 0) {
	    /* antenna has changed direction
	     */
	    etui->new_sweep = YES;
	 }
	 if(fabs(etui->el_diff_sum >= 360.)) {
	    etui->new_sweep = YES;
	 }
      }
      else {
	 if(gri->time > etui->sweep_reference_time) {
	    etui->new_sweep = YES;
	 }
      }
   }

   if(etui->new_sweep) {
      dd_stats->sweep_count++;
      sq = etui->swp_que;
      sq->az_diff_sum = etui->az_diff_sum;
      sq->el_diff_sum = etui->el_diff_sum;
      sq->num_rays = etui->sweep_ray_num;
      gri->sweep_num++;

      if(etui->swpang_num) {
	 if(sq->scan_mode == RHI &&
	    fabs(etui->el_diff_sum) < min_rhi_angle_swept) {
	    gri->ignore_this_sweep = YES;
	 }
	 else if(sq->scan_mode == PPI &&
		 fabs(etui->az_diff_sum) < min_ppi_angle_swept) {
	    gri->ignore_this_sweep = YES;
	 }
      }
      if(etui->sweep_ray_num < dd_min_rays_per_sweep()) {
	 gri->ignore_this_sweep = YES;
      }
      sq = etui->swp_que = sq->next;
      sq->scan_mode = gri->scan_mode;
# ifdef obsolete
      sq->swpang = gri->fixed;
# endif
      sq->time = gri->time;
      sq->fxdang_sum = 0;

      switch(etui->lidar_number) {
       case MCAWS:
	 strcpy(gri->radar_name
		, etui->mc_lidars[etui->mc_lidar_ndx]->lidar_name);
	 gri->dd_radar_num = dd_assign_radar_num(gri->radar_name);
	 sq->etl_scan_type = mc_mom_ptr->beam_loc.scan_type;
	 break;

       case _2MICRON:
       default:
	 break;
      }
      if(etui->swpang_num++) {
	 sq->swpang_diff = angdiff(sq->last->swpang, sq->swpang);
	 if(sq->last->scan_mode != sq->scan_mode) {
	    gri->vol_num++; 
	    etui->new_vol = YES;
	 }
	 else if(etui->swpang_num > 1 &&
		 sq->last->swpang_diff * sq->swpang_diff < 0) {
	    gri->vol_num++; 
	    etui->new_vol = YES;
	 }
      }
      else {
	 gri->vol_num++; 
	 etui->new_vol = YES;
      }
      etui->fxdang_sum = 
	etui->az_diff_sum =
	  etui->el_diff_sum = 
	    etui->sweep_ray_num = 0;
	
      etui->sweep_reference_time = gri->time +
	etui->sweep_time_limit;

      etui->sweep_count_flag = YES;
      if(etui->new_vol) {
	 etui->vol_count_flag = YES;
      }
      sq->new_vol = etui->new_vol;

      if(etui->new_vol) {
	 dd_stats->vol_count++;

	 gri->pulse_width = ms;

	 switch(etui->lidar_number) {

	  case _2MICRON:
	    strcpy(gri->radar_name, "ETL2MICR");
	    gri->dd_radar_num = dd_assign_radar_num(gri->radar_name);
	    strcpy(gri->site_name, fhed_ptr->site_control.sitename);
	    gri->nyquist_vel = 25.25;
	    gri->freq[0] = SPEED_OF_LIGHT/gri->wavelength*1.e-9; /* GHz */
	    gri->num_ipps = gri->num_freq = 1;
	    gri->ipps[0] = 1./gri->PRF;
	    gri->radar_type = LIDAR_FIXED;
	    gri->h_beamwidth = ms;
	    gri->v_beamwidth = ms;
	    gri->pulse_width = ms;
	    gri->unamb_range = ms;
	    gri->wavelength = ms;
	    gri->noise_power = ms;
	    gri->rcvr_gain = ms;
	    gri->ant_gain = ms;
	    gri->rcv_bandwidth = ms;
	    gri->aperature_size = ms;
	    gri->field_of_view = ms;
	    gri->aperature_eff = ms;
	    break;

	  case MCAWS:
	    strcpy(gri->site_name, "UNKNOWN");
	    gri->nyquist_vel = 75.25;
	    gri->freq[0] = SPEED_OF_LIGHT/gri->wavelength*1.e-9; /* GHz */
	    gri->num_ipps = gri->num_freq = 1;
	    gri->ipps[0] = 1./gri->PRF;
	    gri->h_beamwidth = ms;
	    gri->v_beamwidth = ms;
	    gri->pulse_width = ms;
	    gri->unamb_range = ms;
	    gri->wavelength = ms;
	    gri->noise_power = ms;
	    gri->rcvr_gain = ms;
	    gri->ant_gain = ms;
	    gri->rcv_bandwidth = ms;
	    gri->aperature_size = ms;
	    gri->field_of_view = ms;
	    gri->aperature_eff = ms;

	    break;

	  default:
	    break;
	 }	      
	 /* set up lookup tables for moment data
	  */
	 etl_moment_fields();

	 r = gri->range_b1;
	 gs = gri->bin_spacing;
	 for(ii=0; ii < gri->num_bins; ii++ ) {
	    gri->range_value[ii] = r;
	    r += gs;
	 }
      }
   }
   
   if(etui->sweep_ray_num++) {
      etui->az_diff_sum += rq->az_diff;
      etui->el_diff_sum += rq->el_diff;
   }
   switch(etui->lidar_number) {
      
    case _2MICRON:
      gri->fixed = sq->scan_mode == RHI ? gri->azimuth : gri->elevation;
      break;
      
    case MCAWS:
      gri->fixed = sq->scan_mode == RHI ? gri->azimuth : gri->elevation;
      break;
      
    default:
      break;
   }	      
   sq->fxdang_sum += gri->fixed;
   gri->fixed = sq->fxdang_sum/(float)etui->sweep_ray_num;
   sq->swpang = gri->fixed;

   gri->source_sweep_num = 0;
   gri->source_sweep_num += 10 * (gri->sweep_num % 10);
   gri->source_sweep_num += 100 * (gri->vol_num % 10);

   dd_stats->ray_count++;
   dp = dd_return_next_packet(irq);
   dp->len = size;
   etui->count_rays++;
}
/* c------------------------------------------------------------------------ */

int etl_next_ray_mcaws (void)
{
   /* returns a pointer to the next block of hrd data
    * which corresponds to a physical record on the tape
    */
   static int count=0, bp=298;
   int32_t keep_offset, sos;
   int ii, mm, nn, mark, eof_count=0, recordlen=0, ugly;
   int Break=NO, gotta_mom=NO, itsa_header=YES;
   struct input_buf *inbuf;
   int size=-1, ugly_record_count=0, buf_count=0, target_size=0, tmp_size;
   int over_size;
   struct mc_lidar_info *mli;
   DD_TIME dts;
   char *aa, *bb, *cc = NULL, *ee;
   char *tmp_at;
   char *deep6=0;
   char mess[256];
   double d, angle;
   int32_t *lp, xdata_type, data_type_offset, last_data_type;

   /* c...mark */


   for(;;) {
      if(!count++) {
	 mark = 0;
      }
      if(count >= bp) {
	 mark = 0;
      }
      
      if(irq->top->bytes_left < irq->min_block_size) {
	 if(irq->top->bytes_left) {
	    printf("bytes left: %d\n", irq->top->bytes_left);
	    mark = 0;
	 }
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
	    printf("Last read: %d\n", irq->top->read_state);
	    nn = irq->top->read_state;
	    dd_input_read_close(irq);
	    /*
	     * see if there is another file to read
	     */
	    if(aa = dd_next_source_dev_name((char *)"ETL")) {
	       current_file_name = aa;
	       if((ii = dd_input_read_open(irq, aa)) <= 0) {
		  return(size);
	       }
	    }
	    else {
	       return(size);
	    }
	    continue;
	 }
	 else if(irq->top->eof_flag) {
	    eof_count++;
	    dd_stats->file_count++;
	    printf( "EOF number: %d at %.2f MB\n"
		   , dd_stats->file_count
		   , dd_stats->MB);

	    irq->top->bytes_left = 0;
	    if(eof_count > 3) {
	       return(size);
	    }
	    continue;
	 }
	 else {
	    dd_stats->rec_count++;
	    dd_stats->MB += BYTES_TO_MB(irq->top->sizeof_read);
	    eof_count = 0;
	 }
	 xdata_type = 0;
	 lp = (int32_t *)irq->top->at;
	 for(ii=0; ii < 256; ii++) {
	    switch(*(lp + ii)) {
	     case MC_MOM_HEADER :
	     case MC_COV_HEADER :
	     case MC_RAW_HEADER :
	     case MC_ANC_HEADER :
# ifdef obsolete
	       printf("buf: %8x  at: %8x  data_type: %8x\n"
		     , irq->top->buf, ii*sizeof(int32_t), *(lp + ii));
# endif
	       data_type_offset = ii * sizeof(int32_t);
	       xdata_type = *(lp +ii);
	       break;
	     default:
	       break;
	    }
	 }
	 mark = 0;
      }
      aa = irq->top->at;
      bb = aa + 2;
      cc = aa + 3;

      if(buf_count++ == 0) {
	 itsa_header = YES;
	 /* we're at the beginning of a structure */
	 if(hostIsLittleEndian()) {
//	    etui->data_type = PX4(aa);  //Jul 26, 2011
	    etui->data_type = PX4(*((fourB *)aa));  //Jul 26, 2011 need to confirm
	 }
	 else {
	    memcpy(&etui->data_type, aa, 4);
	 }
	 if(gotta_mom && etui->data_type == MC_MOM_HEADER) {
	    return(1);
	 }
	 switch(etui->data_type) {
	  case MC_MOM_HEADER :
	    tmp_at = (char *)mc_mom_ptr;
	    target_size = sizeof(struct moment_header_struct_mcws);
	    break;

	  case MC_COV_HEADER :
	    tmp_at = (char *)mc_cov_ptr;
	    target_size = sizeof(struct cov_header_struct_mcws);
	    break;

	  case MC_RAW_HEADER :
	    tmp_at = (char *)mc_raw_ptr;
	    target_size = sizeof(struct raw_header_struct_mcws);
	    break;

	  case MC_ANC_HEADER :
	    tmp_at = (char *)mc_anc_ptr;
	    target_size = sizeof(struct anc_header_struct_mcws);
	    break;

	  default:
	    if(etlDebug) {
	       printf("Bad data type at: %8x  data_type: %8x  count: %d\n"
		      , aa, etui->data_type, count);
	    }
	    if(xdata_type) {
	       irq->top->at = irq->top->buf + data_type_offset;
	       irq->top->bytes_left =
		 irq->top->sizeof_read - data_type_offset;
	       if(etlDebug) {
		  printf("new at: %8x last data type: %8x data_type %8x\n"
			 , irq->top->at, last_data_type, xdata_type);
	       }
	       buf_count = 0;
	    }
	    else {
	       irq->top->bytes_left = buf_count = 0;
	    }
	    continue;
	    break;
	 }	   
	 tmp_size = 0;
      }

      if(tmp_size + irq->top->bytes_left >= target_size) {	
	 /* don't overrun struct! */
	 over_size = tmp_size + irq->top->bytes_left - target_size;
	 memcpy(tmp_at, aa, irq->top->bytes_left - over_size);
	 tmp_at += irq->top->bytes_left - over_size;
	 irq->top->at += irq->top->bytes_left - over_size;
	 irq->top->bytes_left = over_size;

	 if(itsa_header) {	/* we've finished reading in a header */
	    itsa_header = NO;
	    last_data_type = etui->data_type;

	    switch(etui->data_type) {
	     case MC_MOM_HEADER :
	       gotta_mom = YES;
	       tmp_at = etui->mom_data;
//	       etui->mom_data_ptr = (short *)etui->mom_data;  //Jul 26, 2011
	       etui->mom_data_ptr = (unsigned short *)etui->mom_data;  //Jul 26, 2011
	       dts.time_stamp = mc_mom_ptr->flight_info.end_time;
	       d_unstamp_time(&dts);
	       if(etlDebug) {
		  printf("Mom end_time: %s \n"
			 , dts_print(&dts));
	       }
	       target_size = 6 * mc_mom_ptr->ocs_info.num_gates;

	       gri->PRF = mc_mom_ptr->lidar_info.rep_rate;
	       gri->time = mc_mom_ptr->flight_info.end_time;

	       if(gri->time > etui->sweep_reference_time) {
		  etui->new_sweep = YES;
	       }
	       gri->azimuth = mc_mom_ptr->beam_loc.beam_losa * .01;
	       gri->elevation = mc_mom_ptr->beam_loc.beam_lose * .01;
	       /* corrections? */
	       gri->azimuth = FMOD360(gri->azimuth -180.);
	       gri->elevation *= -1.;
	       gri->heading = mc_mom_ptr->plat_info.true_hdg * .1;

	       gri->num_samples = mc_mom_ptr->ocs_info.num_pulses;
	       gri->bin_spacing = mc_mom_ptr->ocs_info.gate_width;
	       gri->range_b1 = 0;
	       gri->num_bins = mc_mom_ptr->ocs_info.num_gates;
	       d = mc_mom_ptr->location_info.lat;
	       gri->latitude = X32BIT_ANG(d);
	       d = mc_mom_ptr->location_info.lng;
	       gri->longitude = X32BIT_ANG(d);
	       d = mc_mom_ptr->location_info.p_alitude;
	       gri->altitude = FT_TO_METERS(d);
	       d = mc_mom_ptr->location_info.r_alitude;
	       gri->altitude_agl = FT_TO_METERS(d);

	       switch((int)mc_mom_ptr->beam_loc.scan_type) {

		case 3:		/* 2D scan */
		  if(mc_mom_ptr->beam_loc.beam_index < 6 &&
		     etui->ray_que->etl_beam_index > 5) {
		     /* switched from fore to aft */
		     etui->new_sweep = YES;
		  }
		  else if(mc_mom_ptr->beam_loc.beam_index > 5 &&
			etui->ray_que->etl_beam_index < 6) {
		     /* switched from aft to fore */
		     etui->new_sweep = YES;
		  }
		  gri->scan_mode = RHI;
		  gri->radar_type = AIR_LF;
		  /* heading is already part of the azimuth */
		  gri->radar_type = LIDAR_MOVING;

		  gri->rotation_angle = gri->corrected_rotation_angle = 
		    gri->elevation;
		  d = gri->azimuth - (gri->heading -90.);
		  gri->tilt = FMOD360(d);
		
		  if(mc_mom_ptr->beam_loc.beam_index < 6) {
		     etui->mc_lidar_ndx = MC_AFT;
		  }
		  else {
		     etui->mc_lidar_ndx = MC_FORE;
		  }
		  break;

		case 2:		/* side, profile or pointing? */
		  gri->rotation_angle = gri->corrected_rotation_angle = 
		    gri->elevation;
		  gri->tilt = gri->azimuth;
		  etui->mc_lidar_ndx = MC_FXD;
		  gri->scan_mode = TAR;	/* target */
		  gri->radar_type = AIR_LF;
		  /* heading is already part of the azimuth */
		  gri->radar_type = LIDAR_MOVING;
		  break;

		case 1:		/* manual? */
		default:
		  gri->rotation_angle = gri->corrected_rotation_angle = 
		    gri->elevation;
		  gri->tilt = gri->azimuth;
		  etui->mc_lidar_ndx = MC_FXD;
		  gri->scan_mode = MAN;
		  gri->radar_type = AIR_LF;
		  /* heading is already part of the azimuth */
		  gri->radar_type = LIDAR_MOVING;
		  break;
	       }
	       if(etui->swp_que->etl_scan_type !=
		  mc_mom_ptr->beam_loc.scan_type) {
		  etui->new_sweep = YES;
	       }
	       gri->drift = mc_mom_ptr->plat_info.drift_angle * .1;
	       gri->roll = mc_mom_ptr->plat_info.roll * .1;
	       gri->pitch = mc_mom_ptr->plat_info.pitch * .1;
	       gri->track = mc_mom_ptr->plat_info.track * .1;

	       angle = (double)RADIANS((CART_ANGLE(gri->track)));
	       d = KNOTS_TO_MS(mc_mom_ptr->plat_info.gnd_speed);

	       gri->vns = d * sin(angle);
	       gri->vew = d * cos(angle);;
	       gri->vud = ms;
	       gri->ui = ms;
	       gri->vi = ms;
	       gri->wi = ms;
	       gri->sweep_speed = ms;
	       break;

	     case MC_COV_HEADER :
	       tmp_at = etui->cov_data;
//	       etui->cov_data_ptr = (short *)etui->cov_data; //Jul 26, 2011
	       etui->cov_data_ptr = (unsigned short *)etui->cov_data; //Jul 26, 2011
	       dts.time_stamp = mc_cov_ptr->flight_info.end_time;
	       d_unstamp_time(&dts);
	       if(etlDebug) {
		  printf("Cov end_time: %s \n"
			 , dts_print(&dts));
	       }
	       target_size = 4 * mc_cov_ptr->ocs_info.num_gates
		 * mc_cov_ptr->ocs_info.num_lags;
	       break;

	     case MC_RAW_HEADER :
	       tmp_at = etui->raw_data;
//	       etui->raw_data_ptr = (short *)etui->raw_data;  //Jul 26, 2011
	       etui->raw_data_ptr = (unsigned short *)etui->raw_data;  //Jul 26, 2011
	       /*
		* VERIFY this target size!!!
		*/
	       dts.time_stamp = mc_raw_ptr->flight_info.end_time;
	       d_unstamp_time(&dts);
	       if(etlDebug) {
		  printf("Raw end_time: %s \n"
			 , dts_print(&dts));
	       }
	       target_size = mc_raw_ptr->ocs_info.num_gates * 2;
	       break;

	     case MC_ANC_HEADER :
	       /* no data with the ancillary header */
	       dts.time_stamp = mc_anc_ptr->flight_info.end_time;
	       d_unstamp_time(&dts);
	       if(etlDebug) {
		  printf("Anc end_time: %s count: %d \n"
			 , dts_print(&dts), count);
	       }
	       dts.time_stamp = mc_anc_ptr->dads_inu.time;
	       d_unstamp_time(&dts);
	       buf_count = 0;
	       break;
	    }	   
	 }
	 else {			/* we've finished reading in the data */
	    buf_count = 0;
	    switch(etui->data_type) {
	     case MC_MOM_HEADER :
	       break;
	     case MC_COV_HEADER :
	       break;
	     case MC_RAW_HEADER :
	       break;
	    }	   
	 }
      }
      else {
	 memcpy(tmp_at, aa, irq->top->bytes_left);
	 tmp_size += irq->top->bytes_left;
	 tmp_at += irq->top->bytes_left;
	 irq->top->bytes_left = 0;
      }
      if(Break) { break; }
   }
   size = recordlen;
    
   return(size);
}
/* c------------------------------------------------------------------------ */

int etl_next_ray_2micron (void)
{
    /* returns a pointer to the next block of hrd data
     * which corresponds to a physical record on the tape
     */
    static int count=0, bp=298;
    int32_t keep_offset, sos;
    int ii, mm, nn, mark, eof_count=0, recordlen=0, ugly;
    int Break=NO, gotta_mom=NO;
    int size=-1, ugly_record_count=0;
    char *aa, *bb, *cc = NULL, *ee;
    char str[256];

    /* c...mark */


    if(!count++) {
	mark = 0;
    }
    if(count >= bp) {
	mark = 0;
    }

    for(;;) {
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
		printf("Last read: %d\n", irq->top->read_state);
		nn = irq->top->read_state;
		dd_input_read_close(irq);
		/*
		 * see if there is another file to read
		 */
		if(aa = dd_next_source_dev_name((char *)"ETL")) {
		    current_file_name = aa;
		    if((ii = dd_input_read_open(irq, aa)) <= 0) {
			return(size);
		    }
		}
		else {
		    return(size);
		}
		continue;
	    }
	    else if(irq->top->eof_flag) {
		eof_count++;
		dd_stats->file_count++;
		sprintf(str, "EOF at %s %.2f MB file: %d rec: %d ray: %d"
			, dts_print(d_unstamp_time(gri->dts))
			, dd_stats->MB
			, dd_stats->file_count, dd_stats->rec_count
			, dd_stats->ray_count
			);
		printf("%s\n", str);
		dd_append_cat_comment(str);
# ifdef obsolete
		printf( "EOF number: %d at %.2f MB\n"
		       , dd_stats->file_count
		       , dd_stats->MB);
# endif
		irq->top->bytes_left = 0;
		if(eof_count > 3) {
		    return(size);
		}
		if(gotta_mom) {
		   return(1);
		}
		continue;
	    }
	    else {
		dd_stats->rec_count++;
		dd_stats->MB += BYTES_TO_MB(irq->top->sizeof_read);
		eof_count = 0;
	    }
	}
	aa = irq->top->at;
	bb = aa + 2;
	cc = aa + 3;

	if(hostIsLittleEndian()) {
//	   etui->data_type = PX4(aa);  //Jul 26, 2011
	   etui->data_type = PX4(*(fourB*)aa);  //Jul 26, 2011
	}
	else {
	   memcpy(&etui->data_type, aa, 4);
	}
	switch(etui->data_type)
	  {
	   case MC_ANC_HEADER :
	   case MC_RAW_HEADER :
	   case MC_COV_HEADER :
	   case MC_MOM_HEADER :
	     etui->lidar_number = MCAWS;
	     recordlen = REALLY_MCAWS;
	     Break = YES;
	     break;

	   case MOM_HEADER :
	     if(gotta_mom) { Break = YES; break; }

	     gotta_mom = YES;
	     if(hostIsLittleEndian()) {
	     }
	     else {
		memcpy(mom_ptr, aa, sizeof(struct moment_header_struct));
		sos = sizeof(struct moment_header_struct);
	     }		
	     recordlen = sos;
	     etui->mom_data_ptr = (unsigned short *)(irq->top->at + sos);
	     irq->top->bytes_left = 0;
	     gri->time = mom_ptr->position_info.utc_time;
	     gri->azimuth = mom_ptr->scan_info.azimuth;
	     gri->elevation = mom_ptr->scan_info.elevation;
	     gri->PRF = mom_ptr->lidar_info.rep_rate;
	     gri->bin_spacing = mom_ptr->ocs_info.gate_width;
	     gri->range_b1 = mom_ptr->ocs_info.range_to_first_gate;
	     gri->num_bins = mom_ptr->ocs_info.num_gates;
	     gri->latitude = mom_ptr->position_info.latitude;
	     gri->longitude = mom_ptr->position_info.longitude;
	     gri->wavelength =
	       mom_ptr->lidar_info.operating_wavelength * 1.e-6;
	     switch(gri->scan_mode) {
	      case RHI:
		gri->rotation_angle = gri->corrected_rotation_angle = 
		  FMOD360(CART_ANGLE(gri->elevation));
		gri->tilt = gri->azimuth;
		break;
	      default:
		gri->rotation_angle = gri->corrected_rotation_angle = 
		  gri->azimuth;
		gri->tilt = gri->elevation;
		break;
	     }

	     break;
	     
	   case COV_HEADER :
	     if(hostIsLittleEndian()) {
	     }
	     else {
		memcpy(cov_ptr, aa, sizeof(struct cov_header_struct));
		recordlen = sizeof(struct cov_header_struct);
	     }
	     irq->top->bytes_left = 0;
	     break;

	   case RAW_HEADER :
	     if(hostIsLittleEndian()) {
	     }
	     else {
		memcpy(raw_ptr, aa, sizeof(struct raw_header_struct));
		recordlen = sizeof(struct raw_header_struct);
	     }
	     irq->top->bytes_left = 0;
	     break;

	   case ANC_HEADER :
	     if(hostIsLittleEndian()) {
	     }
	     else {
		memcpy(anc_ptr, aa, sizeof(struct anc_header_struct));
		recordlen = sizeof(struct anc_header_struct);
	     }
	     irq->top->bytes_left = 0;
	     break;

	   case FILE_HEADER :
	     if(hostIsLittleEndian()) {
	     }
	     else {
		memcpy(fhed_ptr, aa, sizeof(struct file_header_struct));
		recordlen = sizeof(struct file_header_struct);
	     }
	     etui->data_file_header = YES;
	     etui->lidar_number = fhed_ptr->data_file.lidar_number;
	     etui->lidar_number = _2MICRON;
	     etui->new_sweep = YES;
	     gri->num_fields_present =
	       fhed_ptr->data_file.number_data_fields;
	     gri->num_samples = fhed_ptr->dsp_control.num_pulses_avg;
	     gri->altitude = fhed_ptr->site_control.altitude;

	     switch(fhed_ptr->scan_control.scan_type) {
	      case 0:
		gri->scan_mode = VER;
		gri->fixed = fhed_ptr->scan_control.beg_elevation;
		break;
		
	      case 1:
		gri->scan_mode = PPI;
		gri->fixed = fhed_ptr->scan_control.beg_elevation;
		break;
		
	      case 2:
		gri->scan_mode = RHI;
		gri->fixed = fhed_ptr->scan_control.beg_azimuth;
		break;
		
	      default:
		break;
	     }
	     irq->top->bytes_left = 0;
	     break;
	     
	   case CLOSEFILE :
	     irq->top->bytes_left = 0;
	     break;

	   default:
	     irq->top->bytes_left = 0;
	     break;
	  }
	if(Break) { break; }
    }
    size = recordlen;

    return(size);
}
/* c------------------------------------------------------------------------ */

void etl_positioning (void)
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
Option = "
	   );
    nn = getreply(str, sizeof(str));
    if( cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 13 ) {
	if(ival == -2) exit(1);
	printf( "\nIllegal Option!\n" );
	goto menu2;
    }
    etui->new_sweep = NO;
    etui->new_vol = NO;

    if(ival == -1) {
	solo_unmalloc_list_mgmt(slm);
	return;
    }
    else if(ival < 0)
	  exit(0);
    else if(ival == 0) {
	printf("Type number of rays to skip:");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > K64) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	if(ival > 0) {
	    for(ii=0; ii < ival; ii++) {
		etui->new_sweep = NO;
		etui->new_vol = NO;
		if((jj=etl_next_ray()) < 0)
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
	}
	if(ival < 0)
	    { etl_reset(); }
	etl_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 1) {
	if((nn = etl_inventory(irq)) == -2)
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
	etl_reset();
	dd_stats_reset();
	etl_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 3) {
	dd_rewind_dev(irq);
	dd_stats_reset();
	etl_reset();
	etl_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 4) {
	if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
	    printf("\n");
	    ctypeu16((unsigned char *)irq->top->buf, view_char, view_char_count);
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
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > 1000000) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	direction = ival >= 0 ? FORWARD : BACKWARD;
	dd_skip_recs(irq, direction, ival > 0 ? ival : -ival);
	if(ival < 0)
	    { etl_reset(); }
	etl_next_ray();
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
	    if((nn = etl_next_ray()) <= 0 || gri->time >= dtarget ||
	       dd_control_c()) {
		break;
	    }
	    if(!(mm % 1000)) {
		gri_interest(gri, 1, preamble, postamble);
		printf("%s\n", dd_stats_sprintf());
	    }
	    etui->new_vol = etui->new_sweep = NO;
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
	    if((nn = etl_next_ray()) <= 0 || in_sector(gri->fixed, fx1, fx2)
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
	etl_print_headers(slm);
    }
    else if(ival == 12) {
	etl_dump_this_ray(slm);
    }

    preamble[0] = '\0';

    goto menu2;
}
/* c------------------------------------------------------------------------ */

void etl_reset (void)
{
    etui->vol_count_flag = etui->sweep_count_flag =
	  etui->new_sweep = etui->new_vol = YES;
    etui->count_rays = etui->ray_count = etui->sweep_count = etui->vol_count = 0;
    etui->ref_time = etui->trip_time = 0;
    etui->swpang_num = 0;
    difs->stop_flag = NO;
    /*
     * make sure to start a new sweep and vol
     */
    gri->vol_num++;
    gri->sweep_num++;
}
/* c------------------------------------------------------------------------ */

int etl_select_ray (void)
{
    int ii, jj, ok=YES, mark;


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
	if(dd_stats->ray_count % difs->beam_skip){
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
	       gri->fixed <= difs->fxd_angs[ii]->upper)
		  break;
	}
	if(ii == difs->num_fixed_lims)
	      ok = NO;
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

    return(ok);
}
/* c------------------------------------------------------------------------ */

void etl_nab_mom_data (void)
{
   int pn=0, nn;
   unsigned short *us;
   short *tt, *lut, *ss;

   us = etui->mom_data_ptr;
//   ss = etui->mom_data_ptr;  //Jul 26, 2011
   ss = (short *)etui->mom_data_ptr;  //Jul 26, 2011
   
   for(; pn < 3; pn++) {
      tt = gri->scaled_data[pn];
      lut = etui->mom_lut[pn];
      nn = gri->actual_num_bins[pn];

      for(; nn-- ; us++, ss++) {
	  *tt++ = *(lut + (*us));
      }
   }
}
/* c------------------------------------------------------------------------ */

void etl_print_stat_line (int count)
{
    DD_TIME *dts=gri->dts;
    
    dts->time_stamp = gri->time;
    d_unstamp_time(dts);
    printf(" %5d s:%3d %7.2f %.2f  %s at %.2f MB rec:%d\n"
	   , count
	   , gri->sweep_num
	   , gri->fixed
	   , dts->time_stamp
	   , dts_print(dts)
	   , dd_stats->MB
	   , dd_stats->rec_count
	   );
}
/* c------------------------------------------------------------------------ */

void etl_print_headers (struct solo_list_mgmt *slm)
{
}
/* c------------------------------------------------------------------------ */

void etl_dump_this_ray (struct solo_list_mgmt *slm)
{
    int gg, ii, nf;
    char *aa, str[128];
    float unscale[7], bias[7];

    nf = gri->num_fields_present <= 7 ? gri->num_fields_present : 7;
    aa = str;
    etl_nab_data();
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

int etl_inventory (struct input_read_info *irq)
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
	    etui->new_sweep = NO;
	    etui->new_vol = NO;
	    if((nn=etl_next_ray()) < 1) {
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

void etl_moment_fields (void)
{
    int ii, pn=0, nbins;
    float f, scale = 100., bias = 0, r, gs, vel_scale, intensity_scale;
    float ncp_scale;
    char *aa;
    short *ss, sval;

    
    switch(etui->lidar_number) {
       
     case _2MICRON:
       if(gri->time > Oct_1_1996) {
	   vel_scale = 1./128. * NI_QUEST/4.; /* nyq vel: 25.27 */
	   intensity_scale = 70./1024;
	   ncp_scale = 1./255;
       }
       else {
	   vel_scale = 1./128. * NI_QUEST/4.; /* nyq vel: 25.27 */
	   intensity_scale = 70./1024;
	   ncp_scale = 1./255;
       }
       break;

     case MCAWS:
       vel_scale = 1./128. * 75.25/4.;
       intensity_scale = 70./1024;
       ncp_scale = 1./255;
       break;
    }

    for(pn=0; pn < 3; pn++) {
       if(!(ss = etui->mom_lut[pn])) {
	  ss = etui->mom_lut[pn] = (short *)
	    malloc(K64 * sizeof(short));
	  memset(ss, 0, K64 * sizeof(short));
       }
    }
    nbins = gri->num_bins;
    gri->num_fields_present = 0;

    /* velocity */

    pn = gri->num_fields_present++;
    gri->dd_scale[pn] = scale;
    gri->dd_offset[pn] = bias;
    gri->field_id_num[pn] = VE_ID_NUM;
    strcpy( gri->field_name[pn], "VE" );
    strncpy( gri->field_long_name[pn]
	    , "Doppler velocity                                         "
	    , 40);
    strcpy( gri->field_units[pn], "m/s     " );
    gri->actual_num_bins[pn] = nbins;

    ss = etui->mom_lut[pn];

    for(ii = 0; ii < K64; ii++) {
       sval = (short)ii;	/* turn it into a signed short */
       f = (float)sval * vel_scale;
       *(ss + ii) = (short)DD_SCALE(f, scale, bias);
    }

    /* intensity */

    pn = gri->num_fields_present++;
    gri->dd_scale[pn] = 1;
    gri->dd_offset[pn] = bias;
    gri->field_id_num[pn] = 0;
    strcpy( gri->field_name[pn], "INT" );
    strncpy( gri->field_long_name[pn]
	    , "Lidar Intensity                                          "
	    , 40);
    strcpy( gri->field_units[pn], "???     " );
    gri->actual_num_bins[pn] = nbins;

    ss = etui->mom_lut[pn];

    for(ii = 0; ii < K64; ii++) {
       sval = (short)ii;
       f = (float)sval * intensity_scale;
       *(ss + ii) = (short) DD_SCALE(f, 1, bias);
    }

    /* NCP */

    pn = gri->num_fields_present++;
    gri->dd_scale[pn] = scale;
    gri->dd_offset[pn] = bias;
    gri->field_id_num[pn] = 0;
    strcpy( gri->field_name[pn], "NCP" );
    strncpy( gri->field_long_name[pn]
	    , "Normalized Coherent Power                                    "
	    , 40);
    strcpy( gri->field_units[pn], "???     " );
    gri->actual_num_bins[pn] = nbins;

    ss = etui->mom_lut[pn];

    for(ii = 0; ii < K64; ii++) {
       f = (float)ii * ncp_scale;
       *(ss + ii) = (short) DD_SCALE(f, scale, bias);
    }
}
/* c------------------------------------------------------------------------ */

void etl_nab_data (void)
{
   switch(etui->data_type)
     {
      case MOM_HEADER :
      case MC_MOM_HEADER :
	etl_nab_mom_data();
	break;

      case RAW_HEADER :
	break;
     }
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/*
 * Shane's fortran code for 2 micron
 *
      program dopl3
c===================================================================
c     Written by S Mayor in May of 1996.
c     This version modified from dopl2.f on 7/10/96
c     This version reads version 2010 tapes.
c
c     This program requires mtmnt, mtread, mtwait, and mtskpf
c     subroutines which are in the NCAR-ATD library.
c
c     This program reads DAT tapes written by the 2-micron Doppler
c     lidar real-time computer and writes BSCAN files to hard disk.
c     Real-time data tapes may contain moment and/or raw data.
c
c     This program depends on the first four bytes all records
c     except for data records to be identified with one of the
c     following:
c                 0 0 70 70  (file header)
c                 0 0 77 77  (moment header)
c                 0 0 82 82  (raw header)
c                 0 0 67 70  (EOF)
c
c     A moment header physical record is always followed by 
c     a physical record containing only moment data.
c
c     A raw header physical record is always followed by 
c     a physical record containing only i and q data.
c
c     11 output files may be opened when this program runs.
c     1 ..... post-analysis radial velocities
c     2 ..... post-analysis relative backscatter (total) 
c     3 ..... post-analysis relative backscatter (coherent)
c     4 ..... post-analysis NCP
c     5 ..... post-analysis SNR (in dB from total power)
c     6 ..... post-analysis SNR (in dB from coherent power)
c     7 ..... real-time in-phase data (from Lassen)
c     8 ..... real_time quadrature data (from Lassen)
c     9 ..... real-time radial velocities (from Lassen)
c     10 .... real-time intensity (from Lassen)
c     11 .... real-time NCP (from Lassen)
c
c============================================================
      implicit none

      external mtmnt

      byte byte_buf1(32768)

      real*4 i_data(2000), q_data(2000)
      real*4 rt_vel(2000), rt_int(2000), rt_ncp(2000)
      real*4  header(30), flag, pi, const, norm
      integer*4 nwords, istate, mtmnt, skip_files
      integer*4 tape_drive_select, red_rec_len, red_rec
      integer*4 skip_moment_files, skip_raw_files
      integer*4 j, k, file_num, phys_rec, lun, gate
      integer*4 n1, n2, lnblnk, ngates, fields
      integer*4 raw_files_open, mom_files_open
      integer*4 tarray(9), print_mom_info
      integer*4 cat_mom_files, cat_raw_files
      character*1 answer, advance_tape
      character*60 path, filename(11), primary
      character*60 time_string, ctime

      structure / data_file_info / 
                integer*2 file_number
                integer*2 version_number
                integer*2 lidar_number
                integer*2 data_type
                integer*2 number_data_fields
                integer*2 start_year
                integer*2 start_month
                integer*2 start_day
                integer*2 start_hour
                integer*2 start_minute
                integer*2 start_second
                integer*2 start_hund_sec
                integer*4 time_utc
                integer*4 number_of_records
                integer*2 spare 
                integer*2 platform_type
                real*4 filler1(4)
      end structure

      structure / descriptive_info /
                character exp_name(32)
                character metalog(248)
      end structure

      structure / gps_info_parm /
                integer*4 gps_time_sec
                integer*4 gps_time_nsec
                real*4 gps_heading
                real*4 gps_pitch
                real*4 gps_roll
                real*4 gps_mrms
                real*4 gps_brms
                integer*4  gps_bad_altitude_flag
                integer*4 filler
                real*4 gps_stddev_delta_heading
                real*4 gps_stddev_delta_pitch
                real*4 gps_stddev_delta_roll
                real*4 gps_utc_time
                real*4 gps_latitude
                real*4 gps_longitude 
                real*4 gps_altitude
                real*4 gps_course_over_ground
                real*4 gps_speed_over_ground
                real*4 gps_vertical_velocity
                real*4 gps_pdop
                real*4 gps_hdop
                real*4 gps_vdop
                real*4 gps_tdop
                real*4 spare(2)
      end structure

      structure / lidar_info_parm / 
                real*4 rep_rate
                real*4 operating_wavelength
                real*4 zero_beat_freq
                integer*4 mode
                integer*2 actual_num_bad_pulses
                integer*2 filler(3)
                real*4 pulse_frequency
                real*4 qs_but
                real*4 pzt_volt
                real*4 pulse_energy
                real*4 freq_mean
                real*4 freq_variance
                integer*4 spare(4)
      end structure

      structure / ocs_info_parm / 
                integer*2 ocs_status
                integer*2 data_recording
                integer*2 current_file_no
                integer*2 num_gates
                integer*2 gate_width
                integer*2 range_to_first_gate
                integer*2 num_to_integrate
                integer*2 spare1
                integer*4 current_rec
                integer*4 current_beam_rec
                integer*2 spare2(10)
      end structure

      structure / plat_env_parm / 
                real*4 static_temp
                real*4 dewpt_temp
                real*4 sfc_temp
                real*4 wind_speed
                real*4 wind_direction
                real*4 pressure
                real*4 spare(4)
      end structure

      structure / position_info_parm /
                integer*4 utc_time
                real*4 latitude
                real*4 longitude
                real*4 spare1
                integer*2 month
                integer*2 day
                integer*2 year
                integer*2 hour
                integer*2 minute
                integer*2 second
                integer*2 millisecond
                integer*2 spare2(3)
                integer*4 microsecond
      end structure

      structure / scan_info_parm / 
                real*4 azimuth
                real*4 elevation
		integer*2 scan_type
                integer*2 spare(5)
      end structure

      structure / inu_info_parm /
                integer*4 inu_time_sec
                integer*4 inu_time_nsec
                real*4 inu_pitch
                real*4 inu_roll
                real*4 inu_yaw
                real*4 inu_pitch_rate
                real*4 inu_roll_rate
                real*4 inu_yaw_rate
                real*4 inu_x_velocity
                real*4 inu_y_velocity
                real*4 inu_z_velocity
                real*4 inu_x_accel
                real*4 inu_y_accel
                real*4 inu_z_accel
                real*4 spare1(2)
                integer*2 inu_ang_tilt
                integer*2 spare2(3)
      end structure

      structure / scan_control_parm / 
                integer*2 scan_type
                integer*2 scanner
                integer*2 compensation_flag
                integer*2 coord_file_input
                integer*2 profiler_sync_mode
                integer*2 spare1
                character scan_name(24)
                real*4 beg_azimuth
                real*4 end_azimuth
                real*4 azimuth_step
                real*4 beg_elevation
                real*4 end_elevation
                real*4 elevation_step
                real*4 coord_buf(120)
                integer*4 num_steps
                integer*4 spare2(5)
      end structure
   
      structure / noise_block_parm / 
                real*4 noise_values(14)
      end structure

      structure / dsp_control_parm / 
                integer*4 num_beams_to_send
                integer*2 data_type
                integer*2 AD_rate
                integer*2 number_lags
                integer*2 num_pulses_avg
                integer*2 gate_width
                integer*2 range_to_first_gate
                integer*2 num_gates_pulse
                integer*2 num_to_integrate
                integer*2 freq_corr
                integer*2 start_pt_freq_corr
                integer*2 num_freq_corr_pts
                integer*2 raw_start_pt
                integer*2 raw_num_samples
                integer*2 enable_bad_pul_reject
                integer*2 pulse_rejection_thres
                integer*2 num_bad_pulses
                integer*2 noise_corr_flag
                integer*2 start_noise_gate 
                integer*2 num_noise_gates
                integer*2 spare(5)
      end structure
   
      structure / site_control_parm / 
                integer*2 lidar_number
                integer*2 altitude
                character sitename(16)
                integer*2 latitude_deg
                integer*2 latitude_min
                integer*2 latitude_sec
                integer*2 longitude_deg
                integer*2 longitude_min
                integer*2 longitude_sec
                integer*2 spare(4)
      end structure

      structure / cov_header_struct /
                integer*4 record_type
                record / position_info_parm / position_info
                record / scan_info_parm / scan_info
                record / gps_info_parm / gps_info
                record / inu_info_parm / inu_info
                record / plat_env_parm / plat_env
                record / lidar_info_parm / lidar_info
                record / noise_block_parm / noise_block
                record / ocs_info_parm / ocs_info
                integer*2 spare(8)
      end structure
                
      structure / raw_header_struct /
                integer*4 record_type
                record / position_info_parm / position_info
                record / scan_info_parm / scan_info
                record / gps_info_parm / gps_info
                record / inu_info_parm / inu_info
                record / lidar_info_parm / lidar_info
                record / ocs_info_parm / ocs_info
                integer*2 raw_word_size
                integer*2 spare(9)
      end structure

      structure / file_header_struct / 
                integer*4 record_type
                integer*4 data_ident_key
                record / data_file_info / data_file
                record / scan_control_parm / scan_control
                record / dsp_control_parm / dsp_control
                record / site_control_parm / site_control
                record / descriptive_info / descriptive
                integer*2 data_recording
                integer*2 spare(9)
      end structure

      structure /moment_header_struct/ 
                integer*4 record_type
                record / position_info_parm / position_info
                record / scan_info_parm / scan_info
                record / gps_info_parm / gps_info
                record / inu_info_parm / inu_info
                record / plat_env_parm / plat_env
                record / lidar_info_parm / lidar_info
                record / noise_block_parm / noise_block
                record / ocs_info_parm / ocs_info
                integer*2 spare(8)
      end structure

      structure /anc_header_struct/
                integer*4 record_type
                record / position_info_parm / position_info
                record / data_file_info / data_file
                record / ocs_info_parm / ocs_info
                record / lidar_info_parm / lidar_info
                integer*2 spare(10)
      end structure

      structure / moment_record_struct /
                record / moment_header_struct / mom_header
                integer*2 intby2_buf(1200)
      end structure

      structure / raw_record_struct /
                record / raw_header_struct / raw_header
                integer*2 intby2_buf(7200)
      end structure

      record / file_header_struct / file_header
      record / anc_header_struct / anc_header
      record / raw_record_struct / raw_record
      record / moment_record_struct / mom_record

      equivalence (byte_buf1, file_header)
      equivalence (byte_buf1, raw_record)
      equivalence (byte_buf1, mom_record)
      equivalence (byte_buf1, anc_header)

      parameter (Pi = 3.141592654)
      parameter (const = (1./(4.*Pi))*(10.59e-6/1e-7))
      parameter (norm = 250.*250.)
      parameter (flag = -999.)

      print*,' '
      print*,'2-micron Doppler processing program.'
      print*,' '

75    print*,'Please choose a tape device name to read from.'
      print*,'If running on Steam:'
      print*,'1) /dev/exa0 (left side)'
      print*,'2) /dev/exa1 (right side)'
      print*,'Otherwise, select from:'
      print*,'3) /dev/rmt/0b'
      print*,'4) /dev/rmt/1b'
      print*,'5) /dev/rmt/2b'
      print*,'6) /dev/rmt/3b'
      print*,'7) quit program'
      print*,'Enter a number 1 - 7:'
      read(*,100) tape_drive_select
100   format(i1)
      if ((tape_drive_select .lt. 1) .or.
     +    (tape_drive_select .gt. 7)) then 
	  print*,' '
	  print*,'Sorry, you may only select 1-t.  Try again.'
	  print*,' '
	  goto 75
      endif
      if (tape_drive_select .eq. 1) call mtsdev('/dev/exa0')
      if (tape_drive_select .eq. 2) call mtsdev('/dev/exa1')
      if (tape_drive_select .eq. 3) call mtsdev('/dev/rmt/0b')
      if (tape_drive_select .eq. 4) call mtsdev('/dev/rmt/1b')
      if (tape_drive_select .eq. 5) call mtsdev('/dev/rmt/2b')
      if (tape_drive_select .eq. 6) call mtsdev('/dev/rmt/3b')
      if (tape_drive_select .eq. 7) then 
                stop
      endif
      print*,' '

      if (mtmnt (1) .eq. 0) then 
          print*,'Unable to mount tape.'
          call exit (0)
      else
          print*,'Tape mounted.'
      endif

      path = '/scr/waltz3/lidar/'
200   print*,' '
      print*,'Current default path to write output files is: '
      print*,path
      print*,'Do you want to keep this? (Y or N, default is Y)'
      read(*,300) answer
300   format(a1)
      if (answer .eq. 'n') answer = 'N'
      if (answer .ne. 'N') answer = 'Y'
      if (answer .eq. 'N') then
         print*,' '
         print*,'Enter the new path.  (Be sure to include all slashes!)'
         read(*,400) path
400      format(a40)
         print*,' '
       print*,'Is this the path you really want? (Y or N, Y is default)'
         read(*,300) answer
         if (answer .eq. 'n') answer = 'N'
         if (answer .ne. 'N') answer = 'Y'
         if (answer .eq. 'N') goto 200
      endif

      n1 = lnblnk(path)

      print*,' '
      print*,'Path selected = ',path
500   print*,' '

      print*,'Enter the primary file name to write to.'
      print*,'The primary file name does not include'
      print*,'the path or file extension. (Example: 091295_a)'
      read(*,400) primary
      n2 = lnblnk(primary)
      print*,' '
      print*,'Are you sure? (Y or N, Y is default)'
      read(*,300) answer
      if (answer .eq. 'n') answer = 'N'
      if (answer .ne. 'N') answer = 'Y'
      if (answer .eq. 'N') goto 500

      print*,' ' 
      print*,'Do you want to skip real-time moment files?'
      print*,'(Y or N, N is default)'
      read(*,300) answer
      if (answer .eq. 'y') answer = 'Y'
      if (answer .ne. 'Y') skip_moment_files = 0
      if (answer .eq. 'Y') skip_moment_files = 1

      print*,' ' 
      print*,'Do you want to skip raw (I and Q) files?'
      print*,'(Y or N, N is default)'
      read(*,300) answer
      if (answer .eq. 'y') answer = 'Y'
      if (answer .ne. 'Y') skip_raw_files = 0
      if (answer .eq. 'Y') skip_raw_files = 1

      if ((skip_moment_files .eq. 1) .and.
     +    (skip_raw_files .eq. 1)) then 
           print*,'Program ending since you have chosen to skip'
           print*,'both moment and raw data files.' 
           print*,' '
           goto 999
      endif

      if (skip_moment_files .eq. 0) then 
          print*,' '
          print*,'Concatenate all moment files into one bscan file?'
          print*,'(Y or N, N is default)'
          read(*,300) answer
          if (answer .eq. 'y') answer = 'Y'
          if (answer .ne. 'Y') cat_mom_files = 0
          if (answer .eq. 'Y') cat_mom_files = 1
      endif

      if (skip_raw_files .eq. 0) then 
          print*,' '
          print*,'Concatenate all raw files into one bscan file?'
          print*,'(Y or N, N is default)'
          read(*,300) answer
          if (answer .eq. 'y') answer = 'Y'
          if (answer .ne. 'Y') cat_raw_files = 0
          if (answer .eq. 'Y') cat_raw_files = 1
      endif

      print*,' '
      print*,'Print moment header information to screen?'
      print*,'(Y or N, N is default)'
      read(*,300) answer
      if (answer .eq. 'y') answer = 'Y'
      if (answer .ne. 'Y') print_mom_info = 0
      if (answer .eq. 'Y') print_mom_info = 1

      print*,' '
      print*,'Advance tape by skipping files? (Y or N, N is default)'
      read(*,300) advance_tape
      if (advance_tape .eq. 'y') advance_tape = 'Y'
      if (advance_tape .ne. 'Y') advance_tape = 'N'
      if (advance_tape .eq. 'N')
     +   print*,'Processing will start at beginning of tape.'
      if (advance_tape .eq. 'Y') then
            print*,'Enter number of files to skip'
            read(*,760) skip_files
760         format(i8)
      endif

      file_num = skip_files + 1

      if (skip_files .gt. 0) then
        print*,' Skipping ',skip_files,' files.'
	call mtskpf(1, skip_files)	
      end if

c*************************************************
c     TOP OF MAIN LOOP IS HERE
c*************************************************

1024  phys_rec = 0
      red_rec = 1

c *** Read a physical record
150   call mtread(1, byte_buf1, 32768/2)
      call mtwait(1, istate, nwords)
      if (istate .gt. 0) then 
          print*,'In the 150 call to mtread & mtwait:'
          print*,'nwords = ',nwords
          print*,'istate = ',istate
          stop
      endif
      phys_rec = phys_rec + 1
      if ((byte_buf1(1) .eq. 0) .and. (byte_buf1(2) .eq. 0) .and. 
     +    (byte_buf1(3) .eq. 67) .and. (byte_buf1(4) .eq. 70)) then
          print*,'EOF found! ', phys_rec,' physical records in file.'
          if (mom_files_open .eq. 1) then
             print*,red_rec,' reduced records written to disk.'
             close(7)
             close(8)
             mom_files_open = 0
          endif
          if (raw_files_open .eq. 1) then 
             print*,red_rec,' reduced records written to disk.'
             close(9)
             close(10)
             close(11)
             raw_files_open = 0
          endif
          call mtskpf(1, 1)	
          print*,'======================================'
          print*,' '
          file_num = file_num + 1
          goto 1024
      endif
      if ((byte_buf1(1) .eq. 0) .and. (byte_buf1(2) .eq. 0) .and. 
     +    (byte_buf1(3) .eq. 70) .and. (byte_buf1(4) .eq. 70)) then
          print*,'File header detected. ',file_num
          print*,'FILE NUMBER: ',file_header.data_file.file_number
          print*,'VERSION NUMBER: ',file_header.data_file.version_number
          print*,'LIDAR NUMBER: ',file_header.data_file.lidar_number
          print*,'DATA TYPE: ',file_header.data_file.data_type
          print*,'NUMBER OF DATA FIELDS: ',
     +            file_header.data_file.number_data_fields
          print*,'YEAR:   ',file_header.data_file.start_year
          print*,'MONTH:  ',file_header.data_file.start_month
          print*,'DAY:    ',file_header.data_file.start_day
          print*,'HOUR:   ',file_header.data_file.start_hour
          print*,'MINUTE: ',file_header.data_file.start_minute
          print*,'SECOND: ',file_header.data_file.start_second
          print*,'1/100 s: ',file_header.data_file.start_hund_sec
          print*,'TIME_UTC: ',file_header.data_file.time_utc
          print*,'RECORDS: ',file_header.data_file.number_of_records
          fields=file_header.data_file.number_data_fields
          goto 150
      endif
c**********************************************
c     moment header record
c**********************************************
      if ((byte_buf1(1) .eq. 0) .and. (byte_buf1(2) .eq. 0) .and. 
     +    (byte_buf1(3) .eq. 77) .and. (byte_buf1(4) .eq. 77)) then
          if (skip_moment_files .eq. 1) then
             print*,'Skipping a moment file...'
             call mtskpf(1, 1)	
             file_num = file_num + 1
             goto 1024
          endif
          ngates = mom_record.mom_header.ocs_info.num_gates
          fields = file_header.data_file.number_data_fields
c          print*,'fields = ',fields
c          print*,'ngates = ',ngates
c          print*,'mom_rec_cur_rec: ',mom_record.mom_header.
c     +                                ocs_info.current_rec
          red_rec_len = (ngates + 30)*4
          if (red_rec .eq. 1) then
             print*,' ' 
             print*,'===================================='
             print*,'Opening real-time moment output files'
             filename(9)=path(1:n1)//primary(1:n2)//
     +                   char(48+file_num)//'.rt_vel'
             filename(10)=path(1:n1)//primary(1:n2)//
     +                   char(48+file_num)//'.rt_int'
             filename(11)=path(1:n1)//primary(1:n2)//
     +                   char(48+file_num)//'.rt_ncp'
             mom_files_open = 1
             k=9
             lun=9
             open (UNIT=lun, FILE=filename(k),
     +       access='DIRECT',recl=red_rec_len,STATUS='NEW')
             print*,'lun=',lun,', rec_len=',red_rec_len,', ',filename(k)
             k=10
             lun=10
             open (UNIT=lun, FILE=filename(k),
     +       access='DIRECT',recl=red_rec_len,STATUS='NEW')
             print*,'lun=',lun,', rec_len=',red_rec_len,', ',filename(k)
             k=11
             lun=11
             open (UNIT=lun, FILE=filename(k),
     +       access='DIRECT',recl=red_rec_len,STATUS='NEW')
             print*,'lun=',lun,', rec_len=',red_rec_len,', ',filename(k)
             print*,'===================================='
          endif

          call gmtime(mom_record.mom_header.position_info.utc_time,
     +                tarray)
          header(1) = float(tarray(3))
          header(2) = float(tarray(2))
          header(3) = float(tarray(1))
          header(4) = float(file_header.site_control.altitude)
          header(5) = flag
          header(6) = flag
          header(7) = mom_record.mom_header.scan_info.azimuth
          header(8) = mom_record.mom_header.scan_info.elevation
          header(9) = flag
          header(10) =flag
          header(11) =flag
          header(12) = float(tarray(5)+1.)
          header(13) = float(tarray(4))
          header(14) = float(tarray(6))+1900.
          header(15) = float(mom_record.mom_header.lidar_info.rep_rate)
          header(16) = flag
          header(18) = flag
          header(19) = flag
          header(20) = flag
          header(21) = float(mom_record.mom_header.ocs_info.
     +                        range_to_first_gate)
          header(22) = float(mom_record.mom_header.ocs_info.gate_width)
          header(23) = 9.0001
          header(24) = 30.0
          header(25) = flag
          header(26) = flag
          header(27) = flag
          header(28) = flag
          header(29) = real(ngates)
          header(30) = flag
   
          do k=1,ngates
             rt_vel(k)=((float(mom_record.
     +                   intby2_buf(k))/128.)*25.25)/4.
             rt_int(k)=float(mom_record.
     +                   intby2_buf(ngates+k))
             rt_ncp(k)=float(mom_record.
     +                   intby2_buf((ngates*2)+k))/255.
c             print*,k,mom_record.intby2_buf(k),
c     +                mom_record.intby2_buf(ngates+k),
c     +                mom_record.intby2_buf((ngates*2)+k)
c             print*,k,rt_vel(k),rt_int(k),rt_ncp(k)
          enddo

          header(17)=88.
          write (unit=9, rec=red_rec) (header(k),k=1,30),
     +          (rt_vel(k),k=1,ngates)
          header(17)=89.
          write (unit=10, rec=red_rec) (header(k),k=1,30),
     +          (rt_int(k),k=1,ngates)
          header(17)=90.
          write (unit=11, rec=red_rec) (header(k),k=1,30),
     +          (rt_ncp(k),k=1,ngates)

          time_string = ctime(mom_record.mom_header.
     +                        position_info.utc_time)
          print*,red_rec,' ',time_string
          if (print_mom_info .eq. 1) then
             print*,'OCS info:'
             print*,mom_record.mom_header.ocs_info.ocs_status
             print*,mom_record.mom_header.ocs_info.data_recording
             print*,mom_record.mom_header.ocs_info.current_file_no
             print*,mom_record.mom_header.ocs_info.num_gates
             print*,mom_record.mom_header.ocs_info.gate_width
             print*,mom_record.mom_header.ocs_info.range_to_first_gate
             print*,mom_record.mom_header.ocs_info.num_to_integrate
	     print*,mom_record.mom_header.ocs_info.current_rec
             print*,'lidar info:'
             print*,mom_record.mom_header.lidar_info.rep_rate
             print*,mom_record.mom_header.lidar_info.
     +                operating_wavelength
             print*,mom_record.mom_header.lidar_info.zero_beat_freq
             print*,mom_record.mom_header.lidar_info.mode
             print*,mom_record.mom_header.lidar_info.filler
             print*,mom_record.mom_header.lidar_info.pulse_frequency
             print*,mom_record.mom_header.lidar_info.qs_but
             print*,mom_record.mom_header.lidar_info.pzt_volt
             print*,mom_record.mom_header.lidar_info.pulse_energy
             print*,mom_record.mom_header.lidar_info.freq_mean
             print*,mom_record.mom_header.lidar_info.freq_variance
             print*,'position info:'
             print*,mom_record.mom_header.position_info.utc_time
             print*,mom_record.mom_header.position_info.latitude
             print*,mom_record.mom_header.position_info.longitude
             print*,'scan info:'
             print*,mom_record.mom_header.scan_info.azimuth
             print*,mom_record.mom_header.scan_info.elevation
             read(*,300) answer
          endif
 
          red_rec = red_rec + 1
          goto 150

      endif
c****************************************************************
c    raw data header
c****************************************************************
      if ((byte_buf1(1) .eq. 0) .and. (byte_buf1(2) .eq. 0) .and. 
     +    (byte_buf1(3) .eq. 82) .and. (byte_buf1(4) .eq. 82)) then
          if (skip_raw_files .eq. 1) then
             print*,'Skipping a raw data file...'
             call mtskpf(1, 1)	
             file_num = file_num + 1
             goto 1024
          endif
          ngates = raw_record.raw_header.ocs_info.num_gates
          fields = file_header.data_file.number_data_fields
          red_rec_len = (ngates + 30)*4
          if (red_rec .eq. 1) then
             if (raw_files_open .gt. 0) then 
                print*,'Closing raw data output files...'  
                print*,' '
                close(7)
                close(8)
                raw_files_open = 0
                print*,'===================================='
             end if
             print*,'===================================='
             print*,'Opening raw data output files...'
             filename(7)=path(1:n1)//primary(1:n2)//
     +                   char(48+file_num)//'.i'
             filename(8)=path(1:n1)//primary(1:n2)//
     +                   char(48+file_num)//'.q'
             print*,filename(7)
             print*,filename(8)
             raw_files_open = 1
             k=7
             lun=7
             open (UNIT=lun, FILE=filename(k),
     +       access='DIRECT',recl=red_rec_len,STATUS='NEW')
             print*,'lun=',lun,', rec_len=',red_rec_len,', ',filename(k)
             k=8
             lun=8
             open (UNIT=lun, FILE=filename(k),
     +       access='DIRECT',recl=red_rec_len,STATUS='NEW')
             print*,'lun=',lun,', rec_len=',red_rec_len,', ',filename(k)
          endif

          call gmtime(raw_record.raw_header.position_info.utc_time,
     +               tarray)

          header(1) = float(tarray(3))
          header(2) = float(tarray(2))
          header(3) = float(tarray(1))
          header(4) = flag
          header(5) = flag
          header(6) = flag
          header(7) = flag
          header(8) = flag
          header(9) = flag
          header(10) =flag
          header(11) =flag
          header(12) = float(tarray(5)+1.)
          header(13) = float(tarray(4))
          header(14) = float(tarray(6))+1900.
          header(15) = flag
          header(16) = flag
          header(18) = flag
          header(19) = flag
          header(20) = flag
          header(21) = flag
          header(22) = 3.0
          header(23) = 9.0001
          header(24) = 30.0
          header(25) = flag
          header(26) = flag
          header(27) = 1.0
          header(28) = 1.0
          header(29) = real(ngates)
          header(30) = flag

          gate = 1
   
          do k=1,nwords,2
             j = k+1
             i_data(gate)=float(raw_record.intby2_buf(k))
             q_data(gate)=float(raw_record.intby2_buf(j))
c             print*,k,j,gate,raw_record.intby2_buf(k),
c    +        raw_record.intby2_buf(j)
             gate = gate + 1
          enddo

          header(17)=86.
          write (unit=7, rec=red_rec) (header(k),k=1,30),
     +          (i_data(k),k=1,ngates)
          header(17)=87.
          write (unit=8, rec=red_rec) (header(k),k=1,30),
     +          (q_data(k),k=1,ngates)

          time_string = ctime(raw_record.raw_header.position_info.
     +          utc_time)
          print*,red_rec,' ',time_string

          red_rec = red_rec + 1
         
      endif

      if ((byte_buf1(1) .eq. 0) .and. (byte_buf1(2) .eq. 0) .and. 
     +    (byte_buf1(3) .eq. 65) .and. (byte_buf1(4) .eq. 65)) then
          
          time_string = ctime(raw_record.raw_header.position_info.
     +                       utc_time)
c          print*,'ANC_RECORD ',time_string
c          print*,'DATA_TYPE ',anc_header.data_file.data_type
c          print*,'CURRENT_RECORD ',anc_header.ocs_info.current_rec

      endif

      goto 150

999   continue

      end
 *
 */

# ifdef obsolete
	   ee = fhed_ptr->scan_control.scan_name;
	   if(strncmp(ee, "ppi", 3) == 0) {
	      gri->scan_mode = PPI;
	   }
	   else if(strncmp(ee, "rhi", 3) == 0) {
	      gri->scan_mode = RHI;
	   }
	   else if(strncmp(ee, "ver", 3) == 0) {
	      gri->scan_mode = VER;
	   }
	   if(gri->scan_mode != VER) {
	      for(; *ee && *ee++ != '_'; );
	      gri->fixed = atof(ee);
	   }
	   else {
	      gri->fixed = 90.;
	   }

	   switch(etui->data_type) {
	    case MC_MOM_HEADER :
	      break;
	    case MC_COV_HEADER :
	      break;
	    case MC_RAW_HEADER :
	      break;
	    case MC_ANC_HEADER :
	      break;
	    default:
	      break;
	   }	   
# endif
