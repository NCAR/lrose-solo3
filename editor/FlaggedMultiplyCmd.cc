#ifdef USE_RADX
#else
#include <se_flag_ops.hh>
#endif

#include <se_utils.hh>

#include "FlaggedMultiplyCmd.hh"


/*********************************************************************
 * Constructors
 */

FlaggedMultiplyCmd::FlaggedMultiplyCmd() :
  ForEachRayCmd("flagged-multiply",
		"flagged-multiply by <real> in <field>")
{
}


/*********************************************************************
 * Destructor
 */

FlaggedMultiplyCmd::~FlaggedMultiplyCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool FlaggedMultiplyCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  float multiplier = _cmdTokens[1].getFloatParam();
  std::string param_name = _cmdTokens[2].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->bad_flag_mask = seds->bad_flag_mask_array;
  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  RadxField *field = ray.getField(param_name);
  
  if (field == 0)
  {
    // Field not found

    printf("Flagged add field: %s not found\n", param_name.c_str());
    seds->punt = YES;
    return false;
  }

  size_t num_gates = ray.getNGates();

  const Radx::fl32 *orig_data = field->getDataFl32();
  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  
  double bad = field->getMissingFl32();
  
  unsigned short *boundary_mask = seds->boundary_mask;
  unsigned short *bad_flag_mask = seds->bad_flag_mask_array;

  // Loop through the data

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0 || bad_flag_mask == 0 || data[i] == bad)
      continue;
    
    data[i] *= multiplier;
  }
  
  // Update the ray data

  field->setDataFl32(num_gates, data, false);
  
  return true;
}

#else

bool FlaggedMultiplyCmd::doIt() const
{
  if (se_flagged_add(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
