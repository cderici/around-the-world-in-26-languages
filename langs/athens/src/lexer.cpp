#include <iostream>
#include <string>

#include "lexer.h"

namespace lexer {

std::string IdentifierStr;
double NumVal;

static std::istream *CurIn = &std::cin;
static int LastChar = ' ';

void SetLexerInputStream(std::istream &in) {
  CurIn = &in;
  LastChar = ' ';
}
void ResetLexerInputStreamToSTDIN() {
  CurIn = &std::cin;
  LastChar = ' ';
}

static int getNextChar() { return CurIn->get(); }

Token gettok() {

  // Skip any whitespace.
  while (isspace(LastChar))
    LastChar = getNextChar();

  if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
    IdentifierStr = LastChar;
    while (isalnum((LastChar = getNextChar())))
      IdentifierStr += LastChar;

    if (IdentifierStr == "def")
      return Token::def;
    if (IdentifierStr == "extern")
      return Token::extern_;
    if (IdentifierStr == "if")
      return Token::if_;
    if (IdentifierStr == "then")
      return Token::then_;
    if (IdentifierStr == "else")
      return Token::else_;

    if (IdentifierStr == "for")
      return Token::for_;
    if (IdentifierStr == "in")
      return Token::in_;

    if (IdentifierStr == "binary")
      return Token::binary_;
    if (IdentifierStr == "unary")
      return Token::unary_;

    if (IdentifierStr == "var")
      return Token::var_;

    return Token::identifier;
  }

  if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
    std::string NumStr;
    do {
      NumStr += LastChar;
      LastChar = getNextChar();
    } while (isdigit(LastChar) || LastChar == '.');

    NumVal = strtod(NumStr.c_str(), nullptr);
    return Token::number;
  }

  if (LastChar == '#') {
    // Comment until end of line.
    do
      LastChar = getNextChar();
    while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');

    if (LastChar != EOF)
      return gettok();
  }

  // Check for end of file.  Don't eat the EOF.
  if (LastChar == EOF)
    return Token::eof;

  // Otherwise, just return the character as its ascii value.
  int ThisChar = LastChar;
  LastChar = getNextChar();
  return static_cast<Token>(ThisChar);
}

} // namespace lexer
