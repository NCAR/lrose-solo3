#include <cstdio>

#ifdef USE_RADX
#else
#include <se_proc_data.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "SaveCommandsCmd.hh"


/*********************************************************************
 * Constructors
 */

SaveCommandsCmd::SaveCommandsCmd() :
  OneTimeOnlyCmd("save-commands", "")
{
}


/*********************************************************************
 * Destructor
 */

SaveCommandsCmd::~SaveCommandsCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool SaveCommandsCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  struct swp_file_input_control *sfic = return_swp_fi_in_cntrl();
  struct sed_command_files *scf = se_return_cmd_files_struct();

  // If we don't have a valid directory name, get the dir name from the sfic

  if (scf->directory_text == "")
    scf->directory_text = sfic->directory_text;

  // Get the directory name

  std::string dir = scf->directory_text;
  
  if (dir[dir.size() - 1] != '/')
    dir += std::string("/");

  // Get the radar name

  std::string radar_name;
  if (seds->last_radar_stack.size() > 0 &&
      seds->last_radar_stack[0] != "")
  {
    radar_name = seds->last_radar_stack[0];
  }
  else if (sfic->radar_names_text != "")
  {
    radar_name = sfic->radar_names_text;
    std::size_t first_nonwhite =
      radar_name.find_first_not_of(" \t\n");
    if (first_nonwhite != std::string::npos)
      radar_name = radar_name.substr(first_nonwhite);
    
    std::size_t filename_end =
      radar_name.find_first_of(" \t\n<>;");
    if (filename_end != std::string::npos)
      radar_name = radar_name.substr(0, filename_end-1);
  }
  else
  {
    radar_name = "UNK";
  }

  // Get the file name based on the radar name

  std::string file_name =
    DataManager::getInstance()->getFileBaseName("sed", time(0),
						radar_name, getuid());
  scf->file_name_text = file_name;

  // If a comment was specified, add it to the file name

  if (scf->comment_text != "")
  {
    se_fix_comment(scf->comment_text);
    file_name += std::string(".") + scf->comment_text;
  }

  std::string file_path = dir + file_name;
  
  FILE *stream;
  
  if ((stream = fopen(file_path.c_str(), "w")) == 0)
  {
    g_string_sprintfa (gs_complaints,"Unable to save editor commands to %s\n",
		       file_path.c_str());
    return 1;
  }

  for (size_t i = 0; i < seds->edit_summary.size(); ++i)
  {
    int nn = fputs(seds->edit_summary[i].c_str(), stream);
    if (nn <= 0 || nn == EOF)
    {
      // We have an error
      break;
    }
  }

  fclose(stream);
  
  return true;
}

#else

bool SaveCommandsCmd::doIt() const
{
  if (se_write_sed_cmds(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
