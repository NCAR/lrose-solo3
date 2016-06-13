#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "MinNotchShearCmd.hh"


/*********************************************************************
 * Constructors
 */

MinNotchShearCmd::MinNotchShearCmd() :
  OneTimeOnlyCmd("min-notch-shear", "min-notch-shear is <real>")
{
}


/*********************************************************************
 * Destructor
 */

MinNotchShearCmd::~MinNotchShearCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool MinNotchShearCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->notch_shear = _cmdTokens[1].getFloatParam();
  
  return true;
}

#else

bool MinNotchShearCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
