/* 	$Id$	 */

#include <errno.h>
#include <dirent.h>
#include <glib.h>
#include <stdio.h>
#include <string>
#include <string.h>

#include <dd_math.h>
#include <DataManager.hh>
#include <DateTime.hh>
#include "se_bnd.hh"
#include "se_proc_data.hh"
#include "se_utils.hh"
#include "se_wgt_stf.hh"
#include "SeBoundaryList.hh"
#include <sii_enums.h>
#include <solo_editor_structs.h>
#include <solo_list_mgmt.hh>
#include <solo_list_widget_ids.h>
#include <solo_window_structs.h>
#include <solo2.hh>
#include <sp_basics.hh>
#include <sp_save_frms.hh>

/* c------------------------------------------------------------------------ */

std::vector< std::string > se_update_list(int list_id)
{
  // Update various lists so they can be refreshed on the screen

  struct solo_edit_stuff *seds = return_sed_stuff();
  SeBoundaryList *sebs = SeBoundaryList::getInstance();

  // NOTE:  Need to clean these up so that the above variables have more
  // limited scope and don't use names like "a" and "b".  Look at using
  // strings instead of char*.
  
  if (list_id == CURRENT_CMDS_LIST)
  {
    if (seds->cmdz.size() == 0)
    {
      // Put in the basics

      seds->cmdz.push_back("!");
      seds->cmdz.push_back("! for-each-ray (put one-time cmds before this line)");
      seds->cmdz.push_back("!");
    }

    seds->current_cmds = seds->cmdz;
    
    return seds->cmdz;
  }
  else if(list_id == HELP_LIST)
  {
  }
  else if (list_id == RADARS_LIST)
  {
    DataManager *data_mgr = DataManager::getInstance();
      
    se_dump_sfic_widget(seds->sfic,seds->se_frame);

    std::string dir_text = seds->sfic->directory_text;
    if (dir_text[dir_text.size() - 1] != '/')
      dir_text += "/";
    data_mgr->setRescanUrgent(seds->se_frame);
    
    std::vector< std::string > radar_list;
      
    if (data_mgr->compileFileList(seds->se_frame, dir_text) < 1)
    {
      printf("No sweep files in dir: %s\n", dir_text.c_str());
      return radar_list;
    }
    seds->sfic->directory = dir_text;
    seds->list_radars.clear();
	
    for (int ii = 0; ii < data_mgr->getNumRadars(seds->se_frame); ++ii)
      seds->list_radars.push_back(data_mgr->getRadarNameData(seds->se_frame,
							     ii));
    
    return seds->list_radars;
  }
  else if (list_id == SWEEPS_LIST)
  {
    se_dump_sfic_widget(seds->sfic, seds->se_frame);
    std::string dir_text = seds->sfic->directory_text;
    if (dir_text[dir_text.size()-1] != '/')
      dir_text += "/";
    DataManager::getInstance()->setRescanUrgent(seds->se_frame);

    std::vector< std::string > empty_list;
	
    if (DataManager::getInstance()->compileFileList(seds->se_frame,
						    dir_text) < 1)
    {
      printf("No sweep files in dir: %s\n", dir_text.c_str());
      return empty_list;
    }

    seds->sfic->directory = dir_text;
    seds->list_sweeps.clear();

    // Use the first radar name in the list.  The for loop replaces the
    // directory path that was in "str" with the radar name.  a is a
    // pointer into seds->sfic->radar_names_text, pointing to the first
    // non-white character in the string.  b also points into
    // seds->sfic->radar_names_text and it points to the first
    // delimiter character after a.  Thus, str ends up with the first
    // string in seds->sfic->radar_names_text after white space and
    // before delimiters.

    std::string radar_names = seds->sfic->radar_names_text;
    std::size_t non_white_pos = radar_names.find_first_not_of(" \t\n");
    std::size_t first_delim_pos =
      radar_names.find_first_of(" \t\n<>;", non_white_pos);
    std::string radar_name;
    if (non_white_pos == std::string::npos)
      radar_name = radar_names;
    else if (first_delim_pos == std::string::npos)
      radar_name = radar_names.substr(non_white_pos);
    else
      radar_name = radar_names.substr(non_white_pos,
				      first_delim_pos - non_white_pos);
	
    int rn = DataManager::getInstance()->getRadarNumData(seds->se_frame,
							 radar_name);
    seds->sfic->radar_num = rn;
    if (rn < 0)
    {
      printf("No sweep files for radar: %s in %s\n",
	     radar_name.c_str(), seds->sfic->directory.c_str());
      return empty_list;
    }
    DataManager::getInstance()->getFileInfoList(seds->se_frame, rn,
						seds->list_sweeps);
    return seds->list_sweeps;
  }
  else if (list_id == BND_FILES_LIST)
  {
    std::vector< std::string > file_list;
    
    if (sebs->directoryText == "")
    {
      sebs->directoryText = seds->sfic->directory_text;
    }
    se_nab_files(sebs->directoryText, file_list, "bnd");
    sort(file_list.begin(), file_list.end());
    
    return file_list;
  }
  else if (list_id == SED_CMD_FILES_LIST)
  {
    struct sed_command_files *scf = se_return_cmd_files_struct();
    if (scf->directory_text == "")
    {
      scf->directory_text = seds->sfic->directory_text;
    }
    se_nab_files(scf->directory_text, seds->list_ed_cmd_files, "sed");
    sort(seds->list_ed_cmd_files.begin(), seds->list_ed_cmd_files.end());
    return seds->list_ed_cmd_files;
  }
  else if (list_id == FRAME_STATES_LIST)
  {
    struct solo_frame_state_files *sfsf = se_return_state_files_struct();
    se_nab_files(sfsf->directory_text, seds->list_winfo_files, "wds");
    sort(seds->list_winfo_files.begin(), seds->list_winfo_files.end());
    return seds->list_winfo_files;
  }
  else if (list_id == BND_POINTS_LIST)
  {
    std::vector< std::string > points_list;
    
    struct one_boundary *ob = sebs->currentBoundary;

    if (ob && ob->num_points)
    {
      for (struct bnd_point_mgmt *bpm = ob->top_bpm; bpm; bpm = bpm->next)
      {
	char str[128];
	
	sprintf(str, "x:%8.3fkm y:%8.3fkm",
		M_TO_KM((float)bpm->x),
		M_TO_KM((float)bpm->y));
	points_list.push_back(str);
      }
    }

    return points_list;
  }

  std::vector< std::string > empty_list;
  return empty_list;
}

/* c------------------------------------------------------------------------ */

void se_nab_files(const std::string &dir, std::vector< std::string > &file_list,
		  const std::string &type)
{
  // Open the directory

  DIR *dir_ptr;
  if (!(dir_ptr = opendir(dir.c_str())))
  {
    char message[256];
    
    sprintf(message, "Cannot open directory %s\n", dir.c_str());
    solo_message(message);
    return;
  }

  // Fill file_list with the file names

  file_list.clear();
  
  struct dirent *dp;
  
  while ((dp = readdir(dir_ptr)) != 0)
  {
    if (strncmp(dp->d_name, type.c_str(), type.size()) != 0)
      continue;

    file_list.push_back(dp->d_name);
  }

  // Close the directory

  closedir(dir_ptr);
}

/* c------------------------------------------------------------------------ */

int se_nab_all_files(char *dir, struct solo_list_mgmt *slm)
{
    DIR *dir_ptr;
    struct dirent *dp;
    char *b, *fn, mess[256];

    if(!(dir_ptr = opendir(dir))) {
	sprintf(mess, "Cannot open directory %s\n", dir);
	solo_message(mess);
	return(0);
    }
    SLM_reset_list_entries(slm);

    for(;;) {
	dp = readdir(dir_ptr);
	if(dp == NULL ) {
	    break;
	}
	fn =  dp->d_name;
	b = fn +strlen(fn)-1;	/* point to last character */
	if(*fn == '.' || *b == '~')
	      continue;
	SLM_add_list_entry(slm, fn);
    }
    closedir(dir_ptr);
    return(slm->num_entries);
}
