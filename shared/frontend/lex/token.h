#pragma once

#include "source_loc.h"
#include <string_view>
#include <variant>

namespace frontend::lex {

enum class TokenKind {
  Invalid,
  Eof,

  Identifier,
  Integer,
  Float,

  LParen,
  RParen,
  Comma,
  Semicolon,
  Plus,
  Minus,
  Star,
  Slash,
  Equal,
  Less,
  LessEqual,
  Greater,
  GreaterEqual,
  LogicNot,

  KwFuncDef,
  KwExtern,
  KwIf,
  KwThen,
  KwElse,
  KwFor,
  KwIn,
  KwBinaryOp,
  KwUnaryOp,
  KwVar,

};

using LiteralValue = std::variant<std::monostate, long long>;

struct Token {
  TokenKind kind{TokenKind::Invalid};
  std::string_view lexeme;
  SourceLoc source_loc{};
  LiteralValue literal{};
};

} // namespace frontend::lex
