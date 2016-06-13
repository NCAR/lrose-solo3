#ifdef USE_RADX
#else
#include <se_BB_unfold.hh>
#endif

#include <se_utils.hh>

#include "BBGatesAveragedCmd.hh"


/*********************************************************************
 * Constructors
 */

BBGatesAveragedCmd::BBGatesAveragedCmd() :
  OneTimeOnlyCmd("BB-gates-averaged",
		 "BB-gates-averaged is <integer> gates")
{
}


/*********************************************************************
 * Destructor
 */

BBGatesAveragedCmd::~BBGatesAveragedCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool BBGatesAveragedCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->BB_avg_count = _cmdTokens[1].getIntParam();

  return true;
}

#else

bool BBGatesAveragedCmd::doIt() const
{
  if (se_BB_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
