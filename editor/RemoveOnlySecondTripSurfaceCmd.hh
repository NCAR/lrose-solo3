#ifndef RemoveOnlySecondTripSurfaceCmd_HH
#define RemoveOnlySecondTripSurfaceCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <RemoveSurfaceCmd.hh>

class RemoveOnlySecondTripSurfaceCmd : public RemoveSurfaceCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  RemoveOnlySecondTripSurfaceCmd();

  /**
   * @brief Destructor.
   */

  virtual ~RemoveOnlySecondTripSurfaceCmd();

#ifdef USE_RADX
  
  virtual bool doIt(const int frame_num, RadxRay &ray) const;

#endif  
  
  /**
   * @brief Clone this object.
   *
   * @return Returns a clone of this object.
   */

  virtual UiCommand *clone() const
  {
    return new RemoveOnlySecondTripSurfaceCmd(*this);
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
