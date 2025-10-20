#include <llvm/IR/Constant.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <map>

#include "ast.h"
#include "error.h"

static std::unique_ptr<llvm::LLVMContext> TheContext;
static std::unique_ptr<llvm::Module> TheModule;
static std::unique_ptr<llvm::IRBuilder<>> Builder;
static std::map<std::string, llvm::Value *> NamedValues;

llvm::Value *LogErrorV(const char *Str) {
  error::logError(Str);
  return nullptr;
}

llvm::Value *NumberExprAST::codegen() {
  return llvm::ConstantFP::get(*TheContext, llvm::APFloat(Val));
}

llvm::Value *VariableExprAST::codegen() {
  // NamedValues is a global symbol table
  // Currently filled by the function definition
  // which is the only binding form atm
  llvm::Value *V = NamedValues[Name];
  if (!V) {
    std::string s = std::format("Unknown variable name: {}", Name);
    return LogErrorV(s.c_str());
  }
  return V;
}
