#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "OffsetForRadialShearCmd.hh"


/*********************************************************************
 * Constructors
 */

OffsetForRadialShearCmd::OffsetForRadialShearCmd() :
  OneTimeOnlyCmd("offset-for-radial-shear",
		 "offset-for-radial-shear is <integer> gates")
{
}


/*********************************************************************
 * Destructor
 */

OffsetForRadialShearCmd::~OffsetForRadialShearCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool OffsetForRadialShearCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  seds->gate_diff_interval = _cmdTokens[1].getIntParam();
    
  return true;
}

#else

bool OffsetForRadialShearCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
