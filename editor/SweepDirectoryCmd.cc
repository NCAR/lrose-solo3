#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "SweepDirectoryCmd.hh"


/*********************************************************************
 * Constructors
 */

SweepDirectoryCmd::SweepDirectoryCmd() :
  OneTimeOnlyCmd("sweep-directory", "sweep-directory is directory-name")
{
}


/*********************************************************************
 * Destructor
 */

SweepDirectoryCmd::~SweepDirectoryCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool SweepDirectoryCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command

  std::string command = _cmdTokens[1].getCommandText();

  if (command.compare(0, 8, "dorade-directory") == 0)
  {
    char *a;
      
    if ((a = getenv("DD_DIR")) || (a = getenv("DORADE_DIR")))
    {
      seds->sfic->directory = a;
      if (seds->sfic->directory[seds->sfic->directory.size()-1] != '/')
	seds->sfic->directory += "/";
    }
    else
    {
      g_string_sprintfa(gs_complaints,  "DORADE_DIR undefined\n");
      std::string cmd_text = _cmdTokens[1].getCommandText();
      if (cmd_text[cmd_text.size()-1] != '/')
	cmd_text += "/";
      seds->sfic->directory = cmd_text;
    }
  }
  else
  {
    seds->sfic->directory = command;
  }

  seds->sfic->directory_text = seds->sfic->directory;
  
  return true;
}

#else

bool SweepDirectoryCmd::doIt() const
{
  if (se_dir(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
