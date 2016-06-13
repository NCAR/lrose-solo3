#include <algorithm>

#include <se_utils.hh>

#include "ForEachRayCmd.hh"


/*********************************************************************
 * Constructors
 */

ForEachRayCmd::ForEachRayCmd() :
  UiCommand()
{
}


ForEachRayCmd::ForEachRayCmd(const std::string &keyword,
			     const std::string &cmd_template) :
  UiCommand(keyword, cmd_template)
{
}


/*********************************************************************
 * Destructor
 */

ForEachRayCmd::~ForEachRayCmd()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

