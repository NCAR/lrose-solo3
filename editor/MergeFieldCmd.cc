#ifdef USE_RADX
#else
#include <se_add_mult.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "MergeFieldCmd.hh"


/*********************************************************************
 * Constructors
 */

MergeFieldCmd::MergeFieldCmd() :
  ForEachRayCmd("merge-field",
		"merge-field <field> with <field> put-in <field>")
{
}


/*********************************************************************
 * Destructor
 */

MergeFieldCmd::~MergeFieldCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool MergeFieldCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string first_name = _cmdTokens[1].getCommandText();
  std::string second_name = _cmdTokens[2].getCommandText();
  std::string dst_name = _cmdTokens[3].getCommandText();

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  // Find the first field

  RadxField *first_field = ray.getField(first_name);
  
  if (first_field == 0)
  {
    g_string_sprintfa(gs_complaints,
		      "First parameter %s not found for add/sub\n",
		      first_name.c_str());
    seds->punt = YES;
    return false;
  }

  const Radx::fl32 *first_data = first_field->getDataFl32();
  const double first_bad = first_field->getMissingFl32();

  // Find the second field

  RadxField *second_field = ray.getField(second_name);
  
  if (second_field == 0)
  {
    g_string_sprintfa(gs_complaints,
		      "Second parameter %s not found for add/sub\n",
		      second_name.c_str());
    seds->punt = YES;
    return false;
  }

  const Radx::fl32 *second_data = second_field->getDataFl32();
  const double second_bad = second_field->getMissingFl32();

  // See if the destination field already exists.  If it does exist, we will
  // just replace the data with the addition of the source and add fields.
  // If it doesn't exist, we will create a new field and will start with
  // the source field data.

  const std::size_t num_gates = ray.getNGates();
  RadxField *dst_field = ray.getField(dst_name);
  
  Radx::fl32 *dst_data = new Radx::fl32[num_gates];
  double dst_bad;
  
  if (dst_field == 0)
    dst_bad = first_bad;
  else
    dst_bad = dst_field->getMissingFl32();

  // Loop through the data

  unsigned short *boundary_mask = seds->boundary_mask;

  // In this case the first field "first_field"
  // is considered the dominant field over the second field "second_field"
  // and if first_data does not contain
  // a bad flag it is plunked into the destination field dst_data
  // otherwise second_data is plunked into the destination field

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0)
      continue;
    
    if (first_data[i] != first_bad)
      dst_data[i] = first_data[i];
    else if (second_data[i] != second_bad)
      dst_data[i] = second_data[i];
    else
      dst_data[i] = dst_bad;
  }
  
  // Update the data in the ray

  if (dst_field == 0)
    ray.addField(dst_name, first_field->getUnits(), num_gates,
		 dst_bad, dst_data, false);
  else
    dst_field->setDataFl32(num_gates, dst_data, false);
  
  return true;
}

#else

bool MergeFieldCmd::doIt() const
{
  if (se_add_fields(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
