#ifdef USE_RADX
#else
#include <se_add_mult.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "FlaggedAssignCmd.hh"


/*********************************************************************
 * Constructors
 */

FlaggedAssignCmd::FlaggedAssignCmd() :
  ForEachRayCmd("flagged-assign", "flagged-assign of <real> in <field>")
{
}


/*********************************************************************
 * Destructor
 */

FlaggedAssignCmd::~FlaggedAssignCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool FlaggedAssignCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  float new_value = _cmdTokens[1].getFloatParam();
  std::string param_name = _cmdTokens[2].getCommandText();

  struct solo_edit_stuff *seds = return_sed_stuff();

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  seds->bad_flag_mask = seds->bad_flag_mask_array;

  // Find the field 

  RadxField *field = ray.getField(param_name);
  
  if (field == 0)
  {
    // Field not found

    g_string_sprintfa(gs_complaints, "Field to be assigned: %s not found\n",
		      param_name.c_str());
    seds->punt = YES;
    return false;
  }

  seds->modified = YES;

  const std::size_t num_gates = ray.getNGates();
  
  const Radx::fl32 *orig_data = field->getDataFl32();
  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  
  unsigned short *boundary_mask = seds->boundary_mask;
  unsigned short *bad_flag_mask = seds->bad_flag_mask_array;

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0 || bad_flag_mask[i] == 0)
      continue;
    
    data[i] = new_value;
  }
  
  // Update the ray data

  field->setDataFl32(num_gates, data, false);
  
  return true;
}

#else

bool FlaggedAssignCmd::doIt() const
{
  if (se_assign_value(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
