#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "RadarNamesCmd.hh"


/*********************************************************************
 * Constructors
 */

RadarNamesCmd::RadarNamesCmd() :
  OneTimeOnlyCmd("radar-names", "")
{
}


/*********************************************************************
 * Destructor
 */

RadarNamesCmd::~RadarNamesCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool RadarNamesCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command

  seds->num_radars = 0;
  seds->radar_stack.clear();
  seds->sfic->radar_names_text[0] = '\0';

  for (int token_num = 1;
       _cmdTokens[token_num].getTokenType() != UiCommandToken::UTT_END;
       ++token_num)
  {
    seds->radar_stack.push_back(_cmdTokens[token_num].getCommandText());
    seds->num_radars++;
    if (token_num != 1)
      seds->sfic->radar_names_text += " ";
    seds->sfic->radar_names_text += _cmdTokens[token_num].getCommandText();
  }

  return true;
}

#else

bool RadarNamesCmd::doIt() const
{
  if (se_dir(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
