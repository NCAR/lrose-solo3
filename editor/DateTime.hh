#ifndef DateTime_HH
#define DateTime_HH

#include <string>
#include <cstdio>

class DateTime
{

public:

  //////////////////////
  // Public constants //
  //////////////////////

  /**
   * @brief Value indicating an invalid time.
   */

  static const double INVALID_TIME;
  

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  DateTime();

  /**
   * @brief Constructor.
   *
   * @param[in] time_stamp     The time stamp for the time.
   */

  DateTime(const double time_stamp);

  /**
   * @brief Constructor.
   *
   * @param[in] year     Year.
   * @param[in] month    Month.
   * @param[in] day      Day.
   * @param[in] hour     Hour.
   * @param[in] min      Minute.
   * @param[in] sec      Second.
   * @param[in] ms       Milliseconds.
   */

  DateTime(const int year, const int month, const int day,
	   const int hour, const int min, const int sec,
	   const int ms = 0);

  /**
   * @brief Constructor.
   *
   * @param[in] unix_time  Unix time.
   * @param[in] ms         Milliseconds.
   */

  DateTime(const time_t unix_time, const int ms);
  
  /**
   * @brief Destructor.
   */

  virtual ~DateTime();


  // NOTE: Replaces dd_crack_datime() calls

  /**
   * @brief Parse the given time string and set the values in the object
   *        appropriately.
   *
   * @param[in] time_string     The time string to parse.
   *
   * @return Returns true on success, false on failure.
   */

  bool parse(const std::string &time_string);
  
  // NOTE: Replaces dcdatime() calls

  /**
   * @brief Parse the given time string and set the values in the object
   *        appropriately.
   *
   * @param[in] time_string    The time string to parse.
   *
   * @return Returns true on success, false on failure.
   */

  bool parse2(const std::string &time_string);
  
  // NOTE: Replaces dts_print() calls.

  /**
   * @brief Convert the time value to a string.
   *
   * @return Returns the string representing this time value.
   */

  inline std::string toString() const
  {
    char buffer[32];

    sprintf(buffer, "%02d/%02d/%02d %02d:%02d:%02d.%03d",
	    _month, _day, _year, _hour, _minute, _second, _millisecond);

    return std::string(buffer);
  }
  
  // NOTE: Replaces se_string_time() calls.

  /**
   * @brief Convert the time value to a string.
   *
   * @return Returns the string representing this time value.
   */

  inline std::string toString2() const
  {
    char buffer[32];

    sprintf(buffer, "%02d/%02d/%02d:%02d:%02d:%02d.%03d",
	    _month, _day, _year - 1900, _hour, _minute, _second, _millisecond);

    return std::string(buffer);
  }
  

  ////////////////////
  // Access methods //
  ////////////////////

  /**
   * @brief Get the number of seconds into the day.
   *
   * @return Returns the number of seconds into the day.
   */

  inline double getDaySeconds() const
  {
    return _daySeconds;
  }
  

  // NOTE: Replaces d_time_stamp() calls.

  /**
   * @brief This routine expects year, month, day, and total seconds
   *        into the day to be set.  It will NOT use hour, min, sec, ms info
   *        in the object.  This routine gives a time similar, if not the same
   *        as, unix time but does not worry about time zones and daylight
   *        saving time.
   *
   * @return Returns the time stamp for this time if successful, INVALID_TIME
   *         otherwise.
   */

  double getTimeStamp() const
  {
    return _timeStamp;
  }
  
  
  // NOTE: Replaces d_unstamp_time() calls.
  /**
   * @brief Set the internal member values from the given time stamp value.
   *
   * @param[in] time_stamp    The time stamp value.
   */

  void setTimeStamp(const double time_stamp);
  
  /**
   * @brief Set the time values as indicated.  Leave the date values untouched.
   *
   * @param[in] hour          Hour.
   * @param[in] minute        Minute.
   * @param[in] second        Second.
   * @param[in] millisecond   Millisecond.
   */

  inline void setTime(const int hour = 0, const int minute = 0,
		      const int second = 0, const int millisecond = 0)
  {
    _hour = hour;
    _minute = minute;
    _second = second;
    _millisecond = millisecond;
    _daySeconds = calcSecs(_hour, _minute, _second, _millisecond);
  }
  

  ///////////////
  // Operators //
  ///////////////

  bool operator<(const DateTime &other) const
  {
    return _timeStamp < other._timeStamp;
  }
  
  bool operator<=(const DateTime &other) const
  {
    return _timeStamp <= other._timeStamp;
  }
  
  bool operator>(const DateTime &other) const
  {
    return _timeStamp > other._timeStamp;
  }
  
  bool operator>=(const DateTime &other) const
  {
    return _timeStamp >= other._timeStamp;
  }
  
  bool operator==(const DateTime &other) const
  {
    return _timeStamp == other._timeStamp;
  }
  
  bool operator!=(const DateTime &other) const
  {
    return _timeStamp != other._timeStamp;
  }
  

  ////////////////////
  // Static methods //
  ////////////////////

  /**
   * @brief Calculate the seconds into the day for the given time.
   *
   * @param[in] hour      The hour of the day.
   * @param[in] min       The minute of the day.
   * @param[in] sec       The second of the day.
   * @param[in] ms        The milliseconds of the day.
   *
   * @return Returns the seconds into the day for the given time.
   */

  static double calcSecs(const int hour, const int min, const int sec,
			 const int ms)
  {
    return (double)((hour * 3600) + (min * 60) + sec) + (ms * 0.001);
  }
  
  /**
   * @brief Parse the given time string and return the time in a double.
   *
   * @param[in] time_string    The time string to parse.
   *
   * @return Returns the time represented by the string on success, 0.0 on
   *         failure.
   */

  static double dtimeFromString(const std::string &time_string)
  {
    DateTime time_value;
    
    if (!time_value.parse(time_string))
      return 0.0;
  
    return time_value.getTimeStamp();
  }


  // NOTE: Replaces dd_relative_time() calls.

  /**
   * @brief Detect and extract relative time in seconds.
   *
   * @param[in] time_string    The relative time string.
   *
   * @return Returns the relative time in seconds on success, 0.0 on failure.
   */

  static double relativeTime(const std::string &time_string);
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  /**
   * @brief The reference year for date calculations.
   */

  static const int REFERENCE_YEAR;
  
  /**
   * @brief The maximum number of years we will process.
   */

  static const int MAX_YEARS;
  
  /**
   * @brief Maximum float value.
   */

  static const double DATE_MAX_FLOAT;
  
  /**
   * @brief The number of seconds in a day.
   */

  static const int SECONDS_PER_DAY;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Time stamp.
   */

  double _timeStamp;
  
  /**
   * @brief Day seconds.
   */

  double _daySeconds;
  
  /**
   * @brief Sub-second.
   */

  double _subSecond;
  
  /**
   * @brief Julian day.
   */

  int _julianDay;
  
  /**
   * @brief Year.
   */

  int _year;
  
  /**
   * @brief Month.
   */

  int _month;
  
  /**
   * @brief Day.
   */

  int _day;
  
  /**
   * @brief Hour.
   */

  int _hour;
  
  /**
   * @brief Minute.
   */

  int _minute;
  
  /**
   * @brief Second.
   */

  int _second;
  
  /**
   * @brief Millisecond.
   */

  int _millisecond;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Calculate the time stamp value from the object's year, etc.
   */

  void _calcTimeStamp();
  

  /**
   * @brief Initialize the static values for this class.  This method should
   *        be called only once during any running of the program.
   */

  void _initStatics();
  

  ////////////////////
  // Static members //
  ////////////////////

  /**
   * @brief Flag indicating whether the static values have been initialized.
   */

  static bool _staticsInitialized;

  /**
   * @brief The number of seconds in each month of a regular year.
   */

  static double _regYear[13];
  
  /**
   * @brief The number of seconds in each month of a leap year.
   */

  static double _leapYear[13];
  
  /**
   * @brief Accumulated seconds for each year we will process.  There will be
   *        MAX_YEARS+1 values in this array.
   */

  static double *_accumulatedYears;

  /**
   * @brief Accumulated seconds for each month we will process.  There will be
   *        MAX_YEARS+1 pointers in this array, each pointer pointing to either
   *        _regYear or _leapYear.
   */

  static double **_accumulatedMonths;
  
};

#endif
