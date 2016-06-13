#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <se_utils.hh>

#include "DontAppendHistogramToFileCmd.hh"


/*********************************************************************
 * Constructors
 */

DontAppendHistogramToFileCmd::DontAppendHistogramToFileCmd() :
  OneTimeOnlyCmd("dont-append-histogram-to-file",
		 "dont-append-histogram-to-file")
{
}


/*********************************************************************
 * Destructor
 */

DontAppendHistogramToFileCmd::~DontAppendHistogramToFileCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool DontAppendHistogramToFileCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->histo_output_key = SE_HST_NOCOPY;

  return true;
}

#else

bool DontAppendHistogramToFileCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
