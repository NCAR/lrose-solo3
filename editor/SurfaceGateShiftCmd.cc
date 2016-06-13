#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "SurfaceGateShiftCmd.hh"


/*********************************************************************
 * Constructors
 */

SurfaceGateShiftCmd::SurfaceGateShiftCmd() :
  OneTimeOnlyCmd("surface-gate-shift", "surface-gate-shift is <integer> gates")
{
}


/*********************************************************************
 * Destructor
 */

SurfaceGateShiftCmd::~SurfaceGateShiftCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool SurfaceGateShiftCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  seds->surface_gate_shift = _cmdTokens[1].getIntParam();

  return true;
}

#else

bool SurfaceGateShiftCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
