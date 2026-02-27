#pragma once

#include "source_loc.h"
#include <string_view>

namespace frontend::lex {
enum class TriviaKind { Whitespace, Newline, Comment };

struct TriviaPiece {
  TriviaKind kind;
  std::string_view text;
  SourceLoc source_loc;
};
} // namespace frontend::lex
