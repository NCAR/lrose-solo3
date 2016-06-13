#include "RemoveOnlySecondTripSurfaceCmd.hh"


/*********************************************************************
 * Constructors
 */

RemoveOnlySecondTripSurfaceCmd::RemoveOnlySecondTripSurfaceCmd() :
  RemoveSurfaceCmd("remove-only-second-trip-surface",
		   "remove-only-second-trip-surface in <field>")
{
}


/*********************************************************************
 * Destructor
 */

RemoveOnlySecondTripSurfaceCmd::~RemoveOnlySecondTripSurfaceCmd()
{
}


#ifdef USE_RADX

bool RemoveOnlySecondTripSurfaceCmd::doIt(const int frame_num, RadxRay &ray) const
{
  return _removeSurface(frame_num, ray,	_cmdTokens[1].getCommandText(),
			false, true);
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
