#ifndef GatesShiftedCmd_HH
#define GatesShiftedCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <OneTimeOnlyCmd.hh>

class GatesShiftedCmd : public OneTimeOnlyCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  GatesShiftedCmd();

  /**
   * @brief Destructor.
   */

  virtual ~GatesShiftedCmd();

  virtual bool doIt() const;

  /**
   * @brief Clone this object.
   *
   * @return Returns a clone of this object.
   */

  virtual UiCommand *clone() const
  {
    return new GatesShiftedCmd(*this);
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
