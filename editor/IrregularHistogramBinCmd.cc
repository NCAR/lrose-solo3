#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <se_utils.hh>

#include "IrregularHistogramBinCmd.hh"


/*********************************************************************
 * Constructors
 */

IrregularHistogramBinCmd::IrregularHistogramBinCmd() :
  OneTimeOnlyCmd("irregular-histogram-bin",
		 "irregular-histogram-bin from <real> to <real>")
{
}


/*********************************************************************
 * Destructor
 */

IrregularHistogramBinCmd::~IrregularHistogramBinCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool IrregularHistogramBinCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command

  seds->histo_key |= H_IRREG;
  seds->num_irreg_bins++;

  struct se_pairs *hp = (struct se_pairs *)malloc(sizeof(struct se_pairs));
  memset(hp, 0, sizeof(struct se_pairs));

  if (!seds->h_pairs)
    seds->h_pairs = hp;
  else
    seds->h_pairs->last->next = hp;

  seds->h_pairs->last = hp;
  hp->f_low = _cmdTokens[1].getFloatParam();
  hp->f_high = _cmdTokens[2].getFloatParam();

  return true;
}

#else

bool IrregularHistogramBinCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
