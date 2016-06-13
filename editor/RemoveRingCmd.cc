#ifdef USE_RADX
#else
#include <se_defrkl.hh>
#endif

#include <se_utils.hh>

#include "RemoveRingCmd.hh"


/*********************************************************************
 * Constructors
 */

RemoveRingCmd::RemoveRingCmd() :
  ForEachRayCmd("remove-ring",
		"remove-ring in <field> from <real> to <real> km.")
{
}


/*********************************************************************
 * Destructor
 */

RemoveRingCmd::~RemoveRingCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool RemoveRingCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string dst_name = _cmdTokens[1].getCommandText();
  float ring_start_range = _cmdTokens[2].getFloatParam();
  float ring_end_range = 1.0e19;
  if (_cmdTokens[3].getTokenType() == UiCommandToken::UTT_VALUE)
    ring_end_range = _cmdTokens[3].getFloatParam();

  if (ring_start_range > ring_end_range)
    return true;
  
  // Determine the gate numbers for the ring

  const std::size_t num_gates = ray.getNGates();

  int start_gate = ray.getGateIndex(ring_start_range / 1000.0);
  int end_gate = ray.getGateIndex(ring_end_range / 1000.0) + 1;

  if (start_gate < 0)
    start_gate = 0;
  if (end_gate > num_gates)
    end_gate = num_gates;
  
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  RadxField *field = ray.getField(dst_name);
  
  if (field == 0)
  {
    // Field not found

    printf("Field to be de-ringed: %s not found\n", dst_name.c_str());
    seds->punt = YES;
    return false;
  }

  const Radx::fl32 *orig_data = field->getDataFl32();
  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  const double bad = field->getMissingFl32();
  
  // Update the data

  unsigned short *boundary_mask = seds->boundary_mask;

  for (size_t i = start_gate; i < end_gate; ++i)
  {
    if (boundary_mask[i] == 0)
      continue;	

    data[i] = bad;
  }
  
  // Update the ray data

  field->setDataFl32(num_gates, data, false);
  
  return true;
}

#else

bool RemoveRingCmd::doIt() const
{
  if (se_ring_zap(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
