#ifndef DeglitchMinGatesCmd_HH
#define DeglitchMinGatesCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <running_avg_que.hh>
#include <OneTimeOnlyCmd.hh>

class DeglitchMinGatesCmd : public OneTimeOnlyCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  DeglitchMinGatesCmd();

  /**
   * @brief Destructor.
   */

  virtual ~DeglitchMinGatesCmd();

  virtual bool doIt() const;

  /**
   * @brief Clone this object.
   *
   * @return Returns a clone of this object.
   */

  virtual UiCommand *clone() const
  {
    return new DeglitchMinGatesCmd(*this);
  }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Running average queue.
   */

  mutable struct running_avg_que *_raq0;
  
  /**
   * @brief Last good velocity value.
   */

  mutable double _lastGoodV0;


  ///////////////////////
  // Protected methods //
  ///////////////////////

};

#endif
