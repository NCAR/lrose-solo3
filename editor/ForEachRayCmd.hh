#ifndef ForEachRayCmd_HH
#define ForEachRayCmd_HH

#include <iostream>
#include <string>
#include <vector>

#ifdef USE_RADX
#include <Radx/RadxRay.hh>
#endif

#include <UiCommand.hh>

class ForEachRayCmd : public UiCommand
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  ForEachRayCmd();

  /**
   * @brief Constructor.
   */

  ForEachRayCmd(const std::string &keyword,
		const std::string &cmd_template);

  /**
   * @brief Destructor.
   */

  virtual ~ForEachRayCmd();

  /**
   * @brief Execute the command.  Note that this is called for each ray
   *        individually.
   */

#ifdef USE_RADX
  
  virtual bool doIt(const int frame_num, RadxRay &ray) const = 0;

  virtual void finishUp() const
  {
    // Do nothing by default
  }
  
#else

  virtual bool doIt() const = 0;

#endif  

};

#endif
