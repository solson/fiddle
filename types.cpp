#include "types.h"
#include <llvm/IR/Module.h>

namespace fl {
namespace type {

llvm::Type* Int::llvmType(const llvm::Module* module) const {
  return llvm::IntegerType::get(module->getContext(), bits);
}

llvm::Type* Unit::llvmType(const llvm::Module* module) const {
  return llvm::StructType::get(module->getContext(), false);
}

void Int::dump(std::ostream& o) const {
  o << (signed_ ? 'i' : 'u') << bits;
}

void Unit::dump(std::ostream& o) const {
  o << "()";
}

} // namespace type
} // namespace fl
