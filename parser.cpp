#include "parser.h"
#include "util.h"
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <stack>
#include <string>
#include <utility>

namespace fiddle {

llvm::Value* IntExpr::codegen() const {
  return llvm::ConstantInt::get(llvm::getGlobalContext(),
                                llvm::APInt(32, val));
}

llvm::Value* BinOpExpr::codegen() const {
  llvm::Value* left  = lhs->codegen();
  llvm::Value* right = rhs->codegen();
  if (!left || !right) {
    return nullptr;
  }

  llvm::IRBuilder<> builder(llvm::getGlobalContext());
  if (name == "+") {
    return builder.CreateAdd(left, right, "addtmp");
  } else if (name == "-") {
    return builder.CreateSub(left, right, "subtmp");
  } else if (name == "*") {
    return builder.CreateMul(left, right, "multmp");
  } else if (name == "/") {
    return builder.CreateSDiv(left, right, "sdivtmp");
  } else {
    return nullptr;
  }
}

// TODO(tsion): Remove this.
bool isDigit(char c);

i64 digitToInt(char c) {
  assert(isDigit(c));
  return c - '0';
}

// TODO(tsion): Detect integer overflow.
bool parseInt(StringRef str, i64* result, ParseError* err) {
  *result = 0;
  i64 base = 1;
  for (int i = str.length - 1; i >= 0; --i) {
    if (!isDigit(str[i])) {
      err->message = "non-decimal digit in integer literal";
      return false;
    }
    *result += base * digitToInt(str[i]);
    base *= 10;
  }
  return true;
}

std::unique_ptr<FuncDef> Parser::parseFuncDef(ParseError* err) {
  if (!expectToken(Token::kKeywordFn, err)) { return nullptr; }
  Token name = nextToken();
  if (name.isNot(Token::kIdentifier)) {
    err->message = "expected function name after 'fn' keyword";
    return nullptr;
  }
  consumeToken();
  if (!expectToken(Token::kParenLeft, err)) { return nullptr; }
  std::vector<std::string> argNames;
  while (true) {
    Token token = nextToken();
    if (token.is(Token::kParenRight)) {
      consumeToken();
      break;
    }
    token = nextToken();
    if (token.isNot(Token::kIdentifier)) {
      err->message = "expected argument name in fn argument list";
      return nullptr;
    }
    argNames.push_back(token.text().toString());
    consumeToken();
    token = nextToken();
    if (token.is(Token::kComma)) { consumeToken(); }
  }
  if (!expectToken(Token::kBraceLeft, err)) { return nullptr; }
  auto body = parseExpr(err);
  if (!body) { return nullptr; }
  if (!expectToken(Token::kBraceRight, err)) { return nullptr; }
  return make_unique<FuncDef>(name.text().toString(),
                              std::move(body),
                              std::move(argNames));
}

std::unique_ptr<Expr> Parser::parseExpr(ParseError* err) {
  auto expr = parseExprPrimary(err);
  if (!expr) { return nullptr; }
  return parseExprOperator(std::move(expr), 0, err);
}

std::unique_ptr<Expr> Parser::parseExprPrimary(ParseError* err) {
  Token token = nextToken();
  switch (token.kind) {
    case Token::kInvalid:
      err->message = "invalid token";
      return nullptr;
    case Token::kEOF:
      err->message = "unexpected end of file";
      return nullptr;
    case Token::kInteger:
      i64 value;
      if (!parseInt(token.text(), &value, err)) { return nullptr; }
      consumeToken();
      return make_unique<IntExpr>(value);
    case Token::kParenLeft: {
      consumeToken();
      auto e = parseExpr(err);
      if (!expectToken(Token::kParenRight, err)) { return nullptr; }
      return e;
    }
    default:
      err->message = "unexpected token";
      return nullptr;
  }
}

const std::map<std::string, u16> precedenceTable{
  {"+", 0},
  {"-", 0},
  {"*", 1},
  {"/", 1},
};

bool getPrecedence(const std::string& binOp, u16* precedence) {
  auto it = precedenceTable.find(binOp);
  if (it == precedenceTable.end()) { return false; }
  *precedence = it->second;
  return true;
}

std::unique_ptr<Expr> Parser::parseExprOperator(std::unique_ptr<Expr> lhs,
                                                u16 minPrecedence,
                                                ParseError* err) {
  while (!atEnd()) {
    Token token = nextToken();
    std::string op = token.text().toString();
    u16 precedence;
    if (token.isNot(Token::kOperator) || !getPrecedence(op, &precedence) ||
        precedence < minPrecedence) {
      break;
    }
    consumeToken();
    auto rhs = parseExprPrimary(err);
    if (!rhs) { return nullptr; }

    while (!atEnd()) {
      Token token2 = nextToken();
      std::string op2 = token.text().toString();
      u16 precedence2;
      if (token2.isNot(Token::kOperator) || !getPrecedence(op2, &precedence2) ||
          precedence2 <= precedence) {
        break;
      }
      rhs = parseExprOperator(std::move(rhs), precedence2, err);
      if (!rhs) { return nullptr; }
    }

    lhs = make_unique<BinOpExpr>(op, std::move(lhs), std::move(rhs));
  }

  return lhs;

  // while the next token is a binary operator whose precedence is >= min_precedence
  //   op := next token
  //   rhs := parse_primary ()
  //   while the next token is a binary operator whose precedence is greater
  //             than op's, or a right-associative operator
  //             whose precedence is equal to op's
  //       lookahead := next token
  //       rhs := parse_expression_1 (rhs, lookahead's precedence)
  //   lhs := the result of applying op with operands lhs and rhs
  // return lhs
}

bool Parser::expectToken(Token::TokenKind expected, ParseError* err) {
  Token token = nextToken();
  if (token.kind == expected) {
    consumeToken();
    return true;
  }
  // TODO(tsion): Fix the extreme vagueness of this message.
  err->message = "expected one token kind but got another";
  return false;
}

Token Parser::nextToken() {
  return currToken;
}

Token Parser::consumeToken() {
  assert(!atEnd());
  currToken = lexer.nextToken();
  std::cerr << "token: " << currToken << '\n';
  return currToken;
}

bool Parser::atEnd() const {
  return currToken.is(Token::kEOF);
}

} // namespace fiddle
