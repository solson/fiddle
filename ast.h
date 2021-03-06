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

namespace fl {
namespace ast {

// Abstract base class for AST nodes providing convenience functions like debug
// printing.
struct Node {
  virtual ~Node() {}
  virtual void dump(std::ostream& o = std::cerr) const = 0;
};

inline std::ostream& operator<<(std::ostream& o, const Node& node) {
  node.dump(o);
  return o;
}

// Abstract base class for expressions.
struct Expr : public Node {
  virtual llvm::Value* codegen(FuncContext*) const = 0;
};

struct IntExpr : public Expr {
  i64 val;

  IntExpr(i64 val) : val(val) {}
  llvm::Value* codegen(FuncContext*) const override;
  void dump(std::ostream& o) const override;
};

struct VarExpr : public Expr {
  std::string name;

  VarExpr(std::string name) : name(std::move(name)) {}
  llvm::Value* codegen(FuncContext*) const override;
  void dump(std::ostream& o) const override;
};

struct BinOpExpr : public Expr {
  std::string name;
  std::unique_ptr<Expr> lhs, rhs;

  BinOpExpr(std::string name,
            std::unique_ptr<Expr> lhs,
            std::unique_ptr<Expr> rhs)
      : name(std::move(name)), lhs(std::move(lhs)), rhs(std::move(rhs)) {}

  llvm::Value* codegen(FuncContext*) const override;
  void dump(std::ostream& o) const override;
};

struct CallExpr : public Expr {
  std::unique_ptr<Expr> functionExpr;
  std::vector<std::unique_ptr<Expr>> argumentExprs;

  CallExpr(std::unique_ptr<Expr> functionExpr,
           std::vector<std::unique_ptr<Expr>> argumentExprs)
      : functionExpr(std::move(functionExpr)),
        argumentExprs(std::move(argumentExprs)) {}

  llvm::Value* codegen(FuncContext*) const override;
  void dump(std::ostream& o) const override;
};

struct BlockExpr : public Expr {
  std::vector<std::unique_ptr<Expr>> exprs;

  BlockExpr(std::vector<std::unique_ptr<Expr>> exprs)
      : exprs(std::move(exprs)) {}

  llvm::Value* codegen(FuncContext*) const override;
  void dump(std::ostream& o) const override;
};

// Abstract base class for type expressions.
struct Type : public Node {
  virtual ~Type() {}
};

struct TypeName : public Type {
  std::string name;

  TypeName(std::string name) : name(std::move(name)) {}

  void dump(std::ostream& o) const override;
};

struct UnitType : public Type {
  UnitType() {}

  void dump(std::ostream& o) const override;
};

// Function prototype (name, arguments, types).
struct FuncProto : public Node {
  std::string name;
  std::vector<std::string> argNames;
  std::vector<std::unique_ptr<Type>> argTypes;
  std::unique_ptr<Type> returnType;

  FuncProto(std::string name,
            std::vector<std::string> argNames,
            std::vector<std::unique_ptr<Type>> argTypes,
            std::unique_ptr<Type> returnType)
      : name(std::move(name)),
        argNames(std::move(argNames)),
        argTypes(std::move(argTypes)),
        returnType(std::move(returnType)) {}

  void dump(std::ostream& o) const override;
};

// Abstract parent class for different function types (native and extern).
struct Func : public Node {
  FuncProto proto;

  Func(FuncProto proto) : proto(std::move(proto)) {}
  virtual ~Func() {}
  virtual void codegen(ModuleContext*, llvm::Function*) const = 0;
};

// A declaration of an external function (from ASM, C, etc).
struct ExternFunc : public Func {
  using Func::Func; // Inherited constructor.
  virtual void codegen(ModuleContext*, llvm::Function*) const override {}
  void dump(std::ostream& o) const override;
};

// A natively defined function with a body.
struct FuncDef : public Func {
  std::unique_ptr<Expr> body;

  FuncDef(FuncProto proto, std::unique_ptr<Expr> body)
      : Func(std::move(proto)), body(std::move(body)) {}
  virtual void codegen(ModuleContext*, llvm::Function*) const override;
  void dump(std::ostream& o) const override;
};

struct Module : public Node {
  std::vector<std::unique_ptr<Func>> functions;

  Module(std::vector<std::unique_ptr<Func>> functions)
      : functions(std::move(functions)) {}
  std::unique_ptr<llvm::Module> codegen() const;
  void dump(std::ostream& o) const override;
};

} // namespace ast
} // namespace fl

#endif /* AST_H_ */
