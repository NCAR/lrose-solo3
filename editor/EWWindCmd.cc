#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "EWWindCmd.hh"


/*********************************************************************
 * Constructors
 */

EWWindCmd::EWWindCmd() :
  OneTimeOnlyCmd("ew-wind", "ew-wind is <real>")
{
}


/*********************************************************************
 * Destructor
 */

EWWindCmd::~EWWindCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool EWWindCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->ew_wind = _cmdTokens[1].getFloatParam();

  return true;
}

#else

bool EWWindCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
