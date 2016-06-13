/* 	$Id$	 */

#ifndef lint
static char vcid[] = "$Id$";
#endif /* lint */
# include <sys/types.h>
# include <errno.h>
# include <dirent.h>
# include <time.h>
# include <sys/time.h>

# include <dd_defines.h>
# include <dd_files.h>
# include <dorade_share.h>
# include "dd_files_test.hh"


/* c------------------------------------------------------------------------ */

int 
main (void)
{
    struct dd_file_name_v3 *ddfn;
    int ii, jj, nn, rn, version;
    time_t t;
    char str[12];
    char name[256];
    char *dir="/scr/amber/oye/linda";
    char *radar="KOUN";
    double mb;
    DD_TIME dts;
    double d;



    dts.year = 1992;		/* 5/11/92  160109-161217 */
    dts.month = 5;
    dts.day = 11;
    
# ifdef obsolete
    dts.day_seconds = D_SECS(16,2,9,0);
    d = d_time_stamp(&dts);
    ddir_rescan_urgent(0);
    ddiir_files(0, dir);

    for(ii=0; ii < 8; ii++) {
	mddirx_file_list(0, dir);
	rn = ddiir_radar_num(0, radar);
	ddfn = ddfn_search(0, rn, d, TIME_AFTER, -1);
	if(!ddfn)
	      break;
	ddfn_file_name(ddfn, name);
	d = ddfn->time_stamp;
    }
    ddfnp_list(0, rn);
# endif

# ifdef obsolete
    dd_crack_file_name( str, &t, str, &version
		       , "swp.930218215911.TF-ELDR.0");
    n = ddir_file_list(dir);
    i = ddir_num_radars( 0, str );
    j = ddir_radar_num( radar );
    ddir_file_name( SWP_FILE, 1234567, j, 0, name );
    t = get_filename(t, dir, radar
		     , TIME_NEAREST, 1, name );
# endif

    mb = ddir_free("/scr/amber/oye" );
}
