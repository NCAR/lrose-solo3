#ifndef FileInfo_HH
#define FileInfo_HH

#include <string>

#include <DateTime.hh>

class FileInfo
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   *
   * @param[in] file_name     The file name.
   */

  FileInfo(const std::string &file_name);

  /**
   * @brief Destructor.
   */

  virtual ~FileInfo();


  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Get the file name.
   *
   * @return Returns the file name.
   */

  std::string getFileName() const
  {
    return _fileName;
  }
  
  /**
   * @brief Get the file time.
   *
   * @return Returns the file time.
   */

  DateTime getFileTime() const
  {
    return _fileTime;
  }
  
      
  ///////////////
  // Operators //
  ///////////////

  bool operator<(const FileInfo &other) const
  {
    return _fileName < other._fileName;
  }
  
  bool operator<=(const FileInfo &other) const
  {
    return _fileName <= other._fileName;
  }
  
  bool operator>(const FileInfo &other) const
  {
    return _fileName > other._fileName;
  }
  
  bool operator>=(const FileInfo &other) const
  {
    return _fileName >= other._fileName;
  }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The file name.
   */

  std::string _fileName;
  
  /**
   * @brief The file time as extracted from the file name.
   */

  DateTime _fileTime;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Extract the data time from the internal file name and use it to
   *        set the internal file time.
   */

  void _setTimeFromFileName();


};

#endif
