#ifndef EDITLINE_H_
#define EDITLINE_H_

#include <string>

extern "C" {
#include "histedit.h"
}

namespace fiddle {
namespace el {

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

}
}

#endif /* EDITLINE_H_ */
