#pragma once

#include "char_stream.h"
#include "lex_language_rules.h"
#include "trivia.h"
#include <vector>

namespace frontend::lex {

/*
 * Lexer defines the public contract of the reusable lexer engine.
 * Goal is to separate:
 *  - general lexing mechanics (Lexer), with
 *  - language policy/syntax (ILexLanguageRules)
 *
 * */
class Lexer {
public:
  Lexer(CharStream &cs, const ILexLanguageRules &langLexConfig);

  Token next();
  const std::vector<TriviaPiece> &leadingTrivia() const;

private:
  void consumeTrivia();
  Token lexIdentifierOrKeyword();
  Token lexNumber();
  Token lexPunctOrInvalid();
  bool consumeCommentMaybe();

  CharStream &cs_;
  const ILexLanguageRules &langLexConfig_;
  std::vector<TriviaPiece> trivia_;
};
} // namespace frontend::lex
