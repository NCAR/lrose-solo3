#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "FreckleThresholdCmd.hh"


/*********************************************************************
 * Constructors
 */

FreckleThresholdCmd::FreckleThresholdCmd() :
  OneTimeOnlyCmd("freckle-threshold", "freckle-threshold is <real>")
{
}


/*********************************************************************
 * Destructor
 */

FreckleThresholdCmd::~FreckleThresholdCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool FreckleThresholdCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->freckle_threshold = _cmdTokens[1].getFloatParam();

  return true;
}

#else

bool FreckleThresholdCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
