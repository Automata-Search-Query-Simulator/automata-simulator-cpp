#pragma once

#include <string>
#include <vector>

namespace automata {

class EvaluationHarness {
  public:
    std::vector<std::string> dnaSmokeSet() const;
    std::vector<std::string> rnaSmokeSet() const;
};

}  // namespace automata
