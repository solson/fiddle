#include "editline.h"
#include "lexer.h"
#include "parser.h"
#include <llvm/Support/CommandLine.h>
#include <iostream>
#include <string>
#include <vector>

using namespace fiddle;
using namespace llvm;

void runTest(const char* source) {
  Lexer lexer{SourceFile{"<test>", source}};
  ParseError err;
  auto expr = Parser(std::move(lexer)).parseExpr(&err);
  if (!expr) {
    std::cout << "error: " << err.message << "\n";
    return;
  }
  std::cout << *expr << '\n';
  expr->codegen()->dump();
}

void runFnTest(const char* source) {
  Lexer lexer{SourceFile{"<test>", source}};
  ParseError err;
  auto fn = Parser(std::move(lexer)).parseFuncDef(&err);
  if (!fn) {
    std::cout << "error: " << err.message << "\n";
    return;
  }
  std::cout << *fn << '\n';
  std::cout << source << '\n';
}

void runTests() {
  const char* tests[] = {
    "42",
    "1 + 2",
    "1 * 2",
    "1 + 2 * 3",
    "3 * 1 + 2",
    "3 * (1 + 2)",
    "1 + 2 - 3 + 4"
  };

  for (auto test : tests) {
    runTest(test);
  }
}

int main(int argc, char** argv) {
  cl::ParseCommandLineOptions(argc, argv);

  runFnTest("fn foo(x, y) { 1 + 2 }");
  return 0;

  EL editline(argv[0]);
  editline.prompt = "fiddle> ";

  std::string line;
  while (editline.getLine(&line)) {
    if (line == ".tests\n") {
      runTests();
      continue;
    }
    // runTest(line.c_str());
    runFnTest(line.c_str());
  }

  return 0;
}
