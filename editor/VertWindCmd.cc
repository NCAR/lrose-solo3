#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "VertWindCmd.hh"


/*********************************************************************
 * Constructors
 */

VertWindCmd::VertWindCmd() :
  OneTimeOnlyCmd("vert-wind", "vert-wind is <real>")
{
}


/*********************************************************************
 * Destructor
 */

VertWindCmd::~VertWindCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool VertWindCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  seds->ud_wind = _cmdTokens[1].getFloatParam();
  
  return true;
}

#else

bool VertWindCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
