#ifdef USE_RADX
#else
#include <se_flag_ops.hh>
#endif

#include <se_utils.hh>

#include "CopyBadFlagsCmd.hh"


/*********************************************************************
 * Constructors
 */

CopyBadFlagsCmd::CopyBadFlagsCmd() :
  ForEachRayCmd("copy-bad-flags", "copy-bad-flags from <field>")
{
}


/*********************************************************************
 * Destructor
 */

CopyBadFlagsCmd::~CopyBadFlagsCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool CopyBadFlagsCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments out of the command

  std::string param_name = _cmdTokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();
  
  seds->modified = 1;
  seds->bad_flag_mask = seds->bad_flag_mask_array;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  RadxField *field = ray.getField(param_name);
  
  if (field == 0)
  {
    // Field not found

    printf("Copy bad flags field: %s not found\n", param_name.c_str());
    seds->punt = 1;
    return false;
  }

  std::size_t num_gates = ray.getNGates();

  const Radx::fl32 *data = field->getDataFl32();
  double bad = field->getMissingFl32();

  unsigned short *boundary_mask = seds->boundary_mask;
  unsigned short *bad_flag_mask = seds->bad_flag_mask_array;

  // Loop through the data

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0)
      continue;

    if (data[i] == bad)
      bad_flag_mask[i] = 1;
    else
      bad_flag_mask[i] = 0;
  }

  return true;
}

#else

bool CopyBadFlagsCmd::doIt() const
{
  if (se_copy_bad_flags(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
