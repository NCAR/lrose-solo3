/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */


extern int Sparc_Arch;

/*
 * This file contains routines:
 * 
 * cimm_dd_conv
 * 
 * cimm_dump_this_ray
 * cimm_ini
 * cimm_inventory
 * cimm_map_structs
 * cimm_nab_data
 * cimm_next_ray
 * cimm_new_parms
 * cimm_positioning
 * cimm_print_cbdh
 * cimm_print_stat_line
 * cimm_reset
 * cimm_select_ray
 * cimm_setup_fields
 * 
 */


#include <sys/file.h>
#include <dd_defines.h>
#include <dorade_share.h>
#include "input_limits.hh"
#include "dd_stats.h"
#include <generic_radar_info.h>
#include <dd_io_structs.h>
#include <dd_math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <run_sum.h>
#include "cimm_dd.hh"
#include "dorade_share.hh"
#include "dd_io_mgmt.hh"
#include "dorade_share.hh"
#include "dd_time.hh"
#include "dda_common.hh"
#include "gneric_dd.hh"
//#include <dd_io_routines.hh>

typedef twob    LeShort;
typedef fourB   LeLong;
typedef fourB   LeFloat;

# define bcd2(x) (((int)(x) & 0xf) + 10 * (((int)(x) >> 4) & 0xf) )
# define bcd3(x) \
(((int)(x) & 0xf) + 10 * (((int)(x) >> 4) & 0xf) \
 + 100 * (((int)(x) >> 8) & 0xf) )
# define bcd4(x) \
(((int)(x) & 0xf) + 10 * (((int)(x) >> 4) & 0xf) \
 + 100 * (((int)(x) >> 8) & 0xf) + 1000 * (((int)(x) >> 12) & 0xf) )

struct raw_cimm {
    char             irecord_type;       /* b(1)       */
    unsigned char    irecord_number;     /* b(2) */
    unsigned short   irecord_size;       /* b(3)  or irec(2) */
    unsigned char    month;              /* b(5) */
    unsigned char    iday;               /* b(6) */
    unsigned char    iyear;              /* b(7) */
    unsigned char    ihour;              /* b(8) */
    unsigned char    imin;               /* b(9) */
    unsigned char    isec;               /* b(10) */
    unsigned short   azm;                /* b(11) or irec(6) */
    unsigned short   elev;               /* b(13) or irec(7) */
    unsigned char    itilt_num;          /* b(15) */
    unsigned char    iant_mode;          /* b(16) */
    unsigned char    system_mode;        /* b(17) */
    unsigned char    system_timing;      /* b(18) */
    unsigned char    receiver_mode;      /* b(19) */
    unsigned char    transmitter_mode;   /* b(20) */
    unsigned char    missing_21;         /* b(21) */
    unsigned char    receiver_AGC_mode;  /* b(22) */
    short            hor_ant_spd;        /* b(23) or irec(12) */
    short            ver_ant_spd;        /* b(25) or irec(13) */
    unsigned short   ant_tar_pos;        /* b(27) or irec(14) */
    unsigned short   num_gates;          /* b(29) or irec(15) */
    unsigned short   num_samples;        /* b(31) or irec(16) */

    unsigned char    reflect[768];
    char             velocity[768];
    char             spectrum[768];
    char             zdr[768];
    char             qdp[768];
    char             cc[768];
};

/* antenna mode */

# define  IDLE_OR_SEEK 0
# define      PPI_SCAN 1
# define   SECTOR_SCAN 2
# define      RHI_SCAN 3

/* pulse repetition mode */

# define        UNIFORM 0
# define     INTERLACED 1

/* receiver gain */

# define RECEIVER_GAIN_NORMAL 0
# define   RECEIVER_GAIN_HIGH 1

/* antenna polarization sequence */

# define HORIZONTAL_ONLY 0
# define   VERTICAL_ONLY 1
# define     ALTERNATING 2

/* clutter filter type */
# define CLUTTER_FILTER_DISABLES 0

# define	  BEAMWIDTH .85          /* degrees */
# define	 WAVELENGTH .1094        /* meters */
# define  RNG_KM_FIRST_GATE .3	         /* km. */
# define    FIRST_GOOD_GATE 50           /* fortran index */
# define	 FINAL_GATE 765
# define     MAX_CIMM_GATES 768

# define       ADJUST_RHOHV 0x1

struct cimm_ray_que {
    struct cimm_ray_que *last;
    struct cimm_ray_que *next;
    double time;
    double time_diff;
    double az_diff;
    double el_diff;
    double fxdang_diff;
    double rcvr_pulsewidth;
    float fxdang;
    float az;
    float el;
    float prt;
    int hits;
    int scan_num;
    int vol_num;
    int ray_num;
    int scan_type;
    int transition;
    int num_gates;
    int ptime;
    short subsec;
};

struct cimm_swp_que {
    struct cimm_swp_que *last;
    struct cimm_swp_que *next;
    double swpang_diff;
    float swpang;
    int new_vol;
    int scan_mode;
};

struct cimm_useful_items {
    double fxd_ang_sum;
    double az_diff_sum;
    double el_diff_sum;
    double secs_per_count;
    double x_az_diff_sum;
    double max_rhi_diff;
    double min_az_diff;
    double min_el_diff;
    double rhi_az_tol;
    double ppi_el_tol;
    double min_fxdang_diff;

    struct cimm_ray_que *ray_que;
    struct cimm_swp_que *swp_que;

    float prev_az;
    float prev_el;

    int first_good_gate;
    int32_t trip_time;
    int count_since_trip;

    int min_rays_per_sweep;
    int transition_count;
    int ray_count;
    int sweep_ray_num;
    int reset;
    int options;

    int new_sweep;
    int sweep_count;
    int sweep_num;
    int sweep_count_flag;

    int new_vol;
    int vol_count;
    int vol_count_flag;
    int vol_num;

    int prev_tilt_num;

    short * lut4x;
    short * lut1x;

    int num_az_diffs_avgd;
    int num_el_diffs_avgd;
    int short_rq_len;

    struct run_sum * az_rs;
    struct run_sum * el_rs;
    struct run_sum * az_diff_rs;
    struct run_sum * el_diff_rs;

    double az_diff_run_sum;
    double el_diff_run_sum;
    double az_run_sum;
    double el_run_sum;

    double max_az_diff;
    double max_el_diff;
    double rcp_short_len;

    double az_diff_avg_sum;
    double rcp_num_az_avgs;
    double el_diff_avg_sum;
    double rcp_num_el_avgs;

};

static struct cimm_useful_items *cmui;
static struct raw_cimm * rcimmh;
static struct raw_cimm * cimmh;
static char * raw_data;
static unsigned char * ref;
static char * vel;
static char * sw;
static char * zdr;
static char * qdp;
static char * cc;

static struct dd_input_filters *difs;
static struct dd_stats *dd_stats;
static struct generic_radar_info *gri;
static struct input_read_info *irq;
static int view_char=0, view_char_count=200;
static char preamble[24], postamble[64];
static struct solo_list_mgmt *slm;
static char *pbuf=NULL, **pbuf_ptrs=NULL;
static int pbuf_max=1200;


static double a_sampling_time[] =   {   1.0,     1.2,     1.4,     1.6 };
static double a_gate_spacing[] =   {    150.,    180.,    210.,    240. };
static double a_prt[] =             {   768.0,   921.6,   1075.2,  1228.8 };
static double a_prf[] =            {    1302.1,  1085.1,  930.1,   813.8 };
static double a_nyquist_velocity[] = {  35.7096, 29.7850, 25.5059, 22.3185 };
static double range_km_first_gate = 0.3;

static int    isample_time_index;

static double	   sampling_time;
static double		     prt;
static double		     prf;
static double	nyquist_velocity;
static double	  v_gate_spacing;
static double	  r_gate_spacing;
static double	       zdr_scale;
  
static int   	 ipulse_rep_mode;
static double	     pulse_width;
static int   	 iclutter_filter;
static int   	      ircvr_gain;
static int   	  ircvr_agc_mode;
static double	     hor_ant_spd;
static int   		 rot_dir;
static double	     ver_ant_spd;
static double		     azm;
static double		    elev;
static int   	       num_gates;
static int   	       max_gates;
static int   	      ipolar_seq;

static double slant_rng[768];
static double rng_norm[768];

static int ib1, ib2;
static float snr_cal[257];
static float dbz_cal[257];
static float dbm_cal[257];
static double rho_pow_lut[257];

static int fn_dbm;
static int fn_ldr;
static int fn_zdr;
static int fn_phi;
static int fn_ve;
static int fn_sw;
static int fn_rcc;
static int fn_snr;
static int fn_dbz;
static int fn_rhohv;

struct run_sum *init_run_sum();
void reset_running_sum();
double running_sum();
double short_running_sum();
double avg_running_sum();
double short_avg_running_sum();

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

void 
cimm_dd_conv (int interactive_mode)
{
    /* main loop for converting 88D data
     */
    int i, n, mark;
    static int count=0, nok_count;

    if(!count) {
	cimm_ini();
    }


    if(interactive_mode) {
	dd_intset();
	cimm_positioning();
    }
    cimm_reset();

    for(;;) {
	count++;
	if(difs->stop_flag &&
	   (difs->abrupt_start_stop || cmui->new_sweep)) {
	    printf("\nBreak on stop time!\n");
	    break;
	}

	if(cimm_select_ray()) {

	    if(difs->max_beams)
		  if(++cmui->ray_count > difs->max_beams ) {
		      printf("\nBreak on ray count!\n");
		      break;
		  }
	    if(cmui->sweep_count_flag) {
		cmui->sweep_count_flag = NO;
		if(difs->max_sweeps) {
		    if(++cmui->sweep_count > difs->max_sweeps ) {
			printf("\nBreak on sweep count!\n");
			break;
		    }
		}
		if(cmui->vol_count_flag) {
		    cmui->vol_count_flag = NO;
		    if(difs->max_vols) {
			if(++cmui->vol_count > difs->max_vols ) {
			    printf("\nBreak on volume count!\n");
			    break;
			}
		    }
		}
	    }
	    if(!difs->catalog_only)
		  cimm_nab_data();
	    
	    dd_stuff_ray();	/* pass it off to dorade code */
	}
	else {
	    if(!(nok_count++ % 1000)) {
		cimm_print_stat_line(count);
	    }
	}
	cmui->new_sweep = NO;
	cmui->new_vol = NO;

	if((n = cimm_next_ray()) < 1) {
	    printf("Break on input status: %d\n", n);
	    break;
	}
    }
    cimm_print_stat_line(count);
    return;
}

/* c------------------------------------------------------------------------ */

void 
cimm_positioning (void)
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
11 = Display header\n\
12 = Display data\n\
Option = "
	   );
    nn = getreply(str, sizeof(str));
    if( cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 16 ) {
	if(ival == -2) exit(1);
	printf( "\nIllegal Option!\n" );
	goto menu2;
    }
    cmui->new_sweep = NO;
    cmui->new_vol = NO;

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
		cmui->new_sweep = NO;
		cmui->new_vol = NO;
		if((jj=cimm_next_ray()) < 0)
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
	cimm_reset();
	cimm_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 1) {
	if((nn = cimm_inventory(irq)) == -2)
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
	dd_stats_reset();
	cimm_reset();
	cimm_next_ray();
	gri_interest(gri, 1, preamble, postamble);
    }
    else if(ival == 3) {
	dd_rewind_dev(irq);
	dd_stats_reset();
	cimm_reset();
	cimm_next_ray();
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
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > 1000000) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	direction = ival >= 0 ? FORWARD : BACKWARD;
	dd_skip_recs(irq, direction, ival > 0 ? ival : -ival);
	cimm_reset();
	cimm_next_ray();
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
	    if((nn = cimm_next_ray()) <= 0 || gri->time >= dtarget ||
	       dd_control_c()) {
		break;
	    }
	    if(!(mm % 1000)) {
		gri_interest(gri, 1, preamble, postamble);
		printf("%s\n", dd_stats_sprintf());
	    }
	    cmui->new_vol = cmui->new_sweep = NO;
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
	    if((nn = cimm_next_ray()) <= 0 || in_sector(gri->fixed, fx1, fx2)
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
	cimm_print_header(slm);
    }
    else if(ival == 12) {
	cimm_dump_this_ray(slm);
    }

    preamble[0] = '\0';

    goto menu2;
}

/* c------------------------------------------------------------------------ */

void 
cimm_print_header (struct solo_list_mgmt *slm)
{
    int ii;
    char *aa, str[128], tmp[32];

    aa = str;

    solo_add_list_entry(slm, (char *)" ");
    sprintf(aa, "Contents of cimmaron ray header");
    solo_add_list_entry(slm, aa);

    sprintf(aa, "irecord_type       %d", rcimmh->irecord_type      );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "irecord_number     %d", rcimmh->irecord_number    );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "irecord_size       %d", rcimmh->irecord_size      );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "month              %02x", rcimmh->month             );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "iday               %02x", rcimmh->iday              );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "iyear              %02x", rcimmh->iyear             );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ihour              %02x", rcimmh->ihour             );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "isec               %02x", rcimmh->isec              );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "imin               %02x", rcimmh->imin              );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "azm                %04x", rcimmh->azm               );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "elev               %04x", rcimmh->elev              );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "itilt_num          %d", rcimmh->itilt_num         );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "iant_mode          %d", rcimmh->iant_mode         );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "system_mode        %d", rcimmh->system_mode       );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "system_timing      %02x", rcimmh->system_timing     );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "receiver_mode      %02x", rcimmh->receiver_mode     );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "transmitter_mode   %02x", rcimmh->transmitter_mode  );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "missing_21         %d", rcimmh->missing_21        );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "receiver_AGC_mode  %d", rcimmh->receiver_AGC_mode );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "hor_ant_spd        %d", rcimmh->hor_ant_spd       );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ver_ant_spd        %d", rcimmh->ver_ant_spd       );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "ant_tar_pos        %d", rcimmh->ant_tar_pos       );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "num_gates          %d", rcimmh->num_gates         );
    solo_add_list_entry(slm, aa);
    sprintf(aa, "num_samples        %d", rcimmh->num_samples       );
    solo_add_list_entry(slm, aa);
    slm_print_list(slm);

}
/* c------------------------------------------------------------------------ */

void 
cimm_dump_this_ray (struct solo_list_mgmt *slm)
{
    int mark = 0;
    int gg, ii, nf, nn, fw=10;
    char *aa, *bb, *cc, str[128], tmp_str[16];
    float unscale[8], bias[8], *fp=NULL;

    aa = str;
    bb = tmp_str;
    cimm_nab_data();
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

int 
cimm_inventory (struct input_read_info *irq)
{
    /* the purpose of this routine is to facilitate a more detailed
     * examination of the data
     */
    int ii=0, max=gri_max_lines();
    int nn, ival;
    float val;
    char str[256];
    double d, fabs();


    for(;;) {
	for(ii=0; ii < max; ii++) {
	    cmui->new_sweep = NO;
	    cmui->new_vol = NO;
	    if((nn=cimm_next_ray()) < 1) {
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
cimm_ini (void)
{
    int mark = 0, fn;
    int ii, jj, nn, ib, idum1, idum2;
    short *ss, *tt;
    FILE * strm;
    char *aa, *fname, line[256];
    struct cimm_ray_que *rq, *last_rq;
    struct cimm_swp_que *sq, *last_sq;
    double d, d_snr, StoN;

    /* c...ini() */

    printf("CIMM_FORMAT\n");

    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();
    gri = return_gri_ptr();

    cmui = (struct cimm_useful_items *)
	  malloc(sizeof(struct cimm_useful_items));
    memset((char *)cmui, 0, sizeof(struct cimm_useful_items));
    cmui->prev_tilt_num = 999;
    nn = dd_min_rays_per_sweep();	/* trigger min rays per sweep */

    cmui->reset = YES;
    cmui->min_rays_per_sweep = nn;
    printf("  Min. rays per sweep: %d\n", dd_min_rays_per_sweep());

    if((aa=get_tagged_string("OPTIONS"))) {
	if(strstr(aa, "ADJUST_RHO"))
	  { cmui->options |= ADJUST_RHOHV; }
    }

    cmui->first_good_gate = FIRST_GOOD_GATE;
    if( aa=get_tagged_string( "FIRST_GOOD_GATE" )) {
	ii = atoi( aa );
	if( ii > 0 )
	    { cmui->first_good_gate = ii; }
    }

    cmui->min_fxdang_diff = 0.4;
    if(aa=get_tagged_string("MIN_FXD_DIFF")) {
	d = atof(aa);
	if(d > .01) cmui->min_fxdang_diff = d;
    }
    printf("  Min. Fixed Angle diff: %.2f\n", cmui->min_fxdang_diff);

    cmui->ppi_el_tol = 0.45;
    if(aa=get_tagged_string("PPI_EL_TOL")) {
	d = atof(aa);
	if(d > .01) cmui->ppi_el_tol = d;
    }
    printf("  PPI elevation tolerance: %.2f\n", cmui->ppi_el_tol );

    cmui->rhi_az_tol = 1.75;
    if(aa=get_tagged_string("RHI_AZ_TOL")) {
	d = atof(aa);
	if(d > .01) cmui->rhi_az_tol = d;
    }
    printf("  RHI azimuth tolerance: %.2f\n", cmui->rhi_az_tol );

    cmui->min_el_diff = .2;
    if(aa=get_tagged_string("MIN_EL_DIFF")) {
	d = atof(aa);
	if(d > .01) cmui->min_el_diff = d;
    }
    printf("  Min. EL diff: %.2f\n", cmui->min_el_diff);

    cmui->min_az_diff = 1.5;
    if(aa=get_tagged_string("MIN_AZ_DIFF")) {
       d = atof(aa);
       if(d > .01) cmui->min_az_diff = d;
    }
    printf("  Min. AZ diff: %.2f\n", cmui->min_az_diff);

    cmui->max_rhi_diff = 5.;
    if(aa=get_tagged_string("MAX_RHI_DIFF")) {
	d = atof(aa);
	if(d > .01) cmui->max_rhi_diff = d;
    }
    printf("  Max RHI diff: %.2f\n", cmui->max_rhi_diff);

    cmui->num_az_diffs_avgd = 11;
    if(aa=get_tagged_string("NUM_AZ_DIFFS_AVGD")) {
       ii = atoi(aa);
       if(ii > 1) cmui->num_az_diffs_avgd = ii;
    }
    printf("  Num az diffs averaged: %d\n", cmui->num_az_diffs_avgd);

    cmui->num_el_diffs_avgd = 11;
    if(aa=get_tagged_string("NUM_EL_DIFFS_AVGD")) {
       ii = atoi(aa);
       if(ii > 1) cmui->num_el_diffs_avgd = ii;
    }
    printf("  Num el diffs averaged: %d\n", cmui->num_el_diffs_avgd);

    cmui->short_rq_len = 5;
    if(aa=get_tagged_string("NUM_SHORT_AVG")) {
       ii = atoi(aa);
       if(ii > 1) cmui->short_rq_len = ii;
    }
    printf("  Short que size: %d\n", cmui->short_rq_len);

    cmui->az_diff_rs = init_run_sum( cmui->num_az_diffs_avgd
				    , cmui->short_rq_len );
    cmui->el_diff_rs = init_run_sum( cmui->num_el_diffs_avgd
				    , cmui->short_rq_len );
    cmui->az_rs = init_run_sum( cmui->num_az_diffs_avgd
				    , cmui->short_rq_len );
    cmui->el_rs = init_run_sum( cmui->num_el_diffs_avgd
				    , cmui->short_rq_len );

    for(ii=0; ii < nn; ii++) {
	rq = (struct cimm_ray_que *)malloc(sizeof(struct cimm_ray_que));
	memset(rq, 0, sizeof(struct cimm_ray_que));
	if(!ii) {
	    cmui->ray_que = rq;
	}
	else {
	    rq->last = cmui->ray_que->last;
	    cmui->ray_que->last->next = rq;
	}
	rq->next = cmui->ray_que;
	cmui->ray_que->last = rq;
	last_rq = rq;
	rq->scan_num = -1;
	rq->vol_num = -1;
    }

# define SWEEP_QUE_SIZE 11

    for(ii=0; ii < SWEEP_QUE_SIZE; ii++) {
	sq = (struct cimm_swp_que *)malloc(sizeof(struct cimm_swp_que));
	memset(sq, 0, sizeof(struct cimm_swp_que));
	if(!ii) {
	    cmui->swp_que = sq;
	}
	else {
	    sq->last = cmui->swp_que->last;
	    cmui->swp_que->last->next = sq;
	}
	sq->next = cmui->swp_que;
	cmui->swp_que->last = sq;
	last_sq = sq;
	sq->swpang = EMPTY_FLAG;
    }

    cimmh = (struct raw_cimm *)malloc( sizeof( struct raw_cimm ));
    memset( cimmh, 0, sizeof( struct raw_cimm ));

    cmui->lut4x = (short * ) malloc( SRS_MAX_GATES * sizeof( short ));
    memset( cmui->lut4x, 0, SRS_MAX_GATES * sizeof( short ));
    cmui->lut1x = (short * ) malloc( SRS_MAX_GATES * sizeof( short ));
    memset( cmui->lut1x, 0, SRS_MAX_GATES * sizeof( short ));


    ss = cmui->lut4x;
    tt = cmui->lut1x;

    for( ii = 0; ii < SRS_MAX_GATES; ii++, ss++ ) {
	*ss = ii/4;
	*tt = ii;
    }


    /* set up the default data fields
     */
    for(ii=0; ii < SRS_MAX_FIELDS; gri->dd_scale[ii++] = 100.);

    fn = fn_dbm = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "DM");
    strcpy(gri->field_long_name[fn], "Reflected power");
    strcpy(gri->field_units[fn], "dBm");

    fn = fn_dbz = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "DZ");
    strcpy(gri->field_long_name[fn], "Reflectivity factor");
    strcpy(gri->field_units[fn], "dBz");

    fn = fn_sw = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "SW");
    strcpy(gri->field_long_name[fn], "Spectral width");
    strcpy(gri->field_units[fn], "m/s");

    fn = fn_zdr = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "ZD");
    strcpy(gri->field_long_name[fn], "Differential Reflectivity");
    strcpy(gri->field_units[fn], "dBm");

    fn = fn_ve = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "VE");
    strcpy(gri->field_long_name[fn], "Radial velocity");
    strcpy(gri->field_units[fn], "m/s");

    fn = fn_rhohv = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "RHOHV");
    strcpy(gri->field_long_name[fn], "Rho hv");
    strcpy(gri->field_units[fn], "???");

    fn = fn_phi = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "DP");
    strcpy(gri->field_long_name[fn], "Differential Phase");
    strcpy(gri->field_units[fn], "deg.");

    fn = fn_ldr = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "LDR");
    strcpy(gri->field_long_name[fn], "Linear depolarization ratio");
    strcpy(gri->field_units[fn], "dBm");

# ifdef INCLUDE_RCC
    fn = fn_rcc = gri->num_fields_present++;
    strcpy(gri->field_name[fn], "RCC");
    strcpy(gri->field_long_name[fn], "Unknown");
    strcpy(gri->field_units[fn], "???");
# endif

    for( ii = 0; ii < gri->num_fields_present; ii++ ) {
	/* fill with missing data flags */
	ss = gri->scaled_data[ii];

	for( jj = 0; jj < FINAL_GATE; jj++ ) {
	    *(ss + jj ) = EMPTY_FLAG;
	}
    }
    ss = gri->scaled_data[ fn_ldr ];
    for( jj = 0; jj < FINAL_GATE; jj++ ) {
	*(ss + jj ) = -9900;
    }

    strcpy( gri->radar_name, "CIMMARON" );
    gri->dd_radar_num = dd_assign_radar_num( gri->radar_name );
    gri->binary_format = DD_16_BITS;
    gri->source_format = CIMM_FMT;
    gri->radar_type = DD_RADAR_TYPE_GROUND;
    gri->latitude = 35.47533;
    gri->longitude = -97.81314;
    gri->altitude = 408;
    gri->scan_mode = DD_SCAN_MODE_PPI;
    gri->missing_data_flag = EMPTY_FLAG;
    gri->h_beamwidth = gri->v_beamwidth = BEAMWIDTH;
    gri->polarization = 0;	/* horizontal polarization */
    gri->wavelength = WAVELENGTH;	/* meters */
    gri->num_ipps = gri->num_freq = 1;
    gri->freq[0] = (SPEED_OF_LIGHT/gri->wavelength) * 1.e-9; /* GHz */

    if( !( aa = getenv( "CALIBRATION_FILE" ))) {
	printf( "No Calibration File!\n" );
	exit(1);
    }
    fname = aa;
    /*
     * read in the calibration file
     */
    printf("Opening %s\n", fname);
    if( !(strm = fopen(fname, "r")) ) {
	printf( "Unable to open %s\n", fname );
	exit(1);
    }
    ib = -1;
    while( fgets( line, sizeof( line ) -1, strm ) != NULL ) {

	if( ib < 0 ) {		/* first line */
	    nn = sscanf( line, "%d%d", &ib1, &ib2 );
	    ib = ib1;
	    continue;
	}
	nn = sscanf( line, "%d%d%f%f%f"
		     , &idum1, &idum2
		     , &snr_cal[ib]
		     , &dbz_cal[ib]
		     , &dbm_cal[ib]
		     );
	if( ++ib > ib2 )
	    { break; }		/* there's one more line but
				 * you don't use it */
    }
    /* now fill in the gaps at the beginning and end of the tables
     * with the appropriate values
     */

    for(ib = 0; ib < ib1; ib++ ) {
	snr_cal[ib] = snr_cal[ib1];
	dbz_cal[ib] = dbz_cal[ib1];
	dbm_cal[ib] = dbm_cal[ib1];
    }
    for(ib = ib2+1; ib < 257; ib++ ) {
	snr_cal[ib] = snr_cal[ib2];
	dbz_cal[ib] = dbz_cal[ib2];
	dbm_cal[ib] = dbm_cal[ib2];
    }
    /* build a table for the exponentiation part of the rhohv calc
     */
    for( ib = 0; ib < 257; ib++ ) {
	d_snr = snr_cal[ ib ];
	StoN = WATTZ( d_snr );
	if( cmui->options & ADJUST_RHOHV ) { /* take the cubed root */
	    rho_pow_lut[ ib ] = pow( (StoN +1.)/StoN, (double)0.25 );
	}
	else {
	    rho_pow_lut[ ib ] = pow( (StoN +1.)/StoN, (double)0.75 );
	}
    }


    /*
     * open the input file
     */
    irq = dd_return_ios(0, CIMM_FMT);

    if( aa=get_tagged_string( "SOURCE_DEV" )) {
	dd_input_read_open(irq, aa);
    }
    cimm_next_ray();		/* read in the first data record */
    return;

}

/* c------------------------------------------------------------------------ */

void 
cimm_reset (void)
{
    cmui->vol_count_flag = cmui->sweep_count_flag =
	cmui->new_sweep = cmui->new_vol = cmui->reset = YES;
    cmui->ray_count =
	cmui->sweep_count = cmui->vol_count = 0;
    difs->stop_flag = NO;
    /*
     * make sure to start a new sweep and vol
     */
    gri->vol_num++;
    gri->sweep_num++;
}
/* c------------------------------------------------------------------------ */

int 
cimm_select_ray (void)
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
	if(cmui->ray_count % difs->beam_skip){
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

void 
cimm_nab_data (void)
{
    int mark = 0, ic, ii, jj;
    short * pdbz = gri->scaled_data[ fn_dbz ];
    short * pdbm = gri->scaled_data[ fn_dbm ];
    short * psw = gri->scaled_data[ fn_sw ];
    short * pzdr = gri->scaled_data[ fn_zdr ];
    short * pve = gri->scaled_data[ fn_ve ];
    short * prho = gri->scaled_data[ fn_rhohv ];
    short * pphi = gri->scaled_data[ fn_phi ];
# ifdef INCLUDE_RCC
    short * prcc = gri->scaled_data[ fn_rcc ];
# endif
    double scale = 100., bias = 0, d_dbz, d_snr, StoN, d_rcc, d_rho;
    int gg = cmui->first_good_gate -1; /* C style index */

    
    if( ipulse_rep_mode != UNIFORM ) {
	mark = 0;
    }


    for(; gg < FINAL_GATE; gg++ ) {

	ic = *(ref + gg );

	*( pdbm + gg ) = (short)DD_SCALE( dbm_cal[ ic ], scale, bias );
	d_dbz = dbz_cal[ ic ];
	*( pdbz + gg ) = (short)DD_SCALE( d_dbz + rng_norm[gg], scale, bias );
	d_rcc = *( cc + gg ) * .01;
# ifdef obsolete
	d_snr = snr_cal[ ic ];
	StoN = WATTZ( d_snr );
	d_rho = d_rcc * pow( (StoN +1.)/StoN, (double)0.75 );
# endif
	d_rho = d_rcc * rho_pow_lut[ ic ];

	*( prho + gg ) = (short)DD_SCALE( d_rho, scale, bias );

	*( pphi + gg ) = (short)DD_SCALE( *( qdp + gg ), scale, bias );
	*( pzdr + gg ) = (short)DD_SCALE( *( zdr + gg ) * .05, scale, bias );
	*( psw + gg ) = (short)DD_SCALE( *( sw + gg ) * .25, scale, bias );
	*( pve + gg ) = (short)DD_SCALE( *( vel + gg ), scale, bias );
# ifdef INCLUDE_RCC
	*( prcc + gg ) = DD_SCALE( *( d_rcc, scale, bias );
# endif
    }
}
/* c------------------------------------------------------------------------ */

int 
cimm_next_ray (void)
{
    int ii, jj, mark = 0, eof_count = 0, new_sweep = NO, new_vol = NO;
    float *r, f;
    static int count = 0;
    struct io_packet *dp;
    struct cimm_ray_que *rq;
    struct cimm_swp_que *sq;
    double d;


    for(;;) {
	count++;
	if(dd_control_c()) {
	    dd_reset_control_c();
	    return(-1);
	}
	dd_logical_read(irq, FORWARD);
	
	if(irq->top->read_state < 0) {
	    printf("Last read: %d\n", irq->top->read_state);
	    return(irq->top->read_state);
	}
	else if(irq->top->eof_flag) {
	    dd_stats->file_count++;
	    printf("EOF: %d at %.2f MB\n"
		   , dd_stats->file_count, dd_stats->MB);
	    if(++eof_count > 2)
		return((int)0);
	}
	else if( irq->top->sizeof_read < 4640 ) {
# ifdef obsolete
	    printf( "Record length: %d ... Ignored!\n"
		    , irq->top->sizeof_read );
	    ezhxdmp(irq->top->buf, 0, 80);
# endif
	    mark = 0;
	}
	else {
	    eof_count = 0;
	    dd_stats->rec_count++;
	    dd_stats->MB += BYTES_TO_MB(irq->top->sizeof_read);
	    break;
	}
    }
    /* c...next_ray() */

    dp = dd_return_next_packet(irq);
    raw_data = dp->at;
//    ref = raw_data + 32;  //Jul 26, 2011
//    vel = ref + 768;  //Jul 26, 2011
    ref = (unsigned char*)(raw_data + 32);  //Jul 26, 2011
    vel = (char *)(ref + 768);  //Jul 26, 2011
    sw = vel + 768;
    zdr = sw + 768;
    qdp = zdr + 768;
    cc = qdp + 768;
    
    rcimmh = (struct raw_cimm *)raw_data;
    dp->len = rcimmh->irecord_size;

    isample_time_index = ((int)rcimmh->system_timing) & 0x3;
    sampling_time =     a_sampling_time[ isample_time_index ];
    prt =               a_prt[ isample_time_index ] * 1.e-6;
    gri->nyquist_vel = 
    nyquist_velocity =  a_nyquist_velocity[ isample_time_index ];
    zdr_scale =         nyquist_velocity/127.;
    prf =               a_prf[ isample_time_index ];
    v_gate_spacing =    a_gate_spacing[ isample_time_index ];

    ipulse_rep_mode = ((int)rcimmh->system_timing >> 2 ) & 0x3;
    pulse_width = (int)rcimmh->transmitter_mode & 1 ? 0 : 1.e-6;
    iclutter_filter = (int)rcimmh->receiver_mode & 0xf;
    ircvr_gain = ((int)rcimmh->receiver_mode >> 4 ) & 1;
    ircvr_agc_mode = rcimmh->receiver_AGC_mode;
    hor_ant_spd = rcimmh->hor_ant_spd/10.;
    rot_dir = hor_ant_spd < 0 ? -1 : 1;
    ver_ant_spd = rcimmh->ver_ant_spd/10.;
    num_gates = rcimmh->num_gates;
    
    rq = cmui->ray_que = cmui->ray_que->next;

    rq->az =
	azm = bcd4( rcimmh->azm ) * .1;
    rq->el =
	elev = bcd3( rcimmh->elev ) * .1;
    
    rq->az_diff = angdiff( rq->last->az, rq->az );
    rq->el_diff = angdiff( rq->last->el, rq->el );

    
    if( FABS( rq->az_diff ) < 11. ) {
	cmui->az_diff_sum += rq->az_diff;
	cmui->az_diff_run_sum =
	    running_sum( cmui->az_diff_rs, rq->az_diff );
    }
    if( FABS( rq->el_diff ) < 11. ) {
	cmui->el_diff_sum += rq->el_diff;
	cmui->el_diff_run_sum =
	    running_sum( cmui->el_diff_rs, rq->el_diff );
    }
    cmui->az_run_sum = running_sum( cmui->az_rs, rq->az );
    cmui->el_run_sum = running_sum( cmui->el_rs, rq->el );
    
# ifdef obsolete
    if( fabs( cmui->az_diff_sum ) >= 360. )
	{ new_sweep = YES; }
# endif    

    if( ipulse_rep_mode == UNIFORM ) {
	r_gate_spacing = v_gate_spacing;
	max_gates = num_gates;
    }
    else {
	r_gate_spacing = v_gate_spacing * 4.0;
	max_gates = num_gates * 4;
    }
    ipolar_seq = ((int)rcimmh->system_timing >> 4 ) & 0xf;
    
    gri->PRF = prf;
    gri->corrected_rotation_angle = gri->rotation_angle =
	gri->azimuth = azm;
    gri->elevation = elev;
    gri->ipps[0] = prt;		/* in milliseconds */

    ii = bcd2( rcimmh->iyear );
    gri->dts->year = ii > 1900 ? ii : ii +1900;
    gri->dts->month = bcd2( rcimmh->month );
    gri->dts->day = bcd2( rcimmh->iday );
    gri->dts->julian_day = 0;
    ii = bcd2( rcimmh->ihour);
    ii = bcd2( rcimmh->imin );
    ii = bcd2( rcimmh->isec );

    gri->dts->day_seconds = bcd2( rcimmh->ihour ) * 3600
				  + bcd2( rcimmh->imin ) * 60
				  + bcd2( rcimmh->isec );

    gri->time = d_time_stamp( gri->dts );

    d = gri->time - cmui->trip_time;

    if( d >= 0 ) {
	cmui->trip_time = (int32_t)(gri->time + 1);
	cmui->count_since_trip = 0;
	cmui->secs_per_count = gri->PRF ? rcimmh->num_samples/gri->PRF : 0;
    }
    else {
	gri->time += ++cmui->count_since_trip * cmui->secs_per_count;
    }
    gri->dts->time_stamp = gri->time;
    d_unstamp_time( gri->dts );

    /*
    if( new_sweep || rcimmh->itilt_num != cmui->prev_tilt_num ) {
     */
    if( cmui->reset || cimm_isa_new_sweep() ) {

	sq = cmui->swp_que = cmui->swp_que->next;
	sq->scan_mode = gri->scan_mode;

	gri->fixed = gri->scan_mode == DD_SCAN_MODE_PPI ?
	    short_avg_running_sum( cmui->el_rs ) :
	    short_avg_running_sum( cmui->az_rs );
	sq->swpang = gri->fixed;

	if( sq->last->swpang != EMPTY_FLAG ) {
	    sq->swpang_diff = angdiff( sq->last->swpang, sq->swpang );
	    if( sq->swpang_diff < 0 ) {
		new_vol = YES;
	    }
	}
	if( cmui->reset )
	    { new_vol = YES; }
	cmui->reset = NO;
	sq->new_vol = new_vol;

	gri->num_bins = max_gates;
	for(ii=0; ii < gri->num_fields_present;
	    gri->actual_num_bins[ii++] = gri->num_bins);
	gri->bin_spacing = v_gate_spacing;
	cmui->sweep_count_flag = YES;
	gri->sweep_num++;
	dd_stats->sweep_count++;
	if(cmui->sweep_ray_num < dd_min_rays_per_sweep()) {
	    gri->ignore_this_sweep = YES;
	}
	cmui->new_sweep = YES;

	r = gri->range_value;
	f = gri->range_b1 = KM_TO_M( RNG_KM_FIRST_GATE );
	for(ii=0; ii < gri->num_bins; ii++, f+=gri->bin_spacing) {
	    *r++ = f;
	    rng_norm[ii] = 20 * LOG10( M_TO_KM( f ));
	}

	/*
	if( rcimmh->itilt_num < cmui->prev_tilt_num ) {
	 */
	if( new_vol ) {
	    cmui->new_vol = YES;
	    cmui->vol_count_flag = YES;
	    gri->vol_num++;
	    dd_stats->vol_count++;
	}
	cmui->fxd_ang_sum = 0;
	cmui->sweep_ray_num = 0;
	cmui->prev_tilt_num = rcimmh->itilt_num;
	cmui->x_az_diff_sum = 0;
	cmui->az_diff_sum = 0;
	cmui->el_diff_sum = 0;
	cmui->transition_count = 0;

	reset_running_sum( cmui->az_diff_rs );
	running_sum( cmui->az_diff_rs, rq->az_diff );

	reset_running_sum( cmui->el_diff_rs );
	running_sum( cmui->el_diff_rs, rq->el_diff );

	reset_running_sum( cmui->az_rs );
	running_sum( cmui->az_rs, rq->az );

	reset_running_sum( cmui->el_rs );
	running_sum( cmui->el_rs, rq->el );

	cmui->az_diff_sum += rq->az_diff;
	cmui->el_diff_sum += rq->el_diff;
    }

    cmui->fxd_ang_sum += gri->scan_mode == DD_SCAN_MODE_PPI ? gri->elevation
	: gri->azimuth;
    ++cmui->sweep_ray_num;
    if( !cmui->transition_count ) {
	gri->fixed = cmui->fxd_ang_sum/(double)(cmui->sweep_ray_num);
    }
    cmui->swp_que->swpang = rq->fxdang = gri->fixed;

    gri->source_sweep_num = rcimmh->itilt_num % 10;
    gri->source_sweep_num = cmui->transition_count ? 1 : 0;
    gri->source_sweep_num += 10 * (gri->sweep_num % 10);
    gri->source_sweep_num += 100 * (gri->vol_num % 10);
    

    return 1;
}
/* c------------------------------------------------------------------------ */

void 
cimm_print_stat_line (int count)
{
    DD_TIME *dts=gri->dts;
    
    gri->dts->time_stamp = gri->time;
    d_unstamp_time(gri->dts);
    printf(" %5d %3d %7.2f %.2f  %s\n"
	   , count
	   , gri->sweep_num
	   , gri->fixed
	   , gri->time
	   , dts_print(gri->dts)
	   );
}
/* c------------------------------------------------------------------------ */


/* c...mark */

/* c------------------------------------------------------------------------ */

int 
cimm_isa_new_sweep (void)
{
    static int ebug = 0;
    int ii, nn, n_new = NO, az_turnaround, el_turnaround;  //Jul 26, 2011 new keyword
    int cond_a, cond_b, cond_c;
    int lag_rays = 2;
    double mrd, d, d2, d3, ang_tol, time_tol, dt, da;
    double daz, avgDaz=0, ddaz, dddaz, del, avgDel=0, ddel, halfMad;
    double sumDaz=0, sumDel=0, short_sum_az_diffs, short_sum_el_diffs;
    double short_avg_az, short_avg_az_diff, short_avg_el, short_avg_el_diff;
    float rotang;

    /* c...mark */


    if(cmui->sweep_ray_num < cmui->min_rays_per_sweep) {
	return(NO);		/* pass thru the min. num. of rays first */
    }

    /* Here's where the pain starts */

    daz = cmui->ray_que->az_diff;
    ddaz = cmui->ray_que->last->az_diff;
    short_sum_az_diffs = short_running_sum( cmui->az_diff_rs ); /* NUM_SHORT_AVG */

    avgDaz = avg_running_sum( cmui->az_diff_rs ); /* NUM_AZ_DIFFS_AVGD */
    avgDel = avg_running_sum( cmui->el_diff_rs ); /* NUM_EL_DIFFS_AVGD */
    short_sum_el_diffs =
	short_running_sum( cmui->el_diff_rs ); /* NUM_SHORT_AVG */

    del = cmui->ray_que->el_diff; /* last diff */
    ddel = cmui->ray_que->last->el_diff;	/* second to last diff */

    az_turnaround = short_sum_az_diffs * cmui->az_diff_sum < 0;
    el_turnaround = short_sum_el_diffs * cmui->el_diff_sum < 0;
    /* cmui->el_diff_sum is from start of sweep */


    if(gri->scan_mode == DD_SCAN_MODE_RHI) {
	if(cmui->transition_count) {
	    /*
	     * cmui->x_az_diff_sum set to 0 at start of each sweep
	     * so it should be 0 the first time through
	     */
	    cmui->x_az_diff_sum += cmui->ray_que->az_diff;
	    cmui->transition_count++;

	    if( fabs(cmui->x_az_diff_sum) > cmui->max_rhi_diff ) { /* MAX_RHI_DIFF */
		/*
		 * change in az since transition began > next reasonable rhi 
		 * end of RHIs
		 */
		gri->scan_mode = DD_SCAN_MODE_PPI;
		printf( "PPI:" );
		return(NO);
	    }
	    short_avg_az_diff =
		short_avg_running_sum( cmui->az_diff_rs ); /* NUM_SHORT_AVG */

	    if(fabs(short_avg_az_diff) > cmui->min_az_diff) { /* MIN_AZ_DIFF */
		/* moving in azimuth hopefully to the next rhi */
		return(NO);
	    }
	    short_avg_el =
		short_avg_running_sum( cmui->el_diff_rs ); /* NUM_SHORT_AVG */

	    if(fabs(short_avg_el) > cmui->min_el_diff) { /* MIN_EL_DIFF */
		/* antenna moving vertically */
		return(YES);
	    }
	    return(NO);
	}

	if( el_turnaround ) {
	    printf( "t8:" );
	    cmui->transition_count++;
	    return(NO);
	}
	if((fabs(cmui->el_diff_run_sum) < cmui->min_el_diff)) { 
	    /*
	     * we are examining the sum of the differences between
	     * ths last NUM_EL_DIFFS_AVGD rays not the whole sweep
	     *
	     * the antenna has stopped moving vertically
	     */
	    printf( "t9:" );
	    cmui->transition_count++;
	    return(NO);
	}
	return(NO);
    }


    if(cmui->transition_count) {	/* we're in a transition */

	short_avg_az_diff =
	    short_avg_running_sum( cmui->az_diff_rs ); /* NUM_SHORT_AVG */
	short_avg_el_diff =
	    short_avg_running_sum( cmui->el_diff_rs ); /* NUM_SHORT_AVG */
	cmui->transition_count++;

	if( fabs(short_avg_el_diff) > cmui->min_el_diff
	    && fabs(short_avg_az_diff) < cmui->rhi_az_tol ) {
	    /*
	     * MIN_EL_DIFF and RHI_AZ_TOL
	     *
	     * moving in elevation and "not" moving in azimuth
	     */
	    gri->scan_mode = DD_SCAN_MODE_RHI;
	    printf( "RHI:" );
	    return(YES);
	}

	if(fabs(short_avg_az_diff) > cmui->min_az_diff
	   && fabs(short_avg_el_diff) < cmui->ppi_el_tol ) {
	    /*
	     * using MIN_AZ_DIFF and PPI_EL_TOL
	     *
	     * moving in azimuth and not moving in elevation
	     */
	    gri->scan_mode = DD_SCAN_MODE_PPI;
	    printf( "PPI:" );
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

    short_avg_el = short_avg_running_sum( cmui->el_rs );	/* NUM_SHORT_AVG */

    if( ebug ) {
	d2 = short_running_sum( cmui->az_diff_rs );
	printf( " %.4f %.4f %.4f %.4f %.4f"
		, fabs(avgDaz), fabs(avgDel), short_sum_el_diffs, short_avg_el
		, d2
	    );
	printf( "\n" );
    }

    if( fabs(short_sum_el_diffs) < cmui->ppi_el_tol ) { /* PPI_EL_TOL */
	/*
	 * antenna is stable for several beams but too far away
	 * from the current fixed angle (MIN_FXD_DIFF)
	 */
	if(FABS(gri->fixed - short_avg_el ) > cmui->min_fxdang_diff) {
	    printf( "t0:" );
	    return(YES);
	}
    }
    if( fabs(cmui->el_diff_run_sum) > cmui->ppi_el_tol ) { /* PPI_EL_TOL */
	/*
	 * moving away from cont. elev.
	 */
	cmui->transition_count++;
	printf( "t1:" );
	return(NO);
    }
    if(fabs(avgDaz) < .5 * cmui->min_az_diff) { /* MIN_AZ_DIFF */
	/*
	 * slowing down in azimuth
	 */
	cmui->transition_count++;
	printf( "t2:" );
	return(NO);
    }
    if( az_turnaround ) {
	/*
	 * antenna has changed direction
	 */
	if( ebug ) {
	    printf( "t3:%.4f %.4f %.4f %.4f\n"
		    , ddaz, daz, short_sum_az_diffs, cmui->az_diff_sum);
	}
	else
	    { printf( "t3:" ); }
	cmui->transition_count++;
	return(NO);
    }
    if(fabs(daz + cmui->az_diff_sum) > 360.) { /* for sweep so far */
	/* over rotated
	 */
	cmui->transition_count++;
	printf( "t4:" );
	return(NO);
    }
    return(NO);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */

