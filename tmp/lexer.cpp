#include <cctype>
#include <cstdlib>
#include <string>

#include "lexer.h"

std::string lexer::IdentifierStr;
double lexer::NumVal;

// the actual lexer
int lexer::gettok() {
  // LastChar: read, but not processed
  static int LastChar = ' ';

  // Skip whitespace
  while (isspace(LastChar))
    LastChar = getchar();

  // identifier
  if (isalpha(LastChar)) {
    lexer::IdentifierStr = LastChar;

    while (std::isalnum((LastChar = getchar())))
      lexer::IdentifierStr += LastChar;

    if (lexer::IdentifierStr == "def")
      return tok_def;
    if (lexer::IdentifierStr == "extern")
      return tok_extern;
    return tok_identifier;
  }

  // numbers
  if (isdigit(LastChar) || LastChar == '.') {
    std::string NumStr;

    do {
      NumStr += LastChar;
      LastChar = getchar();
    } while (isdigit(LastChar) || LastChar == '.');

    lexer::NumVal = std::strtod(NumStr.c_str(), nullptr);
    return tok_number;
  }

  // comments
  if (LastChar == '#') {
    do {
      LastChar = getchar();
    } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF)
      return lexer::gettok();
  }

  // check EOF
  if (LastChar == EOF)
    return tok_eof;

  // return char with ascii value
  int ThisChar = LastChar;
  LastChar = getchar();
  return ThisChar;
}
