
#include "ast.h"
#include "codegen.h"
#include "lexer.h"

/*
 * Top-level REPL
 * */

static void HandleDefinition() {
  if (ParseDefinition())
    fprintf(stderr, "Parsed a function definition\n");
  else
    getNextToken(); // FIXME: is this correct?
}

static void HandleExtern() {
  if (ParseExtern())
    fprintf(stderr, "Parsed an extern\n");
  else
    getNextToken();
}

static void HandleTopLevelExpression() {
  if (ParseTopLevelExpr())
    fprintf(stderr, "Parsed a top-level expr\n");
  else
    getNextToken();
}

// top ::= definition | external | expression | ';'
// REPL
static void MainLoop() {
  while (true) {
    fprintf(stderr, "ready> ");
    switch (CurTok) {
    case tok_eof:
      return;
    case ';':
      getNextToken(); // ignore toplevel ; symbols
      break;
    case tok_def:
      HandleDefinition();
      break;
    case tok_extern:
      HandleExtern();
      break;
    default:
      HandleTopLevelExpression();
      break;
    }
  }
}

int main() {
  // Load the precedences for binary operations
  // higher value means higher precedence
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;
  // TODO: extend

  // Fire up the REPL
  fprintf(stderr, "ready> ");
  getNextToken();

  MainLoop();

  return 0;
}
