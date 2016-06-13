#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>

#include "FirstSweepCmd.hh"


/*********************************************************************
 * Constructors
 */

FirstSweepCmd::FirstSweepCmd() :
  OneTimeOnlyCmd("first-sweep", "first-sweep date-time")
{
}


/*********************************************************************
 * Destructor
 */

FirstSweepCmd::~FirstSweepCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool FirstSweepCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command

  if (_cmdTokens[1].getCommandText().compare("first") == 0)
  {
    seds->sfic->start_time = DAY_ZERO;   
    seds->sfic->first_sweep_text = "first";
  }
  else if (_cmdTokens[1].getCommandText().compare("last") == 0)
  {
    seds->sfic->start_time = ETERNITY;   
    seds->sfic->first_sweep_text = "last";
  }
  else
  {
    // Try to interpret this as a date of the form mm/dd/yy:hh:mm:ss.ms

    DateTime data_time;
      
    seds->sfic->first_sweep_text = _cmdTokens[1].getCommandText();
    data_time.parse(seds->sfic->first_sweep_text);
    seds->sfic->start_time = data_time.getTimeStamp();
  }

  return true;
}

#else

bool FirstSweepCmd::doIt() const
{
  if (se_dir(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
