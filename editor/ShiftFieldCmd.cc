#ifdef USE_RADX
#else
#include <se_for_each.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "ShiftFieldCmd.hh"


/*********************************************************************
 * Constructors
 */

ShiftFieldCmd::ShiftFieldCmd() :
  ForEachRayCmd("shift-field", "shift-field <field> put-in <field>")
{
}


/*********************************************************************
 * Destructor
 */

ShiftFieldCmd::~ShiftFieldCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool ShiftFieldCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments out of the command

  std::string src_name = _cmdTokens[1].getCommandText();
  std::string dst_name = _cmdTokens[2].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  // Find the source field

  RadxField *src_field = ray.getField(src_name);
  
  if (src_field == 0)
  {
    g_string_sprintfa(gs_complaints,
		      "Second parameter %s not found for add/sub\n",
		      src_name.c_str());
    seds->punt = YES;
    return false;
  }

  const Radx::fl32 *src_data = src_field->getDataFl32();
  double src_bad = src_field->getMissingFl32();

  // See if the destination field already exists.  If it does exist, we will
  // just replace the data with the addition of the source field and the value.
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

  // Here's where we finally do the copying

  int fshift = seds->gates_of_shift;
	
  int start_gate = fshift > 0 ? num_gates - 1 : 0;
  int inc = fshift > 0 ? -1 : 1;
  int end_gate = fshift > 0 ? num_gates - fshift : num_gates + fshift;

  for (int ii = start_gate; ii < end_gate; ii += inc)
    dst_data[ii] = src_data[ii - fshift];

  // Fill in at whichever end

  start_gate = fshift < 0 ? num_gates - 1 : 0;
  inc = fshift < 0 ? -1 : 1;
  end_gate = fshift < 0 ? -fshift : fshift;

  for (int ii = start_gate; ii < end_gate; ii += inc)
    dst_data[ii] = dst_bad;
  
  // Update the data in the ray

  if (dst_field == 0)
    ray.addField(dst_name, src_field->getUnits(), num_gates,
		 dst_bad, dst_data, false);
  else
    dst_field->setDataFl32(num_gates, dst_data, false);
  
  return true;
}

#else

bool ShiftFieldCmd::doIt() const
{
  if (se_cpy_field(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
