#include "KaleidoscopeJIT.h"
#include "codegen.h"
#include "lexer.h"
#include "parser.h"
#include <fstream>

#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

//===----------------------------------------------------------------------===//
// Top-Level parsing and JIT Driver
//===----------------------------------------------------------------------===//
extern std::unique_ptr<FunctionPassManager> TheFPM;
extern std::unique_ptr<LoopAnalysisManager> TheLAM;
extern std::unique_ptr<FunctionAnalysisManager> TheFAM;
std::unique_ptr<CGSCCAnalysisManager> TheCGAM;
std::unique_ptr<ModuleAnalysisManager> TheMAM;
std::unique_ptr<PassInstrumentationCallbacks> ThePIC;
std::unique_ptr<StandardInstrumentations> TheSI;
static ExitOnError ExitOnErr;

std::unique_ptr<llvm::orc::KaleidoscopeJIT> TheJIT;

void InitializeModuleAndManagers(void) {
  // Open a new context and module
  TheContext = std::make_unique<LLVMContext>();
  TheModule = std::make_unique<Module>("Kaleidoscope JIT", *TheContext);
  TheModule->setDataLayout(TheJIT->getDataLayout());

  // Create a new builder for the module
  Builder = std::make_unique<IRBuilder<>>(*TheContext);

  // Create new pass and analysis managers
  TheFPM = std::make_unique<llvm::FunctionPassManager>();
  TheLAM = std::make_unique<llvm::LoopAnalysisManager>();
  TheFAM = std::make_unique<llvm::FunctionAnalysisManager>();
  TheCGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
  TheMAM = std::make_unique<llvm::ModuleAnalysisManager>();
  ThePIC = std::make_unique<llvm::PassInstrumentationCallbacks>();
  TheSI =
      std::make_unique<llvm::StandardInstrumentations>(*TheContext,
                                                       /*DebugLogging*/ true);
  TheSI->registerCallbacks(*ThePIC, TheMAM.get());

  // Add transform passes.

  // Do simple "peephole" optimizations and big-twiddling opts.
  TheFPM->addPass(llvm::InstCombinePass());

  // Reassociate expressions.
  TheFPM->addPass(llvm::ReassociatePass());

  // Eliminate common subexpressions
  TheFPM->addPass(llvm::GVNPass());

  // Simplify the control flow graph (delete unreachable blocks, etc).
  TheFPM->addPass(llvm::SimplifyCFGPass());

  // Register analysis passes used in these transform passes.
  llvm::PassBuilder PB;
  PB.registerModuleAnalyses(*TheMAM);
  PB.registerFunctionAnalyses(*TheFAM);
  PB.crossRegisterProxies(*TheLAM, *TheFAM, *TheCGAM, *TheMAM);
}

static void HandleDefinition() {
  if (auto FnAST = ParseDefinition()) {
    if (auto *FnIR = FnAST->codegen()) {
      fprintf(stderr, "Read function definition:\n");
      FnIR->print(errs());
      fprintf(stderr, "\n");
      ExitOnErr(TheJIT->addModule(
          orc::ThreadSafeModule(std::move(TheModule), std::move(TheContext))));
      InitializeModuleAndManagers();
    }
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

static void HandleExtern() {
  if (auto ProtoAST = ParseExtern()) {
    if (auto *FnIR = ProtoAST->codegen()) {
      fprintf(stderr, "Read extern:\n");
      FnIR->print(errs());
      fprintf(stderr, "\n");
      FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
    }
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

static void HandleTopLevelExpression() {
  // Evaluate a top-level expression into an anonymous function.
  if (auto FnAST = ParseTopLevelExpr()) {
    if (FnAST->codegen()) {
      // Create a ResourceTracker to track JITted memory allocated to our
      // anonymous expression -- that way we can free it after executing.
      auto RT = TheJIT->getMainJITDylib().createResourceTracker();

      auto TSM = llvm::orc::ThreadSafeModule(std::move(TheModule),
                                             std::move(TheContext));
      ExitOnErr(TheJIT->addModule(std::move(TSM), RT));
      InitializeModuleAndManagers();

      // Search the JIT for the __anon_expr symbol.
      auto ExprSymbol = ExitOnErr(TheJIT->lookup("__anon_expr"));

      // Get the symbol's address and cast it to the right type (takes no
      // arguments, returns a double) so we can call it as a native function.

      double (*FP)() = ExprSymbol.toPtr<double (*)()>();
      fprintf(stderr, "Evaluated to %f\n", FP());

      // FnIR->print(errs());
      // fprintf(stderr, "\n");
      //
      // // Remove the anonymous expression.
      // FnIR->eraseFromParent();

      // Delete the anonymous expression module from the JIT.
      ExitOnErr(RT->remove());
    }
  } else {
    // Skip token for error recovery.
    getNextToken();
  }
}

static void lexerLoop(bool isRepl) {
  if (isRepl)
    fprintf(stderr, "ready> ");

  getNextToken();

  while (CurTok != Token::eof) {

    if (isRepl)
      fprintf(stderr, "ready> ");

    switch (CurTok) {
    case static_cast<Token>(';'): // ignore top-level semicolons.
      getNextToken();
      break;
    case Token::def:
      HandleDefinition();
      break;
    case Token::extern_:
      HandleExtern();
      break;
    default:
      HandleTopLevelExpression();
      break;
    }
  }
}

/// top ::= definition | external | expression | ';'
static void LoadRepl() {
  // Make sure the lexer is reading STDIN
  lexer::ResetLexerInputStreamToSTDIN();

  lexerLoop(true);
}

static void LoadFile(const std::string &Path) {
  std::ifstream in(Path);
  if (!in.is_open()) {
    fprintf(stderr, "could not open %s\n", Path.c_str());
    return;
  }
  lexer::SetLexerInputStream(in);

  lexerLoop(false);

  fprintf(stderr, "\n%s loaded.\n", Path.c_str());

  // "in" goes out of scope when LoadFile returns, and CurIn inside the lexer
  // becomes a dangling pointer
  lexer::ResetLexerInputStreamToSTDIN();
}

//===----------------------------------------------------------------------===//
// Main driver code.
//===----------------------------------------------------------------------===//

int main(int argc, char **argv) {
  InitializeNativeTarget();
  InitializeNativeTargetAsmPrinter();
  InitializeNativeTargetAsmParser();

  // Install standard binary operators.
  // 1 is lowest precedence.
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40; // highest.

  TheJIT = ExitOnErr(llvm::orc::KaleidoscopeJIT::Create());

  // Make the module, which holds all the code.
  // InitializeModule();
  InitializeModuleAndManagers();

  // Load the runtime support library (written in Athens)
  LoadFile("langs/athens/lib/runtime.ath");

  if (argc > 1) {
    LoadFile(argv[1]);
  } else {
    // Run the main "interpreter loop" now.
    LoadRepl();
  }

  // Print out all of the generated code.
  TheModule->print(errs(), nullptr);

  return 0;
}
