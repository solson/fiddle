#ifndef DIAGNOSTIC_H_
#define DIAGNOSTIC_H_

#include "token.h"
#include <iomanip>
#include <iostream>
#include <string>

namespace fl {

struct Diagnostic {
  enum DiagnosticLevel {
    kFatal,
    kError,
    kWarning,
    kInfo,

    kNumDiagnosticLevels
  };

  DiagnosticLevel level;
  std::string message;
  SourceRange location;
};

const char* const kDiagnosticLevelNames[Diagnostic::kNumDiagnosticLevels] = {
  "fatal", "error", "warning", "info"
};

inline std::ostream& operator<<(std::ostream& o, const Diagnostic& diag) {
  SourceCoordinates coords = diag.location.startCoordinates();

  o << diag.location.file->filename << ':' << coords.line << ':'
      << coords.column << ": " << kDiagnosticLevelNames[diag.level] << ": "
      << diag.message << '\n';

  o << diag.location.file->getLine(coords.line) << '\n';

  o << std::setw(coords.column) << '^' << '\n';

  return o;
}

} // namespace fl

#endif /* DIAGNOSTIC_H_ */
