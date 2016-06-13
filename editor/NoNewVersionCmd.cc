#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "NoNewVersionCmd.hh"


/*********************************************************************
 * Constructors
 */

NoNewVersionCmd::NoNewVersionCmd() :
  OneTimeOnlyCmd("no-new-version", "")
{
}


/*********************************************************************
 * Destructor
 */

NoNewVersionCmd::~NoNewVersionCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool NoNewVersionCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command

  seds->sfic->new_version_flag = NO;

  return true;
}

#else

bool NoNewVersionCmd::doIt() const
{
  if (se_dir(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
