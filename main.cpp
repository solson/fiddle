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
  bool getLine(std::string* line);
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

bool EL::getLine(std::string* line) {
  int len;
  const char* rawLine = el_gets(el, &len);
  if (!rawLine) return false;
  line->assign(rawLine, len);
  return true;
}

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
