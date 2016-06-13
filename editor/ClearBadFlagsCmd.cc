#ifdef USE_RADX
#else
#include <se_flag_ops.hh>
#endif

#include <se_flag_ops.hh>
#include <se_utils.hh>

#include "ClearBadFlagsCmd.hh"


/*********************************************************************
 * Constructors
 */

ClearBadFlagsCmd::ClearBadFlagsCmd() :
  ForEachRayCmd("clear-bad-flags", "clear-bad-flags")
{
}


/*********************************************************************
 * Destructor
 */

ClearBadFlagsCmd::~ClearBadFlagsCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool ClearBadFlagsCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Do it

  se_do_clear_bad_flags_array(0);

  return true;
}

#else

bool ClearBadFlagsCmd::doIt() const
{
  if (se_clear_bad_flags(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
