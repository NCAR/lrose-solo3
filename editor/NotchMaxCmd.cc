#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "NotchMaxCmd.hh"


/*********************************************************************
 * Constructors
 */

NotchMaxCmd::NotchMaxCmd() :
  OneTimeOnlyCmd("notch-max", "notch-max is <real>")
{
}


/*********************************************************************
 * Destructor
 */

NotchMaxCmd::~NotchMaxCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool NotchMaxCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  seds->notch_max = _cmdTokens[1].getFloatParam();
  
  return true;
}

#else

bool NotchMaxCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
