#ifdef USE_RADX
#include <Radx/RadxGeoref.hh>
#else
#include <se_for_each.hh>
#endif

#include <dd_math.h>
#include <se_utils.hh>

#include "HeaderValueCmd.hh"


/*********************************************************************
 * Constructors
 */

HeaderValueCmd::HeaderValueCmd() :
  ForEachRayCmd("header-value", "header-value <name> is <real>")
{
}


/*********************************************************************
 * Destructor
 */

HeaderValueCmd::~HeaderValueCmd()
{
}


/*********************************************************************
 * doIt()
 */

#ifdef USE_RADX

bool HeaderValueCmd::doIt(const int frame_num, RadxRay &ray) const
{
  // Pull the arguments from the command

  std::string name = _cmdTokens[1].getCommandText();
  std::string s_val = _cmdTokens[2].getCommandText();
  float f_val = _cmdTokens[2].getFloatParam();

  // Get the editor information

  struct solo_edit_stuff *seds = return_sed_stuff();

  seds->modified = YES;

  // Get the field information

  DataInfo *data_info =
    DataManager::getInstance()->getWindowInfo(frame_num);

  // NOTE: Rethink how to test for the different header field names.  Right
  // now, for example, using "a" will cause "range-to-first-cell" if new
  // sweep and "tilt-angle" to be updated.

  // NOTE: Need to figure out the "new sweep" thing.  When is this true?
  // Can we just check for number of rays updated being 1 like in other
  // commands?

//  if (data_info->isNewSweep())
  if (seds->process_ray_count == 1)
  {
    if (strstr("range-to-first-cell", name.c_str()) != 0)
    {
      data_info->setStartRange(f_val);
    }
    else if (strstr("cell-spacing", name.c_str()) != 0)
    {
      data_info->setGateSpacing(f_val);
    }
    else if (strstr("fixed-angle", name.c_str()) != 0)
    {
      data_info->setFixedAngle(f_val);
    }
    else if (strstr("latitude", name.c_str()) != 0)
    {
      data_info->setRadarLatitude(f_val);
    }
    else if (strstr("longitude", name.c_str()) != 0)
    {
      data_info->setRadarLongitude(f_val);
    }
    else if (strstr("altitude", name.c_str()) != 0)
    {
      data_info->setRadarAltitude(f_val);
    }
    else if (strstr("scan-mode", name.c_str()) != 0)
    {
      data_info->setSweepMode(Radx::sweepModeFromStr(s_val));
    }
  }

  if (strstr("range-to-first-cell", name.c_str()) != 0)
  {
    ray.setStartRangeKm(f_val);
  }
  else if (strstr("cell-spacing", name.c_str()) != 0)
  {
    ray.setGateSpacingKm(f_val);
  }
  else if (strstr("fixed-angle", name.c_str()) != 0)
  {
    ray.setFixedAngleDeg(f_val);
  }
  else if (strstr("scan-mode", name.c_str()) != 0)
  {
    ray.setSweepMode(Radx::sweepModeFromStr(s_val));
  }
  else if (strstr("nyquist-velocity", name.c_str()) != 0)
  {
    ray.setNyquistMps(f_val);
  }
  else if (strstr("tilt-angle", name.c_str()) != 0)
  {
    ray.getGeoreference()->setTilt(f_val);
  }
  else if (strstr("ipp1", name.c_str()) != 0)
  {
    ray.setPrtSec(f_val);
  }
  else if (strstr("ipp2", name.c_str()) != 0)
  {
    ray.setPrtRatio(ray.getPrtSec() / f_val);
  }
  else if (strstr("rotation-angle", name.c_str()) != 0)
  {
    ray.getGeoreference()->setRotation(f_val);
  }
  else if (strstr("elevation-angle", name.c_str()) != 0)
  {
    ray.setElevationDeg(f_val);
  }
  else if (strstr("diddle-elevation", name.c_str()) != 0)
  {
    ray.setElevationDeg(FMOD360(180.0 - ray.getElevationDeg()));
  }
  else if (strstr("corr-elevation", name.c_str()) != 0)
  {
    ray.setElevationDeg(FMOD360(ray.getElevationDeg() + f_val));
  }
  else if (strstr("corr-azimuth", name.c_str()) != 0)
  {
    ray.setAzimuthDeg(FMOD360(ray.getAzimuthDeg() + f_val));
  }
  else if (strstr("corr-rot-angle", name.c_str()) != 0)
  {
    RadxGeoref *georef = ray.getGeoreference();
    georef->setRotation(FMOD360(georef->getRotation() + f_val));
  }
  else if (strstr("latitude", name.c_str()) != 0)
  {
    ray.getGeoreference()->setLatitude(f_val);
  }
  else if (strstr("longitude", name.c_str()) != 0)
  {
    ray.getGeoreference()->setLongitude(f_val);
  }
  else if (strstr("altitude", name.c_str()) != 0)
  {
    ray.getGeoreference()->setAltitudeKmMsl(f_val);
  }
  else if (strstr("agl-altitude", name.c_str()) != 0)
  {
    ray.getGeoreference()->setAltitudeKmAgl(f_val);
  }
  else if (strstr("msl-into-agl-corr", name.c_str()) != 0)
  {
    RadxGeoref *georef = ray.getGeoreference();
    georef->setAltitudeKmAgl(georef->getAltitudeKmMsl() + f_val);
  }

  return true;
}

#else

bool HeaderValueCmd::doIt() const
{
  if (se_header_value(_cmdTokens) >= 0)
    return true;
  
  return false;
}

#endif

/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/
