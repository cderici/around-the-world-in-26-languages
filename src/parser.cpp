
#include "parser.h"
#include "error.h"
#include "lexer.h"

Token CurTok;
Token getNextToken() { return CurTok = lexer::gettok(); }

std::map<char, int> BinopPrecedence;

/// GetTokPrecedence - Get the precedence of the pending binary operator token.
static int GetTokPrecedence() {
  if (!isascii(static_cast<int>(CurTok)))
    return -1;

  // Make sure it's a declared binop.
  int TokPrec = BinopPrecedence[static_cast<int>(CurTok)];
  if (TokPrec <= 0)
    return -1;
  return TokPrec;
}

/// LogError* - These are little helper functions for error handling.
std::unique_ptr<ExprAST> LogError(const char *Str) {
  error::logError(Str);
  return nullptr;
}

std::unique_ptr<PrototypeAST> LogErrorP(const char *Str) {
  error::logError(Str);
  return nullptr;
}

static std::unique_ptr<ExprAST> ParseExpression();

/// numberexpr ::= number
static std::unique_ptr<ExprAST> ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(lexer::NumVal);
  getNextToken(); // consume the number
  return std::move(Result);
}

/// parenexpr ::= '(' expression ')'
static std::unique_ptr<ExprAST> ParseParenExpr() {
  getNextToken(); // eat (.
  auto V = ParseExpression();
  if (!V)
    return nullptr;

  if (CurTok != ')')
    return LogError("expected ')'");
  getNextToken(); // eat ).
  return V;
}

/// identifierexpr
///   ::= identifier
///   ::= identifier '(' expression* ')'
static std::unique_ptr<ExprAST> ParseIdentifierExpr() {
  std::string IdName = lexer::IdentifierStr;

  getNextToken(); // eat identifier.

  if (CurTok != '(') // Simple variable ref.
    return std::make_unique<VariableExprAST>(IdName);

  // Call.
  getNextToken(); // eat (
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
        return LogError("Expected ')' or ',' in argument list");
      getNextToken();
    }
  }

  // Eat the ')'.
  getNextToken();

  return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

/// ifexpr ::= 'if' expression 'then' expression 'else' expression
static std::unique_ptr<ExprAST> ParseIfExpr() {
  getNextToken(); // consume 'if'

  auto Cond = ParseExpression();
  if (!Cond)
    return nullptr;

  if (CurTok != Token::then_)
    return LogError("Expected 'then' keyword in an if statement");
  getNextToken(); // consume 'then'

  auto Then = ParseExpression();
  if (!Then)
    return nullptr;

  if (CurTok != Token::else_)
    return LogError("Expected 'else' keyword in an if statement");
  getNextToken(); // consume 'else'

  auto Else = ParseExpression();
  if (!Else)
    return nullptr;

  return std::make_unique<IfExprAST>(std::move(Cond), std::move(Then),
                                     std::move(Else));
}

// forexpr ::= 'for' identifier '=' expr ',' expr ',' (',' expr)? 'in' expr
static std::unique_ptr<ExprAST> ParseForExpr() {
  getNextToken(); // consume 'for'

  if (CurTok != Token::identifier)
    return LogError("Expected identifier after 'for'");

  std::string IdName = lexer::IdentifierStr;
  getNextToken(); // consume identifier

  if (CurTok != '=')
    return LogError("Expected = initializing loop variable");
  getNextToken(); // consume =

  auto Start = ParseExpression();
  if (!Start)
    return nullptr;

  if (CurTok != ',')
    return LogError("Expected , after initializing loop variable");
  getNextToken();

  auto End = ParseExpression();
  if (!End)
    return nullptr;

  // The step expression is optional
  std::unique_ptr<ExprAST> Step;
  if (CurTok == ',') {
    getNextToken(); // consume ,
    Step = ParseExpression();
    if (!Step)
      return nullptr;
  }

  if (CurTok != Token::in_)
    return LogError("Expected 'in' after for");
  getNextToken(); // move cursor over 'in' (consume in)

  auto Body = ParseExpression();
  if (!Body)
    return nullptr;

  return std::make_unique<ForExprAST>(IdName, std::move(Start), std::move(End),
                                      std::move(Step), std::move(Body));
}

/// primary
///   ::= identifierexpr
///   ::= numberexpr
///   ::= parenexpr
static std::unique_ptr<ExprAST> ParsePrimary() {
  switch (CurTok) {
  case Token::identifier:
    return ParseIdentifierExpr();
  case Token::number:
    return ParseNumberExpr();
  case Token::if_:
    return ParseIfExpr();
  case Token::for_:
    return ParseForExpr();
  case static_cast<Token>('('):
    return ParseParenExpr();
  default:
    return LogError("unknown token when expecting an expression");
  }
}

/// unary
///     ::= primary
///     ::= '!' unary
static std::unique_ptr<ExprAST> ParseUnary() {
  // If the current token is not an operator, then it must be a primary expr
  if (!isascii(static_cast<int>(CurTok)) || CurTok == '(' || CurTok == ',')
    return ParsePrimary();

  // If this is a unary operator, read it
  int Opc = static_cast<int>(CurTok);
  getNextToken();
  if (auto Operand = ParseUnary())
    return std::make_unique<UnaryExprAST>(Opc, std::move(Operand));
  return nullptr;
}

/// binoprhs
///   ::= ('+' primary)*
static std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                              std::unique_ptr<ExprAST> LHS) {
  // If this is a binop, find its precedence.
  while (true) {
    int TokPrec = GetTokPrecedence();

    // If this is a binop that binds at least as tightly as the current binop,
    // consume it, otherwise we are done.
    if (TokPrec < ExprPrec)
      return LHS;

    // Okay, we know this is a binop.
    Token BinOp = CurTok;
    getNextToken(); // eat binop

    // Parse the unary expression after the binary operator.
    auto RHS = ParseUnary();
    if (!RHS)
      return nullptr;

    // If BinOp binds less tightly with RHS than the operator after RHS, let
    // the pending operator take RHS as its LHS.
    int NextPrec = GetTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
      if (!RHS)
        return nullptr;
    }

    // Merge LHS/RHS.
    LHS = std::make_unique<BinaryExprAST>(static_cast<char>(BinOp),
                                          std::move(LHS), std::move(RHS));
  }
}

/// expression
///   ::= primary binoprhs
///
static std::unique_ptr<ExprAST> ParseExpression() {
  auto LHS = ParseUnary();
  if (!LHS)
    return nullptr;

  return ParseBinOpRHS(0, std::move(LHS));
}

/// prototype
///   ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> ParsePrototype() {
  std::string FnName;

  unsigned Kind = 0; // 0 id, 1 unary, 2 binary
  unsigned BinaryPrecedence =
      30; // FIXME: parameterize this with the driver or something

  switch (CurTok) {
  default:
    return LogErrorP("Expected function name in prototype");
  case Token::identifier:
    FnName = lexer::IdentifierStr;
    Kind = 0;
    getNextToken();
    break;

  case Token::unary_:
    getNextToken();
    if (!isascii(static_cast<int>(CurTok)))
      return LogErrorP("Expected unary operator");
    FnName = "unary";
    FnName += (char)CurTok;
    Kind = 1;
    getNextToken();
    break;

  case Token::binary_:
    getNextToken();
    if (!isascii(static_cast<int>(CurTok)))
      return LogErrorP("Expected binary operator");
    FnName = "binary";
    FnName += (char)CurTok;
    Kind = 2;
    getNextToken();

    // Read precedence if present
    if (CurTok == Token::number) {
      if (lexer::NumVal < 1 || lexer::NumVal > 100)
        return LogErrorP("Invalid Precedence: must be [1..100]");
      BinaryPrecedence = (unsigned)lexer::NumVal;
      getNextToken();
    }
    break;
  }

  if (CurTok != '(')
    return LogErrorP("Expected '(' in prototype");

  std::vector<std::string> ArgNames;
  while (getNextToken() == Token::identifier)
    ArgNames.push_back(lexer::IdentifierStr);
  if (CurTok != ')')
    return LogErrorP("Expected ')' in prototype");

  // success.
  getNextToken(); // eat ')'.

  return std::make_unique<PrototypeAST>(FnName, std::move(ArgNames));
}

/// definition ::= 'def' prototype expression
std::unique_ptr<FunctionAST> ParseDefinition() {
  getNextToken(); // eat def.
  auto Proto = ParsePrototype();
  if (!Proto)
    return nullptr;

  if (auto E = ParseExpression())
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  return nullptr;
}

/// toplevelexpr ::= expression
std::unique_ptr<FunctionAST> ParseTopLevelExpr() {
  if (auto E = ParseExpression()) {
    // Make an anonymous proto.
    auto Proto = std::make_unique<PrototypeAST>("__anon_expr",
                                                std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

/// external ::= 'extern' prototype
std::unique_ptr<PrototypeAST> ParseExtern() {
  getNextToken(); // eat extern.
  return ParsePrototype();
}
