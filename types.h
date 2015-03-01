#ifndef TYPES_H_
#define TYPES_H_

#include "util.h"
#include <llvm/IR/Type.h>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>

namespace fl {
namespace type {

// Abstract base class for types.
struct Type {
  virtual ~Type() {}
  virtual llvm::Type* llvmType(const llvm::Module*) const = 0;
  virtual void dump(std::ostream& o = std::cerr) const = 0;
};

inline std::ostream& operator<<(std::ostream& o, const Type& type) {
  type.dump(o);
  return o;
}

// Primitive integer types.
struct Int : public Type {
  u32 bits;
  bool signed_;
  Int(u32 bits, bool signed_) : bits(bits), signed_(signed_) {}
  llvm::Type* llvmType(const llvm::Module*) const override;
  void dump(std::ostream& o = std::cerr) const override;
};

struct Unit : public Type {
  Unit() {}
  llvm::Type* llvmType(const llvm::Module*) const override;
  void dump(std::ostream& o = std::cerr) const override;
};

} // namespace type
} // namespace fl

#endif /* TYPES_H_ */
