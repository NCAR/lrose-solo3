#ifdef USE_RADX
#else
#include <se_flag_ops.hh>
#endif

#include <se_flag_ops.hh>
#include <se_utils.hh>

#include "SetBadFlagsCmd.hh"


/*********************************************************************
 * Constructors
 */

SetBadFlagsCmd::SetBadFlagsCmd() :
  ForEachRayCmd("set-bad-flags",
		"set-bad-flags when <field> <where> <real>")
{
  setNumExtraTokens(1);
}


/*********************************************************************
 * Destructor
 */

SetBadFlagsCmd::~SetBadFlagsCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool SetBadFlagsCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments out of the command

  std::string param_name = _cmdTokens[1].getCommandText();
  std::string where = _cmdTokens[2].getCommandText();
  double threshold1 = _cmdTokens[3].getFloatParam();
  double threshold2 = 0.0;
  if (_cmdTokens[4].getTokenType() != UiCommandToken::UTT_END)
    threshold2 = _cmdTokens[4].getFloatParam();

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

    printf("Set bad flags field: %s not found\n", param_name.c_str());
    seds->punt = YES;
    return false;
  }

  std::size_t num_gates = ray.getNGates();
  const Radx::fl32 *data = field->getDataFl32();
  const double bad = field->getMissingFl32();
  
  se_do_clear_bad_flags_array(num_gates);

  unsigned short *boundary_mask = seds->boundary_mask;
  unsigned short *bad_flags_mask = seds->bad_flag_mask_array;

  // Loop through the data

  if (where.compare(0, 3, "below", 0, 3) == 0)
  {
    for (std::size_t i = 0; i < num_gates; ++i)
    {
      if (boundary_mask[i] == 0 || data[i] == bad)
	continue;

      if (data[i] < threshold1)
	bad_flags_mask[i] = 1;
    }
  }
  else if (where.compare(0, 3, "above", 0, 3) == 0)
  {
    for (std::size_t i = 0; i < num_gates; ++i)
    {
      if (boundary_mask[i] == 0 || data[i] == bad)
	continue;

      if (data[i] > threshold1)
	bad_flags_mask[i] = 1;
    }
  }
  else
  {
    // between

    for (std::size_t i = 0; i < num_gates; ++i)
    {
      if (boundary_mask[i] == 0 || data[i] == bad)
	continue;

      if (data[i] >= threshold1 && data[i] <= threshold2)
	bad_flags_mask[i] = 1;
    }
  }

  return true;
}

#else

bool SetBadFlagsCmd::doIt() const
{
  if (se_set_bad_flags(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
