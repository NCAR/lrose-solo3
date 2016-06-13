#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "FirstGoodGateCmd.hh"


/*********************************************************************
 * Constructors
 */

FirstGoodGateCmd::FirstGoodGateCmd() :
  OneTimeOnlyCmd("first-good-gate", "first-good-gate is <integer>")
{
}


/*********************************************************************
 * Destructor
 */

FirstGoodGateCmd::~FirstGoodGateCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool FirstGoodGateCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->first_good_gate = _cmdTokens[1].getIntParam();

  return true;
}

#else

bool FirstGoodGateCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
