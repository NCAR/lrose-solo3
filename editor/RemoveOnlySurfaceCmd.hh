#ifndef RemoveOnlySurfaceCmd_HH
#define RemoveOnlySurfaceCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <RemoveSurfaceCmd.hh>

class RemoveOnlySurfaceCmd : public RemoveSurfaceCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  RemoveOnlySurfaceCmd();

  /**
   * @brief Destructor.
   */

  virtual ~RemoveOnlySurfaceCmd();

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
    return new RemoveOnlySurfaceCmd(*this);
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
