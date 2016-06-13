#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <se_utils.hh>

#include "CountHistogramCmd.hh"


/*********************************************************************
 * Constructors
 */

CountHistogramCmd::CountHistogramCmd() :
  OneTimeOnlyCmd("count-histogram", "count-histogram on <field>")
{
}


/*********************************************************************
 * Destructor
 */

CountHistogramCmd::~CountHistogramCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool CountHistogramCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command based on which command was received

  seds->histogram_field = _cmdTokens[1].getCommandText();
  seds->num_irreg_bins = 0;

  for (struct se_pairs *hp = seds->h_pairs; hp != 0; )
  {
    // Free irreg pairs if any

    struct se_pairs *hpn = hp->next;
    free(hp);
    hp = hpn;
  }	      

  seds->h_pairs = NULL;
  seds->histo_key = H_BINS;

  return true;
}

#else

bool CountHistogramCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
