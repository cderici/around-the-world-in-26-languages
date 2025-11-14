#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Passes/PassBuilder.h"

#include "codegen.h"
#include "error.h"
#include "parser.h"

using namespace llvm;

std::unique_ptr<LLVMContext> TheContext;
std::unique_ptr<Module> TheModule;
std::unique_ptr<IRBuilder<>> Builder;
std::map<std::string, Value *> NamedValues;

std::unique_ptr<FunctionPassManager> TheFPM;
std::unique_ptr<LoopAnalysisManager> TheLAM;
std::unique_ptr<FunctionAnalysisManager> TheFAM;

std::map<std::string, std::unique_ptr<PrototypeAST>> FunctionProtos;

Value *LogErrorV(const char *Str) {
  error::logError(Str);
  return nullptr;
}

Function *getFunction(std::string Name) {
  // First, see if the function has already been added to the current module
  if (auto *F = TheModule->getFunction(Name))
    return F;

  // Check if we can codegen the declaration from some existing prototype
  auto FI = FunctionProtos.find(Name);
  if (FI != FunctionProtos.end())
    return FI->second->codegen();

  // If no existing prototype exists, return null
  return nullptr;
}

Value *NumberExprAST::codegen() {
  return ConstantFP::get(*TheContext, APFloat(Val));
}

Value *VariableExprAST::codegen() {
  // Look this variable up in the function.
  Value *V = NamedValues[Name];
  if (!V)
    return LogErrorV("Unknown variable name");
  return V;
}

Value *BinaryExprAST::codegen() {
  Value *L = LHS->codegen();
  Value *R = RHS->codegen();
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
    // Convert bool 0/1 to double 0.0 or 1.0
    return Builder->CreateUIToFP(L, Type::getDoubleTy(*TheContext), "booltmp");
  default:
    return LogErrorV("invalid binary operator");
  }
}

Value *CallExprAST::codegen() {
  // Look up the name in the global module table.
  Function *CalleeF = getFunction(Callee);
  if (!CalleeF)
    return LogErrorV("Unknown function referenced");

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return LogErrorV("Incorrect # arguments passed");

  std::vector<Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }

  return Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

Function *PrototypeAST::codegen() {
  // Make the function type:  double(double,double) etc.
  std::vector<Type *> Doubles(Args.size(), Type::getDoubleTy(*TheContext));
  FunctionType *FT =
      FunctionType::get(Type::getDoubleTy(*TheContext), Doubles, false);

  Function *F =
      Function::Create(FT, Function::ExternalLinkage, Name, TheModule.get());

  // Set names for all arguments.
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  return F;
}

Function *FunctionAST::codegen() {
  // Transfer ownership of the prototype to the FunctionProtos map, but keep a
  // reference to it for use below.
  auto &P = *Proto;
  FunctionProtos[Proto->getName()] = std::move(Proto);

  Function *TheFunction = getFunction(P.getName());

  if (!TheFunction)
    TheFunction = Proto->codegen();

  if (!TheFunction)
    return nullptr;

  // Create a new basic block to start insertion into.
  BasicBlock *BB = BasicBlock::Create(*TheContext, "entry", TheFunction);
  Builder->SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  NamedValues.clear();
  for (auto &Arg : TheFunction->args())
    NamedValues[std::string(Arg.getName())] = &Arg;

  if (Value *RetVal = Body->codegen()) {
    // Finish off the function.
    Builder->CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    verifyFunction(*TheFunction);

    // Run the optimizer on the function.
    TheFPM->run(*TheFunction, *TheFAM);

    return TheFunction;
  }

  // Error reading body, remove function.
  TheFunction->eraseFromParent();
  return nullptr;
}

Value *IfExprAST::codegen() {
  Value *CondV = Cond->codegen();
  if (!CondV)
    return nullptr;

  CondV = Builder->CreateFCmpONE(
      CondV, ConstantFP::get(*TheContext, APFloat(0.0)), "ifcond");

  Function *TheFunction = Builder->GetInsertBlock()->getParent();

  // Blocks for then and else cases.
  BasicBlock *ThenBB = BasicBlock::Create(*TheContext, "then", TheFunction);
  BasicBlock *ElseBB = BasicBlock::Create(*TheContext, "else");
  BasicBlock *MergeBB = BasicBlock::Create(*TheContext, "ifcont");

  // This emits the conditional branch code:
  // br i1 %ifcond, label %then, label %else
  Builder->CreateCondBr(CondV, ThenBB, ElseBB);

  // Emit then value
  //
  Builder->SetInsertPoint(ThenBB);

  Value *ThenV = Then->codegen();
  if (!ThenV)
    return nullptr;

  // Emit an unconditional branch to jump to the merger
  // br label %ifcont
  Builder->CreateBr(MergeBB);

  // codegen of 'Then' can change the current block, e.g. nested if,
  // so update ThenBB for the PHI with the up-to-date value.
  ThenBB = Builder->GetInsertBlock();

  // Emit else block
  TheFunction->insert(TheFunction->end(), ElseBB);
  Builder->SetInsertPoint(ElseBB);

  Value *ElseV = Else->codegen();
  if (!ElseV)
    return nullptr;

  // unconditional jump to the merger block
  Builder->CreateBr(MergeBB);

  // Same reason as bbefore, Else->codegen can create bunch of blocks,
  // We need to make sure we have a handle of the last of them
  // to wire up the Phi node correctly
  ElseBB = Builder->GetInsertBlock();

  // Emit merge block
  TheFunction->insert(TheFunction->end(), MergeBB);
  Builder->SetInsertPoint(MergeBB);

  PHINode *PN = Builder->CreatePHI(Type::getDoubleTy(*TheContext), 2, "iftmp");

  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);
  return PN;
}

Value *ForExprAST::codegen() {
  // Output will be several blocks
  //
  // entry
  //  start = startexpr
  //  goto loop
  //
  // loop
  //  variable = phi [start, entry], [nextVarible, loopend]
  //  ...
  //  body (<- this can be multiple blocks)
  //  ...
  //
  // loopend
  //    step = stepexpr
  //    nextVariable = variable + step
  //    endcond = endexpr
  //    br endcond, loop, afterloop
  //
  // afterloop
  //

  Value *StartVal = Start->codegen();
  if (!StartVal)
    return nullptr;

  Function *TheFunction = Builder->GetInsertBlock()->getParent();
  // Reemember the preheader for the phi node
  BasicBlock *PreheaderBB = Builder->GetInsertBlock();
  BasicBlock *LoopBB = BasicBlock::Create(*TheContext, "loop", TheFunction);

  // Fall through the loopBB
  Builder->CreateBr(LoopBB);

  // Start inserting into the LoopBB
  Builder->SetInsertPoint(LoopBB);

  // Start phi node with an entry for Start
  PHINode *Variable =
      Builder->CreatePHI(Type::getDoubleTy(*TheContext), 2, VarName);
  Variable->addIncoming(StartVal, PreheaderBB);

  // Within the loop, the variable is defined equal to the PHI node. If it
  // shadows an existing variable,   we have to restore it, so save now
  Value *OldVal = NamedValues[VarName];
  NamedValues[VarName] = Variable;

  // Emit the body of the loop
  // This, like any other expr, can create many blocks.
  // Though we don't care about the result of the body, we raise the errors
  if (!Body->codegen())
    return nullptr;

  // Emit the step value
  Value *StepVal = nullptr;
  if (Step) {
    StepVal = Step->codegen();
    if (!StepVal)
      return nullptr;
  } else {
    // default to 1.0
    StepVal = ConstantFP::get(*TheContext, APFloat(1.0));
  }

  Value *NextVar = Builder->CreateFAdd(Variable, StepVal, "nextvar");

  // Compute the end condition
  Value *EndCond = End->codegen();
  if (!EndCond)
    return nullptr;

  // Convert condition to a bool by comparing non-equal to 0.0
  EndCond = Builder->CreateFCmpONE(
      EndCond, ConstantFP::get(*TheContext, APFloat(0.0)), "loopcond");

  // Create the afterloop block and insert it
  BasicBlock *LoopEndBB = Builder->GetInsertBlock();
  BasicBlock *AfterBB =
      BasicBlock::Create(*TheContext, "afterloop", TheFunction);

  // insert the conditional branch into the end of LoopEndBB
  Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

  // Any new code will be inserted in After BB
  Builder->SetInsertPoint(AfterBB);

  // Add the new entry to the phi node
  Variable->addIncoming(NextVar, LoopEndBB);

  // Restore the unshadowed variable
  if (OldVal)
    NamedValues[VarName] = OldVal;
  else
    NamedValues.erase(VarName);

  // for expr returns 0.0 for now
  return Constant::getNullValue(Type::getDoubleTy(*TheContext));
}
