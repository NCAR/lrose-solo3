#ifdef USE_RADX
#else
#include <se_for_each.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "ThresholdCmd.hh"


/*********************************************************************
 * Constructors
 */

ThresholdCmd::ThresholdCmd() :
  ForEachRayCmd("threshold",
		"threshold <field> on <field> <where> <real>")
{
}


/*********************************************************************
 * Destructor
 */

ThresholdCmd::~ThresholdCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool ThresholdCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments out of the command

  std::string dst_name = _cmdTokens[1].getCommandText();
  std::string thr_name = _cmdTokens[2].getCommandText();
  std::string where = _cmdTokens[3].getCommandText();
  float thresh1 = _cmdTokens[4].getFloatParam();
  bool use_second_thresh = false;
  float thresh2 = 0.0;
  if (_cmdTokens[5].getTokenType() == UiCommandToken::UTT_VALUE)
  {
    use_second_thresh = true;
    thresh2 = _cmdTokens[5].getFloatParam();
  }
  
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  const std::size_t num_gates = ray.getNGates();

  RadxField *thr_field = ray.getField(thr_name);
  if (thr_field == 0)
  {
    // Field not found

    g_string_sprintfa(gs_complaints, "Threshold field: %s not found\n",
		      thr_name.c_str());
    seds->punt = YES;
    return false;
  }
  
  const Radx::fl32 *thr_data = thr_field->getDataFl32();
  const double thr_bad = thr_field->getMissingFl32();
  
  RadxField *dst_field = ray.getField(dst_name);
  if (dst_field == 0)
  {
    // Field not found

    g_string_sprintfa(gs_complaints, "Field to be thresholded: %s not found\n",
		      dst_name.c_str());
    seds->punt = YES;
    return false;
  }

  const Radx::fl32 *orig_dst_data = dst_field->getDataFl32();
  Radx::fl32 *dst_data = new Radx::fl32[num_gates];
  memcpy(dst_data, orig_dst_data, num_gates * sizeof(Radx::fl32));
  const double dst_bad = dst_field->getMissingFl32();
  
  data_info->setParamThreshold(dst_name, thr_name, thresh1);
  
  int fgg = seds->first_good_gate;
  if (fgg > num_gates)
    fgg = num_gates;
  
  unsigned short *boundary_mask = seds->boundary_mask;

  // Loop through the data

  for (std::size_t i = 0; i < fgg; ++i)
    dst_data[i] = dst_bad;
  
  if (where.compare(0, 3, "below", 0, 3) == 0)
  {
    for (std::size_t i = fgg; i < num_gates; ++i)
    {
      if (boundary_mask[i] == 0 || dst_data[i] == dst_bad)
	continue;

      if (thr_data[i] == thr_bad || thr_data[i] < thresh1)
	dst_data[i] = dst_bad;
    }
  }
  else if (where.compare(0, 3, "above", 0, 3) == 0)
  {
    for (std::size_t i = fgg; i < num_gates; ++i)
    {
      if (boundary_mask[i] == 0 || dst_data[i] == dst_bad)
	continue;

      if (thr_data[i] == thr_bad || thr_data[i] > thresh1)
	dst_data[i] = dst_bad;
    }
  }
  else
  {
    // Between

    if (use_second_thresh)
    {
      for (std::size_t i = fgg; i < num_gates; ++i)
      {
	if (boundary_mask[i] == 0 || dst_data[i] == dst_bad)
	  continue;

	if (thr_data[i] == thr_bad ||
	    (thr_data[i] >= thresh1 && thr_data[i] <= thresh2))
	  dst_data[i] = dst_bad;
      }
    }
  }
  
  // Update the ray data

  dst_field->setDataFl32(num_gates, dst_data, false);
  
  return true;
}

#else

bool ThresholdCmd::doIt() const
{
  if (se_threshold_field(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
