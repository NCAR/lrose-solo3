#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "DeglitchThresholdCmd.hh"


/*********************************************************************
 * Constructors
 */

DeglitchThresholdCmd::DeglitchThresholdCmd() :
  OneTimeOnlyCmd("deglitch-threshold", "deglitch-threshold is <real>")
{
}


/*********************************************************************
 * Destructor
 */

DeglitchThresholdCmd::~DeglitchThresholdCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool DeglitchThresholdCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->deglitch_threshold = _cmdTokens[1].getFloatParam();

  return true;
}

#else

bool DeglitchThresholdCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
