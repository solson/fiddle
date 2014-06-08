#include "editline.h"

namespace fiddle {
namespace el {

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

}
}
