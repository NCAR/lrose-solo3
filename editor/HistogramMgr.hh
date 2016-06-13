#ifndef HistogramMgr_HH
#define HistogramMgr_HH

#include <cstdio>
#include <string>

class HistogramMgr
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Destructor.
   */

  virtual ~HistogramMgr();


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the instance.
   */

  static HistogramMgr *getInstance();
  

  ////////////////////
  // Public methods //
  ////////////////////

  void histoOutput();
  
  /**
   * @brief Initialize the area * value accumulator.
   */

  inline void initAreaXValue()
  {
    _areaXval = 0.0;
  }
  
  /**
   * @brief Add to the area * value accumulator.
   *
   * @param[in] areaXval    The area*value value to add.
   */

  inline void addAreaXValue(const double areaXval)
  {
    _areaXval += areaXval;
  }
  
  /**
   * @brief Get the area * value accumulation.
   *
   * @return Returns the total area*value.
   */

  inline double getAreaXValue() const
  {
    return _areaXval;
  }
  
  /**
   * @brief Add the given values to the X/Y stream.
   *
   * @param[in] x    The X value.
   * @param[in] y    The Y value.
   */

  inline void addToXYStream(const double x, const double y)
  {
    if (_xyStream != 0)
      fprintf(_xyStream, "%.2f %.2f\n", x, y);
  }

  /**
   * @brief Open the X/Y stream using the indicated file.
   *
   * @param[in] file_path    The file path for the X/Y stream.
   *
   * @return Returns true on success, false on failure.
   */

  inline bool openXYStream(const std::string &file_path)
  {
    // If we have a stream open, close it

    if (_xyStream != 0)
      fclose(_xyStream);
    
    // Now open the new stream

    if ((_xyStream = fopen(file_path.c_str(), "w")) == 0)
      return false;

    return true;
  }
  
  /**
   * @brief Close the X/Y stream.
   */

  inline void closeXYStream()
  {
    if (_xyStream != 0)
      fclose(_xyStream);

    _xyStream = 0;
  }
  
  /**
   * @brief Close the histogram stream.
   */

  inline void closeHistogramStream()
  {
    if (_histoStream != 0)
      fclose(_histoStream);

    _xyStream = 0;
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static HistogramMgr *_instance;
  
  /**
   * @brief The accumulated area * value.
   */

  double _areaXval;
  
  /**
   * @brief The histogram output stream.
   */

  FILE *_histoStream;
  
  /**
   * @brief The X/Y output stream.
   */

  FILE *_xyStream;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Constructor -- Protected since this is a singleton object.
   */

  HistogramMgr();


};

#endif
