#ifdef USE_RADX
#else
#include <se_add_mult.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "AssignValueCmd.hh"


/*********************************************************************
 * Constructors
 */

AssignValueCmd::AssignValueCmd() :
  ForEachRayCmd("assign-value", "assign-value <real> to <field>")
{
}


/*********************************************************************
 * Destructor
 */

AssignValueCmd::~AssignValueCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool AssignValueCmd::doIt(const int frame_num, RadxRay &ray) const
{
//  std::cerr << "*** Entering AssignValueCmd::doIt(" << frame_num << ")" << std::endl;
  
  // Pull the arguments from the command

  float new_value = _cmdTokens[1].getFloatParam();
  std::string param_name = _cmdTokens[2].getCommandText();

//  std::cerr << "    new_value = " << new_value << std::endl;
//  std::cerr << "    param_name = " << param_name << std::endl;
  
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  // I don't know what this is for, but it was in the original code so
  // I just copied it for now.

  seds->bad_flag_mask = seds->bad_flag_mask_array;

  // Find the field 

  RadxField *field = ray.getField(param_name);
  
  if (field == 0)
  {
    // Field not found

    g_string_sprintfa(gs_complaints, "Field to be assigned: %s not found\n",
		      param_name.c_str());
    seds->punt = 1;
    return false;
  }

  // Make sure we know that this field has been modified and needs to be
  // written to disk.

  seds->modified = 1;

  // Access the data in the ray

  std::size_t num_gates = ray.getNGates();
  const Radx::fl32 *orig_data = field->getDataFl32();
  
  // Copy the data to a local buffer that we can update

  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  
  // Get a pointer to the boundary mask

  unsigned short *boundary_mask = seds->boundary_mask;

  // Update the data buffer

//  std::cerr << "    Updating data buffer, num_gates = " << num_gates << std::endl;
  
  for (std::size_t i = 0; i < num_gates; ++i)
  {
    // Don't modify data outside of the boundary mask

    if (boundary_mask[i] == 0)
      continue;
    
//      std::cerr << "     Assigning data value: " << new_value << std::endl;
    data[i] = new_value;
  }
  
  // Update the ray data

  // NOTE:  I'm not sure about the isLocal flag.  Would it be better to use
  // true and then free the data buffer after this call?

  field->setDataFl32(num_gates, data, false);
  
  return true;
}

#else

bool AssignValueCmd::doIt() const
{
  if (se_assign_value(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
