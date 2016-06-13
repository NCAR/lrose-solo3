/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */
/*
 * 
 */
//# include <dorade_headers.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>
#include <stdio.h>
#include <input_sweepfiles.h>
#include "input_limits.hh"
#include "dd_stats.h"
#include "dd_files.hh"
#include "dd_ncdf.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "swp_file_acc.hh"
#include "dorade_ncdf.hh"

static struct dd_input_sweepfiles_v3 *dis;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats;

/* c------------------------------------------------------------------------ */

int 
dd_ncdf_conv (void)
{
    static int count=0;
    int ii, rn;
    struct unique_sweepfile_info_v3 *usi;
    struct dd_general_info *dgi;


    printf("In dd_ncdf_conv \n");
    
    if((ii=dd_ncdf_init()) == END_OF_TIME_SPAN ) {
	return(1);
    }
    else if(ii < 0)
	  return(1);

    usi = dis->usi_top;
    /*
     * for each radar, write out the netCDF file
     */
    for(rn=0; rn++ < dis->num_radars; usi=usi->next) {
	dgi = dd_window_dgi(usi->radar_num);
	dgi->radar_name = usi->radar_name;
	for(;;) {
	    dd_stats->ray_count++;
	    if( dgi->new_sweep)
	      {  dd_stats->sweep_count++; }
	    if( dgi->new_vol)
	      {  dd_stats->vol_count++; }
	    if(!(count++ % 500))
	        dgi_interest_really(dgi, NO, (char *)"", (char *)"", dgi->dds->swib);

	    produce_ncdf(dgi, NO);
	    
	    dgi->new_vol = NO;
	    dgi->new_sweep = NO;
	    if(ddswp_next_ray_v3(usi) == END_OF_TIME_SPAN) {
		break;
	    }
	}
	produce_ncdf(dgi, YES);
    }
    return(0);
}
/* c------------------------------------------------------------------------ */

int 
dd_ncdf_init (void)
{
    int ii, ok;
    struct unique_sweepfile_info_v3 *usi;

    printf("In dd_ncdf_init.\n");


    /* do all the necessary initialization including
     * reading in the first ray
     */
    difs = dd_return_difs_ptr();
    dd_stats = dd_return_stats_ptr();
    dis = dd_setup_sweepfile_structs_v3(&ok);
    dis->print_swp_info = YES;

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








