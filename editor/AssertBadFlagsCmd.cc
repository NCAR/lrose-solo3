#ifdef USE_RADX
#else
#include <se_flag_ops.hh>
#endif

#include <se_utils.hh>

#include "AssertBadFlagsCmd.hh"


/*********************************************************************
 * Constructors
 */

AssertBadFlagsCmd::AssertBadFlagsCmd() :
  ForEachRayCmd("assert-bad-flags", "assert-bad-flags in <field>")
{
}


/*********************************************************************
 * Destructor
 */

AssertBadFlagsCmd::~AssertBadFlagsCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool AssertBadFlagsCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string dst_name = _cmdTokens[1].getCommandText();

  //  Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = 1;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  RadxField *field = ray.getField(dst_name);
  
  if (field == 0)
  {
    printf("Field to be asserted: %s not found\n", dst_name.c_str());
    seds->punt = 1;
    return false;
  }

  // Get the data.  We need to make a copy so we can edit it

  std::size_t num_gates = ray.getNGates();

  const Radx::fl32 *orig_data = field->getDataFl32();
  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  
  double bad = field->getMissingFl32();
  
  unsigned short *boundary_mask = seds->boundary_mask;
  unsigned short *bad_flag_mask = seds->bad_flag_mask;

  // Loop through the data

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0)
      continue;
    
    if (bad_flag_mask != 0)
      data[i] = bad;
  }

  // Update the ray data

  field->setDataFl32(num_gates, data, false);
  
  return true;
}

#else

bool AssertBadFlagsCmd::doIt() const
{
  if (se_assert_bad_flags(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
