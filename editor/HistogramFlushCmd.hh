#ifndef HistogramFlushCmd_HH
#define HistogramFlushCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <OneTimeOnlyCmd.hh>

class HistogramFlushCmd : public OneTimeOnlyCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  HistogramFlushCmd();

  /**
   * @brief Destructor.
   */

  virtual ~HistogramFlushCmd();

  virtual bool doIt() const;

  /**
   * @brief Clone this object.
   *
   * @return Returns a clone of this object.
   */

  virtual UiCommand *clone() const
  {
    return new HistogramFlushCmd(*this);
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
