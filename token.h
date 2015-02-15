#ifndef TOKEN_H_
#define TOKEN_H_

#include "util.h"
#include <memory>
#include <string>
#include <vector>

namespace fiddle {

struct SourceCoordinates {
  usize line;
  usize column;
};

struct SourceFile {
  std::string filename;
  std::string source;

  // A list of the offsets of every newline in the source, which is generated
  // during lexing. It can be used to quickly find the line and column of the
  // start and end of a SourceRange (by binary search) and to extract the text
  // for a single line for diagnostic printouts.
  std::vector<usize> newlineOffsets;

  SourceFile(std::string filename, std::string source)
      : filename(std::move(filename)),
        source(std::move(source)) {}

  SourceCoordinates findCoordinates(usize offset) const;
  StringRef getLine(usize line) const;
};

struct SourceRange {
  std::shared_ptr<SourceFile> file;
  usize start;
  usize end;

  SourceCoordinates startCoordinates() const {
    return file->findCoordinates(start);
  }

  StringRef text() const {
    return StringRef{file->source.data() + start, end - start};
  }
};

#define DEFINE_TOKEN_KINDS(X) \
  X(kInvalid, "invalid token") \
  X(kEOF, "end of file") \
  X(kIdentifier, "identifier") \
  X(kInteger, "integer literal") \
  X(kOperator, "operator") \
  X(kKeywordEnum, "keyword 'enum'") \
  X(kKeywordFn, "keyword 'fn'") \
  X(kKeywordStruct, "keyword 'struct'") \
  X(kParenLeft, "'('") \
  X(kParenRight, "')'") \
  X(kBraceLeft, "'{'") \
  X(kBraceRight, "'}'") \
  X(kBracketLeft, "'['") \
  X(kBracketRight, "']'") \
  X(kComma, "','")

struct Token {
  enum TokenKind {
#define X(name, description) name,
    DEFINE_TOKEN_KINDS(X)
#undef X
    kNumTokenKinds
  };

  TokenKind kind;
  SourceRange location;

  // Only set for kInteger tokens.
  i64 intValue;

  StringRef text() const { return location.text(); }
};

const char* const kTokenKindNames[Token::kNumTokenKinds] = {
#define X(name, description) #name,
  DEFINE_TOKEN_KINDS(X)
#undef X
};

const char* const kTokenKindDescriptions[Token::kNumTokenKinds] = {
#define X(name, description) description,
  DEFINE_TOKEN_KINDS(X)
#undef X
};

#undef DEFINE_TOKEN_KINDS

inline std::ostream& operator<<(std::ostream& o, const Token& token) {
  return o << "Token(" << kTokenKindNames[token.kind] << ", \"" << token.text()
      << "\")";
}

} // namespace fiddle

#endif /* TOKEN_H_ */
