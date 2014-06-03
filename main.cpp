#include <llvm/Support/CommandLine.h>
#include <iostream>
#include <cstdio>
extern "C" {
#include <histedit.h>
}

using namespace llvm;

class EL {
 public:
  using PromptFnType = const char* (*)(EditLine*);

  explicit EL(const char* program_name);
  ~EL();

  void setPromptFn(PromptFnType prompt_fn);
  std::string getLine();
 private:
  EditLine* el;
};

EL::EL(const char* program_name) {
  el = el_init(program_name, stdin, stdout, stderr);
  // Load user configuration from ~/.editrc
  el_source(el, nullptr);
}

EL::~EL() {
  el_end(el);
}

void EL::setPromptFn(EL::PromptFnType promptFn) {
  el_set(el, EL_PROMPT, promptFn);
}

std::string EL::getLine() {
  int len;
  const char* rawLine = el_gets(el, &len);
  return std::string(rawLine, len);
}

cl::opt<int> Num(cl::Positional, cl::desc("<number>"), cl::init(3));

int main(int argc, char** argv) {
  cl::ParseCommandLineOptions(argc, argv);

  for (int i = 0; i < Num; ++i) {
    std::cout << "What's going on?\n";
  }

  EL el(argv[0]);
  el.setPromptFn([](EditLine*){ return "> "; });

  while (true) {
    std::string line = el.getLine();
    std::cout << line;
  }

  return 0;
}
