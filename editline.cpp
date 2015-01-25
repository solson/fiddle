#include "editline.h"
#include <cassert>

namespace fiddle {

const char* promptFn(EditLine* el) {
  EL* self;
  int result = el_get(el, EL_CLIENTDATA, &self);
  (void) result;
  assert(result == 0);
  return self->prompt.c_str();
};

EL::EL(const char* program_name)
    : el(el_init(program_name, stdin, stdout, stderr)) {
  // Load user configuration from ~/.editrc
  el_source(el, nullptr);

  // Set the editline client data to this object so callbacks like promptFn can
  // access it.
  el_set(el, EL_CLIENTDATA, this);

  el_set(el, EL_PROMPT, promptFn);
}

EL::~EL() {
  el_end(el);
}

bool EL::getLine(std::string* line) {
  int len;
  const char* rawLine = el_gets(el, &len);
  if (!rawLine) { return false; }
  line->assign(rawLine, len);
  return true;
}

}
