#pragma once

#include <string>

enum class Token : int {
  eof = -1,

  def = -2,
  extern_ = -3,

  identifier = -4,
  number = -5,

  if_ = -6,
  then_ = -7,
  else_ = -8,

  for_ = -9,
  in_ = -10,

  binary_ = -11,
  unary_ = -12,

  var_ = -13,
};

inline bool operator==(Token t, char c) {
  return static_cast<int>(t) == static_cast<unsigned char>(c);
}

inline bool operator==(char c, Token t) { return t == c; }

inline bool operator!=(Token t, char c) { return !(t == c); }

namespace lexer {

extern std::string IdentifierStr;
extern double NumVal;

void SetLexerInputStream(std::istream &in);
void ResetLexerInputStreamToSTDIN();

Token gettok();
} // namespace lexer
