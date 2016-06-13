#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "GatesShiftedCmd.hh"


/*********************************************************************
 * Constructors
 */

GatesShiftedCmd::GatesShiftedCmd() :
  OneTimeOnlyCmd("gates-shifted", "gates-shifted is <integer>")
{
}


/*********************************************************************
 * Destructor
 */

GatesShiftedCmd::~GatesShiftedCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool GatesShiftedCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->gates_of_shift = _cmdTokens[1].getIntParam();
  
  return true;
}

#else

bool GatesShiftedCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
