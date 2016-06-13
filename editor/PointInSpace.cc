#include <iostream>
#include <math.h>
#include <string.h>

#include <dd_crackers.hh>
#include <EarthRadiusCalculator.hh>
#include <solo_defs.h>

#include "PointInSpace.hh"


// Define statics

const size_t PointInSpace::NUM_BYTES = sizeof(point_in_space_t);

/*********************************************************************
 * Constructor
 */

PointInSpace::PointInSpace() :
  _time(0.0),
  _earthRadius(0.0),
  _latitude(0.0),
  _longitude(0.0),
  _altitude(0.0),
  _roll(0.0),
  _pitch(0.0),
  _drift(0.0),
  _heading(0.0),
  _x(0.0),
  _y(0.0),
  _z(0.0),
  _azimuth(0.0),
  _elevation(0.0),
  _range(0.0),
  _rotationAngle(0.0),
  _tilt(0.0),
  _state(0),
  _id("")
{
  memset(&_buffer, 0, sizeof(_buffer));
  memcpy(_buffer.name_struct, "PISP", sizeof(_buffer.name_struct));
  _buffer.sizeof_struct = sizeof(_buffer);
}


/*********************************************************************
 * Destructor
 */

PointInSpace::~PointInSpace()
{
}


/*********************************************************************
 * latLon2xy()
 */

void PointInSpace::latLon2xy(double &x, double &y, double &z,
			     const double ref_lat, const double ref_lon,
			     const double ref_alt,
			     const double earth_rad) const
{
  /* calculate (x,y,z) of (plat,plon) relative to (ref_lat,ref_lon) */
  /* all dimensions in km. */

  /* transform to earth coordinates and then to lat/lon/alt */

  /* These calculations are from the book
   * "Aerospace Coordinate Systems and Transformations"
   * by G. Minkler/J. Minkler
   * these are the ECEF/ENU point transformations
   */

  double h = earth_rad + ref_alt;
  double delta_o = RADIANS(ref_lat);	/* read delta sub oh */
  double lambda_o = RADIANS(ref_lon);

  double sinLambda = sin(lambda_o);
  double cosLambda = cos(lambda_o);
  double sinDelta = sin(delta_o);
  double cosDelta = cos(delta_o);
    
  double R_p = earth_rad + _altitude;
  double delta_p = RADIANS(_latitude);
  double lambda_p = RADIANS(_longitude);
  double R_p_pr = R_p * cos(delta_p);

  double xe = R_p * sin(delta_p);
  double ye = -R_p_pr * sin(lambda_p);
  double ze = R_p_pr * cos(lambda_p);

  /* transform to ENU coordinates */

  double a = -h * sinDelta + xe;
  double b =  h * cosDelta * sinLambda + ye;
  double c = -h * cosDelta * cosLambda + ze;

  x = -cosLambda * b - (sinLambda * c);
  y = (cosDelta * a)  +  (sinLambda * sinDelta * b) -
    (cosLambda * sinDelta * c);
  z = (sinDelta * a)  - (sinLambda * cosDelta * b) + (cosLambda * cosDelta * c);
}


/*********************************************************************
 * latLonRelative()
 */

void PointInSpace::latLonRelative(const PointInSpace &p0)
{
  double earth_radius =
    EarthRadiusCalculator::getInstance()->getRadius(_latitude);
  p0.latLon2xy(_x, _y, _z, _latitude, _longitude, _altitude, earth_radius);
}


/*********************************************************************
 * latLonShift()
 */

void PointInSpace::latLonShift(const PointInSpace &p0)
{
  double earth_radius =
    EarthRadiusCalculator::getInstance()->getRadius(p0.getLatitude()); 
  p0.xy2latLon(_latitude, _longitude, _altitude, p0, earth_radius);
}


/*********************************************************************
 * print()
 */

void PointInSpace::print(std::ostream &stream, const std::string &leader) const
{
  stream << leader << "X = " << _x << std::endl;
  stream << leader << "Y = " << _y << std::endl;
  stream << leader << "lat = " << _latitude << std::endl;
  stream << leader << "lon = " << _longitude << std::endl;
}

/*********************************************************************
 * rangeAzEl2xyz()
 */

void PointInSpace::rangeAzEl2xyz()
{
    /* routine to calculate (x,y,z) (from az,el,rng)
     *
     */

  double phi = RADIANS(_elevation);
  _z = _range * sin(phi);
  double rxy = _range * cos(phi);
  double theta = RADIANS(CART_ANGLE(_azimuth));
  _x = rxy * cos(theta);
  _y = rxy * sin(theta);
}


/*********************************************************************
 * readFromBuffer()
 */

void PointInSpace::readFromBuffer(const char *buffer, const bool swap)
{
  // Copy the buffer to the local buffer, swapping if necessary

  memcpy(&_buffer, buffer, sizeof(_buffer));

  if (swap)
  {
    point_in_space_t swap_buffer;
    
    _crack((char *)&_buffer, (char *)&swap_buffer, 0);
    _buffer = swap_buffer;
  }
  
  // Copy the buffer values to the object members

  _time = _buffer.time;
  _earthRadius = _buffer.earth_radius;
  _latitude = _buffer.latitude;
  _longitude = _buffer.longitude;
  _altitude = _buffer.altitude;
  _roll = _buffer.roll;
  _pitch = _buffer.pitch;
  _drift = _buffer.drift;
  _heading = _buffer.heading;
  _x = _buffer.x;
  _y = _buffer.y;
  _z = _buffer.z;
  _azimuth = _buffer.azimuth;
  _elevation = _buffer.elevation;
  _range = _buffer.range;
  _rotationAngle = _buffer.rotation_angle;
  _tilt = _buffer.tilt;
  _state = _buffer.state;
  _id = _buffer.id;
}


/*********************************************************************
 * xy2latLon()
 */

void PointInSpace::xy2latLon(double &lat, double &lon, double &alt,
			     const PointInSpace &ref_pt,
			     const double earth_radius) const
{
  /* calculate (lat,lon) of a point at (x,y) relative to (ref_lat,ref_lon) */
  /* all dimensions in km. */
  
  /* transform to earth coordinates and then to lat/lon/alt */
  
  /* These calculations are from the book
   * "Aerospace Coordinate Systems and Transformations"
   * by G. Minkler/J. Minkler
   * these are the ECEF/ENU point transformations
   */

  double h = earth_radius + ref_pt.getAltitude();
  double delta_o = RADIANS(ref_pt.getLatitude());	/* read delta sub oh */
  double lambda_o = RADIANS(ref_pt.getLongitude());

  double sinLambda = sin(lambda_o);
  double cosLambda = cos(lambda_o);
  double sinDelta = sin(delta_o);
  double cosDelta = cos(delta_o);
    
  /* transform to earth coordinates */

  double xe = (h * sinDelta) + (cosDelta * ref_pt.getY()) +
    (sinDelta * ref_pt.getZ());

  double ye = (-h * cosDelta * sinLambda) - (cosLambda * ref_pt.getX()) +
    (sinLambda * sinDelta * ref_pt.getY()) -
    (sinLambda * cosDelta * ref_pt.getZ());

  double ze = (h * cosDelta * cosLambda) - (sinLambda * ref_pt.getX()) -
    (cosLambda * sinDelta * ref_pt.getY()) +
    (cosLambda * cosDelta * ref_pt.getZ());

  double lambda_p = atan2(-ye, ze);
  double delta_p = atan2(xe, sqrt(ye * ye + ze * ze));

  lat = DEGREES(delta_p);
  lon = DEGREES(lambda_p);
  alt = sqrt(xe * xe + ye * ye + ze * ze) - earth_radius;
}


/*********************************************************************
 * writeAscii()
 */

void PointInSpace::writeAscii(FILE *stream, const int num) const
{
  fprintf(stream, "bnd:%3d; lon:%.4f; lat:%.4f; alt:%.4f; ",
	  num, _longitude, _latitude, _altitude);
  fprintf(stream, "az:%.4f; el:%.4f; rng:%.4f;",
	  _azimuth, _elevation, _range);
  fprintf(stream, "hdg:%.4f; tlt:%.4f; dft:%.4f; ",
	  _heading, _tilt, _drift);
  fprintf(stream, "\n");
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _crack()
 */

void PointInSpace::_crack(char *srs, char *dst, int limit)
{
  int item_count = 24;
  
  int offs_ndx = 0;
  int sparc_ndx = 6;
  int ndx_count = 7;
  
  static int crk_point_in_space [][7] = {
    {    0,    1,    1,    4,    4,    1,    0},
    {    4,    3,    4,    1,    1,    1,    4},
    {    8,    5,    8,    1,    5,    5,    8},
    {   16,    5,    8,    1,    1,    1,   16},
    {   24,    5,    8,    1,    1,    1,   24},
    {   32,    5,    8,    1,    1,    1,   32},
    {   40,    5,    8,    1,    1,    1,   40},
    {   48,    4,    4,    1,   12,   12,   48},
    {   52,    4,    4,    1,    1,    1,   52},
    {   56,    4,    4,    1,    1,    1,   56},
    {   60,    4,    4,    1,    1,    1,   60},
    {   64,    4,    4,    1,    1,    1,   64},
    {   68,    4,    4,    1,    1,    1,   68},
    {   72,    4,    4,    1,    1,    1,   72},
    {   76,    4,    4,    1,    1,    1,   76},
    {   80,    4,    4,    1,    1,    1,   80},
    {   84,    4,    4,    1,    1,    1,   84},
    {   88,    4,    4,    1,    1,    1,   88},
    {   92,    4,    4,    1,    1,    1,   92},
    {   96,    3,    4,    1,    4,    4,   96},
    {  100,    3,    4,    1,    1,    1,  100},
    {  104,    3,    4,    1,    1,    1,  104},
    {  108,    3,    4,    1,    1,    1,  108},
    {  112,    1,    1,   16,   16,    1,  112},
  };

  crackers(srs, dst, item_count, ndx_count, crk_point_in_space[limit],
	   offs_ndx, sparc_ndx, limit);
}


/*********************************************************************
 * _updateBuffer()
 */

void PointInSpace::_updateBuffer() const
{
  _buffer.time = _time;
  _buffer.earth_radius = _earthRadius;
  _buffer.latitude = _latitude;
  _buffer.longitude = _longitude;
  _buffer.altitude = _altitude;
  _buffer.roll = _roll;
  _buffer.pitch = _pitch;
  _buffer.drift = _drift;
  _buffer.heading = _heading;
  _buffer.x = _x;
  _buffer.y = _y;
  _buffer.z = _z;
  _buffer.azimuth = _azimuth;
  _buffer.elevation = _elevation;
  _buffer.range = _range;
  _buffer.rotation_angle = _rotationAngle;
  _buffer.tilt = _tilt;
  strncpy(_buffer.id, _id.c_str(), 16);
  _buffer.state = _state;
}
