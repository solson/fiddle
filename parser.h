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

namespace fl {

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

  std::unique_ptr<ast::Module> parseModule();
  std::unique_ptr<ast::FuncProto> parseFuncProto();
  std::unique_ptr<ast::FuncDef> parseFuncDef();
  std::unique_ptr<ast::ExternFunc> parseExternFunc();
  std::unique_ptr<ast::Type> parseType();
  std::unique_ptr<ast::Expr> parseExpr();
  std::unique_ptr<ast::Expr> parseExprPrimary();
  std::unique_ptr<ast::Expr> parseExprOperator(std::unique_ptr<ast::Expr> lhs,
                                               u8 minPrecedence);
  std::unique_ptr<ast::Expr> parseBlockExpr();

  Token nextToken();
  Token consumeToken();
  bool atEnd() const;
  void scanToEnd();
  bool expectToken(Token::TokenKind expected);
  void report(Diagnostic::DiagnosticLevel level, StringRef message,
              const Token& token);
};

} // namespace fl

#endif /* PARSER_H */
