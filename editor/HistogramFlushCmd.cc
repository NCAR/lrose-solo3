#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <se_utils.hh>

#include "HistogramFlushCmd.hh"


/*********************************************************************
 * Constructors
 */

HistogramFlushCmd::HistogramFlushCmd() :
  OneTimeOnlyCmd("histogram-flush", "histogram-flush")
{
}


/*********************************************************************
 * Destructor
 */

HistogramFlushCmd::~HistogramFlushCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool HistogramFlushCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command

  seds->histo_flush = YES;

  return true;
}

#else

bool HistogramFlushCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
