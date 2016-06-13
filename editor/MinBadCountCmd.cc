#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "MinBadCountCmd.hh"


/*********************************************************************
 * Constructors
 */

MinBadCountCmd::MinBadCountCmd() :
  OneTimeOnlyCmd("min-bad-count", "min-bad-count is <integer> gates")
{
}


/*********************************************************************
 * Destructor
 */

MinBadCountCmd::~MinBadCountCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool MinBadCountCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->min_bad_count = _cmdTokens[1].getIntParam();
  
  return true;
}

#else

bool MinBadCountCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
