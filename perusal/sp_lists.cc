/* 	$Id$	 */

#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <stdio.h>

#include <solo_defs.h>
#include <solo_window_structs.h>
#include "sp_lists.hh"
#include "sp_basics.hh"
#include "solo2.hh"
// #include <bits/localefwd.h>
#include <DataManager.hh>
#include <DataInfo.hh>

/* c------------------------------------------------------------------------ */

void solo_gen_parameter_list(int frme)
{
  WW_PTR wwptr = solo_return_wwptr(frme);
  int ww = wwptr->lead_sweep->window_num;
  DataInfo *data_info = DataManager::getInstance()->getWindowInfo(ww);
  struct solo_perusal_info *spi = solo_return_winfo_ptr();

  spi->list_parameters.clear();
  
  for (std::size_t i = 0; i < data_info->getNumParams(); i++)
  {
    spi->list_parameters.push_back(data_info->getParamNameClean(i, 8));
  }
}

/* c------------------------------------------------------------------------ */

void solo_gen_radar_list(const int frame_num)
{
  struct solo_perusal_info *perusal_info = solo_return_winfo_ptr();
  DataManager *data_mgr = DataManager::getInstance();
    
  perusal_info->list_radars.clear();
  for (int i = 0; i < data_mgr->getNumRadars(frame_num); i++)
    perusal_info->list_radars.push_back(data_mgr->getRadarNameData(frame_num, i));
}
/* c------------------------------------------------------------------------ */

void solo_gen_swp_list(int frame_index)
{
  struct solo_perusal_info *spi = solo_return_winfo_ptr();
  WW_PTR wwptr = solo_return_wwptr(frame_index);

  spi->list_sweeps.clear();
  DataManager::getInstance()->getFileInfoList(frame_index,
					      wwptr->sweep->radar_num,
					      spi->list_sweeps);
}
/* c------------------------------------------------------------------------ */

int 
solo_get_files (char *dir, struct solo_list_mgmt *lm)
{
    /* tries to create a list of files in a directory
     */
    DIR *dir_ptr;
    struct dirent *dp;
    char mess[256];

    lm->num_entries = 0;

    if(!(dir_ptr = opendir(dir))) {
	sprintf(mess, "Cannot open directory %s\n", dir);
	solo_message(mess);
	return(-1);
    }

    for(;;) {
	dp=readdir(dir_ptr);
	if(dp == NULL ) {
	    break;
	}
	if(isalnum(*dp->d_name)) {
	    SLM_add_list_entry(lm, dp->d_name);
	}
    }
    closedir(dir_ptr);
    if(lm->num_entries > 1)
	  SLM_sort_strings(lm->list, lm->num_entries);

    return(lm->num_entries);
}
