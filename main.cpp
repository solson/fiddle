#include "editline.h"
#include "lexer.h"
#include "parser.h"
#include "util.h"
#include <llvm/IR/Module.h>
#include <llvm/IR/LLVMContext.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace fiddle;
using namespace llvm;

void runFnTest(std::string filename, std::string source) {
  Parser parser{SourceFile{filename, source}};
  auto module = parser.parseModule();
  for (const auto& diag : parser.diagnostics) {
    std::cout << diag;
  }

  if (!module) { return; }
  std::cout << *module << '\n';
  std::cout << source << '\n';
  module->codegen()->dump();
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
    // runTest(test);
  }
}

int main(int argc, char** argv) {
  if (argc == 2) {
    const char* filename = argv[1];
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    runFnTest(filename, buffer.str());
    return 0;
  }

  EL editline(argv[0]);
  editline.prompt = "fiddle> ";

  std::string line;
  while (editline.getLine(&line)) {
    if (line == ".tests\n") {
      runTests();
      continue;
    }
    runFnTest("<repl>", line.c_str());
  }

  return 0;
}
