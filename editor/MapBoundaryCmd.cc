#ifdef USE_RADX
#else
#include <se_histog.hh>
#endif

#include <se_bnd.hh>
#include <se_utils.hh>

#include "MapBoundaryCmd.hh"


/*********************************************************************
 * Constructors
 */

MapBoundaryCmd::MapBoundaryCmd() :
  OneTimeOnlyCmd("map-boundary", "")
{
}


/*********************************************************************
 * Destructor
 */

MapBoundaryCmd::~MapBoundaryCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool MapBoundaryCmd::doIt() const
{
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  // Process the command

  int skip = 1;

  std::string cmd_text =
    se_unquote_string(_cmdTokens[1].getCommandText());

  if (cmd_text == "test")
  {
    cmd_text = "/scr/hotlips/oye/spol/cases/bnd.square";
    cmd_text = "/scr/hotlips/oye/src/spol/misc/tfb.txt";
  }
  else if (cmd_text == "cases")
  {
    cmd_text = "/scr/hotlips/oye/spol/cases/walnut10";
    skip = 5;
  }

  int size;
  char *buf = absorb_zmap_bnd(cmd_text, skip, size);
  if (buf != 0)
  {
    se_unpack_bnd_buf(buf, size);
    free(buf);
  }
  
  return true;
}

#else

bool MapBoundaryCmd::doIt() const
{
  if (se_histog_setup(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
