#pragma once

#include "diagnostics.h"
#include "token_stream.h"

namespace frontend::parse {

template <typename BuilderT> struct ParseContext {
  TokenStream &tokens;
  BuilderT &builder;
  IDiagnostics &diag;
};

} // namespace frontend::parse
