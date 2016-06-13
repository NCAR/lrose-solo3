#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "NewVersionCmd.hh"


/*********************************************************************
 * Constructors
 */

NewVersionCmd::NewVersionCmd() :
  OneTimeOnlyCmd("new-version", "new-version")
{
}


/*********************************************************************
 * Destructor
 */

NewVersionCmd::~NewVersionCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool NewVersionCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command

  seds->sfic->new_version_flag = YES;

  return true;
}

#else

bool NewVersionCmd::doIt() const
{
  if (se_dir(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
