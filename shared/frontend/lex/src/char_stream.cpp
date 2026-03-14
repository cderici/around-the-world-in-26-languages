#include "../include/char_stream.h"
#include <cstddef>
#include <iterator>

namespace frontend::lex {

CharStream::CharStream(std::istream &in) {
  buffer_ = std::string(std::istreambuf_iterator<char>(in),
                        std::istreambuf_iterator<char>());
}

char CharStream::peek() const {
  // eof check
  if (cursor_ >= buffer_.size())
    return '\0';

  return buffer_[cursor_];
}

char CharStream::peek2() const {
  // eof check
  if (cursor_ + 1 >= buffer_.size()) // + 1 can overflow
    return '\0';

  return buffer_[cursor_ + 1];
}

std::size_t CharStream::size() const { return buffer_.size(); }

char CharStream::consumeOne() {
  const char current_char = peek();
  if (current_char == '\0')
    return '\0';

  advance(1);
  return current_char;
}

std::size_t CharStream::advance(std::size_t n) {
  std::size_t consumed = 0;
  while (consumed < n && cursor_ < buffer_.size()) {
    const char current_char = buffer_[cursor_++];
    if (current_char == '\n') {
      ++line_;
      col_ = 1;
    } else {
      ++col_;
    }
    ++consumed;
  }
  return consumed;
}

bool CharStream::eof() const { return cursor_ >= buffer_.size(); }

std::size_t CharStream::offset() const { return cursor_; }
std::size_t CharStream::line() const { return line_; }
std::size_t CharStream::column() const { return col_; }

std::string_view CharStream::view(std::size_t start, std::size_t end) const {
  if (start > end || end > buffer_.size())
    return std::string_view();
  return std::string_view(buffer_.data() + start, end - start);
}

} // namespace frontend::lex
