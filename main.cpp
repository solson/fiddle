#include <llvm/Support/CommandLine.h>
#include <cstdio>
#include <iostream>
#include <string>
#include "editline.h"

using namespace fiddle;
using namespace llvm;

int main(int argc, char** argv) {
  cl::opt<int> Num(cl::Positional, cl::desc("<number>"), cl::init(3));
  cl::ParseCommandLineOptions(argc, argv);

  EL el(argv[0]);
  el.setPromptFn([](EditLine*){ return "> "; });

  std::string line;
  while (el.getLine(&line)) {
    std::cout << line;
  }

  return 0;
}
