#include <memory>
#include <string>
#include <vector>

#include "lexer.h"

// base class for expressions
class ExprAST {
public:
  virtual ~ExprAST() = default;
};

// Numbers
class NumberExprAST : public ExprAST {
  double Val;

public:
  NumberExprAST(double v) : Val(v) {}
};

// Vars
class VariableExprAST : public ExprAST {
  std::string Name;

public:
  VariableExprAST(const std::string &Name) : Name(Name) {}
};

// Binops
class BinaryExprAST : public ExprAST {
  char Op;
  std::unique_ptr<ExprAST> LHS, RHS;

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
};

// PrototypeAST - represents a prototype for a function (name + argnames). Note
// that the language we're implementing doesn't have static types.
class PrototypeAST {
  std::string Name;
  std::vector<std::string> Args;

public:
  PrototypeAST(std::string Name, std::vector<std::string> Args)
      : Name(Name), Args(std::move(Args)) {}

  const std::string &getName() const { return Name; }
};

// FunctionAST - This represents a lambda.
class FunctionAST {
  std::unique_ptr<PrototypeAST> Proto;
  std::unique_ptr<ExprAST> Body;

public:
  FunctionAST(std::unique_ptr<PrototypeAST> Proto,
              std::unique_ptr<ExprAST> Body)
      : Proto(std::move(Proto)), Body(std::move(Body)) {}
};

// CurTok/getNextToken - provide a simple token buffer. Curtok is the current
// token the parser is looking at. getNextToken reads another token from the
// lexer and updates CurTok with its results.
static int CurTok;
// This CurTok is like in the tetris game you'd see the next piece that's
// coming. Parser can look ahead.
static int getNextToken() { return CurTok = lexer::gettok(); }

// Little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}

/*
 * PARSER
 *
 * (for each production in the grammar, there's a function)
 * */

// numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr() {
  auto result = std::make_unique<NumberExprAST>(lexer::NumVal);
  getNextToken(); // consume the number
  return result;
}
