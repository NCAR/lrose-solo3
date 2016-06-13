#include <algorithm>

#include <se_utils.hh>

#include "UiCommand.hh"


/*********************************************************************
 * Constructors
 */

UiCommand::UiCommand() :
  _numTokens(0),
  _oneTimeOnly(false),
  _extraTokens(0),
  _errorMsg(""),
  _keyword(""),
  _cmdLine(""),
  _cmdTemplate("")
{
}


UiCommand::UiCommand(const std::string &keyword,
		     const std::string &cmd_template) :
  _numTokens(0),
  _oneTimeOnly(false),
  _extraTokens(0),
  _errorMsg(""),
  _keyword(keyword),
  _cmdLine(""),
  _cmdTemplate(cmd_template)
{
}


/*********************************************************************
 * Destructor
 */

UiCommand::~UiCommand()
{
}


/*********************************************************************
 * setCommandTemplateTokens()
 */

bool UiCommand::setCommandTemplateTokens()
{
  // Isolate the variable symbols in the command template

  char str[256];
  memset(str, 0, 256);
  char *str_ptr = str;
  char *sptrs[32];
  int num_template_tokens = 0;
  
  char cmd_template[_cmdTemplate.size() + 1];
  strcpy(cmd_template, _cmdTemplate.c_str());
  
  bool inside_variable = false;
    
  for (char *template_ptr = cmd_template; *template_ptr != '\0';
       template_ptr++)
  {
    if (*template_ptr == ' ')
      continue;
    
    if (*template_ptr == '<')
    {
      inside_variable = true;
      sptrs[num_template_tokens++] = str_ptr;
    }

    if (inside_variable)
    {
      *str_ptr = *template_ptr;
      str_ptr++;
    }
    
    if (*template_ptr == '>')
    {
      inside_variable = false;
      *str_ptr = '\0';
      str_ptr++;
    }
  } /* endfor - template_ptr */
  
  // At this point, str contains a list of all of the variables in the command
  // template and sptrs points to the beginning of each of these variables
  // in str.

  // Initialize the error message

  std::string error_message;
  
  error_message =  _cmdLine;
//  int num_cmd_tokens = _numTokens - 1;
  // Ignore keyword and end tokens
  int num_cmd_tokens = _cmdTokens.size() - 2;

  bool ok = true;
  int num_errors = 0;

  if (num_cmd_tokens < num_template_tokens)
  {
    ok = false;
    char buffer[1024];
    sprintf(buffer, "\nmissing at least %d argument(s)",
	    num_template_tokens - num_cmd_tokens);
    error_message += buffer;
    num_errors++;
  }
  else if (_extraTokens == 0 &&
	   num_cmd_tokens > num_template_tokens)
  {
    ok = false;
    char buffer[1024];
    sprintf(buffer, "\n%d extra tokens not expected",
	    num_cmd_tokens - num_template_tokens);
    error_message += buffer;
    num_errors++;
    num_cmd_tokens = num_template_tokens;
  }

  // Process the command tokens.  Skip the first token in the token list since
  // it just contains the keyword so doesn't need to be expanded.

  int prev_uc_ctype = 0;

  std::vector< UiCommandToken >::iterator curr_cmd_token = _cmdTokens.begin();
  curr_cmd_token++;
  
  for (int curr_token = 0; curr_token < num_cmd_tokens;
       curr_token++, curr_cmd_token++, ok = true)
  {
    // (curr_token >= num_template_tokens) => extra optional arguments of
    // the same type as the last

    if (curr_token >= num_template_tokens &&
	prev_uc_ctype == UiCommandToken::UTT_WHERE)
    {
      ok = false;
      num_errors++;
      char buffer[1024];
      sprintf(buffer, "/n%s should not be a <where> ",
	      curr_cmd_token->getCommandText().c_str());
      error_message += buffer;
    }
    else if ((curr_token >= num_template_tokens &&
	      prev_uc_ctype == UiCommandToken::UTT_FIELD) ||
	     (curr_token < num_template_tokens &&
	      strstr(sptrs[curr_token], "<field>") != 0))
    {
      // Already defined as a keyword type

      if (curr_token < num_template_tokens)
	prev_uc_ctype = UiCommandToken::UTT_FIELD;
    }
    else if ((curr_token >= num_template_tokens &&
	      prev_uc_ctype == UiCommandToken::UTT_INT) ||
	     (curr_token < num_template_tokens &&
	      strstr(sptrs[curr_token], "<integer>") != 0))
    {
      int token_value;
      
      if (sscanf(curr_cmd_token->getCommandText().c_str(), "%d",
		 &token_value) == 1)
      {
	curr_cmd_token->setIntParam(token_value);
	curr_cmd_token->setTokenType(UiCommandToken::UTT_VALUE);
	if (curr_token < num_template_tokens)
	  prev_uc_ctype = UiCommandToken::UTT_INT;
      }
      else
      {
	ok = false;
	num_errors++;
	char buffer[1024];
	sprintf(buffer, "\n%s is not an <integer> ",
		curr_cmd_token->getCommandText().c_str());
	error_message += buffer;
      }
    }
    else if ((curr_token >= num_template_tokens &&
	      prev_uc_ctype == UiCommandToken::UTT_REAL) ||
	     (curr_token < num_template_tokens &&
	      strstr(sptrs[curr_token], "<real>") != 0))
    {
      float token_value;
      
      if (sscanf(curr_cmd_token->getCommandText().c_str(), "%f", &token_value) == 1)
      {
	curr_cmd_token->setFloatParam(token_value);
	curr_cmd_token->setTokenType(UiCommandToken::UTT_VALUE);
	if (curr_token < num_template_tokens)
	{
	  prev_uc_ctype = UiCommandToken::UTT_REAL;
	}
      }
      else
      {
	ok = false;
	num_errors++;
	char buffer[1024];
	sprintf(buffer, "\n%s is not a <real> ",
		curr_cmd_token->getCommandText().c_str());
	error_message += buffer;
      }
    }
    else if (strstr(sptrs[curr_token], "<where>") != 0)
    {
      // See if it's one of the wheres (above, below, between)

      if (curr_cmd_token->getCommandText().compare(0, 2, "above", 0, 2) == 0 ||
	  curr_cmd_token->getCommandText().compare(0, 3, "below", 0, 3) == 0 ||
	  curr_cmd_token->getCommandText().compare(0, 3, "between", 0, 3) == 0 )
      {
	prev_uc_ctype = UiCommandToken::UTT_WHERE;
	if (curr_cmd_token->getCommandText().compare(0, 3, "between", 0, 3) == 0 &&
	    curr_token >= num_cmd_tokens - 2)
	{
	  ok = false;
	  num_errors++;
	  char buffer[1024];
	  sprintf(buffer, "\nthere needs to be 2 arguments following %s",
		  curr_cmd_token->getCommandText().c_str());
	  error_message += buffer;
	}
      }
      else
      {
	ok = false;
	num_errors++;
	char buffer[1024];
	sprintf(buffer, "\n%s is not a <where> (above,below,between)",
		curr_cmd_token->getCommandText().c_str());
	error_message += buffer;
      }
    }
  }

  // If we found any errors, set the error string

  if (num_errors > 0)
    _errorMsg = error_message;

  return num_errors == 0;
}


/**********************************************************************
 *              Private Member Functions                              *
 **********************************************************************/

