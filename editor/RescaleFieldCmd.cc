#ifdef USE_RADX
#else
#include <se_defrkl.hh>
#endif

#include "RescaleFieldCmd.hh"


/*********************************************************************
 * Constructors
 */

RescaleFieldCmd::RescaleFieldCmd() :
  ForEachRayCmd("rescale-field",
		"rescale-field <field> <real> <real> scale and bias")
{
}


/*********************************************************************
 * Destructor
 */

RescaleFieldCmd::~RescaleFieldCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool RescaleFieldCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Do nothing since this command doesn't make sense for Radx data which
  // is dynamically scaled
  
  return true;
}

#else

bool RescaleFieldCmd::doIt() const
{
  if (se_rescale_field(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
