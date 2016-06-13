#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <se_utils.hh>

#include "HistogramDirectoryCmd.hh"


/*********************************************************************
 * Constructors
 */

HistogramDirectoryCmd::HistogramDirectoryCmd() :
  OneTimeOnlyCmd("histogram-directory", "histogram-directory <directory>")
{
}


/*********************************************************************
 * Destructor
 */

HistogramDirectoryCmd::~HistogramDirectoryCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool HistogramDirectoryCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command based on which command was received

  seds->histo_directory =
    se_unquote_string(_cmdTokens[1].getCommandText());
  seds->histo_output_key = SE_HST_COPY;

  return true;
}

#else

bool HistogramDirectoryCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
