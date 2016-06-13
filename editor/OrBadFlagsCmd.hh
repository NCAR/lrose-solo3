#ifndef OrBadFlagsCmd_HH
#define OrBadFlagsCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <ForEachRayCmd.hh>

class OrBadFlagsCmd : public ForEachRayCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  OrBadFlagsCmd();

  /**
   * @brief Destructor.
   */

  virtual ~OrBadFlagsCmd();

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
    return new OrBadFlagsCmd(*this);
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
