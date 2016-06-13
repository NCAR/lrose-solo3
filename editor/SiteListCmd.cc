#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <se_bnd.hh>
#include <se_utils.hh>

#include "SiteListCmd.hh"


/*********************************************************************
 * Constructors
 */

SiteListCmd::SiteListCmd() :
  OneTimeOnlyCmd("site-list", ""),
  _gaugeDir("")
{
}


/*********************************************************************
 * Destructor
 */

SiteListCmd::~SiteListCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool SiteListCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command based on which command was received

  if (_gaugeDir == "")
  {
    char *gauge_dir;
    
    if ((gauge_dir = getenv("GAUGES_DIR")) ||
	(gauge_dir = getenv("RAIN_GAUGES_DIR")))
    {
      _gaugeDir = gauge_dir;
      if (_gaugeDir[_gaugeDir.size()-1] != '/')
	_gaugeDir += "/";
    }
    else
    {
      _gaugeDir = "/scr/hotlips/oye/spol/cases/";
    }
  }

  std::string cmd_text =
    se_unquote_string(_cmdTokens[1].getCommandText());

  if (cmd_text == "test")
  {
//    cmd_text = "/scr/hotlips/oye/spol/cases/walnut_gauges";
    cmd_text = "/scr/hotlips/oye/src/spol/misc/precip98_gauges.txt";
  }
  else if (cmd_text.find("/") == std::string::npos)
  {
    // We do not have a full path name
    
    cmd_text = _gaugeDir + cmd_text;
  }

  // code for these routines is in "se_bnd.c"

  absorb_zmap_pts(&seds->top_zmap_list, cmd_text);

  return true;
}

#else

bool SiteListCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
