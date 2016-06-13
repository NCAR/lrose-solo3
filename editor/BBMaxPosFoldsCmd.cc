#ifdef USE_RADX
#else
#include <se_BB_unfold.hh>
#endif

#include <se_utils.hh>

#include "BBMaxPosFoldsCmd.hh"


/*********************************************************************
 * Constructors
 */

BBMaxPosFoldsCmd::BBMaxPosFoldsCmd() :
  OneTimeOnlyCmd("BB-max-pos-folds", "BB-max-pos-folds is <integer>")
{
}


/*********************************************************************
 * Destructor
 */

BBMaxPosFoldsCmd::~BBMaxPosFoldsCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool BBMaxPosFoldsCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->BB_max_pos_folds = _cmdTokens[1].getIntParam();

  return true;
}

#else

bool BBMaxPosFoldsCmd::doIt() const
{
  if (se_BB_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
