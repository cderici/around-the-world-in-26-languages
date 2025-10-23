#pragma once

#include <string>

enum Token {
  tok_eof = -1,

  tok_def = -2,
  tok_extern = -3,

  tok_identifier = -4,
  tok_number = -5,
};

namespace lexer {

extern std::string IdentifierStr;
extern double NumVal;

int gettok();
} // namespace lexer
