#ifndef PARSER_H_
#define PARSER_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <unicode/chariter.h>
#include <llvm/IR/Value.h>

#include <iostream>

namespace fiddle {

/*

fn main() -> int {
  return 3 + 3 * 13;
}

*/

class Expr;

// Abstract base class for expressions
class Expr {
 public:
  virtual ~Expr() {}
  virtual llvm::Value* Codegen() const = 0;
  virtual void Debug() const = 0;
};

class IntExpr : public Expr {
 public:
  IntExpr(int val);
  llvm::Value* Codegen() const override;
  void Debug() const override {
    std::cout << "IntExpr(" << val_ << ")";
  }
 private:
  int val_;
};

enum class BinOp {
  kAdd, kSub, kMul, kDiv
};

class BinOpExpr : public Expr {
 public:
  BinOpExpr(BinOp op, std::unique_ptr<Expr> lhs, std::unique_ptr<Expr> rhs);
  llvm::Value* Codegen() const override;
  void Debug() const override {
    std::cout << "BinOpExpr(" << int(op_) << ", ";
    lhs_->Debug();
    std::cout << ", ";
    rhs_->Debug();
    std::cout << ")";
  }
 private:
  BinOp op_;
  std::unique_ptr<Expr> lhs_, rhs_;
};

class FuncDef {
 public:
  FuncDef(std::string name,
          std::unique_ptr<Expr> body);
 private:
  std::unique_ptr<Expr> body_;
};

class Parser {
 public:
  Parser(std::string source);

  std::unique_ptr<FuncDef> parseFunction();
  std::unique_ptr<Expr> parsePrimary();
  std::unique_ptr<Expr> parseExpr1(std::unique_ptr<Expr> lhs,
                                   unsigned min_precedence);
  std::unique_ptr<Expr> parseExpr();
  std::unique_ptr<IntExpr> parseInt();

 private:
  void skipWhitespace();
  char32_t peekChar();
  char32_t nextChar();
  bool atEnd() const;

  const std::string source_;
  int32_t position_ = 0, column_ = 1, line_ = 1;
};

}

#endif /* PARSER_H_ */
