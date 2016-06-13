#include <math.h>

#ifdef USE_RADX
#else
#include <se_BB_unfold.hh>
#endif

#include <se_utils.hh>

#include "RemoveAircraftMotionCmd.hh"


/*********************************************************************
 * Constructors
 */

RemoveAircraftMotionCmd::RemoveAircraftMotionCmd() :
  ForEachRayCmd("remove-aircraft-motion",
		"remove-aircraft-motion in <field>")
{
}


/*********************************************************************
 * Destructor
 */

RemoveAircraftMotionCmd::~RemoveAircraftMotionCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool RemoveAircraftMotionCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string param_name = _cmdTokens[1].getCommandText();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;
  
  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  // Find the field

  RadxField *field = ray.getField(param_name);
  
  if (field == 0)
  {
    printf("Source parameter %s not found for aircraft motion removal\n",
	   param_name.c_str());
    seds->punt = YES;
    return false;
  }

  const std::size_t num_gates = ray.getNGates();
  const Radx::fl32 *orig_data = field->getDataFl32();
  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  const double bad = field->getMissingFl32();

  double ac_vel = data_info->getAircraftVelocity();
  double nyqv = seds->nyquist_velocity ?
    seds->nyquist_velocity : ray.getNyquistMps();
  double nyqi = 2.0 * nyqv;
  int folds = (int)(ac_vel / nyqi);
  double adjust = ac_vel - (folds * nyqi);

  if (fabs(adjust) > nyqv)
    adjust = adjust > 0.0 ? adjust - nyqi : adjust + nyqi;

  short *boundary_mask = (short *) seds->boundary_mask;

  // Process the command

  for (std::size_t i = 0; i < num_gates; ++i)
  {
    if (boundary_mask[i] == 0 || data[i] == bad)
      continue;

    double vx = data[i] + adjust;
    if (fabs(vx) > nyqv)
      vx = vx > 0 ? vx - nyqi : vx + nyqi;

    data[i] = vx;
  }
  
  // Update the ray data

  field->setDataFl32(num_gates, data, false);
  
  return true;
}

#else

bool RemoveAircraftMotionCmd::doIt() const
{
  if (se_remove_ac_motion(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
