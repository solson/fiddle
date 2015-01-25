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
  Lexer lexer{SourceFile{"<file>", source}};
  ParseError err;
  auto expr = Parser(std::move(lexer)).parseExpr(&err);
  if (!expr) {
    std::cout << "error: " << err.message << "\n";
    return;
  }
  expr->debug();
  std::cout << '\n';
  expr->codegen()->dump();
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
