#pragma once

#include "../../lex/include/lexer.h"
#include "diagnostics.h"

#include <deque>
#include <initializer_list>
#include <string_view>
#include <vector>

namespace frontend::parse {

// TokenStream is the driver for lexer
// Parser uses TokenStream API, and TokenStream calls lexer's methods (e.g.
// next()) only when needed.
class TokenStream {
public:
  explicit TokenStream(frontend::lex::Lexer &lexer) : lexer_(lexer) {}

  const frontend::lex::Token &peek(std::size_t lookahead = 0) {
    fillUntil(lookahead);
    return tokens_[lookahead];
  }

  const frontend::lex::Token &current() { return peek(0); }

  const std::vector<frontend::lex::TriviaPiece> &
  leadingTrivia(std::size_t lookahead = 0) {
    fillUntil(lookahead);
    return trivia_[lookahead];
  }

  bool is(frontend::lex::TokenKind kind, std::size_t lookahead = 0) {
    return peek(lookahead).kind == kind;
  }

  frontend::lex::Token consume() {
    fillUntil(0);
    frontend::lex::Token tok = tokens_.front();
    tokens_.pop_front();
    trivia_.pop_front();
    return tok;
  }

  bool match(frontend::lex::TokenKind kind) {
    if (!is(kind))
      return false;
    (void)consume();
    return true;
  }

  bool matchAny(std::initializer_list<frontend::lex::TokenKind> kinds) {
    for (auto kind : kinds) {
      if (is(kind)) {
        (void)consume();
        return true;
      }
    }
    return false;
  }

  bool expect(frontend::lex::TokenKind kind, IDiagnostics &diag,
              std::string_view message) {
    if (is(kind)) {
      (void)consume();
      return true;
    }

    diag.error(current().source_loc, message);
    return false;
  }

private:
  void fillUntil(std::size_t lookahead) {
    while (tokens_.size() <= lookahead) {
      frontend::lex::Token tok = lexer_.next();
      tokens_.push_back(tok);
      trivia_.push_back(lexer_.leadingTrivia());
    }
  }

  frontend::lex::Lexer &lexer_;
  std::deque<frontend::lex::Token> tokens_;
  std::deque<std::vector<frontend::lex::TriviaPiece>> trivia_;
};

} // namespace frontend::parse
