#include <stdio.h>

#include "error.h"

void error::logError(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return;
}
