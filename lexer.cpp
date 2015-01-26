#include "lexer.h"
#include <cstring>

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

Token Lexer::nextToken() {
  scanChars(isWhitespace);
  usize tokenStart = byteOffset;
  Token::TokenKind kind = Token::kInvalid;

  if (atEnd()) {
    kind = Token::kEOF;
  } else {
    char c = nextChar();
    if (isDigit(c)) {
      kind = Token::kInteger;
      scanChars(isIdentifierChar);
    } else if (isIdentifierChar(c)) {
      scanChars(isIdentifierChar);
      StringRef text = textFrom(tokenStart);
      if (text == "enum") {
        kind = Token::kKeywordEnum;
      } else if (text == "fn") {
        kind = Token::kKeywordFn;
      } else if (text == "struct") {
        kind = Token::kKeywordStruct;
      } else {
        kind = Token::kIdentifier;
      }
    } else if (isOperatorChar(c)) {
      kind = Token::kOperator;
      scanChars(isOperatorChar);
    } else {
      switch (c) {
        case '(': kind = Token::kParenLeft;    break;
        case ')': kind = Token::kParenRight;   break;
        case '{': kind = Token::kBraceLeft;    break;
        case '}': kind = Token::kBraceRight;   break;
        case '[': kind = Token::kBracketLeft;  break;
        case ']': kind = Token::kBracketRight; break;
        case ',': kind = Token::kComma;        break;
        default:  kind = Token::kInvalid;
      }
      consumeChar();
    }
  }

  usize length = byteOffset - tokenStart;
  return Token{kind, SourceLocation{sourceFile, tokenStart}, length};
}

void Lexer::scanChars(bool (*pred)(char)) {
  while (!atEnd()) {
    char c = nextChar();
    if (!pred(c)) { break; }
    consumeChar();
  }
}

StringRef Lexer::textFrom(usize byteStart) const {
  return StringRef{source().data() + byteStart, byteOffset - byteStart};
}

char Lexer::nextChar() {
  return source()[byteOffset];
}

char Lexer::consumeChar() {
  byteOffset++;
  return nextChar();
}

bool Lexer::atEnd() const {
  return byteOffset == source().size();
}

} // namespace fiddle
