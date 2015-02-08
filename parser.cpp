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

// std::vector<std::unique_ptr<FuncDef>> Parser::parseFuncDefs() {
//   std::vector<std::unique_ptr<FuncDef>> fns;
//   while (!atEnd()) {
//     Token token = nextToken();
//     if (token.isNot(Token::kKeywordFn)) { break; }
//     auto fn = parseFuncDef();
//     if (!fn) { break }
//     fns.push_back(std::move(fn));
//   }
//   return fns;
// }

std::unique_ptr<FuncDef> Parser::parseFuncDef() {
  if (!expectToken(Token::kKeywordFn)) { return nullptr; }
  Token name = nextToken();
  if (name.isNot(Token::kIdentifier)) {
    report(Diagnostic::kError, "expected function name after 'fn' keyword");
    return nullptr;
  }
  consumeToken();
  if (!expectToken(Token::kParenLeft)) { return nullptr; }
  std::vector<std::string> argNames;
  while (true) {
    Token token = nextToken();
    if (token.is(Token::kParenRight)) {
      consumeToken();
      break;
    }
    token = nextToken();
    if (token.isNot(Token::kIdentifier)) {
      report(Diagnostic::kError, "expected argument name in fn argument list");
      return nullptr;
    }
    argNames.push_back(token.text().toString());
    consumeToken();
    token = nextToken();
    if (token.is(Token::kComma)) { consumeToken(); }
  }
  if (!expectToken(Token::kBraceLeft)) { return nullptr; }
  auto body = parseExpr();
  if (!body) { return nullptr; }
  if (!expectToken(Token::kBraceRight)) { return nullptr; }
  return make_unique<FuncDef>(name.text().toString(),
                              std::move(body),
                              std::move(argNames));
}

std::unique_ptr<Expr> Parser::parseExpr() {
  auto expr = parseExprPrimary();
  if (!expr) { return nullptr; }
  return parseExprOperator(std::move(expr), 0);
}

std::unique_ptr<Expr> Parser::parseExprPrimary() {
  Token token = nextToken();
  switch (token.kind) {
    case Token::kInvalid:
      report(Diagnostic::kError, "invalid token");
      // Skip past the invalid token and keep trying to parse an expression.
      consumeToken();
      return parseExprPrimary();

    case Token::kEOF:
      report(Diagnostic::kError, "unexpected end of file");
      return nullptr;

    case Token::kInteger:
      consumeToken();
      return make_unique<IntExpr>(token.intValue);

    case Token::kIdentifier:
      consumeToken();
      return make_unique<VarExpr>(token.text().toString());

    case Token::kParenLeft: {
      consumeToken();
      auto e = parseExpr();
      if (!expectToken(Token::kParenRight)) { return nullptr; }
      return e;
    }

    default:
      report(Diagnostic::kError, "unexpected token");
      return nullptr;
  }
}

const std::map<std::string, u8> kPrecedenceTable{
  {"+", 0},
  {"-", 0},
  {"*", 1},
  {"/", 1},
};

bool getPrecedence(const std::string& binOp, u8* precedence) {
  auto it = kPrecedenceTable.find(binOp);
  if (it == kPrecedenceTable.end()) { return false; }
  *precedence = it->second;
  return true;
}

std::unique_ptr<Expr> Parser::parseExprOperator(std::unique_ptr<Expr> lhs,
                                                u8 minPrecedence) {
  while (!atEnd()) {
    Token token = nextToken();
    std::string op = token.text().toString();
    u8 precedence;
    if (token.isNot(Token::kOperator) || !getPrecedence(op, &precedence) ||
        precedence < minPrecedence) {
      break;
    }
    consumeToken();
    auto rhs = parseExprPrimary();
    if (!rhs) { return nullptr; }

    while (!atEnd()) {
      Token token2 = nextToken();
      std::string op2 = token.text().toString();
      u8 precedence2;
      if (token2.isNot(Token::kOperator) || !getPrecedence(op2, &precedence2) ||
          precedence2 <= precedence) {
        break;
      }
      rhs = parseExprOperator(std::move(rhs), precedence2);
      if (!rhs) { return nullptr; }
    }

    lhs = make_unique<BinOpExpr>(op, std::move(lhs), std::move(rhs));
  }

  return lhs;
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

bool Parser::expectToken(Token::TokenKind expected) {
  Token token = nextToken();
  if (token.kind == expected) {
    consumeToken();
    return true;
  }
  // TODO(tsion): Fix the extreme vagueness of this message.
  report(Diagnostic::kError, "expected one token kind but got another");
  return false;
}

void Parser::report(Diagnostic::DiagnosticLevel level, StringRef message) {
  diagnostics.emplace_back(Diagnostic{level, message.toString()});
}

} // namespace fiddle
