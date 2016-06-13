#ifdef USE_RADX
#else
#include <se_bnd.hh>
#endif

#include <se_utils.hh>

#include "UseBoundaryCmd.hh"


/*********************************************************************
 * Constructors
 */

UseBoundaryCmd::UseBoundaryCmd() :
  ForEachRayCmd("use-boundary", "use-boundary")
{
}


/*********************************************************************
 * Destructor
 */

UseBoundaryCmd::~UseBoundaryCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool UseBoundaryCmd::doIt(const int frame_num, RadxRay &ray) const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->boundary_mask = seds->boundary_mask_array;
  seds->use_boundary = YES;
  
  return true;
}

#else

bool UseBoundaryCmd::doIt() const
{
  if (se_use_bnd(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
