#include "token.h"
#include <algorithm>
#include <cassert>
#include <functional>

namespace fl {

SourceCoordinates SourceFile::findCoordinates(usize offset) const {
  assert(offset < source.size());
  assert(std::is_sorted(newlineOffsets.begin(), newlineOffsets.end()));

  // Find the first newline at or after the offset.
  auto after = std::lower_bound(newlineOffsets.begin(), newlineOffsets.end(),
                                offset);
  usize newlineAfterIdx = after - newlineOffsets.begin();

  usize line = newlineAfterIdx + 1;
  usize column;

  if (newlineAfterIdx == 0) {
    column = offset + 1;
  } else {
    column = offset - newlineOffsets[newlineAfterIdx - 1];
  }

  return SourceCoordinates{line, column};
}

StringRef SourceFile::getLine(usize line) const {
  assert(line > 0 && line <= newlineOffsets.size() + 1);

  // Index of the newline at the end of the line.
  usize newlineAfterIdx = line - 1;

  usize start;
  if (newlineAfterIdx == 0) {
    start = 0;
  } else {
    start = newlineOffsets[newlineAfterIdx - 1] + 1;
  }

  usize end;
  if (newlineAfterIdx == newlineOffsets.size()) {
    end = source.size();
  } else {
    end = newlineOffsets[newlineAfterIdx];
  }

  return StringRef{&source[start], end - start};
}

} // namespace fl
