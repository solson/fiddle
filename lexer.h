#ifndef LEXER_H_
#define LEXER_H_

#include "diagnostic.h"
#include "token.h"
#include "util.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace fiddle {

struct Lexer {
  usize byteOffset = 0;
  std::shared_ptr<SourceFile> sourceFile;
  std::vector<Diagnostic>& diagnostics;

  explicit Lexer(std::shared_ptr<SourceFile> file,
                 std::vector<Diagnostic>& diagnostics)
      : sourceFile(file),
        diagnostics(diagnostics) {}

  Token nextToken();
  const std::string& source() const { return sourceFile->source; }

 private:
  void scanInt(Token* token);
  void scanChars(bool (*pred)(char));
  char currChar();
  void consumeChar();
  bool atEnd() const;
  StringRef textFrom(usize position) const;
  void report(Diagnostic::DiagnosticLevel level, StringRef message,
              usize location);
};

} // namespace fiddle

#endif /* LEXER_H_ */
