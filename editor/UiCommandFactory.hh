#ifndef UiCommandFactory_HH
#define UiCommandFactory_HH

#include <UiCommand.hh>
#include <UiCommandToken.hh>

class UiCommandFactory
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Destructor.
   */

  virtual ~UiCommandFactory();


  /**
   * @brief Retrieve the singleton instance of this class.
   *
   * @return Returns a pointer to the instance.
   */

  static UiCommandFactory *getInstance();
  

  /**
   * @brief Create the for-each-ray command object from the given
   *        command line.
   *
   * @param[in] line     The command line.
   *
   * @return Returns a pointer to the command on success, 0 on failure.
   */

  ForEachRayCmd *createFerCommand(const std::string &line) const;
  
  
  /**
   * @brief Create the one-time-only command object from the given
   *        command line.
   *
   * @param[in] line     The command line.
   *
   * @return Returns a pointer to the command on success, 0 on failure.
   */

  OneTimeOnlyCmd *createOtoCommand(const std::string &line) const;
  
  
  /**
   * @brief Get the list of templates for the for-each-ray commands.
   *
   * @return Returns the list of templates.
   */

  std::vector< std::string > getFERTemplates() const
  {
    return _ferTemplates;
  }
  
  /**
   * @brief Get the list of templates for the other commands.
   *
   * @return Returns the list of templates.
   */

  std::vector< std::string > getOtherTemplates() const
  {
    return _otoTemplates;
  }
  

protected:

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief Singleton instance pointer.
   */

  static UiCommandFactory *_instance;
  

  /**
   * @brief List of syntactical sugar that should be removed from commands
   */

  std::vector< std::string > _synSugar;
  

  /**
   * @brief List of all possible for-each-ray commands.
   */

  std::vector< ForEachRayCmd* > _ferCmdList;
  
  /**
   * @brief List of all possible one-time-only commands.
   */

  std::vector< OneTimeOnlyCmd* > _otoCmdList;
  
  /**
   * @brief Templates for all of the for-each-ray commands.
   */

  std::vector< std::string > _ferTemplates;
  
  /**
   * @brief Templates for all of the other commands.
   */

  std::vector< std::string > _otoTemplates;
  

  ///////////////////////
  // Protected methods //
  ///////////////////////

  /**
   * @brief Constructor -- Protected since this is a singleton object.
   */

  UiCommandFactory();


  /**
   * @brief Find the given for-each-ray command in our command list and create
   *        a copy of that command object.
   *
   * @param[in] keyword       The command keyword.
   * @param[in] cmd_line      The full command line, used for error messages.
   *
   * @return Returns a pointer to the command object for this command.
   */

  UiCommand *_findFERCommand(const std::string &keyword,
			     const std::string &cmd_line) const;
  

  /**
   * @brief Find the given one-time-only command in our command list and create
   *        a copy of that command object.
   *
   * @param[in] keyword       The command keyword.
   * @param[in] cmd_line      The full command line, used for error messages.
   *
   * @return Returns a pointer to the command object for this command.
   */

  UiCommand *_findOTOCommand(const std::string &keyword,
			     const std::string &cmd_line) const;
  

  /**
   * @brief Parse the command text into tokens.
   *
   * @param[in] line    The command text.
   *
   * @return Returns the command tokens from the command text.
   */

  std::vector< UiCommandToken > _getCommandTokens(const std::string &line) const;

  /**
   * @brief Initialize the list of all commands.
   */

  void _initCommandList();
  

  /**
   * @brief Initialize the syntactical sugar vector.
   */

  void _initSynSugar();
  
};

#endif
