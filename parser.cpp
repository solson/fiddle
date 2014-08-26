#include "parser.h"
#include <cctype>
#include <queue>
#include <stack>
#include <string>
#include <utility>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/LLVMContext.h>
#include <unicode/utf8.h>

#include <iostream>

namespace fiddle {

IntExpr::IntExpr(int val) : val_(val) {}

llvm::Value* IntExpr::Codegen() const {
  return llvm::ConstantInt::get(llvm::getGlobalContext(),
                                llvm::APInt(32, val_));
}

BinOpExpr::BinOpExpr(BinOp op,
                     std::unique_ptr<Expr> lhs,
                     std::unique_ptr<Expr> rhs)
    : op_(op), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

llvm::Value* BinOpExpr::Codegen() const {
  llvm::Value* left  = lhs_->Codegen();
  llvm::Value* right = rhs_->Codegen();
  if (!left || !right) {
    return nullptr;
  }

  llvm::IRBuilder<> b(llvm::getGlobalContext());
  switch (op_) {
    case BinOp::kAdd:
      return b.CreateAdd(left, right, "addtmp");
    case BinOp::kSub:
      return b.CreateSub(left, right, "subtmp");
    case BinOp::kMul:
      return b.CreateMul(left, right, "multmp");
    case BinOp::kDiv:
      return b.CreateSDiv(left, right, "sdivtmp");
  }
}

Parser::Parser(std::string source) : source_(std::move(source)) {}

std::unique_ptr<FuncDef> Parser::parseFunction() {
  return nullptr;
}

std::unique_ptr<Expr> Parser::parsePrimary() {
  skipWhitespace();
  if (atEnd()) {
    // TODO: parse error
    return nullptr;
  }

  char32_t c = peekChar();
  if (std::isdigit(c)) {
    return parseInt();
  } else if (c == '(') {
    std::unique_ptr<Expr> e = parseExpr();
    // expect ')'
    return e;
  } else {
    return nullptr;
  }
}

// TODO: strings, not chars
std::map<char32_t, BinOp> bin_ops{
  {'+', BinOp::kAdd},
  {'-', BinOp::kSub},
  {'*', BinOp::kMul},
  {'/', BinOp::kDiv},
};

std::map<BinOp, unsigned> precedence_table{
  {BinOp::kAdd, 0},
  {BinOp::kSub, 0},
  {BinOp::kMul, 1},
  {BinOp::kDiv, 1},
};

// TODO: move to utils header
template <typename T>
typename T::mapped_type lookupOrDie(const T& t, const typename T::key_type& k) {
  return t.find(k)->second;
}

template <typename T>
bool containsKey(const T& t, const typename T::key_type& k) {
  return t.find(k) != t.end();
}

BinOp binOp(char32_t c) {
  return lookupOrDie(bin_ops, c);
}

unsigned precedence(char32_t c) {
  return lookupOrDie(precedence_table, lookupOrDie(bin_ops, c));
}

std::unique_ptr<Expr> Parser::parseExpr1(std::unique_ptr<Expr> lhs,
                                         unsigned min_precedence) {
  while (!atEnd()) {
    skipWhitespace();
    char32_t c = peekChar();
    if (!containsKey(bin_ops, c) || precedence(c) < min_precedence) {
      break;
    }
    nextChar();
    std::unique_ptr<Expr> rhs = parsePrimary();

    while (!atEnd()) {
      skipWhitespace();
      char32_t c2 = peekChar();
      if (!containsKey(bin_ops, c) || precedence(c2) <= precedence(c)) {
        break;
      }
      rhs = parseExpr1(std::move(rhs), precedence(c2));
    }

    lhs = std::unique_ptr<Expr>(
        new BinOpExpr(binOp(c), std::move(lhs), std::move(rhs)));
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

std::unique_ptr<Expr> Parser::parseExpr() {
  return parseExpr1(parsePrimary(), 0);
}

std::unique_ptr<IntExpr> Parser::parseInt() {
  std::string buf;
  while (!atEnd()) {
    char32_t c = peekChar();
    if (!std::isdigit(c)) {
      break;
    }
    buf.push_back(c);
    nextChar();
  }

  // TODO: Handle overflow from stoi
  return std::unique_ptr<IntExpr>(new IntExpr(std::stoi(buf)));
}

void Parser::skipWhitespace() {
  while (!atEnd()) {
    char32_t c = peekChar();
    if (!std::isspace(c)) {
      return;
    }
    nextChar();
  }
}

char32_t Parser::peekChar() {
  int32_t position_before = position_;

  const char* string = source_.data();
  int32_t length = source_.length();
  char32_t c;
  U8_NEXT(string, position_, length, c);

  position_ = position_before;
  return c;
}

char32_t Parser::nextChar() {
  const char* string = source_.data();
  int32_t length = source_.length();
  char32_t c;
  U8_NEXT(string, position_, length, c);

  // TODO: Check for error condition (negative c).

  if (c == '\n') {
    ++line_;
    column_ = 0;
  } else {
    column_++;
  }

  return c;
}

bool Parser::atEnd() const {
  return static_cast<size_t>(position_) == source_.length();
}

}
