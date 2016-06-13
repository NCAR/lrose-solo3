#ifdef USE_RADX
#else
#include <se_catch_all.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "OmitSourceFileInfoCmd.hh"


/*********************************************************************
 * Constructors
 */

OmitSourceFileInfoCmd::OmitSourceFileInfoCmd() :
  OneTimeOnlyCmd("omit-source-file-info", "")
{
}


/*********************************************************************
 * Destructor
 */

OmitSourceFileInfoCmd::~OmitSourceFileInfoCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool OmitSourceFileInfoCmd::doIt() const
{
  struct solo_edit_stuff *seds = return_sed_stuff();
  struct sed_command_files *scf = se_return_cmd_files_struct();

  scf->omit_source_file_info = !scf->omit_source_file_info;
  g_string_sprintfa(gs_complaints, "Source file info will be %s\n",
		    scf->omit_source_file_info ? "IGNORED" : "USED");
    
  return true;
}

#else

bool OmitSourceFileInfoCmd::doIt() const
{
  if (se_once_only(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
