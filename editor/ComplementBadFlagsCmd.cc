#ifdef USE_RADX
#else
#include <se_flag_ops.hh>
#endif

#include <se_utils.hh>

#include "ComplementBadFlagsCmd.hh"


/*********************************************************************
 * Constructors
 */

ComplementBadFlagsCmd::ComplementBadFlagsCmd() :
  ForEachRayCmd("complement-bad-flags", "complement-bad-flags")
{
}


/*********************************************************************
 * Destructor
 */

ComplementBadFlagsCmd::~ComplementBadFlagsCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool ComplementBadFlagsCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Do it

  unsigned short *bad_flag_mask = seds->bad_flag_mask_array;
  int num_cells = seds->max_gates;

  for (int i = 0; i < num_cells; ++i)
  {
    if (bad_flag_mask[i] == 0)
      bad_flag_mask[i] = 1;
    else
      bad_flag_mask[i] = 0;
  }

  return true;
}

#else

bool ComplementBadFlagsCmd::doIt() const
{
  if (se_clear_bad_flags(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
