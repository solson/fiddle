#include "ast.h"

namespace fl {
namespace ast {

void IntExpr::dump(std::ostream& o) const {
  o << "Int(" << val << ")";
}

void VarExpr::dump(std::ostream& o) const {
  o << "Var(" << name << ")";
}

void BinOpExpr::dump(std::ostream& o) const {
  o << "BinOp(" << name << ", " << *lhs << ", " << *rhs << ")";
}

void CallExpr::dump(std::ostream& o) const {
  o << "FuncCall(func = " << *functionExpr << ", args = {";
  for (int i = 0, len = argumentExprs.size(); i < len; ++i) {
    if (i != 0) { o << ", "; }
    o << *argumentExprs[i];
  }
  o << "})";
}

void BlockExpr::dump(std::ostream& o) const {
  o << "Block{";
  for (int i = 0, len = exprs.size(); i < len; ++i) {
    if (i != 0) { o << "; "; }
    o << *exprs[i];
  }
  o << "}";
}

void Type::dump(std::ostream& o) const {
  o << "Type(" << name << ")";
}

void FuncProto::dump(std::ostream& o) const {
  o << "FuncProto(name = " << name << ", args = {";
  for (int i = 0, len = argNames.size(); i < len; ++i) {
    if (i != 0) { o << ", "; }
    o << argNames[i] << ": " << argTypes[i];
  }
  o << "}, returnType = " << returnType << ")";
}

void ExternFunc::dump(std::ostream& o) const {
  o << "ExternFunc(proto = " << proto << ")";
}

void FuncDef::dump(std::ostream& o) const {
  o << "FuncDef(proto = " << proto << ", body = " << *body << ")";
}

void Module::dump(std::ostream& o) const {
  o << "Module{\n";
  for (const auto& fn : functions) {
    o << "  " << *fn << ";\n";
  }
  o << "}\n";
}

} // namespace ast
} // namespace fl
