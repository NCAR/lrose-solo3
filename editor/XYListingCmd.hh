#ifndef XYListingCmd_HH
#define XYListingCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <HistogramMgr.hh>
#include <ForEachRayCmd.hh>

class XYListingCmd : public ForEachRayCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  XYListingCmd();

  /**
   * @brief Destructor.
   */

  virtual ~XYListingCmd();

#ifdef USE_RADX
  
  virtual bool doIt(const int frame_num, RadxRay &ray) const;

  virtual void finishUp() const
  {
    HistogramMgr::getInstance()->closeXYStream();
  }
  
#else

  virtual bool doIt() const;

#endif  
  
  /**
   * @brief Clone this object.
   *
   * @return Returns a clone of this object.
   */

  virtual UiCommand *clone() const
  {
    return new XYListingCmd(*this);
  }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  ///////////////////////
  // Protected methods //
  ///////////////////////

};

#endif
