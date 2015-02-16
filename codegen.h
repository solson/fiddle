#ifndef CODEGEN_H_
#define CODEGEN_H_

#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <unordered_map>
#include <vector>

namespace fiddle {

/**
 * A structure to be passed around to the AST node codegen functions during code
 * generation.
 */
struct CodegenContext {
  llvm::Module* module;
  llvm::BasicBlock* currentBlock;
  std::unordered_map<std::string, std::vector<llvm::Value*>>* identifierMap;
};

} // namespace fiddle

#endif /* CODEGEN_H_ */
