#ifndef CODEGEN_H_
#define CODEGEN_H_

#include <llvm/IR/Value.h>
#include <unordered_map>

namespace fiddle {

/**
 * A structure to be passed around to the AST node codegen functions during code
 * generation.
 */
struct CodegenContext {
  llvm::BasicBlock* currentBlock;
  std::unordered_map<std::string, llvm::Value*> argumentValues;
};

} // namespace fiddle

#endif /* CODEGEN_H_ */
