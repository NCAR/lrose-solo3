#ifndef ShowSiteValuesCmd_HH
#define ShowSiteValuesCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <OneTimeOnlyCmd.hh>

class ShowSiteValuesCmd : public OneTimeOnlyCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  ShowSiteValuesCmd();

  /**
   * @brief Destructor.
   */

  virtual ~ShowSiteValuesCmd();

  virtual bool doIt() const;

  /**
   * @brief Clone this object.
   *
   * @return Returns a clone of this object.
   */

  virtual UiCommand *clone() const
  {
    return new ShowSiteValuesCmd(*this);
  }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  ///////////////////////
  // Protected methods //
  ///////////////////////

  int _listZmapValues(const std::string &fname,
		      const int frame) const;
  

};

#endif
