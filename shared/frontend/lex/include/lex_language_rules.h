#pragma once

#include "token.h"
#include <optional>
#include <span>
#include <string_view>

namespace frontend::lex {
struct CommentDelimiter {
  enum class Kind { Line, Block };
  Kind kind;
  std::string_view open;
  std::string_view close; // empty for line comments
};

/*
 * ILexLanguageRules is an interface that represents the lexing preferences of
 * the language that imlements it.
 *
 * keyword -> which identifiers become keywords
 *  --> TokenKind if it's a keyword
 *  --> std::nullopt if it's a normal identifier
 * punctuator --> which symbol strings are punctuators/operators
 * which comment delimiters exist, etc.
 *
 * */

class ILexLanguageRules {
public:
  virtual ~ILexLanguageRules() = default;

  virtual std::optional<TokenKind> keyword(std::string_view ident) const = 0;
  virtual std::optional<TokenKind> punctuator(std::string_view text) const = 0;

  // returns comment delimiters for this language
  virtual std::span<const CommentDelimiter> comments() const = 0;

  virtual bool isIdentStart(char c) const = 0;
  virtual bool isIdentContinue(char c) const = 0;

  virtual bool emitNewlineToken() const = 0;
  virtual bool emitIndentDedent() const = 0;
  virtual bool allowRationalLiteral() const = 0;
};
} // namespace frontend::lex
