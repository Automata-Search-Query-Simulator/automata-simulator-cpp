#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace automata {

enum class ModePreference { Auto, Nfa, Dfa, Efa, Pda, PdaOnly };

struct PatternSpec {
    std::string pattern;
    std::vector<std::string> datasets;
    std::string datasetPath;
    std::size_t mismatchBudget{0};
    bool trace{false};
    bool allowDotBracket{false};
    
    // RNA validation
    std::string rnaSecondaryStructure;  // Dot-bracket notation
    
    ModePreference requestedMode{ModePreference::Auto};
};

}  // namespace automata
