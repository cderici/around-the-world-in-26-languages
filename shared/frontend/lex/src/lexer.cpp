#include "lexer.h"
#include "token.h"
#include <cctype>
#include <string_view>
#include <vector>

namespace frontend::lex {

Lexer::Lexer(CharStream &cs, const ILexLanguageRules &langLexConfig)
    : cs_(cs), langLexConfig_(langLexConfig) {}

// invariant: next always consumes at least one char unless EOF
//
// Trivia flow:
// - Start of next, we clear the trivia
// - Then consumeTrivia fills trivia_
// - next() returns the non-trivia token
// - leaadingTrivia refers to that token's leading trivia.
Token Lexer::next() {
  // clear trivia from previous token
  trivia_.clear();

  consumeTrivia();

  if (cs_.eof())
    return Token{TokenKind::Eof, std::string_view{},
                 SourceLoc{
                     .start_offset = cs_.offset(),
                     .end_offset = cs_.offset(),
                     .start_line = cs_.line(),
                     .start_column = cs_.column(),
                     .end_line = cs_.line(),
                     .end_column = cs_.column(),
                 },
                 LiteralValue{}};

  char c = cs_.peek();

  // can it be a keyword?
  if (langLexConfig_.isIdentStart(c)) {
    return lexIdentifierOrKeyword();
    // maybe a number?
  } else if (std::isdigit(static_cast<unsigned char>(c))) {
    return lexNumber();
  } else {
    // then it must be punctuation or invalid
    return lexPunctOrInvalid();
  }
}

const std::vector<TriviaPiece> &Lexer::leadingTrivia() const { return trivia_; }

void Lexer::consumeTrivia() {
  bool consumed_any = true;
  // Keep going as long as we don't hit an eof and we consumed something in the
  // previous run (if we didn't consume anything in the previous run, then we're
  // done)
  while (!cs_.eof() && consumed_any) {
    consumed_any = false;

    char c = cs_.peek();

    if (c == '\n') {
      const std::size_t start_offset = cs_.offset();
      const std::size_t start_line = cs_.line();
      const std::size_t start_column = cs_.column();

      cs_.get();

      const std::size_t end_offset = cs_.offset();
      trivia_.push_back(TriviaPiece{
          TriviaKind::Newline,
          cs_.view(start_offset, end_offset),
          SourceLoc{
              .start_offset = start_offset,
              .end_offset = end_offset,
              .start_line = start_line,
              .start_column = start_column,
              .end_line = cs_.line(),
              .end_column = cs_.column(),
          },
      });
      consumed_any = true;
      continue;
    }

    if (std::isspace(static_cast<unsigned char>(c))) {
      const std::size_t start_offset = cs_.offset();
      const std::size_t start_line = cs_.line();
      const std::size_t start_column = cs_.column();

      while (!cs_.eof()) {
        const char wc = cs_.peek();
        if (wc == '\n' || !std::isspace(static_cast<unsigned char>(wc)))
          break;
        cs_.get();
      }

      const std::size_t end_offset = cs_.offset();
      trivia_.push_back(TriviaPiece{
          TriviaKind::Whitespace,
          cs_.view(start_offset, end_offset),
          SourceLoc{
              .start_offset = start_offset,
              .end_offset = end_offset,
              .start_line = start_line,
              .start_column = start_column,
              .end_line = cs_.line(),
              .end_column = cs_.column(),
          },
      });
      consumed_any = true;
      continue;
    }

    if (consumeCommentMaybe()) {
      consumed_any = true;
      continue;
    }
  }
}

bool Lexer::consumeCommentMaybe() { return false; }

// invariant must be kept for lexing functions, always consume at least one char
// unless EOF

/*

private:
Token lexIdentifierOrKeyword();
Token lexNumber();
Token lexPunctOrInvalid();

CharStream & cs_;
const ILexLanguageRules &langLexConfig_;
std::vector<TriviaPiece> trivia_;
*/

} // namespace frontend::lex
