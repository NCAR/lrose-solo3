#ifndef UiCommand_HH
#define UiCommand_HH

#include <iostream>
#include <string>
#include <vector>

#include <UiCommandToken.hh>

class UiCommand
{

public:

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  UiCommand();

  /**
   * @brief Constructor.
   */

  UiCommand(const std::string &keyword,
	    const std::string &cmd_template);

  /**
   * @brief Destructor.
   */

  virtual ~UiCommand();

  /**
   * @brief Clone this object.
   *
   * @return Returns a clone of this object.
   */

  virtual UiCommand *clone() const = 0;
  

  ////////////////////
  // Access methods //
  ////////////////////

  // set methods //

  /**
   * @brief Set the one-time-only flag.
   *
   * @param[in] one_time_only   The new one-time-only flag value.
   */

  inline void setOneTimeOnly(const bool one_time_only = true)
  {
    _oneTimeOnly = one_time_only;
  }
  
  /**
   * @brief Set the number of extra tokens in the command.
   *
   * @param[in] num_tokens   The number of extra tokens.
   */

  inline void setNumExtraTokens(const int num_tokens)
  {
    _extraTokens = num_tokens;
  }
  
  /**
   * @brief Set the command tokens.
   *
   * @param[in] tokens    The command tokens.
   */

  inline void setCommandTokens(const std::vector< UiCommandToken > &tokens)
  {
    _cmdTokens = tokens;
  }
  

  /**
   * @brief Set the command template tokens.
   *
   * @return Returns true if successful, false otherwise.
   */

  bool setCommandTemplateTokens();
  
  /**
   * @brief Set the error message.
   *
   * @param[in] message   The error messsage.
   */

  inline void setErrorMsg(const std::string &message)
  {
    _errorMsg = message;
  }
  
  /**
   * @brief Set the keyword associated with this command.
   *
   * @param[in] keyword  The keyword associated with this command.
   */

  inline void setKeyword(const std::string &keyword)
  {
    _keyword = keyword;
  }
  
  /**
   * @brief Set the command line.
   *
   * @param[in] cmd_string    The command line.
   */

  inline void setCommandLine(const std::string &cmd_string)
  {
    _cmdLine = cmd_string;
  }
  

  // get methods //

  /**
   * @brief Get the number of tokens in the command.
   *
   * @return Returns the number of tokens.
   */

  inline int getNumTokens() const
  {
    return _numTokens;
  }
  
  /**
   * @brief Get the number of extra tokens in the command.
   *
   * @return Returns the number of extra tokens.
   */

  inline int getNumExtraTokens() const
  {
    return _extraTokens;
  }
  
  /**
   * @brief Get the error message.
   *
   * @return Returns the error messsage.
   */

  inline std::string getErrorMsg() const
  {
    return _errorMsg;
  }
  
  /**
   * @brief Get the keyword associated with this command.
   *
   * @return Returns the keyword associated with this command.
   */

  inline std::string getKeyword() const
  {
    return _keyword;
  }
  
  /**
   * @brief Get the command template.
   *
   * @return Returns the command template.
   */

  inline std::string getTemplate() const
  {
    return _cmdTemplate;
  }
  

protected:

  /////////////////////
  // Protected types //
  /////////////////////

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const int MAX_TOKENS = 66;
  static const int MAX_TOKEN_LEN = 48;
  static const int MAX_LINE_LEN = 128;
  static const int ERROR_MESSAGE_LEN = 512;
  static const int COMMAND_LEN = 128;
  

  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The number of tokens in the command.
   */

  int _numTokens;

  /**
   * @brief Flag indicating that this is a one-time-only command.
   */

  // NOTE: This value is set, but never used.  Can probably remove it.

  bool _oneTimeOnly;

  /**
   * @brief The number of extra tokens in the command.
   */

  int _extraTokens;

  /**
   * @brief The error message associated with the command.
   */

  std::string _errorMsg;

  /**
   * @brief The keyword associated with the command.
   */

  std::string _keyword;

  /**
   * @brief The command line entered by the user for this command.
   */

  std::string _cmdLine;

  /**
   * @brief The command template.
   */

  std::string _cmdTemplate;

  /**
   * @brief The list of command tokens.
   */

  std::vector< UiCommandToken > _cmdTokens;


  ///////////////////////
  // Protected methods //
  ///////////////////////

};

#endif
