#include <stdlib.h>
#include <string>
#include <vector>

#include <DateTime.hh>
#include <se_utils.hh>

#include "FileInfo.hh"

/*********************************************************************
 * Constructors
 */

FileInfo::FileInfo(const std::string &file_name) :
  _fileName(file_name)
{
  _setTimeFromFileName();
}


/*********************************************************************
 * Destructor
 */

FileInfo::~FileInfo()
{
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

/*********************************************************************
 * _setTimeFromFileName()
 */

void FileInfo::_setTimeFromFileName()
{
  // Separate the file name into tokens

  std::vector< std::string > tokens;
  tokenize(_fileName, tokens, ".");
  
  // Parse the time.  The last 10 characters are MMDDHHMMSS.  The first
  // characters are the year, which can be specified explicitly (e.g. "2012")
  // or as the number of years since 1900 (e.g. "112")

  std::string time_string = tokens[1];
  std::string day_time_string = time_string.substr(time_string.size()-10);
  std::string year_string = time_string.substr(0, time_string.size()-10);
  
  int month, day, hr, min, sec;

  sscanf(day_time_string.c_str(), "%2d%2d%2d%2d%2d",
	 &month, &day, &hr, &min, &sec);

  int year = atoi(year_string.c_str());
  if (year < 1900)
    year += 1900;
  
  _fileTime = DateTime(year, month, day, hr, min, sec);
}
