#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "VersionCmd.hh"


/*********************************************************************
 * Constructors
 */

VersionCmd::VersionCmd() :
  OneTimeOnlyCmd("version", "")
{
}


/*********************************************************************
 * Destructor
 */

VersionCmd::~VersionCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool VersionCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command

  if (_cmdTokens[1].getCommandText().compare("first") == 0)
  {
    seds->sfic->version = -1;
    seds->sfic->version_text = "first";
  }
  else if (_cmdTokens[1].getCommandText().compare("last") == 0)
  {
    seds->sfic->version = 99999;
    seds->sfic->version_text = "last";
  }
  else
  {
    // Try to interpret this as a number

    seds->sfic->version = 99999;
      
    if (sscanf(_cmdTokens[1].getCommandText().c_str(), "%d",
	       &seds->sfic->version) == 0)
    {
      // Not a number

      g_string_sprintfa(gs_complaints, 
			"\nCould not interpret version number: %s  Using: %d\n",
			_cmdTokens[1].getCommandText().c_str(),
			seds->sfic->version);
    }
    char version_text[80];
    sprintf(version_text, "%d", seds->sfic->version);
    seds->sfic->version_text = version_text;
  }
  
  return true;
}

#else

bool VersionCmd::doIt() const
{
  if (se_dir(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
