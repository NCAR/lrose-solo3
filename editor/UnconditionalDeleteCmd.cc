#ifdef USE_RADX
#else
#include <se_for_each.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "UnconditionalDeleteCmd.hh"


/*********************************************************************
 * Constructors
 */

UnconditionalDeleteCmd::UnconditionalDeleteCmd() :
  ForEachRayCmd("unconditional-delete",
		"unconditional-delete in <field>")
{
}


/*********************************************************************
 * Destructor
 */

UnconditionalDeleteCmd::~UnconditionalDeleteCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool UnconditionalDeleteCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguements from the command

  std::string dst_name = _cmdTokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  if (!seds->boundary_exists)
    return 1;

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  RadxField *field = ray.getField(dst_name);
  
  if (field == 0)
  {
    // Field not found

    g_string_sprintfa(gs_complaints, "Field to be deleted: %s not found\n",
		      dst_name.c_str());
    seds->punt = YES;
    return false;
  }

  const std::size_t num_gates = ray.getNGates();
  const Radx::fl32 *orig_data = field->getDataFl32();
  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  const double bad = field->getMissingFl32();
  
  unsigned short *boundary_mask = seds->boundary_mask;

  // Loop through the data

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0)
      continue;
    
    data[i] = bad;
  }
  
  // Update the ray data

  field->setDataFl32(num_gates, data, false);
  
  return true;
}

#else

bool UnconditionalDeleteCmd::doIt() const
{
  if (se_hard_zap(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
