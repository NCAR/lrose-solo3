#ifndef PointInSpace_HH
#define PointInSpace_HH

#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>
#include <sys/types.h>

class PointInSpace
{

public:

  //////////////////////
  // Public constants //
  //////////////////////

  /**
   * @brief The number of bytes in the object when saved to disk.
   */

  static const size_t NUM_BYTES;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  PointInSpace();

  /**
   * @brief Destructor.
   */

  virtual ~PointInSpace();

  /**
   * @brief Calculate the (x, y, z) of the position of this point relative
   *        to (olat, olon).
   *
   * @param[out] x          The X value in km.
   * @param[out] y          The Y value in km.
   * @param[out] z          The Z value in km.
   * @param[in] ref_lat     The latitude of the reference point.
   * @param[in] ref_lon     The longitude of the reference point.
   * @param[in] ref_alt     The altitude of the reference point.
   * @param[in] earth_rad   The radius of the earth at the reference point
   *                          in km.
   */

  void latLon2xy(double &x, double &y, double &z,
		 const double ref_lat, const double ref_lon,
		 const double ref_alt,
		 const double earth_rad) const;
  

  /**
   * @brief Calculate (lat, lon, alt) of a point at the reference pt's
   *        (x, y, z) relative to the reference pt's (lat, lon, alt).
   *
   * @param[out] lat            The latitude.
   * @param[out] lon            The longitude.
   * @param[out] alt            The altitude.
   * @param[in] ref_pt          The reference point.
   * @param[in] earth_radius    The earth's radius.
   */

  void xy2latLon(double &lat, double &lon, double &alt,
		 const PointInSpace &ref_pt,
		 const double earth_radius) const;
  

  /**
   * @brief Calculate the (x, y, z) of the position of this point so as to
   *        line up with (0, 0, 0) for the given point.
   *
   * @param[in] p0   The given point.
   */

  void latLonRelative(const PointInSpace &p0);
  

  /**
   * @brief Routine to calculate lat/lon/alt so as to line up with (x,y,z)
   *        for for the given point.
   *
   * @param[in] p0   The given point.
   */

  void latLonShift(const PointInSpace &p0);
  

  ////////////////////
  // Public methods //
  ////////////////////

  /////////////////
  // set methods //
  /////////////////
  
  /**
   * @brief Set the unix time.
   *
   * @param[in] unix_time    The unix time.
   */

  inline void setTime(const double unix_time)
  {
    _time = unix_time;
  }
  
  // NOTE: Set this automatically when latitude changes...

  /**
   * @brief Set the earth's radius.
   *
   * @param[in] radius    The earth's radius.
   */

  inline void setEarthRadius(const double radius)
  {
    _earthRadius = radius;
  }
  
  // NOTE:  Combine these set methods as appropriate

  /**
   * @brief Set the latitude.
   *
   * @param[in] lat     The latitude.
   */

  inline void setLatitude(const double lat)
  {
    _latitude = lat;
  }
  
  /**
   * @brief Set the longitude.
   *
   * @param[in] lon   The longitude.
   */

  inline void setLongitude(const double lon)
  {
    _longitude = lon;
  }
  
  /**
   * @brief Set the altitude.
   *
   * @param[in] alt   The altitude.
   */

  inline void setAltitude(const double alt)
  {
    _altitude = alt;
  }
  
  /**
   * @brief Set the roll.
   *
   * @param[in] roll    The roll.
   */

  inline void setRoll(const double roll)
  {
    _roll = roll;
  }
  
  /**
   * @brief Set the pitch.
   *
   * @param[in] pitch   The pitch.
   */

  inline void setPitch(const double pitch)
  {
    _pitch = pitch;
  }
  
  /**
   * @brief Set the drift.
   *
   * @param[in] drift    The drift.
   */

  inline void setDrift(const double drift)
  {
    _drift = drift;
  }
  
  /**
   * @brief Set the heading.
   *
   * @param[in] heading   The heading.
   */

  inline void setHeading(const double heading)
  {
    _heading = heading;
  }
  
  /**
   * @brief Set the X value.
   *
   * @param[in] x    The X value.
   */

  inline void setX(const double x)
  {
    _x = x;
  }
  
  /**
   * @brief Set the Y value.
   *
   * @param[in] y    The Y value.
   */

  inline void setY(const double y)
  {
    _y = y;
  }
  
  /**
   * @brief Set the Z value.
   *
   * @param[in] z    The z value.
   */

  inline void setZ(const double z)
  {
    _z = z;
  }
  
  /**
   * @brief Set the azimuth.
   *
   * @param[in] az    The azimuth.
   */

  inline void setAzimuth(const double az)
  {
    _azimuth = az;
  }
  
  /**
   * @brief Set the elevation.
   *
   * @param[in] el   The elevation.
   */

  inline void setElevation(const double el)
  {
    _elevation = el;
  }
  
  /**
   * @brief Set the range.
   *
   * @param[in] range   The range.
   */

  inline void setRange(const double range)
  {
    _range = range;
  }
  
  /**
   * @brief Set the rotation angle.
   *
   * @param[in] angle    The rotation angle.
   */

  inline void setRotationAngle(const double angle)
  {
    _rotationAngle = angle;
  }
  
  /**
   * @brief Set the tilt.
   *
   * @param[in] tilt    The tilt.
   */

  inline void setTilt(const double tilt)
  {
    _tilt = tilt;
  }
  
  // NOTE:  Should this be called automatically?  See if we can replace
  //  setRange(), setAzimuth() and setElevation() with a single set method
  //  and do this when that set method is called.

  /**
   * @brief Calculate the contained (x,y,z) values from the contained range,
   *        azimuth and elevation value.
   */

  void rangeAzEl2xyz();
  
  /**
   * @brief Set the ID.
   *
   * @param[in] id   The ID.
   */

  inline void setId(const std::string &id)
  {
    _id = id;
  }
  
  /**
   * @brief Clear the state mask.
   */

  inline void clearState()
  {
    _state = 0;
  }
  
  /**
   * @brief Set the earth state flag.
   *
   * @param[in] flag   The new flag value.
   */

  inline void setStateEarth(const bool flag)
  {
    if (flag)
    {
      _state |= PISP_STATE_EARTH;
    }
    else
    {
      _state &= ~PISP_STATE_EARTH;
    }
  }
  
  /**
   * @brief Set the XYZ state flag.
   *
   * @param[in] flag   The new flag value.
   */

  inline void setStateXYZ(const bool flag)
  {
    if (flag)
    {
      _state |= PISP_STATE_XYZ;
    }
    else
    {
      _state &= ~PISP_STATE_XYZ;
    }
  }
  
  /**
   * @brief Set the azimuth/elevation/range state flag.
   *
   * @param[in] flag   The new flag value.
   */

  inline void setStateAzElRg(const bool flag)
  {
    if (flag)
    {
      _state |= PISP_STATE_AZELRG;
    }
    else
    {
      _state &= ~PISP_STATE_AZELRG;
    }
  }
  
  /**
   * @brief Set the plot relative state flag.
   *
   * @param[in] flag   The new flag value.
   */

  inline void setStatePlotRelative(const bool flag)
  {
    if (flag)
    {
      _state |= PISP_STATE_PLOT_RELATIVE;
    }
    else
    {
      _state &= ~PISP_STATE_PLOT_RELATIVE;
    }
  }
  
  /**
   * @brief Set the time series state flag.
   *
   * @param[in] flag   The new flag value.
   */

  inline void setStateTimeSeries(const bool flag)
  {
    if (flag)
    {
      _state |= PISP_STATE_TIME_SERIES;
    }
    else
    {
      _state &= ~PISP_STATE_TIME_SERIES;
    }
  }
  

  /////////////////
  // get methods //
  /////////////////

  /**
   * @brief Get the time.
   *
   * @return Returns the unix time associated with this point.
   */

  inline double getTime() const
  {
    return _time;
  }
  
  /**
   * @brief Get the latitude.
   *
   * @return Returns the latitude.
   */

  inline double getLatitude() const
  {
    return _latitude;
  }
  
  /**
   * @brief Get the longitude.
   *
   * @return Returns the longitude.
   */

  inline double getLongitude() const
  {
    return _longitude;
  }
  
  /**
   * @brief Get the altitude.
   *
   * @return Returns the altitude in km.
   */

  inline double getAltitude() const
  {
    return _altitude;
  }
  
  /**
   * @brief Get the roll.
   *
   * @return Returns the roll.
   */

  inline double getRoll() const
  {
    return _roll;
  }
  
  /**
   * @brief Get the pitch.
   *
   * @return Returns the pitch.
   */

  inline double getPitch() const
  {
    return _pitch;
  }
  
  /**
   * @brief Get the drift.
   *
   * @return Returns the drift.
   */

  inline double getDrift() const
  {
    return _drift;
  }
  
  /**
   * @brief Get the heading.
   *
   * @return Returns the heading.
   */

  inline double getHeading() const
  {
    return _heading;
  }
  
  /**
   * @brief Get the X value.
   *
   * @return Returns the X value in km.
   */

  inline double getX() const
  {
    return _x;
  }
  
  /**
   * @brief Get the Y value.
   *
   * @return Returns the Y value in km.
   */

  inline double getY() const
  {
    return _y;
  }
  
  /**
   * @brief Get the Z value.
   *
   * @return Returns the Z value in km.
   */

  inline double getZ() const
  {
    return _z;
  }
  
  /**
   * @brief Get the azimuth.
   *
   * @return Returns the azimuth.
   */

  inline double getAzimuth() const
  {
    return _azimuth;
  }
  
  /**
   * @brief Get the elevation.
   *
   * @return Returns the elevation.
   */

  inline double getElevation() const
  {
    return _elevation;
  }
  
  /**
   * @brief Get the range.
   *
   * @return Returns the range.
   */

  inline double getRange() const
  {
    return _range;
  }
  
  /**
   * @brief Gets the rotation angle.
   *
   * @return Returns the rotation angle.
   */

  inline double getRotationAngle() const
  {
    return _rotationAngle;
  }
  
  /**
   * @brief Get the tilt.
   *
   * @return Returns the tilt.
   */

  inline double getTilt() const
  {
    return _tilt;
  }
  
  /**
   * @brief Get the ID.
   *
   * @return Returns the ID.
   */

  inline std::string getId() const
  {
    return _id;
  }
  
  /**
   * @brief Get a pointer to the buffer used for writing the information
   *        to disk.
   *
   * @return Returns a pointer to the buffer.
   */

  inline const void *getBufPtr() const
  {
    _updateBuffer();
    return (void *)&_buffer;
  }
  
  /**
   * @brief Get the size of the buffer used for writing the information
   *        to disk.
   */

  inline size_t getBufLen() const
  {
    return sizeof(_buffer);
  }

  /**
   * @brief Check if the time series bit is set in the state mask.
   *
   * @return Returns true if the bit is set, false otherwise.
   */

  inline bool isTimeSeries() const
  {
    return _state & PISP_STATE_TIME_SERIES;
  }
  

  //////////////////////////
  // Input/Output methods //
  //////////////////////////
  
  /**
   * @brief Read the point information from the given buffer.
   *
   * @param[in] buffer     The character buffer.
   * @param[in] swap       Flag indicating whether we need to swap the buffer.
   */

  void readFromBuffer(const char *buffer, const bool swap);
  
  /**
   * @brief Write the point information to the given output stream.
   *
   * @param[in,out] stream    The stream to use for writing.
   * @param[in] num           The point number.
   */

  void writeAscii(FILE *stream, const int num) const;
  
  /**
   * @brief Print the object information to the given stream.
   *
   * @param[in,out] stream   The stream to use for output.
   */

  void print(std::ostream &stream, const std::string &leader = "") const;
  

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  /**
   * @brief The structure used for storing the point information to disk.
   */

  typedef struct
  {
    char name_struct[4];        /* "PISP" */
    int32_t sizeof_struct;         /* sizeof(point_in_space_t) */
    
    double time;                /* unix time */
    double earth_radius;
    double latitude;
    double longitude;
    double altitude;
    
    float roll;
    float pitch;
    float drift;
    float heading;
    float x;
    float y;
    float z;
    float azimuth;
    float elevation;
    float range;
    float rotation_angle;
    float tilt;
    
    int32_t cell_num;
    int32_t ndx_last;
    int32_t ndx_next;
    int32_t state;                 /* which bits are set */
    
    char id[16];
  } point_in_space_t;


  /////////////////////////
  // Protected constants //
  /////////////////////////

  // NOTE:  Change these to flags.

  /**
   * @brief Set this bit in state if there is a lat/lon/alt/earthr
   */

  static const int32_t PISP_STATE_EARTH = 0x00000001;

  /**
   * @brief Set this bit in state if x,y,z vals relative to lat/lon
   */

  static const int32_t PISP_STATE_XYZ = 0x00000002;

  /**
   * @brief Set this bit in state if az,el,rng relative to lat/lon
   */

  static const int32_t PISP_STATE_AZELRG = 0x00000004;

  /**
   * @brief Set this bit in state for aircraft postitioning relative to
   *        lat/lon rotation angle, tilt, range, roll, pitch, drift, heading
   */

  static const int32_t PISP_STATE_AIR = 0x00000008;
  static const int32_t PISP_STATE_PLOT_RELATIVE = 0x00000010;
  static const int32_t PISP_STATE_TIME_SERIES = 0x00000020;


  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Unix time.
   */

  double _time;
  
  /**
   * @brief The earth's radius.
   */

  double _earthRadius;
  
  /**
   * @brief Latitude.
   */

  double _latitude;
  
  /**
   * @brief Longitude.
   */

  double _longitude;
  
  /**
   * @brief Altitude in km.
   */

  double _altitude;
  
  /**
   * @brief Roll.
   */

  double _roll;
  
  /**
   * @brief Pitch.
   */

  double _pitch;
  
  /**
   * @brief Drift.
   */

  double _drift;
  
  /**
   * @brief Heading.
   */

  double _heading;
  
  /**
   * @brief X in km.
   */

  double _x;
  
  /**
   * @brief Y.
   */

  double _y;
  
  /**
   * @brief Z.
   */

  double _z;
  
  /**
   * @brief Azimuth.
   */

  double _azimuth;
  
  /**
   * @brief Elevation.
   */

  double _elevation;
  
  /**
   * @brief Range.
   */

  double _range;
  
  /**
   * @brief Rotation angle.
   */

  double _rotationAngle;
  
  /**
   * @brief Tilt.
   */

  float _tilt;
  
  /**
   * @brief State.  Which bits are set.
   */

  int32_t _state;
  
  /**
   * @brief ID.
   */

  std::string _id;
  
  /**
   * @brief The point-in-space buffer for reading from/writing to disk.
   */

  mutable point_in_space_t _buffer;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Use the crackers() routine to swap the data in this structure.
   *
   * @param[in] srs      The source buffer.
   * @param[out] dst     The destination buffer.
   * @param[in] limit    ????
   */

  void _crack(char *srs, char *dst, int limit);
  
  
  /**
   * @brief Update the internal buffer to contain the current data values.
   */

  void _updateBuffer() const;


};

#endif
