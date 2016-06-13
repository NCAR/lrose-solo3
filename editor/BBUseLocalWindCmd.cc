#ifdef USE_RADX
#else
#include <se_BB_unfold.hh>
#endif

#include <se_utils.hh>

#include "BBUseLocalWindCmd.hh"


/*********************************************************************
 * Constructors
 */

BBUseLocalWindCmd::BBUseLocalWindCmd() :
  OneTimeOnlyCmd("BB-use-local-wind", "BB-use-local-wind")
{
}


/*********************************************************************
 * Destructor
 */

BBUseLocalWindCmd::~BBUseLocalWindCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool BBUseLocalWindCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->BB_init_on = BB_USE_LOCAL_WIND;

  return true;
}

#else

bool BBUseLocalWindCmd::doIt() const
{
  if (se_BB_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
