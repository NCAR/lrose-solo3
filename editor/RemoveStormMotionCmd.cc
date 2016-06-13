#include <math.h>

#ifdef USE_RADX
#else
#include <se_BB_unfold.hh>
#endif

#include <dd_math.h>
#include <se_utils.hh>

#include "RemoveStormMotionCmd.hh"


/*********************************************************************
 * Constructors
 */

RemoveStormMotionCmd::RemoveStormMotionCmd() :
  ForEachRayCmd("remove-storm-motion",
		"remove-storm-motion in <field> of <real> deg <real> mps")
{
}


/*********************************************************************
 * Destructor
 */

RemoveStormMotionCmd::~RemoveStormMotionCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool RemoveStormMotionCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string param_name = _cmdTokens[1].getCommandText();
  float wind = _cmdTokens[2].getFloatParam(); /* angle */
  wind = FMOD360(wind + 180.0); /* change to wind vector */
  float speed = _cmdTokens[3].getFloatParam();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  RadxField *field = ray.getField(param_name);
  
  if (field == 0)
  {
    // Field not found

    printf("Source parameter %s not found for copy\n", param_name.c_str());
    seds->punt = YES;
    return false;
  }

  const std::size_t num_gates = ray.getNGates();
  const Radx::fl32 *orig_data = field->getDataFl32();
  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  const double bad = field->getMissingFl32();

  // Precalculate needed values

  double az = ray.getAzimuthDeg();
  double cos_el = cos(RADIANS(ray.getElevationDeg()));
  if (fabs(cos_el) < 0.0001)
    return true;

  double theta = angle_diff(az, wind); /* clockwise from az to wind */
  double adjust = cos(RADIANS(theta)) * speed;

  // Process the command

  short *boundary_mask = (short *) seds->boundary_mask;

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0 || data[i] == bad)
      continue;

    data[i] = (data[i] * cos_el - adjust) / cos_el;
  }

  return true;
}

#else

bool RemoveStormMotionCmd::doIt() const
{
  if (se_remove_storm_motion(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
