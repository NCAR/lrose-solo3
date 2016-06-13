#ifndef EarthRadiusCalculator_HH
#define EarthRadiusCalculator_HH

class EarthRadiusCalculator
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Destructor.
   */

  virtual ~EarthRadiusCalculator();


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the instance.
   */

  static EarthRadiusCalculator *getInstance();
  

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Get the earth's radius at the given latitude.
   *
   * @param[in] lat  The desired latitude.
   *
   * @return Returns the earth's radius in kilometers.
   */

  double getRadius(const double lat);
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief The major axis of the earth in meters.
   */

  static const double MAJOR_AXIS_M;

  /**
   * @brief The minor axis of the earth in meters.
   */

  static const double MINOR_AXIS_M;


  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static EarthRadiusCalculator *_instance;
  
  /**
   * @brief Buffer containing the earth radius value for every 5 degrees of
   *        latitude.
   */

  double _radiusList[20];
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Constructor -- Protected since this is a singleton object.
   */

  EarthRadiusCalculator();


  /**
   * @brief Calculate the list of earth radius values.
   */

  void _calcRadiusList();
  

};

#endif
