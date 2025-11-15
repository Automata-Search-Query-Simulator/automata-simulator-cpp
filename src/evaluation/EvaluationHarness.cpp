#include "evaluation/EvaluationHarness.hpp"

namespace automata {

std::vector<std::string> EvaluationHarness::dnaSmokeSet() const {
    return {"ACGTACGT", "TTTTACGT", "GGGGCCCC"};
}

std::vector<std::string> EvaluationHarness::rnaSmokeSet() const { return {"((..))", "(()())", "(.)"}; }

}  // namespace automata
