#ifndef PARSER_H_
#define PARSER_H_

#include "diagnostic.h"
#include "lexer.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace fiddle {

struct FuncDef;

// Abstract base class for expressions
struct Expr {
  virtual ~Expr() {}
  virtual llvm::Value* codegen(
      llvm::BasicBlock*,
      const std::unordered_map<std::string, llvm::Value*>&) const = 0;
  virtual void print(std::ostream&) const = 0;
};

inline std::ostream& operator<<(std::ostream& o, const Expr& expr) {
  expr.print(o);
  return o;
}

struct IntExpr : public Expr {
  int val;

  IntExpr(int val) : val(val) {}
  llvm::Value* codegen(
      llvm::BasicBlock*,
      const std::unordered_map<std::string, llvm::Value*>&) const override;
  void print(std::ostream& o) const override {
    o << "Int(" << val << ")";
  }
};

struct VarExpr : public Expr {
  std::string name;

  VarExpr(std::string name) : name(std::move(name)) {}
  llvm::Value* codegen(
      llvm::BasicBlock*,
      const std::unordered_map<std::string, llvm::Value*>&) const override;
  void print(std::ostream& o) const override {
    o << "Var(" << name << ")";
  }
};

struct BinOpExpr : public Expr {
  std::string name;
  std::unique_ptr<Expr> lhs, rhs;

  BinOpExpr(std::string name,
            std::unique_ptr<Expr> lhs,
            std::unique_ptr<Expr> rhs)
      : name(std::move(name)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

  llvm::Value* codegen(
      llvm::BasicBlock*,
      const std::unordered_map<std::string, llvm::Value*>&) const override;
  void print(std::ostream& o) const override {
    o << "BinOp(" << name << ", " << *lhs << ", " << *rhs << ")";
  }
};

struct FuncDef {
  std::string name;
  std::unique_ptr<Expr> body;
  std::vector<std::string> args;

  FuncDef(std::string name,
          std::unique_ptr<Expr> body,
          std::vector<std::string> args)
      : name(name), body(std::move(body)), args(std::move(args)) {}

  void codegen(llvm::Module* module) const;
};

inline std::ostream& operator<<(std::ostream& o, const FuncDef& fn) {
  o << "FuncDef(name = " << fn.name << ", body = " << *fn.body << ", args = {";
  for (int i = 0, len = fn.args.size(); i < len; ++i) {
    if (i != 0) { o << ", "; }
    o << fn.args[i];
  }
  o << "})";
  return o;
}

struct Parser {
  std::shared_ptr<SourceFile> sourceFile;
  std::vector<Diagnostic> diagnostics;
  Lexer lexer;
  Token currToken;

  explicit Parser(SourceFile file)
      : sourceFile(std::make_shared<SourceFile>(std::move(file))),
        diagnostics(),
        lexer(sourceFile, diagnostics) {
    // Initialize currToken.
    consumeToken();
  }

  // std::unique_ptr<Module> parseModule();
  std::unique_ptr<FuncDef> parseFuncDef();
  std::unique_ptr<Expr> parseExpr();
  std::unique_ptr<Expr> parseExprPrimary();
  std::unique_ptr<Expr> parseExprOperator(std::unique_ptr<Expr> lhs,
                                          u8 minPrecedence);

  Token nextToken();
  Token consumeToken();
  bool atEnd() const;
  bool expectToken(Token::TokenKind expected);
  void report(Diagnostic::DiagnosticLevel level, StringRef message);
};

} // namespace fiddle

#endif /* PARSER_H */
