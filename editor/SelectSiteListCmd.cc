#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <se_utils.hh>

#include "SelectSiteListCmd.hh"


/*********************************************************************
 * Constructors
 */

SelectSiteListCmd::SelectSiteListCmd() :
  OneTimeOnlyCmd("select-site-list", "")
{
}


/*********************************************************************
 * Destructor
 */

SelectSiteListCmd::~SelectSiteListCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool SelectSiteListCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command based on which command was received

  std::string field_name = _cmdTokens[1].getCommandText();
  struct zmap_points *zmpc;
    
  for (zmpc = seds->top_zmap_list; zmpc != 0; zmpc = zmpc->next_list)
  {
    if (strcmp(zmpc->list_id, field_name.c_str()) == 0)
      break;
  }

  if (zmpc != 0)
  {
    seds->curr_zmap_list = zmpc;
  }
  else
  {
    printf( "zmap-list: %s cannot be found!\n", field_name.c_str());
    return 1;
  }

  return true;
}

#else

bool SelectSiteListCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
