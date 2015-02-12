#include "ast.h"
#include "codegen.h"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>

namespace fiddle {

llvm::Value* IntExpr::codegen(CodegenContext*) const {
  return llvm::ConstantInt::get(llvm::getGlobalContext(),
                                llvm::APInt(32, val));
}

llvm::Value* VarExpr::codegen(CodegenContext* context) const {
  auto it = context->argumentValues.find(name);
  if (it == context->argumentValues.end()) { return nullptr; }
  return it->second;
}

llvm::Value* BinOpExpr::codegen(CodegenContext* context) const {
  llvm::Value* left  = lhs->codegen(context);
  llvm::Value* right = rhs->codegen(context);
  if (!left || !right) {
    return nullptr;
  }

  llvm::IRBuilder<> builder{context->currentBlock};
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

void FuncDef::codegen(llvm::Module* module) const {
  using namespace llvm;

  Type* i32Type = IntegerType::get(module->getContext(), 32);
  std::vector<Type*> argTypes{args.size(), i32Type};

  Function* func = Function::Create(
      FunctionType::get(i32Type, argTypes, false),
      GlobalValue::ExternalLinkage,
      name,
      module);

  std::unordered_map<std::string, Value*> argumentValues;

  usize i = 0;
  for (Function::arg_iterator it = func->arg_begin();
       i != args.size();
       ++it, ++i) {
    it->setName(args[i]);
    argumentValues[args[i]] = it;
  }

  BasicBlock* entryBlock = BasicBlock::Create(
      module->getContext(),
      "entry",
      func,
      nullptr);

  CodegenContext context{entryBlock, argumentValues};
  Value* result = body->codegen(&context);

  IRBuilder<> builder{entryBlock};
  builder.CreateRet(result);
}

void Module::codegen(llvm::Module* module) const {
  for (const auto& fn : functions) {
    fn.codegen(module);
  }
};

} // namespace fiddle
