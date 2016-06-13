#ifndef FlagGlitchesCmd_HH
#define FlagGlitchesCmd_HH

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <ForEachRayCmd.hh>

class FlagGlitchesCmd : public ForEachRayCmd
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  FlagGlitchesCmd();

  /**
   * @brief Destructor.
   */

  virtual ~FlagGlitchesCmd();

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
    return new FlagGlitchesCmd(*this);
  }
  
protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  mutable int _queSize;
  mutable double *_que;
  mutable double *_qctr;


  ///////////////////////
  // Protected methods //
  ///////////////////////

};

#endif
