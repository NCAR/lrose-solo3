#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "NSWindCmd.hh"


/*********************************************************************
 * Constructors
 */

NSWindCmd::NSWindCmd() :
  OneTimeOnlyCmd("ns-wind", "ns-wind is <real>")
{
}


/*********************************************************************
 * Destructor
 */

NSWindCmd::~NSWindCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool NSWindCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  seds->ns_wind = _cmdTokens[1].getFloatParam();

  return true;
}

#else

bool NSWindCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
