/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */
/*
 * 
 */
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <input_sweepfiles.h>
#include <dd_general_info.h>
#include "input_limits.hh"
#include "dd_stats.h"
#include "dd_files.hh"
#include "dd_swpfi.hh"
#include "dda_common.hh"
#include "gpro_data.hh"
#include "swp_file_acc.hh"
#include "dd_swp_files.hh"
#include "dorade_share.hh"

static struct dd_input_sweepfiles_v3 *dis;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats;
static char *acmrg = NULL;


/* c------------------------------------------------------------------------ */

int 
dd_swpfi_conv (void)
{
    static int count=0;
    int ii, jj, nn, rn;
    struct unique_sweepfile_info_v3 *usi;
    struct dd_general_info *dgi;
    DDS_PTR dds;
    char *aa, str[256];
    
    if((ii=dd_swpfi_init()) == END_OF_TIME_SPAN ) {
	return(1);
    }
    else if(ii < 0)
	  return(1);

    usi = dis->usi_top;
    /*
     * for each radar
     */
    for(rn=0; rn++ < dis->num_radars; usi=usi->next) {
	dgi = dd_window_dgi(usi->radar_num);
	dds = dgi->dds;

	dgi->radar_name = usi->radar_name;
	for(;;) {
	    acmrg = eld_nimbus_fix_asib(dgi->dds->dts, dgi->dds->asib
					, 0, dgi->radar_num);
	    dd_stats->ray_count++;

	    if( dgi->new_sweep)
	      {  dd_stats->sweep_count++; }
	    if( dgi->new_vol) {
	      dd_stats->vol_count++;
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
	    }
				/*
	    if(!(count++ % 500))
		  dgi_interest_really(dgi, NO, "", "", dgi->dds->swib);
				 */
	    dd_dump_ray(dgi);

	    if(ddswp_last_ray(usi)) { /* end of current sweep */
	      dd_flush(usi->radar_num);	 
	    }
	    
	    dgi->new_vol = NO;
	    dgi->new_sweep = NO;
	    if(ddswp_next_ray_v3(usi) == END_OF_TIME_SPAN) {
		break;
	    }
	}
	dd_flush(usi->radar_num);	 
    }
    return(0);
}
/* c------------------------------------------------------------------------ */

int 
dd_swpfi_init (void)
{
    int ii, jj, kk, ok;
    struct unique_sweepfile_info_v3 *usi;

    /* do all the necessary initialization including
     * reading in the first ray
     */
    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();
    dis = dd_setup_sweepfile_structs_v3(&ok);
    dis->print_swp_info = YES;
    dis->print_swp_info = NO;
    dis->editing = YES;

    if(dis->num_radars < 1)
      return(-1);

    for(ii=0,usi=dis->usi_top; ii++ < dis->num_radars; usi=usi->next) {
	/* now try and find files for each radar by
	 * reading the first record from each file
	 */
	if(ddswp_next_ray_v3(usi) == END_OF_TIME_SPAN) {
	    printf("Problems reading first record of %s/%s\n",
		   usi->directory.c_str(), usi->filename.c_str());
	    ok = NO;
	}
    }
    if(!ok)
	  return(-1);
    return(0);
}
/* c------------------------------------------------------------------------ */

/* c------------------------------------------------------------------------ */








