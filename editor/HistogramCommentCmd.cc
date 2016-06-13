#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <se_utils.hh>

#include "HistogramCommentCmd.hh"


/*********************************************************************
 * Constructors
 */

HistogramCommentCmd::HistogramCommentCmd() :
  OneTimeOnlyCmd("histogram-comment", "histogram-comment <comment>")
{
}


/*********************************************************************
 * Destructor
 */

HistogramCommentCmd::~HistogramCommentCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool HistogramCommentCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command based on which command was received

  seds->histo_comment =
    se_unquote_string(_cmdTokens[1].getCommandText());
  seds->histo_output_key = SE_HST_COPY;

  return true;
}

#else

bool HistogramCommentCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
