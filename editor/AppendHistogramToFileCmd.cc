#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <se_utils.hh>

#include "AppendHistogramToFileCmd.hh"


/*********************************************************************
 * Constructors
 */

AppendHistogramToFileCmd::AppendHistogramToFileCmd() :
  OneTimeOnlyCmd("append-histogram-to-file", "append-histogram-to-file")
{
}


/*********************************************************************
 * Destructor
 */

AppendHistogramToFileCmd::~AppendHistogramToFileCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool AppendHistogramToFileCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command based on which command was received

  seds->histo_output_key = SE_HST_COPY;

  return true;
}

#else

bool AppendHistogramToFileCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
