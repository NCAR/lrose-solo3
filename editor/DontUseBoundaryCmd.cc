#ifdef USE_RADX
#else
#include <se_bnd.hh>
#endif

#include <se_utils.hh>

#include "DontUseBoundaryCmd.hh"


/*********************************************************************
 * Constructors
 */

DontUseBoundaryCmd::DontUseBoundaryCmd() :
  ForEachRayCmd("dont-use-boundary", "dont-use-boundary")
{
}


/*********************************************************************
 * Destructor
 */

DontUseBoundaryCmd::~DontUseBoundaryCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool DontUseBoundaryCmd::doIt(const int frame_num, RadxRay &ray) const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->use_boundary = 0;
  seds->boundary_mask = seds->all_ones_array;

  return true;
}

#else

bool DontUseBoundaryCmd::doIt() const
{
  if (se_use_bnd(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
