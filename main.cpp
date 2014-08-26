#include <iostream>
#include <string>
#include <llvm/Support/CommandLine.h>
#include "editline.h"
#include "parser.h"

using namespace fiddle;
using namespace llvm;

int main(int argc, char** argv) {
  cl::ParseCommandLineOptions(argc, argv);

  auto e1 = Parser("42").parseExpr();
  e1->Debug(); std::cout << '\n';
  e1->Codegen()->dump();

  auto e2 = Parser("1 + 2").parseExpr();
  e2->Debug(); std::cout << '\n';
  e2->Codegen()->dump();

  auto e3 = Parser("1 * 2").parseExpr();
  e3->Debug(); std::cout << '\n';
  e3->Codegen()->dump();

  auto e4 = Parser("1 + 2 * 3").parseExpr();
  e4->Debug(); std::cout << '\n';
  e4->Codegen()->dump();

  return 0;
}
