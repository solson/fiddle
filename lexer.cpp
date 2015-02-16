#include "lexer.h"
#include <cstring>
#include <unordered_map>

namespace fiddle {

bool isAscii(char c) {
  return !(c & 0x80);
}

bool isDigit(char c) {
  return c >= '0' && c <= '9';
}

bool isOperatorChar(char c) {
  const char kOperatorChars[] = "~!@#$%^&*-+=|/?<>.";
  return std::strchr(kOperatorChars, c) != nullptr;
}

bool isIdentifierChar(char c) {
  return (c >= 'a' && c <= 'z')
      || (c >= 'A' && c <= 'Z')
      || c == '_'
      || isDigit(c);
}

bool isWhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

i64 digitToInt(char c) {
  assert(isDigit(c));
  return c - '0';
}

void Lexer::scanInt(Token* token) {
  token->kind = Token::kInteger;
  scanChars(isIdentifierChar);
  token->location.end = byteOffset;
  StringRef str = token->text();

  i64 result = 0;
  i64 base = 1;

  for (int i = str.length - 1; i >= 0; --i) {
    char c = str[i];
    if (!isDigit(c)) {
      report(Diagnostic::kError, "non-decimal digit in integer literal",
             token->location.start + i);
      token->intValue = 0;
      return;
    }

    // TODO(tsion): Detect integer overflow.
    result += base * digitToInt(c);
    base *= 10;
  }

  token->intValue = result;
}

const std::unordered_map<std::string, Token::TokenKind> kKeywords{
  {"enum", Token::kKeywordEnum},
  {"fn", Token::kKeywordFn},
  {"struct", Token::kKeywordStruct},
};

Token Lexer::nextToken() {
  if (!atEnd()) {
    scanChars(isWhitespace);
  }

  if (atEnd()) {
    Token token;
    token.kind = Token::kEOF;
    token.location = SourceRange{sourceFile, byteOffset, byteOffset};
    return token;
  }

  Token token;
  token.kind = Token::kInvalid;
  token.location.file = sourceFile;
  token.location.start = byteOffset;

  char c = currChar();

  if (isDigit(c)) {
    scanInt(&token);
  } else if (isIdentifierChar(c)) {
    scanChars(isIdentifierChar);
    auto it = kKeywords.find(textFrom(token.location.start).toString());
    if (it == kKeywords.end()) {
      token.kind = Token::kIdentifier;
    } else {
      token.kind = it->second;
    }
  } else if (isOperatorChar(c)) {
    token.kind = Token::kOperator;
    scanChars(isOperatorChar);
  } else {
    switch (c) {
      case '(': token.kind = Token::kParenLeft;    break;
      case ')': token.kind = Token::kParenRight;   break;
      case '{': token.kind = Token::kBraceLeft;    break;
      case '}': token.kind = Token::kBraceRight;   break;
      case '[': token.kind = Token::kBracketLeft;  break;
      case ']': token.kind = Token::kBracketRight; break;
      case ',': token.kind = Token::kComma;        break;
      default:  token.kind = Token::kInvalid;
    }
    consumeChar();
  }

  token.location.end = byteOffset;
  return token;
}

void Lexer::scanChars(bool (*pred)(char)) {
  while (!atEnd()) {
    char c = currChar();
    if (!pred(c)) { break; }
    consumeChar();
  }
}

StringRef Lexer::textFrom(usize byteStart) const {
  return StringRef{source().data() + byteStart, byteOffset - byteStart};
}

char Lexer::currChar() {
  assert(!atEnd());
  return source()[byteOffset];
}

void Lexer::consumeChar() {
  assert(!atEnd());
  if (currChar() == '\n') {
    sourceFile->newlineOffsets.push_back(byteOffset);
  }
  ++byteOffset;
}

bool Lexer::atEnd() const {
  return byteOffset == source().size();
}

void Lexer::report(Diagnostic::DiagnosticLevel level, StringRef message,
                   usize offset) {
  diagnostics.emplace_back(
      Diagnostic{level,
                 message.toString(),
                 SourceRange{sourceFile, offset, offset}});
}

} // namespace fiddle
