#include <llvm/Support/CommandLine.h>
#include <iostream>
#include <cstdio>
extern "C" {
#include <histedit.h>
}

using namespace llvm;

cl::opt<int> Num(cl::Positional, cl::desc("<number>"), cl::init(3));

int main(int argc, char** argv) {
  cl::ParseCommandLineOptions(argc, argv);

  for (int i = 0; i < Num; ++i) {
    std::cout << "What's going on?\n";
  }

  EditLine* el = el_init(argv[0], stdin, stdout, stderr);
  el_set(el, EL_PROMPT, (const char*(*)(EditLine*)) [](EditLine*){ return "> "; });

  while (true) {
    int count;
    const char* raw_line = el_gets(el, &count);

    std::string line(raw_line, count);
    std::cout << line;
  }

  return 0;
}
