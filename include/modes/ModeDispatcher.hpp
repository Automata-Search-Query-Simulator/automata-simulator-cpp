#pragma once

#include "AutomatonPlan.hpp"
#include "PatternSpec.hpp"

namespace automata {

class ModeDispatcher {
  public:
    AutomatonPlan decide(const PatternSpec& spec) const;
};

}  // namespace automata
