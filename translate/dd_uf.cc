/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */
/*
 * This file contains the following routines
 * 
 */

#include <string.h>
#include <stdio.h>
#include <input_sweepfiles.h>
#include "dd_uf.hh"
#include "dda_common.hh"
#include "ddb_common.hh"
#include "swp_file_acc.hh"
#include "dorade_uf.hh"

static struct dd_input_sweepfiles_v3 *dis;
static struct dd_input_filters *difs;
static struct dd_stats *dd_stats;

/* c------------------------------------------------------------------------ */

int dd_uf_conv (void)
{
    static int count=0;
    int ii, rn;
    struct unique_sweepfile_info_v3 *usi;
    struct dd_general_info *dgi;
    
    if((ii=dd_uf_init()) == END_OF_TIME_SPAN ) {
	return(1);
    }
    else if(ii < 0)
	  return(1);

    usi = dis->usi_top;
    /*
     * for each radar, write out the UF file
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

	    produce_uf(dgi);
	    
	    dgi->new_vol = NO;
	    dgi->new_sweep = NO;
	    if(ddswp_next_ray_v3(usi) == END_OF_TIME_SPAN) {
		break;
	    }
	}
    }
    return(0);
}
/* c------------------------------------------------------------------------ */

int 
dd_uf_init (void)
{
    int ii, ok;
    struct unique_sweepfile_info_v3 *usi;

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
        if( usi->num_swps < 1 ) {
	  printf( "There are no acceptable sweeps in %s\n",
		  usi->directory.c_str());
	  ok = NO;
	  continue;
	}

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






