#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <HistogramMgr.hh>
#include <se_utils.hh>

#include "NewHistogramFileCmd.hh"


/*********************************************************************
 * Constructors
 */

NewHistogramFileCmd::NewHistogramFileCmd() :
  OneTimeOnlyCmd("new-histogram-file", "new-histogram-file")
{
}


/*********************************************************************
 * Destructor
 */

NewHistogramFileCmd::~NewHistogramFileCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool NewHistogramFileCmd::doIt() const
{
  HistogramMgr::getInstance()->closeHistogramStream();
  
  struct solo_edit_stuff *seds = return_sed_stuff();
  seds->histo_output_key = SE_HST_COPY;

  return true;
}

#else

bool NewHistogramFileCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
