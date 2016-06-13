#ifdef USE_RADX
#else
#include <se_for_each.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "DuplicateCmd.hh"


/*********************************************************************
 * Constructors
 */

DuplicateCmd::DuplicateCmd() :
  ForEachRayCmd("duplicate", "duplicate <field> in <field>")
{
}


/*********************************************************************
 * Destructor
 */

DuplicateCmd::~DuplicateCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool DuplicateCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments out of the command

  std::string src_name = _cmdTokens[1].getCommandText();
  std::string dst_name = _cmdTokens[2].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = 1;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  // Find the source field

  RadxField *src_field = ray.getField(src_name);
  
  if (src_field == 0)
  {
    g_string_sprintfa(gs_complaints,
		      "Source parameter %s not found for establish field\n",
		      src_name.c_str());
    seds->punt = 1;
    return false;
  }

  // Update the data in the ray

  std::size_t num_gates = ray.getNGates();

  RadxField *dst_field = ray.getField(dst_name);
  if (dst_field == 0)
  {
    ray.addField(dst_name, src_field->getUnits(), num_gates,
		 src_field->getMissingFl32(), src_field->getDataFl32(), true);
  }
  else
  {
    dst_field->setMissingFl32(src_field->getMissingFl32());
    dst_field->setDataFl32(num_gates, src_field->getDataFl32(), true);
  }
  
  return true;
}

#else

bool DuplicateCmd::doIt() const
{
  if (se_cpy_field(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
