#ifdef USE_RADX
#else
#include <se_BB_unfold.hh>
#endif

#include <se_utils.hh>

#include "BBMaxNegFoldsCmd.hh"


/*********************************************************************
 * Constructors
 */

BBMaxNegFoldsCmd::BBMaxNegFoldsCmd() :
  OneTimeOnlyCmd("BB-max-neg-folds", "BB-max-neg-folds is <integer>")
{
}


/*********************************************************************
 * Destructor
 */

BBMaxNegFoldsCmd::~BBMaxNegFoldsCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool BBMaxNegFoldsCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->BB_max_neg_folds = _cmdTokens[1].getIntParam();

  return true;
}

#else

bool BBMaxNegFoldsCmd::doIt() const
{
  if (se_BB_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
