#pragma once

#include <llvm/IR/Value.h>
#include <map>

// base class for expressions
class ExprAST {
public:
  virtual ~ExprAST() = default;
  virtual llvm::Value *codegen() = 0;
};

// Numbers
class NumberExprAST : public ExprAST {
  double Val;

public:
  NumberExprAST(double v) : Val(v) {}
  llvm::Value *codegen() override;
};

// Vars
class VariableExprAST : public ExprAST {
  std::string Name;

public:
  VariableExprAST(const std::string &Name) : Name(Name) {}
  llvm::Value *codegen() override;
};

// Binops
class BinaryExprAST : public ExprAST {
  char Op;
  std::unique_ptr<ExprAST> LHS, RHS;
  llvm::Value *codegen() override;

public:
  BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
      : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
};

// CallExprAST - function calls
class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public:
  CallExprAST(const std::string &Callee,
              std::vector<std::unique_ptr<ExprAST>> Args)
      : Callee(Callee), Args(std::move(Args)) {}
  llvm::Value *codegen() override;
};

// PrototypeAST - represents a prototype for a function (name + argnames). Note
// that the language we're implementing doesn't have static types. So the
// prototype is just the name of the function and the names of the arguments.
class PrototypeAST {
  std::string Name;
  std::vector<std::string> Args;

public:
  PrototypeAST(std::string Name, std::vector<std::string> Args)
      : Name(Name), Args(std::move(Args)) {}

  const std::string &getName() const { return Name; }
  llvm::Function *codegen();
};

// FunctionAST - This represents a function def (lambda).
class FunctionAST {
  std::unique_ptr<PrototypeAST> Proto;
  std::unique_ptr<ExprAST> Body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> Proto,
              std::unique_ptr<ExprAST> Body)
      : Proto(std::move(Proto)), Body(std::move(Body)) {}
  llvm::Function *codegen();
};

// CurTok/getNextToken - provide a simple token buffer. Curtok is the current
// token the parser is looking at. getNextToken reads another token from the
// lexer and updates CurTok with its results.
extern int CurTok;
// This CurTok is like in the tetris game you'd see the next piece that's
// coming. Parser can look ahead.
int getNextToken();

// BinopPrecedence
static std::map<char, int> BinopPrecedence;

std::unique_ptr<FunctionAST> ParseDefinition();
std::unique_ptr<FunctionAST> ParseTopLevelExpr();
std::unique_ptr<PrototypeAST> ParseExtern();
