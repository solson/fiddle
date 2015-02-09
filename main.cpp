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

void compile(FuncDef* func) {
  Module module{"fiddle", getGlobalContext()};
  func->codegen(&module);
  module.dump();
}

void runFnTest(std::string filename, std::string source) {
  Parser parser{SourceFile{filename, source}};
  auto fn = parser.parseFuncDef();
  for (const auto& diag : parser.diagnostics) {
    std::cout << diag;
  }

  if (!fn) { return; }
  std::cout << *fn << '\n';
  std::cout << source << '\n';
  compile(fn.get());
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
