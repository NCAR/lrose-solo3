#ifdef USE_RADX
#else
#include <se_for_each.hh>
#endif

#include <se_utils.hh>

#include "IgnoreFieldCmd.hh"


/*********************************************************************
 * Constructors
 */

IgnoreFieldCmd::IgnoreFieldCmd() :
  ForEachRayCmd("ignore-field", "ignore-field <field>")
{
}


/*********************************************************************
 * Destructor
 */

IgnoreFieldCmd::~IgnoreFieldCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool IgnoreFieldCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string param_name = _cmdTokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Remove the field

  if (ray.removeField(param_name) == 0)
    seds->modified = YES;
  
  return true;
}

#else

bool IgnoreFieldCmd::doIt() const
{
  if (se_remove_field(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
