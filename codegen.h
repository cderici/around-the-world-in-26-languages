#pragma once

#include <map>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

static std::unique_ptr<llvm::LLVMContext> TheContext;
static std::unique_ptr<llvm::Module> TheModule;
static std::unique_ptr<llvm::IRBuilder<>> Builder;
static std::map<std::string, llvm::Value *> NamedValues;
