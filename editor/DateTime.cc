#include <iostream>
#include <stdlib.h>

#include "DateTime.hh"

// Define the static members

const int DateTime::REFERENCE_YEAR = 1970;
const int DateTime::MAX_YEARS = 100;
const double DateTime::DATE_MAX_FLOAT = 1.0e22;
const double DateTime::INVALID_TIME = -DATE_MAX_FLOAT;
const int DateTime::SECONDS_PER_DAY = 86400;

bool DateTime::_staticsInitialized = false;
double DateTime::_regYear[13];
double DateTime::_leapYear[13];
double *DateTime::_accumulatedYears = new double[MAX_YEARS+1];
double **DateTime::_accumulatedMonths = new double*[MAX_YEARS+1];

/*********************************************************************
 * Constructors
 */

DateTime::DateTime()
{
  // Initialize the static members

  _initStatics();
}


DateTime::DateTime(const double time_stamp)
{
  // Initialize the static members

  _initStatics();

  // Set the time stamp

  setTimeStamp(time_stamp);
}


DateTime::DateTime(const int year, const int month, const int day,
		   const int hour, const int min, const int sec, const int ms) :
  _year(year),
  _month(month),
  _day(day),
  _hour(hour),
  _minute(min),
  _second(sec),
  _millisecond(ms)
{
  _daySeconds = calcSecs(_hour, _minute, _second, _millisecond);
  _calcTimeStamp();
}


DateTime::DateTime(const time_t unix_time, const int ms) :
  _millisecond(ms)
{
  // Extract the time elements from the unix time

  struct tm *time_struct = gmtime(&unix_time);
  _year = time_struct->tm_year;
  _month = time_struct->tm_mon;
  _day = time_struct->tm_mday;
  _hour = time_struct->tm_hour;
  _minute = time_struct->tm_min;
  _second = time_struct->tm_sec;
  
  // Calculate the seconds in the day

  _daySeconds = calcSecs(_hour, _minute, _second, _millisecond);

  // Calculate the time stamp

  _calcTimeStamp();
}


/*********************************************************************
 * Destructor
 */

DateTime::~DateTime()
{
}


/*********************************************************************
 * parse()
 */

bool DateTime::parse(const std::string &time_string)
{
  // Make a local copy of the time string for working, removing any leading
  // white space

  std::string local_time_string = time_string;
  std::size_t nonblank_pos = local_time_string.find_first_not_of(" \t");
  if (nonblank_pos != std::string::npos)
    local_time_string = local_time_string.substr(nonblank_pos);
  
  // If the string begins with "+" or "-", then it is a relative time and
  // can't be parsed here.
  
  if (local_time_string[0] == '+' || local_time_string[0] == '-')
    return false;
  
  // This string can be of the form mon/dd/yy:hh:mm:ss.ms.  Look to see if
  // there are ms specified

  std::size_t dot_pos;
  int ms = 0;
  
  if ((dot_pos = local_time_string.rfind('.')) != std::string::npos)
    ms = (int)(atof(local_time_string.substr(dot_pos).c_str()) * 1000.0);
  
  // Try to parse the other time values using sscanf.  sscanf will put the
  // parsed values directly into our variables in most cases

  int yy;
  int mon;
  int dd;
  int hh = 0;
  int mm = 0;
  int ss = 0;
  
  int ii;
  
  if (sscanf(local_time_string.c_str(), "%d/%d/%d:%d:%d:%d",
	     &mon, &dd, &yy, &hh, &mm, &ss) == 6)
  {
    // Assume this string is of the form mon/dd/yy:hh:mm:ss
  }
  else if (sscanf(local_time_string.c_str(), "%d/%d/%d%d:%d:%d",
		  &mon, &dd, &yy, &hh, &mm, &ss) == 6)
  {
    // Assume this string is of the form mon/dd/yy hh:mm:ss
  }
  else if (sscanf(local_time_string.c_str(), "%d/%d/%d:%d:%d",
		  &mon, &dd, &yy, &hh, &mm) == 5)
  {
    // Assume this string is of the form mon/dd/yy:hh:mm
  }
  else if (sscanf(local_time_string.c_str(), "%d/%d/%d%d:%d",
		  &mon, &dd, &yy, &hh, &mm) == 5)
  {
    // Assume this string is of the form mon/dd/yy hh:mm
  }
  else if (sscanf(local_time_string.c_str(), "%d/%d/%d:%d",
		  &mon, &dd, &yy, &hh) == 4)
  {
    // Assume this string is of the form mon/dd/yy:hh
  }
  else if (sscanf(local_time_string.c_str(), "%d/%d/%d%d",
		  &mon, &dd, &yy, &hh) == 4)
  {
    // Assume this string is of the form mon/dd/yy hh
  }
  else if (sscanf(local_time_string.c_str(), "%d:%d:%d", &hh, &mm, &ss) == 3)
  {
    // Assume this string is of the form hh:mm:ss

    yy = 0;
    mon = 0;
    dd = 0;
  }
  else if (sscanf(local_time_string.c_str(), "%d:%d", &hh, &mm) == 2)
  {
    // Assume this string is of the form hh:mm

    yy = 0;
    mon = 0;
    dd = 0;
  }
  else if (sscanf(local_time_string.c_str(), "%d", &ii) == 1)
  {
    if (ii == 0)
      return false;
    
    yy = 0;
    mon = 0;
    dd = 0;

    if (local_time_string.compare(0, 4, "0000") == 0)
    {
      // Has 4 leading zeroes

      ss = ii;
      ii = 0;
      hh = 0;
      mm = 0;
    }
    else if (local_time_string.compare(0, 2, "00") == 0)
    {
      // Has 2 leading zeroes

      hh = 0;
      mm = ii / 100;
      ss = ii % 100;
    }
    else
    {
      hh = ii % 100;
      ii /= 100;
      if (ii != 0)
      {
	mm = hh;
	hh = ii % 100;
	ii /= 100;
      }
      if (ii != 0)
      {
	ss = mm;
	mm = hh;
	hh = ii % 100;
	ii /= 100;
      }
    }

    if (ii != 0 || hh > 23 || mm > 59 || ss > 59)
      return false;
    
  }
  else
  {
    return false;
  }

  // Set the structure return values

  if (yy > 0)
    _year = yy > 1900 ? yy : yy + 1900;
  if (mon > 0)
    _month = mon;
  if (dd > 0)
    _day = dd;
  
  _daySeconds = calcSecs(hh, mm, ss, ms);
  _calcTimeStamp();
  
  return true;
}


/*********************************************************************
 * parse2()
 */

//int dcdatime (const char *str, int n, short *yy, short *mon, short *dd, short *hh, short *mm, short *ss, short *ms)
bool DateTime::parse2(const std::string &time_string)
{
  _year = 0;
  _month = 0;
  _day = 0;
  _hour = 0;
  _minute = 0;
  _second = 0;
  _millisecond = 0;
  _daySeconds = 0.0;
  
  std::string time_string_local = time_string;
  
  // If the string contains a '/' character, assume it contains date and time
  // of the form mm/dd/yy:hh:mm:ss.ms

  std::size_t delim_pos = time_string_local.find('/');
  if (delim_pos != std::string::npos)
  {
    _month = atoi(time_string_local.substr(0, delim_pos-1).c_str());
    time_string_local = time_string_local.substr(delim_pos+1);
    
    if ((delim_pos = time_string_local.find('/')) == std::string::npos)
      return false;

    _day = atoi(time_string_local.substr(0, delim_pos-1).c_str());
    time_string_local = time_string_local.substr(delim_pos+1);
    
    if ((delim_pos = time_string_local.find(':')) != std::string::npos)
    {
      _year = atoi(time_string_local.substr(0, delim_pos-1).c_str());
      time_string_local = time_string_local.substr(delim_pos+1);
    }
    else if (time_string_local.size() != 0)
    {
      _year = atoi(time_string_local.c_str());
      _year = _year > 1900 ? _year : _year + 1900;
      _calcTimeStamp();
      return true;
    }
    else
    {
      return false;
    }
  }

  // Now get hh, mm, ss, ms

  if ((delim_pos = time_string_local.find(':')) != std::string::npos)
  {
    _hour = atoi(time_string_local.substr(0, delim_pos-1).c_str());
    time_string_local = time_string_local.substr(delim_pos+1);
  }
  else
  {
    if(time_string_local.size() != 0)
      _hour = atoi(time_string_local.c_str());
    _year = _year > 1900 ? _year : _year + 1900;
    _daySeconds = calcSecs(_hour, _minute, _second, _millisecond);
    _calcTimeStamp();
    return true;
  }    

  if ((delim_pos = time_string_local.find(':')) != std::string::npos)
  {
    _minute = atoi(time_string_local.substr(0, delim_pos-1).c_str());
    time_string_local = time_string_local.substr(delim_pos+1);
  }
  else
  {
    if (time_string_local.size() != 0)
      _minute = atoi(time_string_local.c_str());
    _year = _year > 1900 ? _year : _year + 1900;
    _daySeconds = calcSecs(_hour, _minute, _second, _millisecond);
    _calcTimeStamp();
    return true;
  }    

  if ((delim_pos = time_string_local.find('.'))!= std::string::npos)
  {
    _second = atoi(time_string_local.substr(0, delim_pos-1).c_str());
    time_string_local = time_string_local.substr(delim_pos+1);
  }
  else
  {
    if (time_string_local.size() != 0)
      _second = atoi(time_string_local.c_str());
    _year = _year > 1900 ? _year : _year + 1900;
    _daySeconds = calcSecs(_hour, _minute, _second, _millisecond);
    _calcTimeStamp();
    return true;
  }

  if (time_string_local.size() != 0)
    _millisecond = atoi(time_string_local.c_str());

  _year = _year > 1900 ? _year : _year + 1900;
  _daySeconds = calcSecs(_hour, _minute, _second, _millisecond);
  _calcTimeStamp();
      
  return true;
}


/*********************************************************************
 * relativeTime()
 */

double DateTime::relativeTime(const std::string &time_string)
{
  if (time_string == "")
    return 0.0;

  // If the string contains a slash, it's probably a date

  if (time_string.find('/') != std::string::npos)
    return 0.0;

  // Create a copy of the time string for doing our work

  std::string local_time_string = time_string;
  
  // Relative implies a number preceeded by a + or - and optionally followed
  // by "h", "m", "s", or not implying seconds.
  //
  // First skip over leading blanks or tabs

  std::size_t non_blank_pos =
    local_time_string.find_first_not_of(" \t");

  if (non_blank_pos == std::string::npos)
    return 0.0;
  
  local_time_string = local_time_string.substr(non_blank_pos);
  
  // The time string must be preceded by "+" or "-", which determines the
  // sign of the return value.  If it doesn't, return 0.0.

  double sign;
  
  if (local_time_string[0] == '+')
    sign = 1.0;
  else if (local_time_string[0] != '-')
    sign = -1.0;
  else
    return 0.0;
  
  // Skip past the sign character

  local_time_string = local_time_string.substr(1);

  // Remove other blanks and tabs from the string

  std::size_t blank_pos;
  
  while ((blank_pos =
	  local_time_string.find_first_of(" \t")) != std::string::npos)
    local_time_string.erase(blank_pos, 1);
  
  if (local_time_string == "")
    return 0.0;
  
  // Pull out the units of the relative time, which are kept in the last
  // character of the string.  If the last character is numeric, then store
  // a NULL in hms and handle it as seconds.

  char hms = local_time_string[local_time_string.size() - 1];

  if (isalpha(hms))
  {
    if (isupper(hms))
      hms = tolower(hms);

    hms = (hms == 'h' || hms == 'm' || hms == 's') ? hms : '\0';
    if (hms == '\0')
      return 0.0;

    // Remove the unit indicator from the string

    local_time_string =
      local_time_string.substr(0, local_time_string.size() - 1);
  }
  else
  {
    hms = '\0';
  }
  
  if (local_time_string == "")
    return 0.0;
  
  // Now parse the value and calculate the return.  If there are any problems
  // in the parsing, we'll end up returning 0.0.

  double return_time = 0.0;
  float f_val;
  
  if (local_time_string.find(':') != std::string::npos)
  {
    // Try and decode it as hh:mm:ss.ms

    DateTime time_value;
    
    if (time_value.parse(local_time_string))
      return_time = sign * time_value.getDaySeconds();
  }
  else if (sscanf(local_time_string.c_str(), "%f", &f_val) == 1)
  {
    if (hms == 'h')
      return_time = sign * f_val * 3600.0;
    else if (hms == 'm')
      return_time = sign * f_val * 60.0;
    else
      // Absence of a character also implies seconds
      return_time = sign * f_val;
  }

  return return_time;
}


/*********************************************************************
 * setTimeStamp()
 */

void DateTime::setTimeStamp(const double time_stamp)
{
  _timeStamp = time_stamp;
  
  if (time_stamp < 0 || time_stamp > _accumulatedYears[MAX_YEARS])
  {
    _year= -1;       
    _month= -1;      
    _day= -1;        
    _hour= -1;       
    _minute= -1;     
    _second= -1;     
    _millisecond= -1;
    return;
  }

  double time_stamp_local = time_stamp;
  
  // If you're at exactly midnight the hours will come out as 24...sigh

  double fudge = 0.0;
  
  if ((time_stamp_local - (int32_t)time_stamp_local) == 0)
    if (((int32_t)time_stamp_local % SECONDS_PER_DAY) == 0)
      fudge = 1.0;

  time_stamp_local += fudge;
  
  int i = 1992 - REFERENCE_YEAR;
  
  if (time_stamp_local <= (double)_accumulatedYears[i])
    i = 0;

  for (; i <= MAX_YEARS; i++ )
  {
    if ((double)_accumulatedYears[i] > time_stamp_local)
      break;
  }

  double *mp = _accumulatedMonths[--i];
  _year = REFERENCE_YEAR +i;
  time_stamp_local -= _accumulatedYears[i];
  
  _julianDay = (int)(time_stamp_local * SECONDS_PER_DAY + 1.0);

  i = mp[5] < time_stamp_local ? 5 : 0;
  for (; i < 13; i++)
  {
    if (mp[i] > time_stamp_local)
      break;
  }

  _month = i;
  time_stamp_local -= mp[i-1];

  _day = (int)(time_stamp_local / (double)SECONDS_PER_DAY);
  time_stamp_local -= (double)((_day++) * SECONDS_PER_DAY);
  time_stamp_local -= fudge;

  _hour = (int)(time_stamp_local / 3600.0);
  time_stamp_local -= 3600.0 * _hour;

  _minute = (int)(time_stamp_local / 60.0);
  time_stamp_local -= 60.0 * _minute;

  _second = (int)time_stamp_local;
  time_stamp_local -= (double)_second;

  _millisecond = (int)(1000.0 * time_stamp_local + 0.5);
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _calcTimeStamp()
 */

void DateTime::_calcTimeStamp()
{
  if (_year < REFERENCE_YEAR || _year >= REFERENCE_YEAR + MAX_YEARS)
  {
    _timeStamp = INVALID_TIME;
    return;
  }
  
  int year_index = _year - REFERENCE_YEAR;
  _subSecond = _daySeconds - (int)_daySeconds;
  
  int month_index = _month > 0 ? _month - 1 : 0;
  int day_index = _day > 0 ? _day - 1 : 0;

  _timeStamp = _accumulatedYears[year_index] +
    _accumulatedMonths[year_index][month_index] + 
    (day_index * SECONDS_PER_DAY) + _daySeconds;
}


/*********************************************************************
 * _initStatics()
 */

void DateTime::_initStatics()
{
  if (_staticsInitialized)
    return;
  
  // Number of seconds in months of different lengths

  int t28 = 2419200;
  int t29 = 2505600;
  int t30 = 2592000;
  int t31 = 2678400;
    
  // Brute force seconds per month initialization

  int i = 0;
  _regYear[i] = _leapYear[i] = 0;
  i++;			// jan
  _regYear[i] = _regYear[i-1] + t31;
  _leapYear[i] = _leapYear[i-1] + t31;
  i++;			// feb
  _regYear[i] = _regYear[i-1] + t28;
  _leapYear[i] = _leapYear[i-1] + t29;
  i++;			// mar
  _regYear[i] = _regYear[i-1] + t31;
  _leapYear[i] = _leapYear[i-1] + t31;
  i++;			// apr
  _regYear[i] = _regYear[i-1] + t30;
  _leapYear[i] = _leapYear[i-1] + t30;
  i++;			// may
  _regYear[i] = _regYear[i-1] + t31;
  _leapYear[i] = _leapYear[i-1] + t31;
  i++;			// jun
  _regYear[i] = _regYear[i-1] + t30;
  _leapYear[i] = _leapYear[i-1] + t30;
  i++;			// jul
  _regYear[i] = _regYear[i-1] + t31;
  _leapYear[i] = _leapYear[i-1] + t31;
  i++;			// aug
  _regYear[i] = _regYear[i-1] + t31;
  _leapYear[i] = _leapYear[i-1] + t31;
  i++;			// sep
  _regYear[i] = _regYear[i-1] + t30;
  _leapYear[i] = _leapYear[i-1] + t30;
  i++;			// oct
  _regYear[i] = _regYear[i-1] + t31;
  _leapYear[i] = _leapYear[i-1] + t31;
  i++;			// nov
  _regYear[i] = _regYear[i-1] + t30;
  _leapYear[i] = _leapYear[i-1] + t30;
  i++;			// dec
  _regYear[i] = _regYear[i-1] + t31;
  _leapYear[i] = _leapYear[i-1] + t31;
    
  _accumulatedYears[0] = 0;
    
  int yy;
  
  for (i = 0, yy = REFERENCE_YEAR; i <= MAX_YEARS; i++, yy++ )
  {
    _accumulatedMonths[i] = yy % 4 ? _regYear : _leapYear;

    if (i != 0)
    {
      _accumulatedYears[i] =
	_accumulatedYears[i-1] + _accumulatedMonths[i-1][12];
    }
  }

  _staticsInitialized = true;
  
}
