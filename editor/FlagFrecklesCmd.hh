#ifndef FlagFrecklesCmd_HH
#define FlagFrecklesCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <running_avg_que.hh>

#include <ForEachRayCmd.hh>

class FlagFrecklesCmd : public ForEachRayCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  FlagFrecklesCmd();

  /**
   * @brief Destructor.
   */

  virtual ~FlagFrecklesCmd();

#ifdef USE_RADX
  
  virtual bool doIt(const int frame_num, RadxRay &ray) const;

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
    return new FlagFrecklesCmd(*this);
  }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  mutable struct running_avg_que *_raq0;
  mutable struct running_avg_que *_raq1;

  ///////////////////////
  // Protected methods //
  ///////////////////////

};

#endif
