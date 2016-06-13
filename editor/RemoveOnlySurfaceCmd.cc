#include "RemoveOnlySurfaceCmd.hh"


/*********************************************************************
 * Constructors
 */

RemoveOnlySurfaceCmd::RemoveOnlySurfaceCmd() :
  RemoveSurfaceCmd("remove-only-surface",
		   "remove-only-surface in <field>")
{
}


/*********************************************************************
 * Destructor
 */

RemoveOnlySurfaceCmd::~RemoveOnlySurfaceCmd()
{
}


#ifdef USE_RADX

bool RemoveOnlySurfaceCmd::doIt(const int frame_num, RadxRay &ray) const
{
  return _removeSurface(frame_num, ray,	_cmdTokens[1].getCommandText(),
			true, false);
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
