#pragma once

#include <cstddef>
#include <istream>
#include <string>
#include <string_view>

namespace frontend::lex {

// CharStream class is for hiding std::istream quirks from lexer logic.
// provide a clean api: peek, peek2, get, eof
// track source position cleanly
// Main idea is: Lexer(interface) should think in terms of characters/tokens,
// not stream internals. Language logic should never touch stream mechanics
// directly.
class CharStream {
public:
  explicit CharStream(std::istream &in);

  char peek() const; // '\0' on eof
  char peek2() const;
  char get(); // consume
  bool eof() const;

  std::size_t offset() const;
  std::size_t line() const;   // 1-based
  std::size_t column() const; // same

  std::string_view view(std::size_t start, std::size_t end) const;

private:
  std::string buffer_;
  std::size_t pos_{0};
  std::size_t line_{1};
  std::size_t col_{1};

  // Keep focus in 3 concerns only: char access, cursor track, stable source
  // storage (RAII) Avoid: tokenization stuff, comment parsing, id/num logic etc

  // We read stuff into the buffer, and lexeme_view (with string_view) will
  // index into this buffer
};

} // namespace frontend::lex
