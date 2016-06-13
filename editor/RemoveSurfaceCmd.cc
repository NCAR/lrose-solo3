#ifdef USE_RADX
#else
#include <se_BB_unfold.hh>
#endif

#include <dd_math.h>
#include <EarthRadiusCalculator.hh>
#include <se_utils.hh>

#include "RemoveSurfaceCmd.hh"


/*********************************************************************
 * Constructors
 */

RemoveSurfaceCmd::RemoveSurfaceCmd() :
  ForEachRayCmd("remove-surface", "remove-surface in <field>")
{
}


RemoveSurfaceCmd::RemoveSurfaceCmd(const std::string &keyword,
				   const std::string &cmd_template) :
  ForEachRayCmd(keyword, cmd_template)
{
}


/*********************************************************************
 * Destructor
 */

RemoveSurfaceCmd::~RemoveSurfaceCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool RemoveSurfaceCmd::doIt(const int frame_num, RadxRay &ray) const
{
  return _removeSurface(frame_num, ray,
			_cmdTokens[1].getCommandText(),
			false, false);
}

#else

bool RemoveSurfaceCmd::doIt() const
{
  if (se_ac_surface_tweak(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

#ifdef USE_RADX

/*********************************************************************
 * _removeSurface()
 */

bool RemoveSurfaceCmd::_removeSurface(const int frame_num, RadxRay &ray,
				      const std::string &param_name,
				      const bool surface_only,
				      const bool only_2_trip_in) const
{
  // Copy the const flag to a local variable so it can be updated

  bool only_2_trip = only_2_trip_in;
  
  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);
  
  // We probably need to make a calculation to determine if the ground echo
  // goes above the aircraft i.e. is the distance (arc length) defined by a
  // ray of length max_range rotated through half the beam width greater than
  // or equal to the altitude of the aircraft?

  double bmwidth = RADIANS(seds->optimal_beamwidth ? seds->optimal_beamwidth :
			   data_info->getVertBeamWidth());
  double half_vbw = 0.5 * bmwidth;

  double alt = data_info->getPlatformAltitudeAGL() * 1000.0;
  double max_range = ray.getMaxRangeKm() * 1000.0;
  double elev = ray.getElevationDeg();

  double min_grad = 0.08;	/* dbz per meter */
  double elev_limit = -0.0001;
  double fudge = 1.0;
  int g1;
  
  char *alt_gecho_string = getenv ("ALTERNATE_GECHO");
  bool alt_gecho_flag = false;

  if (surface_only && alt_gecho_string != 0)
  {
    if (elev > -0.002)	/* -.10 degrees */
      return true;

    alt_gecho_flag = true;
    double d = atof(alt_gecho_string);
    if (d > 0)
      min_grad = d;
  }

  double d = max_range * fudge * bmwidth;
  if (!surface_only && d >= alt)
  {
    d -= alt;
    elev_limit = atan2(d, (double)max_range);

    if (elev > elev_limit)
      return 1;

    if (d >= 0 && elev > -fudge * bmwidth)
    {
      only_2_trip = true;
      g1 = 0;
    }
  }

  if (elev > elev_limit)
    return true;

  if (!only_2_trip)
  {
    double earthr =
      EarthRadiusCalculator::getInstance()->getRadius(data_info->getPlatformLatitude());
    elev -= half_vbw;
    double tan_elev = tan(elev);
    double range1 = (-alt / sin(elev)) *
      (1.0 + alt / (2.0 * earthr * 1000.0 * tan_elev * tan_elev));
    if (range1 > max_range || range1 < 0 )
      return 1;
    
    g1 = ray.getGateIndex(range1 / 1000.0);
  }

  int gate_shift = seds->surface_gate_shift;

  if (alt_gecho_flag)
  {
    int zmax_cell;
    
    int ii = data_info->alternateGecho(min_grad, zmax_cell);
    if (ii > 0)
    {
      g1 = ii;
      gate_shift = 0;
    }
    else
    {
      return true;
    }
  }

  // Find the field 

  RadxField *field = ray.getField(param_name);
  
  if (field == 0)
  {
    // Field not found

    printf("Source parameter %s not found for surface removal\n",
	   param_name.c_str());
    seds->punt = YES;
    return false;
  }

  const std::size_t num_gates = ray.getNGates();
  const Radx::fl32 *orig_data = field->getDataFl32();
  Radx::fl32 *data = new Radx::fl32[num_gates];
  memcpy(data, orig_data, num_gates * sizeof(Radx::fl32));
  const double bad = field->getMissingFl32();
  
  g1 += gate_shift;
  if (g1 < 0)
    g1 = 0;

  for (std::size_t i = g1; i < num_gates; ++i)
    data[i] = bad;
  
  // Update the ray data

  field->setDataFl32(num_gates, data, false);
  
  return true;
}
#endif
