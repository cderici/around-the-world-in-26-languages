#pragma once

#include <cstddef>

namespace frontend::lex {

struct SourceLoc {
  std::size_t start_offset{0};
  std::size_t end_offset{0};
  std::size_t start_line{1};
  std::size_t start_column{1};
  std::size_t end_line{1};
  std::size_t end_column{1};
};

} // namespace frontend::lex
