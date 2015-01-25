#ifndef PARSER_H_
#define PARSER_H_

#include "lexer.h"
#include <llvm/IR/Value.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace fiddle {

// Abstract base class for expressions
struct Expr {
  virtual ~Expr() {}
  virtual llvm::Value* codegen() const = 0;
  virtual void debug(std::ostream&) const = 0;
  void debug() const { debug(std::cerr); }
};

struct IntExpr : public Expr {
  int val;

  IntExpr(int val) : val(val) {}
  llvm::Value* codegen() const override;
  void debug(std::ostream& o) const override {
    o << "Int(" << val << ")";
  }
};

struct BinOpExpr : public Expr {
  std::string name;
  std::unique_ptr<Expr> lhs, rhs;

  BinOpExpr(std::string name, std::unique_ptr<Expr> lhs,
            std::unique_ptr<Expr> rhs)
      : name(std::move(name)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

  llvm::Value* codegen() const override;

  void debug(std::ostream& o) const override {
    o << "BinOp(" << name << ", ";
    lhs->debug(o);
    o << ", ";
    rhs->debug(o);
    o << ")";
  }
};

struct FuncDef {
  std::unique_ptr<Expr> body;

  FuncDef(std::string name, std::unique_ptr<Expr> body);
};

struct ParseError {
  std::string message;
};

struct Parser {
  Lexer lexer;
  Token currToken;

  explicit Parser(Lexer lexer) : lexer(std::move(lexer)) {
    // Initialize currToken.
    consumeToken();
  }

  std::unique_ptr<Expr> parseExpr(ParseError* err);
  std::unique_ptr<Expr> parseExprPrimary(ParseError* err);
  std::unique_ptr<Expr> parseExprOperator(std::unique_ptr<Expr> lhs,
                                          u16 minPrecedence,
                                          ParseError* err);

  bool expectToken(Token::TokenKind expected, ParseError* err);
  Token nextToken();
  Token consumeToken();
  bool atEnd() const;
};

} // namespace fiddle

#endif /* PARSER_H */
