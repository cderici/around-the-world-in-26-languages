
#include "ast.h"
#include "codegen.h"
#include "lexer.h"

#include <llvm/IR/IRBuilder.h>

/*
 * Top-level REPL
 * */

static void InitializeModule() {
  // Open a new context and module
  TheContext = std::make_unique<llvm::LLVMContext>();
  TheModule = std::make_unique<llvm::Module>("Athens compiler", *TheContext);

  // Create a new builder for the module.
  Builder = std::make_unique<llvm::IRBuilder<>>(*TheContext);
}

static void HandleDefinition() {
  if (auto FnAST = ParseDefinition()) {
    if (auto *FnIR = FnAST->codegen()) {
      fprintf(stderr, "Read function definition:");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");
    }
  } else
    getNextToken(); // FIXME: is this correct?
}

static void HandleExtern() {
  if (auto ProtoAST = ParseExtern()) {
    if (auto *FnIR = ProtoAST->codegen()) {
      fprintf(stderr, "Read extern:");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");
    }
  } else
    getNextToken();
}

static void HandleTopLevelExpression() {
  // Eval a top-level expr in an anonymous function
  if (auto FnAST = ParseTopLevelExpr()) {
    if (auto *FnIR = FnAST->codegen()) {
      fprintf(stderr, "Read top-level expr:");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");

      // Remove anonymous expression
      FnIR->eraseFromParent();
    }
  } else
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

  InitializeModule();

  MainLoop();

  TheModule->print(llvm::errs(), nullptr);

  return 0;
}
