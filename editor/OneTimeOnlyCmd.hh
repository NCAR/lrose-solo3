#ifndef OneTimeOnlyCmd_HH
#define OneTimeOnlyCmd_HH

#include <iostream>
#include <string>
#include <vector>

#include <UiCommand.hh>

class OneTimeOnlyCmd : public UiCommand
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  OneTimeOnlyCmd();

  /**
   * @brief Constructor.
   */

  OneTimeOnlyCmd(const std::string &keyword,
		 const std::string &cmd_template);

  /**
   * @brief Destructor.
   */

  virtual ~OneTimeOnlyCmd();

  /**
   * @brief Execute the command.
   */

  virtual bool doIt() const = 0;


};

#endif
