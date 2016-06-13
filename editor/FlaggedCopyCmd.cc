#ifdef USE_RADX
#else
#include <se_for_each.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "FlaggedCopyCmd.hh"


/*********************************************************************
 * Constructors
 */

FlaggedCopyCmd::FlaggedCopyCmd() :
  ForEachRayCmd("flagged-copy", "flagged-copy <field> to <field>")
{
}


/*********************************************************************
 * Destructor
 */

FlaggedCopyCmd::~FlaggedCopyCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool FlaggedCopyCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments out of the command

  std::string src_name = _cmdTokens[1].getCommandText();
  std::string dst_name = _cmdTokens[2].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  // Find the source field

  RadxField *src_field = ray.getField(src_name);
  
  if (src_field == 0)
  {
    g_string_sprintfa(gs_complaints,
		      "Source parameter %s not found for copy\n",
		      src_name.c_str());
    seds->punt = YES;
    return false;
  }

  const Radx::fl32 *src_data = src_field->getDataFl32();
  double src_bad = src_field->getMissingFl32();

  // See if the destination field already exists.  If it does exist, we will
  // just replace the data with the addition of the source field and the value.
  // If it doesn't exist, we will create a new field and will start with
  // the source field data.

  const std::size_t num_gates = ray.getNGates();
  RadxField *dst_field = ray.getField(dst_name);
  
  Radx::fl32 *dst_data;
  double dst_bad;
  
  if (dst_field == 0)
  {
    dst_data = new Radx::fl32[num_gates];
    memcpy(dst_data, src_data, num_gates * sizeof(Radx::fl32));
    dst_bad = src_bad;
  }
  else
  {
    const Radx::fl32 *orig_dst_data = dst_field->getDataFl32();
    dst_data = new Radx::fl32[num_gates];
    memcpy(dst_data, orig_dst_data, num_gates * sizeof(Radx::fl32));
    dst_bad = dst_field->getMissingFl32();
  }

  unsigned short *boundary_mask = seds->boundary_mask;

  // Copy using flags and boundary

  seds->bad_flag_mask = seds->bad_flag_mask_array;
  unsigned short *bad_flag_mask = seds->bad_flag_mask_array;
      
  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0 || bad_flag_mask[i] == 0)
      continue;
    
    // Copies everything including missing data

    if (src_data[i] == src_bad)
      dst_data[i] = dst_bad;
    else
      dst_data[i] = src_data[i];
  }
  
  // Update the data in the ray

  if (dst_field == 0)
    ray.addField(dst_name, src_field->getUnits(), num_gates,
		 dst_bad, dst_data, false);
  else
    dst_field->setDataFl32(num_gates, dst_data, false);
  
  return true;
}

#else

bool FlaggedCopyCmd::doIt() const
{
  if (se_cpy_field(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
