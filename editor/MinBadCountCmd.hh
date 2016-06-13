#ifndef MinBadCountCmd_HH
#define MinBadCountCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <OneTimeOnlyCmd.hh>

class MinBadCountCmd : public OneTimeOnlyCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  MinBadCountCmd();

  /**
   * @brief Destructor.
   */

  virtual ~MinBadCountCmd();

  virtual bool doIt() const;

  /**
   * @brief Clone this object.
   *
   * @return Returns a clone of this object.
   */

  virtual UiCommand *clone() const
  {
    return new MinBadCountCmd(*this);
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
