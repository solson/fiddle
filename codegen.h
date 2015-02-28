#ifndef CODEGEN_H_
#define CODEGEN_H_

#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <unordered_map>
#include <vector>

namespace fl {

struct ModuleContext {
  llvm::Module* module;
  std::unordered_map<std::string, std::vector<llvm::Value*>> identifierMap;

  ModuleContext(llvm::Module* module) : module(module) {}
};

/**
 * A structure to be passed around to the expression codegen functions during
 * code generation.
 */
struct FuncContext {
  llvm::Module* module;
  llvm::BasicBlock* currentBlock;
  std::unordered_map<std::string, std::vector<llvm::Value*>>* identifierMap;
};

} // namespace fl

#endif /* CODEGEN_H_ */
