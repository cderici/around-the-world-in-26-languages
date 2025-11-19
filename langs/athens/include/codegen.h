#pragma once

#include <map>

#include "KaleidoscopeJIT.h"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

extern std::unique_ptr<llvm::LLVMContext> TheContext;
extern std::unique_ptr<llvm::Module> TheModule;
extern std::unique_ptr<llvm::IRBuilder<>> Builder;
extern std::map<std::string, llvm::AllocaInst *> NamedValues;
