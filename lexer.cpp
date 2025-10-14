#include <cctype>
#include <cstdlib>
#include <string>

#include "lexer.h"

std::string IdentifierStr;
double NumVal;

// the actual lexer
Token gettok() {
  // LastChar: read, but not processed
  static int LastChar = ' ';

  // Skip whitespace
  while (isspace(LastChar))
    LastChar = getchar();

  // identifier
  if (isalpha(LastChar)) {
    IdentifierStr = LastChar;

    while (std::isalnum((LastChar = getchar())))
      IdentifierStr += LastChar;

    if (IdentifierStr == "def")
      return tok_def;
    if (IdentifierStr == "extern")
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

    NumVal = std::strtod(NumStr.c_str(), 0);
    return tok_number;
  }

  // comments
  if (LastChar == '#') {
    do {
      LastChar = getchar();
    } while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF)
      return gettok();
  }

  // check EOF
  if (LastChar == EOF)
    return tok_eof;

  // return char with ascii value
  int ThisChar = LastChar;
  LastChar = getchar();
  return ThisChar;
}
