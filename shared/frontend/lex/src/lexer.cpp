#include "lexer.h"
#include "token.h"
#include <cctype>
#include <string_view>

namespace frontend::lex {

Lexer::Lexer(CharStream &cs, const ILexLanguageRules &langLexConfig)
    : cs_(cs), langLexConfig_(langLexConfig) {}

// invariant: next always consumes at least one char unless EOF
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

// invariant must be kept for lexing functions, always consume at least one char
// unless EOF

/*

const std::vector<TriviaPiece> &leadingTrivia() const;

private:
void consumeTrivia();
Token lexIdentifierOrKeyword();
Token lexNumber();
Token lexPunctOrInvalid();
bool consumeCommentMaybe();

CharStream & cs_;
const ILexLanguageRules &langLexConfig_;
std::vector<TriviaPiece> trivia_;
*/

} // namespace frontend::lex
