#ifndef PARSER_H_
#define PARSER_H_

#include "ast.h"
#include "diagnostic.h"
#include "lexer.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

namespace fiddle {

struct Parser {
  std::shared_ptr<SourceFile> sourceFile;
  std::vector<Diagnostic> diagnostics;
  Lexer lexer;
  Token currToken;

  explicit Parser(SourceFile file)
      : sourceFile(std::make_shared<SourceFile>(std::move(file))),
        diagnostics(),
        lexer(sourceFile, diagnostics) {
    // Initialize currToken.
    consumeToken();
  }

  // std::unique_ptr<Module> parseModule();
  std::unique_ptr<FuncDef> parseFuncDef();
  std::unique_ptr<Expr> parseExpr();
  std::unique_ptr<Expr> parseExprPrimary();
  std::unique_ptr<Expr> parseExprOperator(std::unique_ptr<Expr> lhs,
                                          u8 minPrecedence);

  Token nextToken();
  Token consumeToken();
  bool atEnd() const;
  bool expectToken(Token::TokenKind expected);
  void report(Diagnostic::DiagnosticLevel level, StringRef message);
};

} // namespace fiddle

#endif /* PARSER_H */
