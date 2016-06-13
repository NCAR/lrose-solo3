#include <math.h>

#ifdef USE_RADX
#else
#include <se_BB_unfold.hh>
#endif

#include <se_utils.hh>

#include "NyquistVelocityCmd.hh"


/*********************************************************************
 * Constructors
 */

NyquistVelocityCmd::NyquistVelocityCmd() :
  OneTimeOnlyCmd("nyquist-velocity", "nyquist-velocity is <real>")
{
}


/*********************************************************************
 * Destructor
 */

NyquistVelocityCmd::~NyquistVelocityCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool NyquistVelocityCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  seds->nyquist_velocity = fabs(_cmdTokens[1].getFloatParam());

  return true;
}

#else

bool NyquistVelocityCmd::doIt() const
{
  if (se_BB_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
