#include <algorithm>

#include <se_utils.hh>

#include "OneTimeOnlyCmd.hh"


/*********************************************************************
 * Constructors
 */

OneTimeOnlyCmd::OneTimeOnlyCmd() :
  UiCommand()
{
}


OneTimeOnlyCmd::OneTimeOnlyCmd(const std::string &keyword,
			       const std::string &cmd_template) :
  UiCommand(keyword, cmd_template)
{
}


/*********************************************************************
 * Destructor
 */

OneTimeOnlyCmd::~OneTimeOnlyCmd()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

