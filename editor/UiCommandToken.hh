#ifndef UiCommandToken_HH
#define UiCommandToken_HH

#include <string>

class UiCommandToken
{

public:

/*
 * Token types.
 */

  typedef enum
  {
    UTT_END,          // End of the token list
    UTT_VALUE,        // Value parameter
    UTT_OTHER,        // Something else
    UTT_KW,           // Keyword token type
    UTT_PARTIAL,      // End of a partial command
    UTT_SYM,          // Non-subst sym

    UTT_INT,          // <integer> flag
    UTT_REAL,         // <float> flag
    UTT_WHERE,        // <where> flag
    UTT_FIELD         // <field> flag
  } token_type_t;
    

  ////////////////////
  // Public methods //
  ////////////////////

  /**
   * @brief Constructor.
   */

  UiCommandToken();

  /**
   * @brief Destructor.
   */

  virtual ~UiCommandToken();


  ////////////////////
  // Access methods //
  ////////////////////

  // set methods //

  /**
   * @brief Set the command token type.
   *
   * @param[in] token_type The command token type.
   */

  inline void setTokenType(const token_type_t token_type)
  {
    _tokenType = token_type;
  }
  
  /**
   * @brief Set the command text.
   *
   * @param[in] text  The command text.
   */

  inline void setCommandText(const std::string &text)
  {
    _commandText = text;
  }
  
  /**
   * @brief Set the integer parameter.
   *
   * @param[in] param The integer parameter.
   */

  inline void setIntParam(const int param)
  {
    _intParam = param;
  }
  
  /**
   * @brief Set the float parameter.
   *
   * @param[in] param The float parameter.
   */

  inline void setFloatParam(const float param)
  {
    _floatParam = param;
  }
  

  // get methods //

  /**
   * @brief Get the command token type.
   *
   * @return Returns the command token type.
   */

  inline token_type_t getTokenType() const
  {
    return _tokenType;
  }
  
  /**
   * @brief Get the command text.
   *
   * @return Returns the command text.
   */

  inline std::string getCommandText() const
  {
    return _commandText;
  }
  
  /**
   * @brief Get the integer parameter.
   *
   * @return Returns the integer parameter.
   */

  inline int getIntParam() const
  {
    return _intParam;
  }
  
  /**
   * @brief Get the float parameter.
   *
   * @return Returns the float parameter.
   */

  inline float getFloatParam() const
  {
    return _floatParam;
  }
  

protected:

  /////////////////////////
  // Protected constants //
  /////////////////////////

  static const int TOKEN_LEN = 48;
  
  ///////////////////////
  // Protected members //
  ///////////////////////

  /**
   * @brief The token type.
   */

  token_type_t _tokenType;

  /**
   * @brief The actual token text.
   */

  std::string _commandText;

  /**
   * @brief The integer parameter.
   */

  int _intParam;
  
  /**
   * @brief The float parameter.
   */

  float _floatParam;
  
  
};

#endif
