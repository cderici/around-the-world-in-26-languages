#pragma once

#include "../../lex/include/source_loc.h"

#include <string_view>

namespace frontend::parse {

// This is to allow langauges/tooling to provide their own way to handle errors
// or issues.
//
// e.g. InvalidNumber handling
// e.g. collect errors for later
// e.g. fail-fast after N errors
// etc
class IDiagnostics {
public:
  virtual ~IDiagnostics() = default;

  virtual void error(const frontend::lex::SourceLoc &loc,
                     std::string_view message) = 0;
};

class NullDiagnostics final : public IDiagnostics {
public:
  void error(const frontend::lex::SourceLoc &, std::string_view) override {}
};

} // namespace frontend::parse
