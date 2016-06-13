#ifndef BBUnfoldingCmd_HH
#define BBUnfoldingCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <running_avg_que.hh>
#include <ForEachRayCmd.hh>

class BBUnfoldingCmd : public ForEachRayCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  BBUnfoldingCmd();

  /**
   * @brief Destructor.
   */

  virtual ~BBUnfoldingCmd();

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
    return new BBUnfoldingCmd(*this);
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
