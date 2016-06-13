#ifndef SiteListCmd_HH
#define SiteListCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <OneTimeOnlyCmd.hh>

class SiteListCmd : public OneTimeOnlyCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  SiteListCmd();

  /**
   * @brief Destructor.
   */

  virtual ~SiteListCmd();

  virtual bool doIt() const;

  /**
   * @brief Clone this object.
   *
   * @return Returns a clone of this object.
   */

  virtual UiCommand *clone() const
  {
    return new SiteListCmd(*this);
  }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  mutable std::string _gaugeDir;

    
  ///////////////////////
  // Protected methods //
  ///////////////////////

};

#endif
