#ifndef LEXER_H_
#define LEXER_H_

#include "util.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace fiddle {

struct SourceFile {
  std::string filename;
  std::string source;
};

struct SourceLocation {
  std::shared_ptr<SourceFile> file;
  usize byteOffset;
  // TODO(tsion): Add line and column numbers.
};

struct Token {
  enum TokenKind {
    // Special tokens
    kInvalid,
    kEOF,

    // Tokens with data
    kIdentifier,
    kInteger,
    kOperator,

    // Keywords
    kKeywordEnum,
    kKeywordFn,
    kKeywordStruct,

    // Basic syntax tokens
    kParenLeft,
    kParenRight,
    kBraceLeft,
    kBraceRight,
    kBracketLeft,
    kBracketRight,
    kComma
  };

  TokenKind kind;
  SourceLocation loc;
  usize length;

  StringRef text() const {
    return StringRef{loc.file->source.data() + loc.byteOffset, length};
  }

  bool is(TokenKind kind) const { return this->kind == kind; }
  bool isNot(TokenKind kind) const { return !is(kind); }
};

inline std::ostream& operator<<(std::ostream& o, const Token& token) {
  return o << "Token(kind = " << token.kind << ", text = \"" << token.text()
      << "\")";
}

struct Lexer {
  std::shared_ptr<SourceFile> sourceFile;
  usize byteOffset = 0;

  explicit Lexer(SourceFile sourceFile)
      : sourceFile(std::make_shared<SourceFile>(std::move(sourceFile))) {}

  Token nextToken();
  const std::string& source() const { return sourceFile->source; }

 private:
  Token makeToken(Token::TokenKind kind);
  void scanChars(bool (*pred)(char));
  char nextChar();
  char consumeChar();
  StringRef textFrom(usize position) const;
  bool atEnd() const;
};

} // namespace fiddle

#endif /* LEXER_H_ */
