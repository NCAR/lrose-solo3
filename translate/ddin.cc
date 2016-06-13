/* 	$Id: ddin.c 190 2003-10-09 18:55:33Z oye $	 */

#ifndef lint
static char vcid[] = "$Id: ddin.c 190 2003-10-09 18:55:33Z oye $";
#endif /* lint */

#include <LittleEndian.hh>
#include <dd_math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <math.h>
#include "dd_stats.h"
#include "ddin.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "gpro_data.hh"
#include "swp_file_acc.hh"
#include "dorade_share.hh"
#include "dd_swp_files.hh"
#include "to_fb.hh"
#include "dorade_tape.hh"
#include "dd_crackers.hh"
#include "elda_dd.hh"
#include "by_products.hh"

# ifndef TOGACOARE_FIXES
# define      TOGACOARE_FIXES 0x8000
# endif

# define      ROT_ANG_SECTORS 0x80000

struct dorade_unique_info {
    struct dd_general_info *dgi;
    struct dor_inidividual_radar_info *diri[MAX_SENSORS];
    time_t current_time;
    double time_correction;
    float sw_fudge;
    int current_radar_ndx;
    int dorade_fid;
    int io_type;
    int options;
    int radar_num;
    int ascending_fxd_only;
    char *current_file_name;
};

static struct dorade_unique_info *dui=NULL;

static struct generic_descriptor *gd;
static struct volume_d *vold=NULL;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats=NULL;
static double time_for_nimbus;

static struct input_read_info *irq;
static struct solo_list_mgmt *slm;
static char preamble[24], postamble[64];
static struct io_packet vol_dp;
static char *acmrg=NULL;

/*# ifdef obsolete

void slm_print_list();
void solo_add_list_entry();
void solo_unmalloc_list_mgmt();
void solo_reset_list_entries();
void solo_unmalloc_list_mgmt();
void difs_terminators();
int dd_return_id_num();
void dd_radar_selected();
void dd_intset();
void dd_set_control_c();
void dd_reset_control_c();
void dd_flush();
void dd_tape();
void dd_set_uniform_cells();
void dd_alloc_data_field();
void dd_radar_angles();
void dd_gen_packet_info();
void dd_skip_files();
void dd_rewind_dev();
void ctypeu16();
void ezhxdmp();
void ezascdmp();
void dd_clear_dts();
void gri_nab_input_filters();
void dd_io_reset_offset();
char *eld_nimbus_fix_asib();
void eld_gpro_fix_asib();
void dd_dump_headers();
void by_products();
void dd_dump_ray();
int dd_input_read_open();
int gri_max_lines();
int getreply();
int cdcode();
int dd_control_c();
int dd_logical_read();
int dd_hrd16_uncompressx();
int dd_datum_size();
int dd_assign_radar_num();
int dd_skip_recs();
int gri_start_stop_chars();
int dd_crack_datime();
int in_sector();
int dd_clip_gate();

# else

//# include <function_decl.h>
# include <dgi_func_decl.h>

# endif
*/
//void eld_dump_raw_header();
/*
 * This file contains the following routines
 */


/*
void ddin_init_sweep();



void ddin_stuff_ray();

void dgi_interest_really();
void dor_print_asib();
void dor_print_celv();
void dor_print_parm();
void dor_print_radd();
void dor_print_ryib();
void dor_print_swib();
void dor_print_vold();


char *dd_establish_source_dev_names();
char *dd_next_source_dev_name();*/


/* c------------------------------------------------------------------------ */

void 
dgi_interest (struct dd_general_info *dgi, int verbosity, char *preamble, char *postamble)
{
    /* 
     * 
     */
    int ii, jj, nn;
    char str[256], *a;
    struct dds_structs *dds;
    struct sweepinfo_d *swib;

    if(vold->volume_des_length == 0) {
	printf("\nNo volume header here.\n");
	printf("Use skip file to get to a volume header\n");
	return;
    }
    dds = dgi->dds;
    swib = &dui->diri[dgi->radar_num]->swib;
    dgi_interest_really(dgi, verbosity, preamble, postamble, swib);
}
/* c------------------------------------------------------------------------ */

void 
ddin_reset (void)
{
    /* reset various parameters so the program will startup right
     */
    int ii;
    struct dor_inidividual_radar_info *diri;
    struct dd_general_info *dgi;

    difs->stop_flag = NO;

    for(ii=0; ii < MAX_SENSORS; ii++){ /* for all the radars so far */
	if(dui->diri[ii]->radar_num < 0)
	      continue;
	diri = dui->diri[ii];
	diri->ray_count = diri->sweep_count = diri->vol_count = 0;
	diri->sweep_skip_num = -1;
	diri->sweep_count_flag = diri->vol_count_flag = YES;
	diri->dgi->new_sweep = diri->dgi->new_vol = YES;
	dgi = dd_get_structs(diri->radar_num);
	dgi->source_sweep_num = -1;
	dgi->sweep_count = 0;
	dgi->new_sweep = dgi->new_vol = YES;
    }
}
/* c------------------------------------------------------------------------ */

void 
ddin_dd_conv (int interactive)
{
    static int count=0, trip=123;
    int ii, n, rn, mark, num_rejects=0;
    char str[64];
    struct dor_inidividual_radar_info *diri;
    DGI_PTR dgi;


    if(!count) {
	/* initialize and read in first ray
	 */
	ddin_ini(interactive);
    }

    if(interactive) {
	dd_intset();
	ddin_positioning();
    }
    ddin_reset();

    /* loop through the data
     */
    while(1){
	count++;
	if(count >= trip) {
	    mark=0;
	}
	dgi = dui->dgi;
# ifdef obsolete
	difs_terminators(dgi, difs, dd_stats);
# endif
	if(difs->stop_flag &&
	   (difs->abrupt_start_stop || dgi->new_sweep))
	      break;

	if(ddin_select_ray(dgi)) {
	    diri = dui->diri[rn = dui->radar_num];

	    if(diri->sweep_count_flag) {
		diri->sweep_count_flag = NO;
		if(difs->max_sweeps) {
		    if(++diri->sweep_count > difs->max_sweeps )
			  break;
		}
		if(diri->vol_count_flag) {
		    diri->vol_count_flag = NO;
		    if(difs->max_vols) {
			if(++diri->vol_count > difs->max_vols)
			      break;
		    }
		}
	    }
	    if(difs->max_beams) {
		if(++dui->diri[rn]->ray_count > difs->max_beams )
		      break;
	    }
	    /*
	     * pass it off to dorade code
	     */
	    ddin_stuff_ray(dgi, dui->current_time);
	}
	else {
	    if(!(num_rejects++ % 555)) {
		d_unstamp_time(dgi->dds->dts);
		printf( "f:%d r:%3d b:%4d %.1fMB %s %s s:%2d, v:%2d, f:%.1f\n",
			dd_stats->file_count,
			dd_stats->rec_count,
			dd_stats->ray_count,
			dd_stats->MB,
			dts_print(dgi->dds->dts),
			dgi->radar_name.c_str(),
			dui->diri[dui->radar_num]->raw_sweep_count,
			dui->diri[dui->radar_num]->raw_vol_count,
			dui->diri[dui->radar_num]->swib.fixed_angle);
	    }
	    mark = 0;
	}
	if((n = ddin_next_ray()) < 1)
	      break;
    }
}
/* c------------------------------------------------------------------------ */

void 
ddin_dump_this_ray (struct dd_general_info *dgi, struct solo_list_mgmt *slm)
{
    int gg, ii, nn, np=dgi->source_num_parms;
    short *flds[7];
    float unscale[7], bias[7];
    char ts[12];
    char *aa, bstr[128];

    aa = bstr;

    solo_reset_list_entries(slm);
    dor_print_swib(&dui->diri[dgi->radar_num]->swib, slm);
    dor_print_ryib(dgi->dds->ryib, slm);
    dor_print_asib(dgi->dds->asib, slm);

    solo_add_list_entry(slm, " ");

    sprintf(aa, "Contents of data fields");
    solo_add_list_entry(slm, aa);
    sprintf(aa, "name ");
    if(np > 7) np = 7;

    for(ii=0; ii < np; ii++) {
	sprintf(aa+strlen(aa), "%10s"
		, str_terminate(ts, dgi->dds->parm[ii]->parameter_name, 8));
	flds[ii] = (short *)dgi->dds->qdat_ptrs[ii];
	bias[ii] = dgi->dds->parm[ii]->parameter_bias;
	unscale[ii] = dgi->dds->parm[ii]->parameter_scale ?
	      1./dgi->dds->parm[ii]->parameter_scale : 1.;
    }
    solo_add_list_entry(slm, aa);

    for(gg=0; gg < dgi->dds->celv->number_cells; gg++) {
	sprintf(aa, "%4d)", gg);

	for(ii=0; ii < np; ii++) {
	    sprintf(aa+strlen(aa), "%10.2f"
		    , DD_UNSCALE(*(flds[ii]+gg), unscale[ii], bias[ii]));
	}
	solo_add_list_entry(slm, aa);
    }
    slm_print_list(slm);
}
/* c------------------------------------------------------------------------ */

int 
ddin_ini (int interactive)
{
    int ii, kk, n, mark;
    char *aa;
    struct dor_inidividual_radar_info *diri;
    DD_TIME dts;


    dts.year = 1995;
    dts.month = 1;		/* Jan. 1995 */
    dts.day = 1;
    dts.day_seconds = 0;
    time_for_nimbus = d_time_stamp(&dts);
    /*
     * set up input filters
     */
    dd_min_rays_per_sweep();	/* trigger min rays per sweep */
    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();

    dui = (struct dorade_unique_info *)
	  malloc(sizeof(struct dorade_unique_info));
    memset((char *)dui, 0, sizeof(struct dorade_unique_info));

    memset(&vol_dp, 0, sizeof(struct io_packet));

    for(kk=0; kk < MAX_SENSORS; kk++) {
	dui->diri[kk] = 
	diri = (struct dor_inidividual_radar_info *)
	      malloc(sizeof(struct dor_inidividual_radar_info));
	memset(diri, 0, sizeof(struct dor_inidividual_radar_info));
	diri->radar_num = -1;
    }

    if(aa=get_tagged_string("ASCENDING_ONLY")) {
	dui->ascending_fxd_only = YES;
	printf( "Allow only ASCENDING fixed angles\n");
    }
    if(aa=get_tagged_string("TIME_CORRECTION")) {
	dui->time_correction += atof(aa);
	printf( "Time correction: %.3f seconds\n", dui->time_correction);
    }
    if((aa=get_tagged_string("OPTIONS"))) {
	if(strstr(aa, "ROT_ANG_S")) { /* ROT_ANG_SECTORS */
	    dui->options |= ROT_ANG_SECTORS;
	}
	if(strstr(aa, "COARE_FIX")) {
	    dui->options |= TOGACOARE_FIXES;
	}
	if(strstr(aa, "COARE_SW")) {
	    dui->sw_fudge = 1./sqrt(PI);
	}
    }
    /* common volume header */
    vold = (struct volume_d *)malloc(sizeof(struct volume_d));
    memset((char *)vold, 0, sizeof(struct volume_d));

    irq = dd_return_ios(5, DORADE_FMT);

    if((aa=get_tagged_string("SOURCE_DEV"))) {
      aa = dd_establish_source_dev_names((char *)"DDI", aa);
      aa = dd_next_source_dev_name((char *)"DDI");
	dui->current_file_name = aa;
	dd_input_read_open(irq, aa);
    }
    n = ddin_next_ray();
    
    if(!vold->volume_des_length) {
	if(!interactive) {
	    printf("\nNo Volume Header information!\n");
	    printf("Move the tape to the beginning of a file.\n\n");
	    exit(1);
	}
    }
    return (0); //Jun 23, 2011
}
/* c------------------------------------------------------------------------ */

void ddin_init_sweep(DGI_PTR dgi, time_t current_time, int new_vol)
{
    DDS_PTR dds=dgi->dds;
    short tdate[6];
    int ii;
    struct super_SWIB *sswb=dds->sswb;
    struct sweepinfo_d *swib=dds->swib;
    struct prev_swps *pss;
    struct dor_inidividual_radar_info *diri;
    DD_TIME *dts=dgi->dds->dts;

    
    dd_flush(dgi->radar_num);

    /* nab the new sweep info block
     */
    diri = dui->diri[dgi->radar_num];
    memcpy(swib, &diri->swib, diri->swib.sweep_des_length);

    dgi->sweep_time_stamp = (int32_t) dgi->time;
    dgi->sweep_count++;
    d_unstamp_time(dts);
    pss = dgi->swp_que = dgi->swp_que->next;
    pss->source_vol_num = dgi->source_vol_num;
    pss->source_sweep_num = dgi->source_sweep_num;
    pss->segment_num = dts->millisecond;
    pss->listed = NO;
    pss->new_vol = dgi->new_vol;
//    pss->sweep_time_stamp = 
//	  dgi->sweep_time_stamp = pss->start_time = dgi->time;
	pss->start_time = dgi->time;
    pss->sweep_time_stamp = 
	  dgi->sweep_time_stamp = (int32_t) pss->start_time;
    pss->num_rays = 0;
    pss->sweep_file_size = sizeof(struct sweepinfo_d);

    /* super sweep info block */
    pss->volume_time_stamp = 
	  sswb->volume_time_stamp = dgi->volume_time_stamp;
    sswb->last_used = todays_date(tdate);

    dd_tape(dgi, NO);

    /* sweep info block
     */
    swib->filter_flag = NO_FILTERING;
    strncpy((char *)swib->radar_name, (char *)dds->radd->radar_name, 8);

}
/* c------------------------------------------------------------------------ */

int 
ddin_inventory (void)
{
    /* the purpose of this routine is to facilitate a more detailed
     * examination of the data
     */
    int ii=0;
    int nn, ival;
    float val;
    char str[256];
    int max = 256; //added Jun 27, 2011


    for(;;) {
	for(ii=0; ii < max; ii++) {
	    if((nn=ddin_next_ray()) < 1)
		  break;
	    sprintf(preamble, "%2d", irq->top->b_num);
	    dgi_interest(dui->dgi, 0, preamble, postamble);
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
ddin_next_ray (void)
{
    static int rdat_sos = 0;
    static int count=0, ddin_bytes_left=0, rec_count=0, eof_count=0;
    static char *ddin_next_block=NULL;
    static int trip=0, rippin=7000, trip_inc=1000, drn;
    static DGI_PTR dgi;
    static DDS_PTR dds;
    struct generic_descriptor xgd;
    static struct dor_inidividual_radar_info *diri;
    static double trip_time=729274170.;
    int gotta_cfac = NO;
    int gotta_celv = NO;
    int ignore_cfacs = NO;
    int ii, jj, n, mm, bd, ncopy, ds, err_count=0;
    int ryib_flag=NO, mark, final_bad_count;
    int num_rdats=0, zeros_rec=0, nmax, pn;
    char *aa, *bb, *cfac_id;
    char radar_name[16];
    short *ss;
    double d, fudge, scale, rcp_scale, bias;

    struct paramdata_d *rdat;
    struct qparamdata_d *qdat;
    struct radar_d *radd;
    struct field_radar_i *frib = 0;
    struct sweepinfo_d *swib;
    struct dd_comm_info *dd_comm;
    struct io_packet *dp;
    struct cfac_wrap *cfw;


    while(1) {
	/* absorb the next ray as well as all intervening headers
	 */
	if( count >= rippin ) {
	    mark = 0;
	}
	count++;
	if(dd_control_c()) {
	    dd_reset_control_c();
	    return(-1);
	}

	while(irq->top->bytes_left <= sizeof(struct generic_descriptor)) {
	    /* read in the next record */
	    if(dd_control_c()) {
		dd_reset_control_c();
		return(-1);
	    }
	    rec_count++;
	    dd_logical_read(irq, FORWARD);
	    
	    if(irq->top->read_state < 0 && err_count > 3 ) {

		printf("Last read: %d\n", irq->top->read_state);
		irq->top->bytes_left = 0;
		dd_input_read_close(irq);
		/*
		 * see if there is another source dev
		 */
		if(aa = dd_next_source_dev_name((char *)"DDI")) {
		    dui->current_file_name = aa;
		    if((ii = dd_input_read_open(irq, aa)) <= 0) {
			return(-1);
		    }
		}
		else
		    { return(-1); }
	    }
	    else if(irq->top->read_state < 0 ) {
		printf("Last read: %d\n", irq->top->read_state);
		err_count++;
	    }
	    else if(irq->top->eof_flag) {
		dd_stats->file_count++;
		printf("EOF at %.2f MB file: %d rec: %d ray: %d\n"
		       , dd_stats->MB
		       , dd_stats->file_count, dd_stats->rec_count
		       , dd_stats->ray_count
		       );
		err_count = 0;
		if( ryib_flag )	/* assume last ray in the file */
		      return(1);

		else if(++eof_count > 2 ) {
		    irq->top->bytes_left = 0;
		    dd_input_read_close(irq);
		    /*
		     * see if there is another source dev
		     */
		    if(aa = dd_next_source_dev_name((char *)"DDI")) {
			dui->current_file_name = aa;
			if((ii = dd_input_read_open(irq, aa)) <= 0) {
			    return(-1);
			}
		    }
		    else
			{ return(-1); }
		}
	    }
	    else if(irq->top->sizeof_read <
		    sizeof(struct generic_descriptor)) {
		printf("Record length: %d too small\n", n);
		return(-1);
	    }
	    else {
		if(irq->io_type == BINARY_IO) {
		    /* reset the disk offset to point to the start
		     * of the last incomplete ray in the buffer
		     */
		    dorade_reset_offset(irq);
		}
		err_count = 0;
		eof_count = 0;
		dd_stats->MB += BYTES_TO_MB(irq->top->sizeof_read);
		dd_stats->rec_count++;
		if(!strncmp(irq->top->at, "DORADE", 4)) {
		    irq->top->bytes_left = 0;
		    continue;
		}
		break;
	    }
	}

	ddin_bytes_left = irq->top->bytes_left;
	ddin_next_block = irq->top->at;

	if(hostIsLittleEndian()) {
	   gd = &xgd;
	   memcpy(gd->name_struct, ddin_next_block, 4);
//	   swack4(ddin_next_block+4, &gd->sizeof_struct);  //Jul 26,2011
	   swack4(ddin_next_block+4, (char *)&gd->sizeof_struct);  //Jul 26,2011
	}
	else {
	   gd = (struct generic_descriptor *)ddin_next_block;
	}	
	if(gd->sizeof_struct < sizeof(struct generic_descriptor)) {
	    if(++zeros_rec > 2) {
		/* This can happen with Exabytes */
		printf("Number of zero recs > 2 %d\n", n);
		return(-1);
	    }
	    gd->sizeof_struct = 0;
	    ddin_bytes_left = 0;
	    irq->top->bytes_left = 0;
	}
	else if(strncmp(ddin_next_block,"RDAT",4) == 0 ||
		strncmp(ddin_next_block,"QDAT",4) == 0) {
	   if( !dgi->dds->uniform_cell_count[0] ) {
	      mark = 0;
	   }
	    pn = num_rdats++;
	    qdat = (struct qparamdata_d *)ddin_next_block;
	    dds->field_present[pn] = YES;

	    ncopy = *ddin_next_block == 'R'
		  ? sizeof(struct paramdata_d) : dds->parm[pn]->offset_to_data;
	    ds = dd_datum_size((int)dds->parm[pn]->binary_format);

	    if(hostIsLittleEndian()) {
//	       ddin_crack_qdat(ddin_next_block, dds->qdat[pn], (int)0);  //Jul 26, 2011
	       ddin_crack_qdat(ddin_next_block, (char *)dds->qdat[pn], (int)0);  //Jul 26, 2011
	    }
	    else {
	       memcpy((char *)dds->qdat[pn], ddin_next_block, ncopy);
	    }

	    if (rdat_sos != dds->qdat[pn]->pdata_length &&
		dds->radd->data_compress == NO_COMPRESSION) {
	      mark = 0;
	    }
	    rdat_sos = dds->qdat[pn]->pdata_length;

	    if(dds->radd->data_compress == NO_COMPRESSION) {
		mm = gd->sizeof_struct - ncopy;
		if(hostIsLittleEndian() && ds > 1) {
		   if(ds == sizeof(short)) {
		      swack_short(ddin_next_block+ncopy, dds->qdat_ptrs[pn]
				  , mm/sizeof(short));
		   }
		   else {	/* punt! */
		   }
		}
		else {
		   memcpy(dds->qdat_ptrs[pn], ddin_next_block+ncopy, mm);
		}
	    }
	    else {
		aa = ddin_next_block + ncopy;
		bb = dds->qdat_ptrs[pn];
		nmax = dds->celv->number_cells;
		if(hostIsLittleEndian()) {
//Jul 26, 2011 start
/*		   n = dd_hrd16LE_uncompressx
		     ((short *)aa, (short *)bb, (int)dds->parm[pn]->bad_data
		      , &final_bad_count, nmax);*/
		   n = dd_hrd16LE_uncompressx
		     ((unsigned short *)aa, (unsigned short *)bb, (int)dds->parm[pn]->bad_data
		      , &final_bad_count, nmax); 
//Jul 26, 2011 end		      
		}
		else {
		   n = dd_hrd16_uncompressx
		     ((short *)aa, (short *)bb, (int)dds->parm[pn]->bad_data
		      , &final_bad_count, nmax);
		}
		mm = n * dd_datum_size((int)dds->parm[pn]->binary_format);
	    }
	    dds->qdat[pn]->pdata_length = ncopy + dds->sizeof_qdat[pn];

	    if(dui->sw_fudge) {		/* fudge spectral width */
		if(!strncmp(qdat->pdata_name, "SW", 2)) {
		    bd = dds->parm[pn]->bad_data;
		    mm = dds->celv->number_cells;
		    scale = dds->parm[pn]->parameter_scale;
		    rcp_scale = 1./scale;
		    bias = dds->parm[pn]->parameter_bias;
		    fudge = dui->sw_fudge;
		    ss = (short *)dds->qdat_ptrs[pn];

		    for(; mm--; ss++) {
			if(*ss != bd) {
			    d = DD_UNSCALE(*ss, rcp_scale, bias) * fudge;
			    *ss = (short)DD_SCALE(d, scale, bias);
			}
		    }
		}
	    }
	}
	else if( ryib_flag && (strncmp(ddin_next_block,"RYIB",4)==0 ||
			       strncmp(ddin_next_block,"SWIB",4)==0 ||
			       strncmp(ddin_next_block,"VOLD",4)==0)) {
	    break;
	}
	else if(strncmp(ddin_next_block,"RYIB",4)==0 ) {
	    if(vold->volume_des_length == 0) {
		return(-1);
	    }
	    dp = dd_return_next_packet(irq);
	    num_rdats = 0;
	    ryib_flag = YES;
	    dgi = dui->dgi;
	    dds = dui->dgi->dds;
	    /*
	     * dui->dgi holds the current struct pointer for this sweep
	     */
	    if(hostIsLittleEndian()) {
//	       ddin_crack_ryib(ddin_next_block, dds->ryib, (int)0);  //Jul 26, 2011
	       ddin_crack_ryib(ddin_next_block, (char *)dds->ryib, (int)0);  //Jul 26, 2011
	    }
	    else {
	       memcpy((char *)dds->ryib, ddin_next_block, gd->sizeof_struct);
	    }
	    dd_stats->ray_count++;
	    d = dorade_time_stamp(dds->vold, dds->ryib, dds->dts);
	    if (dui->time_correction) {
	       dds->dts->time_stamp += dui->time_correction;
	       d_unstamp_time(dds->dts);
	       dds->vold->year = dds->dts->year;
	       dds->ryib->julian_day = dds->dts->julian_day;
	       dds->ryib->hour = dds->dts->hour;
	       dds->ryib->minute = dds->dts->minute;
	       dds->ryib->second = dds->dts->second;
	       dds->ryib->millisecond = dds->dts->millisecond;
	    }
//	    dui->current_time = dui->dgi->time = dds->dts->time_stamp;
	    dui->dgi->time = dds->dts->time_stamp;
	    dui->current_time = (time_t) dui->dgi->time;
	}
	else if(strncmp(ddin_next_block,"ASIB",4)==0) {
	    if(hostIsLittleEndian()) {
//	       ddin_crack_asib(ddin_next_block, dds->asib, (int)0);  //Jul 26, 2011
	       ddin_crack_asib(ddin_next_block, (char *)dds->asib, (int)0);  //Jul 26, 2011
	    }
	    else {
	       memcpy((char *)dgi->dds->asib, ddin_next_block
		      , gd->sizeof_struct);
	    }
	}
	else if(strncmp(ddin_next_block,"SWIB",4)==0 ) {
	    num_rdats = 0;
	    ryib_flag = NO;
	    swib = (struct sweepinfo_d *)ddin_next_block;
	    str_terminate(radar_name, swib->radar_name, 8);
	    dui->dgi = dgi =
		  dd_get_structs(dd_assign_radar_num(radar_name));
	    dds = dgi->dds;
	    dgi->new_sweep = YES;
	    drn = dgi->radar_num;
	    diri = dui->diri[drn];
	    diri->sweep_count_flag = YES;
	    diri->sweep_skip_num++;
	    diri->raw_sweep_count++;
	    dui->radar_num = diri->radar_num = drn;
	    diri->dgi = dgi;
	    str_terminate(diri->radar_name, swib->radar_name, 8);
	    if(hostIsLittleEndian()) {
//	       ddin_crack_swib(ddin_next_block, &diri->swib, (int)0);   //Jul 26, 2011
	       ddin_crack_swib(ddin_next_block, (char *)&diri->swib, (int)0);   //Jul 26, 2011
	    }
	    else {
	       memcpy(&diri->swib, ddin_next_block
		      , gd->sizeof_struct);
	    }
	    dgi->source_sweep_num = diri->swib.sweep_num;
	    diri->fixed_angle = diri->swib.fixed_angle;
	}
	else if(strncmp(ddin_next_block,"PARM",4)==0 ) {
	    /* assume dds & vs6 defined by the previous
	     * radd block encounter
	     */
	    pn = dgi->num_parms++;
	    dgi->source_num_parms = dgi->num_parms;
	    dds->field_present[pn] = YES;
	    if(hostIsLittleEndian()) {
//	       ddin_crack_parm(ddin_next_block, dds->parm[pn], (int)0);  //Jul 26, 2011
	       ddin_crack_parm(ddin_next_block, (char *)dds->parm[pn], (int)0);  //Jul 26, 2011
	       if(gd->sizeof_struct < sizeof(struct parameter_d))
		 { dds->parm[pn]->extension_num = 0; }
	    }
	    else {
	       memcpy((char *)dds->parm[pn]
		      , ddin_next_block, gd->sizeof_struct);
	    }
	    dds->parm[pn]->parameter_des_length = sizeof(struct parameter_d);

	    jj = dds->field_id_num[pn] =
		  dd_return_id_num(dds->parm[pn]->parameter_name);

	    dgi->parm_type[pn] = !(jj == DBZ_ID_NUM || jj == DZ_ID_NUM)
		  ? VELOCITY : OTHER;
	    str_terminate(diri->parm_names[pn][0] //before change: diri->parm_names[pn]
			  , dds->parm[pn]->parameter_name, 8);
	    mark = 0;

struct dor_inidividual_radar_info {
    struct dd_general_info *dgi;
    int radar_num;
    int ray_count;
    int sweep_count;
    int vol_count;
    int sweep_count_flag;
    int vol_count_flag;
    int sweep_skip_num;
    int use_this_sweep;
    char radar_name[12];
    char *parm_names[MAX_PARMS][12];
    struct sweepinfo_d swib;
    float fixed_angle;
    float ref_fixed_angle;
    int raw_sweep_count;
    int raw_vol_count;
};

	}
	else if(strncmp(ddin_next_block,"RADD",4)==0 ) {
	   gotta_cfac = NO;
	   gotta_celv = NO;
	    radd = (struct radar_d *)ddin_next_block;
	    /* set structs for subsequent descriptors */
	    str_terminate(radar_name, radd->radar_name, 8);
	    dgi = dd_get_structs(dd_assign_radar_num(radar_name));
	    dds = dgi->dds;
	    diri = dui->diri[dgi->radar_num];
	    diri->dgi = dgi;
	    str_terminate(diri->radar_name, radd->radar_name, 8);
	    dd_radar_selected(radd->radar_name, dgi->radar_num, difs);
	    if(strstr(radd->radar_name, "ARMAR")) {
		radd->scan_mode = DD_SCAN_MODE_AIR;
	    }
	    if(hostIsLittleEndian()) {
//	       ddin_crack_radd(ddin_next_block, dds->radd, (int)0);  //Jul 26, 2011
	       ddin_crack_radd(ddin_next_block, (char *)dds->radd, (int)0);  //Jul 26, 2011
	    }
	    else {
	       memcpy((char *)dds->radd, ddin_next_block
		      , gd->sizeof_struct);
	    }
	    memcpy((char *)dds->vold, (char *)vold
		   , sizeof(struct volume_d));

	    dds->vold->number_sensor_des = 1;
	    dgi->new_vol = dgi->new_sweep = YES;
	    diri->sweep_count_flag = diri->vol_count_flag = YES;
	    diri->raw_vol_count++;
	    diri->ref_fixed_angle = -999.;
	    dgi->source_vol_num = dds->vold->volume_num;
	    dgi->num_parms = 0;
	    dds->asib->longitude = dds->radd->radar_longitude;
	    dds->asib->latitude = dds->radd->radar_latitude;
	    dds->asib->altitude_msl = dds->radd->radar_altitude;
	    for(ii=0; ii < MAX_PARMS; dds->field_present[ii++]=NO);
	}
	else if(strncmp(ddin_next_block,"CELV",4)==0 ) {
	   gotta_celv = YES;
	    if(hostIsLittleEndian()) {
//	       ddin_crack_celv(ddin_next_block, dds->celv, (int)0);  //Jul 26, 2011
	       ddin_crack_celv(ddin_next_block, (char *)dds->celv, (int)0);  //Jul 26, 2011

	       swack_long(ddin_next_block+12, (char *)&dds->celv->dist_cells[0]  //Jul 26, 2011
			  , (int)dds->celv->number_cells);
	    }
	    else {
	       memcpy((char *)dds->celv, ddin_next_block
		      , gd->sizeof_struct);
	    }
	    memcpy((char *)dds->celvc, (char *)dds->celv
		   , gd->sizeof_struct);

	    for(ii=0; ii < MAX_PARMS; ii++) {
		if(dgi->dds->field_present[ii])
		      dd_alloc_data_field(dgi, ii);
	    }
	   if(gotta_cfac) {	/* in case the cfac is before the celv */
	      /* correct cell vector */
	      for(ii=0; ii < dds->celv->number_cells; ii++) {
		 dds->celvc->dist_cells[ii] = dds->celv->dist_cells[ii]
		   + dds->cfac->range_delay_corr;
	      }
	      dd_set_uniform_cells(dgi->dds);
	   }
	}
	else if(strncmp(ddin_next_block,"FRIB",4)==0 ) {
	   frib = (struct field_radar_i *)ddin_next_block;
	   if (!dds->frib) {
	      dds->frib = (struct field_radar_i *)malloc (sizeof (*frib));
	   }
	   memcpy (dds->frib, frib, sizeof (*frib));
	}
	else if(strncmp(ddin_next_block,"CFAC",4)==0 ) {
	   ignore_cfacs = NO;

	   if (cfw = ddswp_nab_cfacs (diri->radar_name)) { /* external cfac files */
//	      cfac_id = (frib) ? frib->file_name : "default";  //Jul 26, 2011
	      cfac_id = const_cast<char *> ((frib) ? frib->file_name : "default");  //Jul 26, 2011

	      for(; cfw ; cfw = cfw->next ) {
		 if( strstr (cfw->frib_file_name, cfac_id)) {
		    memcpy( dgi->dds->cfac, cfw->cfac, sizeof( *cfw->cfac ));
		    ignore_cfacs = YES;
		    break;
		 }
	      }
	   }
	   gotta_cfac = YES;

	    if(!ignore_cfacs) {
	       if(hostIsLittleEndian()) {
//		  ddin_crack_cfac(ddin_next_block, dds->cfac, (int)0);  //Jul 26, 2011
		  ddin_crack_cfac(ddin_next_block, (char *)dds->cfac, (int)0);  //Jul 26, 2011
	       }
	       else {
		  memcpy((char *)dds->cfac, ddin_next_block
			 , sizeof(struct correction_d));
	       }
	    }
	   if(gotta_celv) {	/* in case the celv is before the cfac */
	      /* correct cell vector */
	      for(ii=0; ii < dds->celv->number_cells; ii++) {
		 dds->celvc->dist_cells[ii] = dds->celv->dist_cells[ii]
		   + dds->cfac->range_delay_corr;
	      }
	      dd_set_uniform_cells(dgi->dds);
	   }
	}
	else if(strncmp(ddin_next_block,"VOLD",4)==0) {
	    dd_gen_packet_info(irq, &vol_dp); /* mark this spot! */
	    num_rdats = 0;
	    ryib_flag = NO;
	    if(hostIsLittleEndian()) {
//	       ddin_crack_vold(ddin_next_block, vold, (int)0);  //Jul 26, 2011
	       ddin_crack_vold(ddin_next_block, (char *)vold, (int)0);  //Jul 26, 2011
	    }
	    else {
	       memcpy((char *)vold, ddin_next_block
		      , gd->sizeof_struct);
	    }
	    if(bb=strstr(vold->proj_name, "NEXRAD_")) {
	      aa = (char *)"NEXRAD.";
		strncpy(bb, aa, strlen(aa));
	    }
	}
	else if(strncmp(ddin_next_block,"COMM",4)==0) {
	    dd_comm = dd_next_comment((struct comment_d *)ddin_next_block
				      , hostIsLittleEndian());
	}
	else if(strncmp(ddin_next_block,"WAVE",4)==0 ||
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
	irq->top->at += gd->sizeof_struct;
	irq->top->bytes_left -= gd->sizeof_struct;

	if(num_rdats > 0) {
	    if(num_rdats >= dgi->source_num_parms)
		break;
	}
    }
    if(dgi->new_vol) dd_stats->vol_count++;
    if(dgi->new_sweep) dd_stats->sweep_count++;
    dp->len = dp->bytes_left - irq->top->bytes_left;
    return(1);
}
/* c------------------------------------------------------------------------ */
/* c------------------------------------------------------------------------ */

void 
ddin_positioning (void)
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
    static int view_char=0, view_char_count=200;

    static char *pbuf=NULL, **pbuf_ptrs=NULL;
    static int pbuf_max=1200;



    slm = solo_malloc_list_mgmt(87);

    postamble[0] = '\0';
    preamble[0] = '\0';
    dgi_interest(dui->dgi, 1, preamble, postamble);


 menu2:
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
11 = Display ray contents\n\
12 = Display raw header contents\n\
Option = "
	   );
    nn = getreply(str, sizeof(str));
    if( cdcode(str, nn, &ival, &val) != 1 || ival < -2 || ival > 14 ) {
	if(ival == -2) exit(1);
	printf( "\nIllegal Option!\n" );
	goto menu2;
    }

    if(ival == -1) {
	solo_unmalloc_list_mgmt(slm);
	return;
    }
    else if(ival < 0)
	  exit(0);


    if(ival == 0) {
	printf("Type number of rays to skip:");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > K64) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	if(ival > 0) {
	    for(ii=0; ii < ival; ii++) {
		if((jj=ddin_next_ray()) < 1)
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
	ddin_next_ray();
	dgi_interest(dui->dgi, 1, preamble, postamble);
    }
    else if(ival == 1) {
	if((nn = ddin_inventory()) == -2)
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
	ddin_next_ray();
	dgi_interest(dui->dgi, 1, preamble, postamble);
    }
    else if(ival == 3) {
	dd_rewind_dev(irq);
	ddin_next_ray();
	dgi_interest(dui->dgi, 1, preamble, postamble);
    }
    else if(ival == 4) {
	if(gri_start_stop_chars(&view_char, &view_char_count) >= 0) {
	    printf("\n");
	    ctypeu16((unsigned char *)irq->top->buf, view_char, view_char_count);  //Jul 26, 2011
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
	printf("Type record skip # or hit <return> to read next rec:");
	nn = getreply(str, sizeof(str));
	if(cdcode(str, nn, &ival, &val) != 1 || fabs((double)val) > K64) {
	    printf( "\nIllegal Option!\n" );
	    goto menu2;
	}
	direction = ival >= 0 ? FORWARD : BACKWARD;
	dd_skip_recs(irq, direction, ival > 0 ? ival : -ival);
	ddin_next_ray();
	nn = irq->top->sizeof_read;
	printf("\n Read %d bytes\n", nn);
	dgi_interest(dui->dgi, 1, preamble, postamble);
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
		 dtarget = dui->dgi->time + skip_secs; 
	}
	else if(skip_secs = dd_relative_time(str)) {
	    dtarget = dui->dgi->time + skip_secs;
	}
	else if(kk = dd_crack_datime(str, nn, &dts)) {
	    if(!dts.year) dts.year = dui->dgi->dds->dts->year;
	    if(!dts.month) dts.month = dui->dgi->dds->dts->month;
	    if(!dts.day) dts.day = dui->dgi->dds->dts->day;
	    dtarget = d_time_stamp(&dts);
	}
	if(!dtarget) {
	    printf( "\nCannot derive a time from %s!\n", str);
	    goto menu2;
	}
	printf("Skipping ahead %.3f secs\n", dtarget -dui->dgi->time);
	/* loop until the target time
	 */
	for(mm=1;; mm++) {
	    if((nn = ddin_next_ray()) <= 0 || dui->dgi->time >= dtarget ||
	       dd_control_c()) {
		break;
	    }
	    if(!(mm % 1000))
		  dgi_interest(dui->dgi, 1, preamble, postamble);
	    mark = 0;
	}
	if(nn)
	      dgi_interest(dui->dgi, 1, preamble, postamble);
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
	    if((nn = ddin_next_ray()) < 1 ||
	       in_sector(dui->diri[dui->dgi->radar_num]->swib.fixed_angle
			 , fx1, fx2)
	       || dd_control_c()) {
		break;
	    }
	    if(!(mm % 1000))
		  dgi_interest(dui->dgi, 1, preamble, postamble);
	    mark = 0;
	}
	if(nn)
	      dgi_interest(dui->dgi, 1, preamble, postamble);
    }
    else if(ival == 10) {
	gri_nab_input_filters(dui->dgi->time, difs, 0);
    }
    else if(ival == 11) {
	ddin_dump_this_ray(dui->dgi, slm);
    }
    else if(ival == 12) {
	eld_dump_raw_header(dui->dgi, irq, &vol_dp, slm);
    }
    preamble[0] = '\0';

    goto menu2;
}
/* c------------------------------------------------------------------------ */

void 
dorade_reset_offset (struct input_read_info *iri)
{
    /* reset the disk offset to point to the start
     * of the last incomplete ray in the buffer
     * the end of a ray is signaled by the presence of
     * descriptors like "RYIB", "SWIB", "VOLD"
     */
    int ii, jj, mark, nn=iri->top->sizeof_read;
    int32_t mm=iri->sizeof_file, oo=iri->top->offset;
    char *c=iri->top->buf,  name[8];
    struct generic_descriptor *gd;
    struct generic_descriptor xgd;

    memset( name, 0, 8 );

    /* loop through the input buffer
     */
    for(ii=0; ii < nn;) {
        if( nn -ii < sizeof(struct generic_descriptor)) {
	   break;
	}
	if(hostIsLittleEndian()) {
	   gd = &xgd;
	   memcpy(gd->name_struct, c, 4);
//	   swack4(c+4, &gd->sizeof_struct);  //Jul 26, 2011
	   swack4(c+4, (char *)&gd->sizeof_struct);  //Jul 26, 2011
	}
	else {
	   gd = (struct generic_descriptor *)c;
	}
	if(gd->sizeof_struct < sizeof(struct generic_descriptor)) {
	    printf("Bad descriptor length for: %s  at: %d of %d bytes\n"
		   , iri->dev_name, iri->top->offset+ii
		   , iri->sizeof_file);
	    mark = ii;
	    break;
	}
	strncpy( name, c, 4 );
	if(strstr(DD_RAY_BOUNDARIES, name)) {
	    mark = ii;		/* mark a ray boundary */
	}
	c += gd->sizeof_struct;
	ii += gd->sizeof_struct;
    }
    if(iri->sizeof_file -(iri->top->offset +ii) <
       sizeof(struct generic_descriptor)) {
	/* we are extremely close to the end of the file */
	mark = ii;
    }
    iri->top->bytes_left = mark;
    dd_io_reset_offset(iri, iri->top->offset+mark);
}
/* c------------------------------------------------------------------------ */

int 
ddin_select_ray (DGI_PTR dgi)
{
    int ii, jj, n, mark, rn=dui->radar_num, ok=YES;
    float az, el;
    double  d;
    char *a;
    static struct dor_inidividual_radar_info *diri;
    diri = dui->diri[rn];

    if(dgi->new_sweep) {
	diri->use_this_sweep = YES;
	/* increment the sweep count for this radar
	 * and see if it should be used
	 */
	if(difs->sweep_skip) {
	    if((diri->sweep_skip_num % difs->sweep_skip)) {
		diri->use_this_sweep = NO;
	    }
	}
    }	
    if(!diri->use_this_sweep)
	  ok = NO;

    if(dgi->time >= difs->final_stop_time)
	  difs->stop_flag = YES;
    
    if(difs->num_time_lims) {
	for(ii=0; ii < difs->num_time_lims; ii++ ) {
	    if(dgi->time >= difs->times[ii]->lower &&
	       dgi->time <= difs->times[ii]->upper)
		  break;
	}
	if(ii == difs->num_time_lims)
	      ok = NO;
    }
    
    if(!difs->radar_selected[dgi->radar_num]) 
	  ok = NO;
    
    if(difs->num_fixed_lims) {
	for(ii=0; ii < difs->num_fixed_lims; ii++ ) {
	    if(diri->swib.fixed_angle >= difs->fxd_angs[ii]->lower &&
	       diri->swib.fixed_angle <= difs->fxd_angs[ii]->upper)
		  break;
	}
	if(ii == difs->num_fixed_lims)
	      ok = NO;
    }
    
    if( dui->ascending_fxd_only ) {
	if( diri->swib.fixed_angle >= diri->ref_fixed_angle ) {
	    diri->ref_fixed_angle = diri->swib.fixed_angle;
	}
	else
	    { ok = NO; }
    }

    if(difs->num_modes) {
	if(difs->modes[dgi->dds->radd->scan_mode]) {
	    mark = 0;
	}
	else
	      ok = NO;
    }

    if(difs->num_az_sectors) {
       d = dui->options & ROT_ANG_SECTORS ? dd_rotation_angle(dgi) :
	 dd_azimuth_angle(dgi);
	for(ii=0; ii < difs->num_az_sectors; ii++) {
	    if(in_sector((float)d
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
	    if(in_sector((float)dd_elevation_angle(dgi)
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
	    if(in_sector((float)dd_azimuth_angle(dgi)
			 , (float)difs->xout_azs[ii]->lower
			 , (float)difs->xout_azs[ii]->upper)) {
		if(difs->num_el_xouts) {
		    for(jj=0; jj < difs->num_el_xouts; jj++) {
			if(in_sector((float)dd_elevation_angle(dgi)
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

    if(!vold->volume_des_length) {
	ok = NO;
    }
    return(ok);
}
/* c------------------------------------------------------------------------ */

void ddin_stuff_ray(DGI_PTR dgi, time_t current_time)
{
    /*
     * this routine assumes that there is a ray of data to be
     * dealt with
     *
     * if this is the first ray of a volume, dump out volume headers
     * if this is the first ray of a sweep,
     *    close out the previous sweep file and
     *    open a new one with the appropriate sweep and parameter
     *    headers
     *
     */
    int ii, jj, nn, new_vol=NO, mark;
    static int count=0, trip=123;
    double d;

    DDS_PTR dds=dgi->dds;
    struct radar_d *radd=dds->radd;
    struct super_SWIB *sswb=dds->sswb;
    struct ray_i *ryib=dds->ryib;
    struct platform_i *asib=dds->asib;
    struct prev_rays *prs;
    char *aa, str[256];


    count++;
    if(count >= trip) {
	mark=0;
    }

    if( dgi->new_vol ) {
	if( dgi->num_parms != radd->num_parameter_des ) {
	    /* Complain! */
	}
	if (acmrg) {
	  nn = sizeof (dds->vold->proj_name);
	  aa = dds->vold->proj_name;
	  str_terminate (str, aa, nn);
	  if (strstr (str, acmrg))
	    { jj = 0; }
	  else if (strlen (str) + strlen (acmrg) < nn ) {
	    strcat (str, acmrg);
	  }
	  else {
	    jj = nn-strlen (acmrg)-1;
	    strcat (str+jj, acmrg);
	  }
	  strcpy (aa, str);
	}
	dd_dump_headers(dgi);
    }
    if( dgi->new_sweep ) {
	ddin_init_sweep(dgi, current_time, new_vol);
    }
    dgi->swp_que->num_rays++;
    prs = dgi->ray_que = dgi->ray_que->next;

//    sswb->stop_time = dgi->swp_que->end_time = prs->time = dgi->time;
    dgi->swp_que->end_time = prs->time = dgi->time;
    sswb->stop_time = (int32_t) dgi->swp_que->end_time;
    prs->rotation_angle = dd_rotation_angle(dgi);
    prs->source_sweep_num = ryib->sweep_num;

    /* merge ac data
     */
    if(dgi->time > time_for_nimbus) {
	acmrg = eld_nimbus_fix_asib(dgi->dds->dts, asib, dui->options
				 , dgi->radar_num);
    }
    else {
	eld_gpro_fix_asib(dgi->dds->dts, asib, dui->options); 
    }
    if(dgi->dds->radd->scan_mode == DD_SCAN_MODE_AIR ||
       dgi->dds->radd->radar_type == DD_RADAR_TYPE_AIR_LF ||
	dgi->dds->radd->radar_type == DD_RADAR_TYPE_AIR_NOSE )
      {
	 dd_radar_angles( dds->asib, dds->cfac, dds->ra, dgi );
      }
    if(difs->altitude_truncations) {
	dgi->clip_gate = dd_clip_gate
	      (dgi, dds->ra->elevation, asib->altitude_msl
	       , difs->altitude_limits->lower
	       , difs->altitude_limits->upper);
    }
    else
	  dgi->clip_gate = dds->celv->number_cells-1;

    prs->clip_gate = dgi->clip_gate;

    /* ray info block */
    ryib->ray_info_length = sizeof(struct ray_i);

    /* platform info block */
    asib->platform_info_length = sizeof(struct platform_i);

    ddin_clip_data( dgi );
    by_products(dgi, current_time);
    dd_dump_ray(dgi);

    dgi->new_vol = NO;
    dgi->new_sweep = NO;
    dgi->prev_scan_mode = dgi->dds->radd->scan_mode;
    dgi->prev_vol_num = dgi->dds->vold->volume_num;
}
/* c------------------------------------------------------------------------ */

int 
ddin_clip_data (DGI_PTR dgi)
{
   /* wipe out the data beyond the clip gate if there is any
    */
   int pn;
   short *ss, *zz, sbad;
   char *aa, *cc, bbad;

   if( ( dgi->dds->celv->number_cells - dgi->clip_gate ) < 2 ) {
      return( 0 );
   }

   for( pn = 0; pn < dgi->source_num_parms; pn++ ) {

      switch( dgi->dds->parm[pn]->binary_format ) {

	 case DD_16_BITS:

	 sbad = (short)dgi->dds->parm[pn]->bad_data;
	 ss = (short *)dgi->dds->qdat_ptrs[pn];
	 zz = ss + dgi->dds->celv->number_cells;
	 ss += dgi->clip_gate;

	 for(; ss < zz; *ss++ = sbad );
	 break;

	 case DD_8_BITS:
	 bbad = (char)dgi->dds->parm[pn]->bad_data;
	 aa = dgi->dds->qdat_ptrs[pn];
	 cc = aa + dgi->dds->celv->number_cells;
	 aa += dgi->clip_gate;
	 for(; aa < cc; *aa++ = bbad );
	 break;
      }
   }
}
/* c------------------------------------------------------------------------ */

