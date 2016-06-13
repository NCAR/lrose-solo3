#include <iostream>
#include <math.h>

#include <dd_math.h>
#include "EarthRadiusCalculator.hh"

// Global variables

EarthRadiusCalculator *EarthRadiusCalculator::_instance =
  (EarthRadiusCalculator *)0;

const double EarthRadiusCalculator::MAJOR_AXIS_M = 6378388.0;
const double EarthRadiusCalculator::MINOR_AXIS_M = 6356911.946;


/*********************************************************************
 * Constructor
 */

EarthRadiusCalculator::EarthRadiusCalculator()
{
  // Calculate the radius list

  _calcRadiusList();

  // Set the instance pointer to point to this singleton instance

  _instance = this;
}


/*********************************************************************
 * Destructor
 */

EarthRadiusCalculator::~EarthRadiusCalculator()
{
}


/*********************************************************************
 * getInstance()
 */

EarthRadiusCalculator *EarthRadiusCalculator::getInstance()
{
  if (_instance == 0)
    new EarthRadiusCalculator();
  
  return _instance;
}


/*********************************************************************
 * getRadius()
 */

double EarthRadiusCalculator::getRadius(const double lat)
{
  int nn = (int)fabs(lat * 0.2);
  double radius_m = nn < 18 ? _radiusList[nn] : MINOR_AXIS_M;

  return radius_m * 0.001;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _calcRadiusList()
 */

void EarthRadiusCalculator::_calcRadiusList()
{
  // Calculate the list of radius values, one for every 5 degrees of latitude

  double theta = 0.0;
  
  for (int ii = 0; theta < 90.0; ii++, theta += 5.0)
  {
    double tt = tan(RADIANS(theta));
    double d = sqrt(1.0 + SQ(tt * MAJOR_AXIS_M / MINOR_AXIS_M));
    double x = MAJOR_AXIS_M / d;
    double y = x * tt;
    _radiusList[ii] = sqrt(SQ(x) + SQ(y));
  }
}
