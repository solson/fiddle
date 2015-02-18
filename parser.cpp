#include "parser.h"
#include "util.h"
#include <string>
#include <utility>

namespace fiddle {

std::unique_ptr<Module> Parser::parseModule() {
  std::vector<std::unique_ptr<Func>> fns;
  while (!atEnd()) {
    switch (currToken.kind) {
      case Token::kKeywordFn: {
        auto fn = parseFuncDef();
        if (!fn) { return nullptr; }
        fns.push_back(std::move(fn));
        break;
      }

      case Token::kKeywordExtern: {
        auto fn = parseExternFunc();
        if (!fn) { return nullptr; }
        fns.push_back(std::move(fn));
        break;
      }

      default:
        return nullptr;
    }
  }

  return make_unique<Module>(std::move(fns));
}

// Parse the prototype of a function (its name and arguments).
// E.g. "fn foo(a, b, c)"
std::unique_ptr<FuncProto> Parser::parseFuncProto() {
  if (!expectToken(Token::kKeywordFn)) { return nullptr; }

  if (currToken.kind != Token::kIdentifier) {
    report(Diagnostic::kError, "expected function name after 'fn' keyword",
           currToken);
    return nullptr;
  }
  std::string functionName = currToken.text().toString();
  consumeToken();

  if (!expectToken(Token::kParenLeft)) { return nullptr; }
  std::vector<std::string> argNames;

  while (true) {
    if (currToken.kind == Token::kParenRight) {
      consumeToken();
      break;
    }
    if (currToken.kind != Token::kIdentifier) {
      report(Diagnostic::kError, "expected argument name in fn argument list",
             currToken);
      return nullptr;
    }
    argNames.push_back(currToken.text().toString());
    consumeToken();
    if (currToken.kind == Token::kComma) { consumeToken(); }
  }

  return make_unique<FuncProto>(std::move(functionName), std::move(argNames));
}

std::unique_ptr<ExternFunc> Parser::parseExternFunc() {
  if (!expectToken(Token::kKeywordExtern)) { return nullptr; }
  std::unique_ptr<FuncProto> proto = parseFuncProto();
  if (!proto) { return nullptr; }
  return make_unique<ExternFunc>(std::move(*proto));
}

std::unique_ptr<FuncDef> Parser::parseFuncDef() {
  std::unique_ptr<FuncProto> proto = parseFuncProto();
  if (!proto) { return nullptr; }
  std::unique_ptr<Expr> body = parseBlockExpr();
  if (!body) { return nullptr; }
  return make_unique<FuncDef>(std::move(*proto), std::move(body));
}

std::unique_ptr<Expr> Parser::parseBlockExpr() {
  if (!expectToken(Token::kBraceLeft)) { return nullptr; }
  std::vector<std::unique_ptr<Expr>> exprs;

  while (true) {
    if (currToken.kind == Token::kBraceRight) {
      consumeToken();
      break;
    }

    std::unique_ptr<Expr> expr = parseExpr();
    if (!expr) { return nullptr; }
    exprs.push_back(std::move(expr));

    if (currToken.kind == Token::kSemicolon) { consumeToken(); }
  }

  return make_unique<BlockExpr>(std::move(exprs));
}

std::unique_ptr<Expr> Parser::parseExpr() {
  auto expr = parseExprPrimary();
  if (!expr) { return nullptr; }
  return parseExprOperator(std::move(expr), 0);
}

std::unique_ptr<Expr> Parser::parseExprPrimary() {
  std::unique_ptr<Expr> expr;
  Token token = currToken;

  switch (token.kind) {
    case Token::kInvalid:
      report(Diagnostic::kError, "invalid token", token);
      // Skip past the invalid token and keep trying to parse an expression.
      consumeToken();
      expr = parseExprPrimary();
      break;

    case Token::kEOF:
      report(Diagnostic::kError, "unexpected end of file", token);
      return nullptr;

    case Token::kInteger:
      consumeToken();
      expr = make_unique<IntExpr>(token.intValue);
      break;

    case Token::kIdentifier:
      consumeToken();
      expr = make_unique<VarExpr>(token.text().toString());
      break;

    case Token::kParenLeft:
      consumeToken();
      expr = parseExpr();
      if (!expectToken(Token::kParenRight)) { return nullptr; }
      break;

    case Token::kBraceLeft:
      expr = parseBlockExpr();
      break;

    default:
      report(Diagnostic::kError, "unexpected token", token);
      return nullptr;
  }

  // Parse function call expressions.
  if (currToken.kind == Token::kParenLeft) {
    consumeToken();
    std::vector<std::unique_ptr<Expr>> argumentExprs;

    while (true) {
      if (currToken.kind == Token::kParenRight) {
        consumeToken();
        break;
      }
      auto argumentExpr = parseExpr();
      if (!argumentExpr) { return nullptr; }
      argumentExprs.push_back(std::move(argumentExpr));
      if (currToken.kind == Token::kComma) { consumeToken(); }
    }

    return make_unique<CallExpr>(std::move(expr), std::move(argumentExprs));
  }

  return expr;
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
    std::string op = currToken.text().toString();
    u8 precedence;
    if (currToken.kind != Token::kOperator ||
        !getPrecedence(op, &precedence) ||
        precedence < minPrecedence) {
      break;
    }
    consumeToken();

    auto rhs = parseExprPrimary();
    if (!rhs) { return nullptr; }

    while (!atEnd()) {
      std::string op2 = currToken.text().toString();
      u8 precedence2;
      if (currToken.kind != Token::kOperator ||
          !getPrecedence(op2, &precedence2) ||
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

Token Parser::consumeToken() {
  assert(!atEnd());
  currToken = lexer.nextToken();
  std::cerr << "token: " << currToken << '\n';
  return currToken;
}

bool Parser::atEnd() const {
  return currToken.kind == Token::kEOF;
}

bool Parser::expectToken(Token::TokenKind expected) {
  if (currToken.kind == expected) {
    consumeToken();
    return true;
  }
  // TODO(tsion): Fix the extreme vagueness of this message.
  report(Diagnostic::kError, "expected one token kind but got another",
         currToken);
  return false;
}

void Parser::report(Diagnostic::DiagnosticLevel level, StringRef message,
                    const Token& token) {
  diagnostics.emplace_back(
      Diagnostic{level, message.toString(), token.location});
}

} // namespace fiddle
