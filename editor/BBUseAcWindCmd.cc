#ifdef USE_RADX
#else
#include <se_BB_unfold.hh>
#endif

#include <se_utils.hh>

#include "BBUseAcWindCmd.hh"


/*********************************************************************
 * Constructors
 */

BBUseAcWindCmd::BBUseAcWindCmd() :
  OneTimeOnlyCmd("BB-use-ac-wind", "BB-use-ac-wind")
{
}


/*********************************************************************
 * Destructor
 */

BBUseAcWindCmd::~BBUseAcWindCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool BBUseAcWindCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->BB_init_on = BB_USE_AC_WIND;

  return true;
}

#else

bool BBUseAcWindCmd::doIt() const
{
  if (se_BB_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
