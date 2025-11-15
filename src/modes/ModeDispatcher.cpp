#include "modes/ModeDispatcher.hpp"

namespace automata {

AutomatonPlan ModeDispatcher::decide(const PatternSpec& spec) const {
    AutomatonPlan plan;
    plan.spec = spec;
    switch (spec.requestedMode) {
        case ModePreference::Nfa:
            plan.kind = AutomatonKind::Nfa;
            return plan;
        case ModePreference::Dfa:
            plan.kind = AutomatonKind::Dfa;
            return plan;
        case ModePreference::Efa:
            plan.kind = AutomatonKind::Efa;
            return plan;
        case ModePreference::Pda:
        case ModePreference::PdaOnly:
            plan.kind = AutomatonKind::Pda;
            return plan;
        case ModePreference::Auto:
        default:
            break;
    }
    if (spec.allowDotBracket) {
        plan.kind = AutomatonKind::Pda;
    } else if (spec.mismatchBudget > 0) {
        plan.kind = AutomatonKind::Efa;
    } else {
        plan.kind = AutomatonKind::Nfa;
    }
    return plan;
}

}  // namespace automata
