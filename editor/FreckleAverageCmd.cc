#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "FreckleAverageCmd.hh"


/*********************************************************************
 * Constructors
 */

FreckleAverageCmd::FreckleAverageCmd() :
  OneTimeOnlyCmd("freckle-average", "freckle-average is <integer> gates")
{
}


/*********************************************************************
 * Destructor
 */

FreckleAverageCmd::~FreckleAverageCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool FreckleAverageCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->freckle_avg_count = _cmdTokens[1].getIntParam();
  
  return true;
}

#else

bool FreckleAverageCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
