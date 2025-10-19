#include <map>
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
// that the language we're implementing doesn't have static types. So the
// prototype is just the name of the function and the names of the arguments.
class PrototypeAST {
  std::string Name;
  std::vector<std::string> Args;

public:
  PrototypeAST(std::string Name, std::vector<std::string> Args)
      : Name(Name), Args(std::move(Args)) {}

  const std::string &getName() const { return Name; }
};

// FunctionAST - This represents a function def (lambda).
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

// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken(); // consume (
  auto V = ParseExpression();
  if (!V) {
    return nullptr;
  }

  if (CurTok != ')')
    return LogError("parser: expected ')'");
  getNextToken(); // consume )
  return V;
}

// identifierexpr
//  ::= identifier
//  ::= identifier '(' expression* ')' <----- function call
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
  std::string IdName = lexer::IdentifierStr;

  getNextToken(); // eat the identifier

  if (CurTok != '(') // simple variable reference
    return std::make_unique<VariableExprAST>(IdName);

  // It's a function call
  getNextToken(); // consume (

  // make the args
  std::vector<std::unique_ptr<ExprAST>> Args;
  if (CurTok != ')') {
    while (true) {
      if (auto Arg = ParseExpression())
        Args.push_back(std::move(Arg));
      else
        return nullptr;

      if (CurTok == ')')
        break;

      if (CurTok != ',')
        return LogError("parser: expected ')' or ',' in argument list");

      getNextToken();
    }
  }

  getNextToken(); // eat )

  return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

// primary
//  ::= identifierexpr
//  ::= numberexpr
//  ::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary() {
  switch (CurTok) {
  default:
    return LogError("parser: unknown token when expecting expression");
  case tok_identifier:
    return ParseIdentifierExpr();
  case tok_number:
    return ParseNumberExpr();
  case '(':
    return ParseParenExpr();
  }
}

// BinopPrecedence
static std::map<char, int> BinopPrecedence;

static int GetTokenPrecedence() {
  if (!isascii(CurTok))
    return -1;

  int TokPrec = BinopPrecedence[CurTok];
  return (TokPrec > 0) ? TokPrec : -1;
}

// expression ::= primary | primary binoprhs
// an expression is a parimary expression, potentially followed by a sequence of
// [binop, primaryexpr] pairs, e.g., a [+, b][+, (c+d)], ...
static std::unique_ptr<ExprAST> ParseExpression() {
  auto LHS = ParsePrimary();
  if (!LHS)
    return nullptr;

  // 0 down here is the precedence
  return ParseBinOpRHS(0, std::move(LHS));
}

// binoprhs ::= ('+' primary)*
// ParseBinOpRHS takes the "currrent" precedence as input, and checks if the
// precedence of the CurTok is at least as big as the "current" precedence.
static std::unique_ptr<ExprAST> ParseBinOpRHS(int CurrentExprPrec,
                                              std::unique_ptr<ExprAST> LHS) {
  while (true) {
    int TokPrec = GetTokenPrecedence();

    if (TokPrec < CurrentExprPrec)
      return LHS;

    // At this point, we know that it's a binop
    int BinOP = CurTok;
    getNextToken(); // consume binop, i.e. move CurTok on the expression comes
                    // after the operator

    auto RHS = ParsePrimary();
    if (!RHS)
      return nullptr;

    int NextPrec = GetTokenPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
      if (!RHS)
        return nullptr;
    }

    LHS =
        std::make_unique<BinaryExprAST>(BinOP, std::move(LHS), std::move(RHS));
  }
}

// prototype
//  ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {
  if (CurTok != tok_identifier)
    return LogErrorP("Expected function name in prototype");

  std::string FnName = lexer::IdentifierStr;
  getNextToken();

  if (CurTok != '(')
    return LogErrorP("Expected argument names in prototype");

  // Read list of argument names
  std::vector<std::string> ArgNames;
  while (getNextToken() == tok_identifier)
    ArgNames.push_back(lexer::IdentifierStr);

  if (CurTok != ')')
    return LogErrorP("Expected ')' in prototype");

  // success
  getNextToken(); // consume the last ')'

  return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

// definition ::= 'def' prototype expression
static std::unique_ptr<FunctionAST> ParseDefinition() {
  getNextToken(); // consume 'def' keyword

  auto Proto = ParsePrototype();
  if (!Proto)
    return nullptr;

  if (auto E = ParseExpression())
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  return nullptr;
}

// external ::= 'extern' prototype
// (for forward declaring user functions, and actual external functions, such as
// sin, cos, etc.)
static std::unique_ptr<PrototypeAST> ParseExtern() {
  getNextToken(); // consume 'extern' keyword
  return ParsePrototype();
}

int main() {
  // Load the precedences for binary operations
  // higher value means higher precedence
  BinopPrecedence['<'] = 10;
  BinopPrecedence['+'] = 20;
  BinopPrecedence['-'] = 20;
  BinopPrecedence['*'] = 40;
  // TODO: extend
}
