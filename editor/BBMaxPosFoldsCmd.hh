#ifndef BBMaxPosFoldsCmd_HH
#define BBMaxPosFoldsCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <OneTimeOnlyCmd.hh>

class BBMaxPosFoldsCmd : public OneTimeOnlyCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  BBMaxPosFoldsCmd();

  /**
   * @brief Destructor.
   */

  virtual ~BBMaxPosFoldsCmd();

  virtual bool doIt() const;

  /**
   * @brief Clone this object.
   *
   * @return Returns a clone of this object.
   */

  virtual UiCommand *clone() const
  {
    return new BBMaxPosFoldsCmd(*this);
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
