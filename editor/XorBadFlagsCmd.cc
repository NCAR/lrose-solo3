#ifdef USE_RADX
#else
#include <se_flag_ops.hh>
#endif

#include <se_utils.hh>

#include "XorBadFlagsCmd.hh"


/*********************************************************************
 * Constructors
 */

XorBadFlagsCmd::XorBadFlagsCmd() :
  ForEachRayCmd("xor-bad-flags",
		"xor-bad-flags with <field> <where> <real>")
{
  setNumExtraTokens(1);
}


/*********************************************************************
 * Destructor
 */

XorBadFlagsCmd::~XorBadFlagsCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool XorBadFlagsCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments out of the command

  std::string param_name = _cmdTokens[1].getCommandText();
  std::string where = _cmdTokens[2].getCommandText();
  double thresh1 = _cmdTokens[3].getFloatParam();
  double thresh2 = 0.0;
  if (_cmdTokens[4].getTokenType() != UiCommandToken::UTT_END)
    thresh2 = _cmdTokens[4].getFloatParam();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;
  seds->bad_flag_mask = seds->bad_flag_mask_array;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  RadxField *field = ray.getField(param_name);
  
  if (field == 0)
  {
    // Field not found

    printf("Bad flag logic field: %s not found\n", param_name.c_str());
    seds->punt = YES;
    return false;
  }

  std::size_t num_gates = ray.getNGates();

  const Radx::fl32 *data = field->getDataFl32();
  double bad = field->getMissingFl32();
  
  unsigned short *boundary_mask = seds->boundary_mask;
  unsigned short *bad_flag_mask = seds->bad_flag_mask_array;

  // Loop through the data

  if (where.compare(0, 3, "below", 0, 3) == 0)
  {
    for (std::size_t i = 0; i < num_gates; ++i)
    {
      if (boundary_mask[i] == 0)
	continue;

      if (data[i] == bad)
	continue;
      
      bad_flag_mask[i] ^= data[i] < thresh1;
    }
  }
  else if (where.compare(0, 3, "above", 0, 3) == 0)
  {
    for (std::size_t i = 0; i < num_gates; ++i)
    {
      if (boundary_mask[i] == 0)
	continue;

      if (data[i] == bad)
	continue;
      
      bad_flag_mask[i] ^= data[i] > thresh1;
    }
  }
  else
  {
    // between

    for (std::size_t i = 0; i < num_gates; ++i)
    {
      if (boundary_mask[i] == 0)
	continue;
      
      if (data[i] == bad)
	continue;

      bad_flag_mask[i] ^= data[i] >= thresh1 && data[i] <= thresh2;
    }
  }

  return true;
}

#else

bool XorBadFlagsCmd::doIt() const
{
  if (se_bad_flags_logic(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
