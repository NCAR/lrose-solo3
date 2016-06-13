#ifdef USE_RADX
#else
#include <se_add_mult.hh>
#endif

#include <se_utils.hh>
#include <sii_externals.h>

#include "RainRateCmd.hh"


/*********************************************************************
 * Constructors
 */

RainRateCmd::RainRateCmd() :
  ForEachRayCmd("rain-rate",
		"rain-rate <field> by <real> put-in <field>")
{
}


/*********************************************************************
 * Destructor
 */

RainRateCmd::~RainRateCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool RainRateCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string src_name = _cmdTokens[1].getCommandText();
  double rain_value = _cmdTokens[2].getFloatParam();
  std::string dst_name = _cmdTokens[3].getCommandText();

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;

  // Get the information about the fields

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  // Find the source field

  RadxField *src_field = ray.getField(src_name);
  
  if (src_field == 0)
  {
    g_string_sprintfa(gs_complaints,
		      "Second parameter %s not found for multiply\n",
		      src_name.c_str());
    seds->punt = YES;
    return false;
  }

  const Radx::fl32 *src_data = src_field->getDataFl32();
  const double src_bad = src_field->getMissingFl32();

  // See if the destination field already exists.  If it does exist, we will
  // just replace the data with the addition of the source field and the value.
  // If it doesn't exist, we will create a new field and will start with
  // the source field data.

  const std::size_t num_gates = ray.getNGates();
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

  // Here's where we finally do the work

  unsigned short *boundary_mask = seds->boundary_mask;

  double A = 0.003333333;
  double X = 0.7142857;

  if (rain_value != 0.0)
  {
    X = rain_value;
  }

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0)
      continue;
    
    if (src_data[i] == src_bad)
      dst_data[i] = dst_bad;
    else
      dst_data[i] = A * pow((double)10.0, (double)(0.1 * src_data[i] * X));
  }
  
  return true;
}

#else

bool RainRateCmd::doIt() const
{
  if (se_mult_const(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
