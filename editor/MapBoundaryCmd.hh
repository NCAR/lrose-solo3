#ifndef MapBoundaryCmd_HH
#define MapBoundaryCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <OneTimeOnlyCmd.hh>

class MapBoundaryCmd : public OneTimeOnlyCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  MapBoundaryCmd();

  /**
   * @brief Destructor.
   */

  virtual ~MapBoundaryCmd();

  virtual bool doIt() const;

  /**
   * @brief Clone this object.
   *
   * @return Returns a clone of this object.
   */

  virtual UiCommand *clone() const
  {
    return new MapBoundaryCmd(*this);
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
