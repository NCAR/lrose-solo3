#ifdef USE_RADX
#else
#include <se_for_each.hh>
#endif

#include <se_utils.hh>

#include "RewriteCmd.hh"


/*********************************************************************
 * Constructors
 */

RewriteCmd::RewriteCmd() :
  ForEachRayCmd("rewrite", "rewrite")
{
}


/*********************************************************************
 * Destructor
 */

RewriteCmd::~RewriteCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool RewriteCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;
  
  return true;
}

#else

bool RewriteCmd::doIt() const
{
  if (se_rewrite(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
