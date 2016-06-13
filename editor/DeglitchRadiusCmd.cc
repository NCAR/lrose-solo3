#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "DeglitchRadiusCmd.hh"


/*********************************************************************
 * Constructors
 */

DeglitchRadiusCmd::DeglitchRadiusCmd() :
  OneTimeOnlyCmd("deglitch-radius", "deglitch-radius is <integer>")
{
}


/*********************************************************************
 * Destructor
 */

DeglitchRadiusCmd::~DeglitchRadiusCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool DeglitchRadiusCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->deglitch_radius = _cmdTokens[1].getIntParam();

  return true;
}

#else

bool DeglitchRadiusCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
