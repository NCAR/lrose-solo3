#ifndef SoloState_HH
#define SoloState_HH

class SoloState
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Destructor.
   */

  virtual ~SoloState();

  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the instance.
   */

  static SoloState *getInstance();
  

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Set the halt flag.
   *
   * @param halt_flag    The new halt flag state.
   */

  inline void setHaltFlag(const bool halt_flag)
  {
    _haltFlag = halt_flag;
  }
  

  /**
   * @brief Is the halt flag set?
   *
   * @return Returns the halt flag.
   */

  inline bool isHalt() const
  {
    return _haltFlag;
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static SoloState *_instance;
  
  /**
   * @brief The halt flag.
   */

  bool _haltFlag;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Constructor -- Protected since this is a singleton object.
   */

  SoloState();

};

#endif
