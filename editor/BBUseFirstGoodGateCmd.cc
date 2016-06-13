#ifdef USE_RADX
#else
#include <se_BB_unfold.hh>
#endif

#include <se_utils.hh>

#include "BBUseFirstGoodGateCmd.hh"


/*********************************************************************
 * Constructors
 */

BBUseFirstGoodGateCmd::BBUseFirstGoodGateCmd() :
  OneTimeOnlyCmd("BB-use-first-good-gate", "BB-use-first-good-gate")
{
}


/*********************************************************************
 * Destructor
 */

BBUseFirstGoodGateCmd::~BBUseFirstGoodGateCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool BBUseFirstGoodGateCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->BB_init_on = BB_USE_FGG;

  return true;
}

#else

bool BBUseFirstGoodGateCmd::doIt() const
{
  if (se_BB_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
