#include "ast.h"
#include "codegen.h"
#include <llvm/Analysis/Verifier.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

namespace fiddle {

llvm::Value* IntExpr::codegen(CodegenContext* context) const {
  return llvm::ConstantInt::get(context->module->getContext(),
                                llvm::APInt(32, val));
}

llvm::Value* VarExpr::codegen(CodegenContext* context) const {
  const std::vector<llvm::Value*> v = (*context->identifierMap)[name];
  if (v.empty()) {
    // TODO(tsion): Diagnose reference to undefined name.
    return nullptr;
  }
  return v.back();
}

llvm::Value* BinOpExpr::codegen(CodegenContext* context) const {
  llvm::Value* left  = lhs->codegen(context);
  llvm::Value* right = rhs->codegen(context);
  if (!left || !right) {
    return nullptr;
  }

  llvm::IRBuilder<> builder{context->currentBlock};
  if (name == "+") {
    return builder.CreateAdd(left, right, "add");
  } else if (name == "-") {
    return builder.CreateSub(left, right, "sub");
  } else if (name == "*") {
    return builder.CreateMul(left, right, "mul");
  } else if (name == "/") {
    return builder.CreateSDiv(left, right, "div");
  } else {
    return nullptr;
  }
}

llvm::Value* CallExpr::codegen(CodegenContext* context) const {
  llvm::Value* func = functionExpr->codegen(context);
  std::vector<llvm::Value*> args;
  args.reserve(argumentExprs.size());
  for (const auto& argExpr : argumentExprs) {
    args.push_back(argExpr->codegen(context));
  }
  llvm::IRBuilder<> builder{context->currentBlock};
  return builder.CreateCall(func, args, "call");
}

llvm::Function* createFunc(const FuncDef& def, llvm::Module* module) {
  llvm::Type* i32Type = llvm::IntegerType::get(module->getContext(), 32);
  std::vector<llvm::Type*> argTypes(def.args.size(), i32Type);

  return llvm::Function::Create(
      llvm::FunctionType::get(i32Type, argTypes, false),
      llvm::GlobalValue::ExternalLinkage,
      def.name,
      module);
}

llvm::Value* BlockExpr::codegen(CodegenContext* context) const {
  return nullptr;
}

void transFuncDef(
    const FuncDef& def,
    llvm::Function* func,
    llvm::Module* module,
    std::unordered_map<std::string, std::vector<llvm::Value*>>* identifierMap) {
  usize i = 0;
  for (auto it = func->arg_begin(); it != func->arg_end(); ++it, ++i) {
    it->setName(def.args[i]);
    (*identifierMap)[def.args[i]].push_back(it);
  }

  llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(
      module->getContext(),
      "entry",
      func,
      nullptr);

  CodegenContext context{module, entryBlock, identifierMap};
  llvm::Value* result = def.body->codegen(&context);

  for (const auto& arg : def.args) {
    (*identifierMap)[arg].pop_back();
  }

  llvm::IRBuilder<> builder{entryBlock};
  builder.CreateRet(result);

  assert(!llvm::verifyFunction(*func));
}

std::unique_ptr<llvm::Module> Module::codegen() const {
  auto llmodule = make_unique<llvm::Module>("fiddle", llvm::getGlobalContext());

  std::unordered_map<std::string, llvm::Function*> functionMap;
  std::unordered_map<std::string, std::vector<llvm::Value*>> identifierMap;

  for (const auto& fn : functions) {
    llvm::Function* llfunc = createFunc(fn, llmodule.get());
    identifierMap[fn.name].push_back(llfunc);
    functionMap[fn.name] = llfunc;
  }

  for (const auto& fn : functions) {
    transFuncDef(fn, functionMap[fn.name], llmodule.get(), &identifierMap);
  }

  assert(!llvm::verifyModule(*llmodule));

  return llmodule;
};

} // namespace fiddle
