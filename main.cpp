#include "editline.h"
#include "lexer.h"
#include "parser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace fiddle;
using namespace llvm;

void runTest(const char* source) {
  Parser parser{SourceFile{"<test>", source}};
  auto expr = parser.parseExpr();
  for (const auto& diag : parser.diagnostics) {
    std::cout << diag;
  }

  if (!expr) { return; }
  std::cout << *expr << '\n';
  expr->codegen()->dump();
}

void runFnTest(const char* filename) {
  std::ifstream file(filename);
  std::stringstream buffer;
  buffer << file.rdbuf();
  std::string source = buffer.str();

  Parser parser{SourceFile{"<test>", source}};
  auto fn = parser.parseFuncDef();
  for (const auto& diag : parser.diagnostics) {
    std::cout << diag;
  }

  if (!fn) { return; }
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
  if (argc == 2) {
    runFnTest(argv[1]);
    return 0;
  }

  // runFnTest("fn doubleSum(x, y) { (x + y) * 2 }");

  EL editline(argv[0]);
  editline.prompt = "fiddle> ";

  std::string line;
  while (editline.getLine(&line)) {
    if (line == ".tests\n") {
      runTests();
      continue;
    }
    runTest(line.c_str());
  }

  return 0;
}
