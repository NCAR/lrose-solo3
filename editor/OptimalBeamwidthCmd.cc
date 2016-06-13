#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "OptimalBeamwidthCmd.hh"


/*********************************************************************
 * Constructors
 */

OptimalBeamwidthCmd::OptimalBeamwidthCmd() :
  OneTimeOnlyCmd("optimal-beamwidth", "optimal-beamwidth is <real> degrees")
{
}


/*********************************************************************
 * Destructor
 */

OptimalBeamwidthCmd::~OptimalBeamwidthCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool OptimalBeamwidthCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  seds->optimal_beamwidth = _cmdTokens[1].getFloatParam();
    
  return true;
}

#else

bool OptimalBeamwidthCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
