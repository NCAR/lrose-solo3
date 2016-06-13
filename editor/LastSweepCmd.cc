#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "LastSweepCmd.hh"


/*********************************************************************
 * Constructors
 */

LastSweepCmd::LastSweepCmd() :
  OneTimeOnlyCmd("last-sweep", "last-sweep date-time")
{
}


/*********************************************************************
 * Destructor
 */

LastSweepCmd::~LastSweepCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool LastSweepCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command

  if (_cmdTokens[1].getCommandText().compare("first") == 0)
  {
    seds->sfic->stop_time = DAY_ZERO;   
    seds->sfic->last_sweep_text = "first";
  }
  else if (_cmdTokens[1].getCommandText().compare("last") == 0)
  {
    seds->sfic->stop_time = ETERNITY;   
    seds->sfic->last_sweep_text = "last";
  }
  else
  {
    // Try to interpret this as a number

    DateTime data_time;
      
    seds->sfic->last_sweep_text = _cmdTokens[1].getCommandText();
    data_time.parse2(seds->sfic->last_sweep_text);
    seds->sfic->stop_time = data_time.getTimeStamp();
  }

  return true;
}

#else

bool LastSweepCmd::doIt() const
{
  if (se_dir(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
