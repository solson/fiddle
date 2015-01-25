#ifndef UTIL_H_
#define UTIL_H_

#include <cassert>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <memory>

namespace fiddle {

using i8    = std::int8_t;
using i16   = std::int16_t;
using i32   = std::int32_t;
using i64   = std::int64_t;
using isize = std::intptr_t;
using u8    = std::uint8_t;
using u16   = std::uint16_t;
using u32   = std::uint32_t;
using u64   = std::uint64_t;
using usize = std::uintptr_t;

/*
 * A structure representing an immutable view into a string. *Implicitly*
 * convertible from char* and std::string for use in function arguments. Just
 * write foo(StringRef str) instead of foo(const std::string&) and foo(char*).
 *
 * Note that StringRef doesn't own the data it's viewing.
 */
struct StringRef {
  const char* data;
  usize length;

  // These constructors allow implicit conversion.
  StringRef(const std::string& str) : data(str.data()), length(str.size()) {}
  StringRef(const char* str) : data(str), length(std::strlen(str)) {}

  // More efficient char* construction if the caller knows the length.
  StringRef(const char* str, usize size) : data(str), length(size) {}

  std::string toString() const {
    return std::string{data, length};
  }

  bool operator==(StringRef other) const {
    return length == other.length &&
        std::strncmp(data, other.data, length) == 0;
  }

  bool operator!=(StringRef other) const {
    return !(*this == other);
  }

  const char& operator[](usize i) const {
    assert(i < length);
    return data[i];
  }

  const char* begin() const { return data; }
  const char* end() const { return data + length; }
};

inline std::ostream& operator<<(std::ostream& o, StringRef str) {
  return o.write(str.data, str.length);
}

template<typename T, typename... Args>
std::unique_ptr<T> make_unique(Args&&... args) {
  return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

template <typename T>
typename T::mapped_type lookupOrDie(const T& t, const typename T::key_type& k) {
  return t.find(k)->second;
}

template <typename T>
bool containsKey(const T& t, const typename T::key_type& k) {
  return t.find(k) != t.end();
}

} // namespace fiddle

#endif /* UTIL_H_ */
