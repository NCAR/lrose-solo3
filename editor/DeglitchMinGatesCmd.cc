#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "DeglitchMinGatesCmd.hh"


/*********************************************************************
 * Constructors
 */

DeglitchMinGatesCmd::DeglitchMinGatesCmd() :
  OneTimeOnlyCmd("deglitch-min-gates", "deglitch-min-gates is <integer>")
{
}


/*********************************************************************
 * Destructor
 */

DeglitchMinGatesCmd::~DeglitchMinGatesCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool DeglitchMinGatesCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->deglitch_min_bins = _cmdTokens[1].getIntParam();

  return true;
}

#else

bool DeglitchMinGatesCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
