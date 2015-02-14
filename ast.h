#ifndef AST_H_
#define AST_H_

#include "codegen.h"
#include "diagnostic.h"
#include "lexer.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

namespace fiddle {

// Abstract base class for expressions
struct Expr {
  virtual ~Expr() {}
  virtual llvm::Value* codegen(CodegenContext*) const = 0;
  virtual void print(std::ostream&) const = 0;
};

inline std::ostream& operator<<(std::ostream& o, const Expr& expr) {
  expr.print(o);
  return o;
}

struct IntExpr : public Expr {
  int val;

  IntExpr(int val) : val(val) {}
  llvm::Value* codegen(CodegenContext*) const override;
  void print(std::ostream& o) const override {
    o << "Int(" << val << ")";
  }
};

struct VarExpr : public Expr {
  std::string name;

  VarExpr(std::string name) : name(std::move(name)) {}
  llvm::Value* codegen(CodegenContext*) const override;
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

  llvm::Value* codegen(CodegenContext*) const override;
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

struct Module {
  std::vector<FuncDef> functions;

  Module(std::vector<FuncDef> functions) : functions(std::move(functions)) {}
  std::unique_ptr<llvm::Module> codegen() const;
};

inline std::ostream& operator<<(std::ostream& o, const Module& module) {
  o << "Module\n";
  for (const auto& fn : module.functions) {
    o << "  " << fn << '\n';
  }
  return o;
}

} // namespace fiddle

#endif /* AST_H_ */
