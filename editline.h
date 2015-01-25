#ifndef EDITLINE_H_
#define EDITLINE_H_

#include <string>

extern "C" {
#include "histedit.h"
}

namespace fiddle {

struct EL {
  EditLine* el;
  std::string prompt = "> ";

  explicit EL(const char* program_name);
  ~EL();

  bool getLine(std::string* line);
};

} // namespace fiddle

#endif /* EDITLINE_H_ */
