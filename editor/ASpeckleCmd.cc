#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "ASpeckleCmd.hh"


/*********************************************************************
 * Constructors
 */

ASpeckleCmd::ASpeckleCmd() :
  OneTimeOnlyCmd("a-speckle", "a-speckle is <integer> gates")
{
}


/*********************************************************************
 * Destructor
 */

ASpeckleCmd::~ASpeckleCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool ASpeckleCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->a_speckle = _cmdTokens[1].getIntParam();

  return true;
}

#else

bool ASpeckleCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
