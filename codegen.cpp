#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Verifier.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <format>
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

llvm::Value *BinaryExprAST::codegen() {
  llvm::Value *L = LHS->codegen();
  llvm::Value *R = RHS->codegen();

  if (!L || !R)
    return nullptr;

  switch (Op) {
  case '+':
    return Builder->CreateFAdd(L, R, "addtmp");
  case '-':
    return Builder->CreateFSub(L, R, "subtmp");
  case '*':
    return Builder->CreateFMul(L, R, "multmp");
  case '<':
    L = Builder->CreateFCmpULT(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0/1.0
    return Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*TheContext),
                                 "booltmp");
  default:
    std::string s = std::format("Invalid binary operator: {}", Op);
    return LogErrorV(s.c_str());
  }
}

llvm::Value *CallExprAST::codegen() {
  // Lookup for function in global module table
  llvm::Function *CalleeF = TheModule->getFunction(Callee);
  if (!CalleeF) {
    // FIXME: make this log error formatting
    std::string s = std::format("Unknown function reference: {}", Callee);
    return LogErrorV(s.c_str());
  }

  // Check for arg mismatch
  if (CalleeF->arg_size() != Args.size()) {
    std::string s = std::format("Expected {} arguments to {}, given: {}",
                                CalleeF->arg_size(), Callee, Args.size());
    return LogErrorV(s.c_str());
  }

  std::vector<llvm::Value *> ArgsV;
  ArgsV.reserve(Args.size());
  for (auto &arg : Args) {
    llvm::Value *v = arg->codegen();
    if (!v)
      return nullptr;
    ArgsV.push_back(v);
  }

  return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

llvm::Function *PrototypeAST::codegen() {
  // Make the function type
  // Note that we currently only support functions of type double
  std::vector<llvm::Type *> Doubles(Args.size(),
                                    llvm::Type::getDoubleTy(*TheContext));

  llvm::FunctionType *FT = llvm::FunctionType::get(
      // return type (double), arg types (N doubles), is variadic?
      llvm::Type::getDoubleTy(*TheContext), Doubles, false);

  // Create the function object with user specified Name
  // and register it to TheModule's symbol table
  llvm::Function *F = llvm::Function::Create(
      FT, llvm::Function::ExternalLinkage, Name, TheModule.get());

  // Names for arguments
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  return F;
}

llvm::Function *FunctionAST::codegen() {
  // First, check for an existing function from a previous extern declaration.
  // We might have already created a function object for this Proto before.
  llvm::Function *TheFunction = TheModule->getFunction(Proto->getName());

  if (!TheFunction)
    TheFunction = Proto->codegen();

  if (!TheFunction)
    return nullptr;

  // empty() means it doesn't have any body yet.
  if (!TheFunction->empty()) {
    std::string s =
        std::format("Function {} cannot be redefined", Proto->getName());
    return (llvm::Function *)LogErrorV(s.c_str());
  }

  // Create a new basic block named "entry" and insert it into the function.
  llvm::BasicBlock *BB =
      llvm::BasicBlock::Create(*TheContext, "entry", TheFunction);
  Builder->SetInsertPoint(BB);

  // Record function arguments in the NamedValues map.
  NamedValues.clear();
  for (auto &Arg : TheFunction->args())
    NamedValues[std::string(Arg.getName())] = &Arg;

  if (llvm::Value *RetVal = Body->codegen()) {
    // Finish off the function
    Builder->CreateRet(RetVal);

    // Validate generated code, check for consistency
    llvm::verifyFunction(*TheFunction);

    return TheFunction;
  }

  // err reading body, remove function
  TheFunction->eraseFromParent();
  return nullptr;
}
