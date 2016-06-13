#ifdef USE_RADX
#else
#include <se_add_mult.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "AddFieldCmd.hh"


/*********************************************************************
 * Constructors
 */

AddFieldCmd::AddFieldCmd() :
  ForEachRayCmd("add-field", "add-field <field> to <field> put-in <field>")
{
}


/*********************************************************************
 * Destructor
 */

AddFieldCmd::~AddFieldCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool AddFieldCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string add_name = _cmdTokens[1].getCommandText();
  std::string src_name = _cmdTokens[2].getCommandText();
  std::string dst_name = _cmdTokens[3].getCommandText();

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = 1;

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  // Find the add field

  RadxField *add_field = ray.getField(add_name);
  
  if (add_field == 0)
  {
    g_string_sprintfa(gs_complaints,
		      "First parameter %s not found for add/sub\n",
		      add_name.c_str());
    seds->punt = 1;
    return false;
  }

  const Radx::fl32 *add_data = add_field->getDataFl32();
  double add_bad = add_field->getMissingFl32();

  // Find the source field

  RadxField *src_field = ray.getField(src_name);
  
  if (src_field == 0)
  {
    g_string_sprintfa(gs_complaints,
		      "Second parameter %s not found for add/sub\n",
		      src_name.c_str());
    seds->punt = 1;
    return false;
  }

  const Radx::fl32 *src_data = src_field->getDataFl32();
  double src_bad = src_field->getMissingFl32();

  // See if the destination field already exists.  If it does exist, we will
  // just replace the data with the addition of the source and add fields.
  // If it doesn't exist, we will create a new field and will start with
  // the source field data.

  std::size_t num_gates = ray.getNGates();
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

  // Loop through the data

  unsigned short *boundary_mask = seds->boundary_mask;

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    // Don't do anything if we aren't inside the boundary

    if (boundary_mask[i] == 0)
      continue;
    
    // If both fields are missing, don't change the destination value

    if (src_data[i] == src_bad && add_data[i] == add_bad)
      continue;
    
    // Set the value based on the data that we have

    if (src_data[i] == src_bad)
      dst_data[i] = add_data[i];
    else if(add_data[i] == add_bad)
      dst_data[i] = src_data[i];
    else
      dst_data[i] = src_data[i] + add_data[i];
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

bool AddFieldCmd::doIt() const
{
  if (se_add_fields(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
