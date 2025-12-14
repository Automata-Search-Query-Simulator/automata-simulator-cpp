#pragma once

#include "PatternSpec.hpp"

namespace automata {

enum class AutomatonKind { Nfa, Dfa, Efa, Pda };

struct AutomatonPlan {
    AutomatonKind kind{AutomatonKind::Nfa};
    PatternSpec spec;
};

}  // namespace automata
