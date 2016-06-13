#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <se_utils.hh>

#include "RegularHistogramParametersCmd.hh"


/*********************************************************************
 * Constructors
 */

RegularHistogramParametersCmd::RegularHistogramParametersCmd() :
  OneTimeOnlyCmd("regular-histogram-parameters",
		 "regular-histogram-parameters low <real> high <real> increment <real>")
{
}


/*********************************************************************
 * Destructor
 */

RegularHistogramParametersCmd::~RegularHistogramParametersCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool RegularHistogramParametersCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command based on which command was received

  seds->histo_key |= H_REG;
  seds->histo_low = _cmdTokens[1].getFloatParam();
  seds->histo_high = _cmdTokens[2].getFloatParam();
  seds->histo_increment = _cmdTokens[3].getFloatParam();

  return true;
}

#else

bool RegularHistogramParametersCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
