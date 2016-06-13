#ifndef FixVortexVelocitiesCmd_HH
#define FixVortexVelocitiesCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <ForEachRayCmd.hh>

class FixVortexVelocitiesCmd : public ForEachRayCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  FixVortexVelocitiesCmd();

  /**
   * @brief Destructor.
   */

  virtual ~FixVortexVelocitiesCmd();

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
    return new FixVortexVelocitiesCmd(*this);
  }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  mutable double _vConsts[16];
  mutable int _levelBias;
  mutable double _rcpHalfNyqL;
  
  ///////////////////////
  // Protected methods //
  ///////////////////////

};

#endif
