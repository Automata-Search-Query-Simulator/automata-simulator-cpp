#include "automata/runners/Runners.hpp"

namespace automata {

RunnerPtr RunnerFactory::create(const AutomatonPlan& plan, const RegexParser& parser) const {
    NfaBuilder nfaBuilder(parser);
    switch (plan.kind) {
        case AutomatonKind::Nfa: {
            auto nfa = nfaBuilder.build(plan.spec.pattern);
            return std::make_unique<NfaRunner>(std::move(nfa), plan.spec.trace);
        }
        case AutomatonKind::Dfa: {
            auto nfa = nfaBuilder.build(plan.spec.pattern);
            DfaBuilder dfaBuilder;
            auto dfa = dfaBuilder.build(nfa);
            return std::make_unique<DfaRunner>(std::move(dfa), plan.spec.trace);
        }
        case AutomatonKind::Efa: {
            EfaBuilder builder;
            auto efa = builder.build(plan.spec.pattern, plan.spec.mismatchBudget);
            return std::make_unique<EfaRunner>(std::move(efa), plan.spec.trace);
        }
        case AutomatonKind::Pda: {
            PdaBuilder builder;
            return std::make_unique<PdaRunner>(builder.build(), plan.spec.trace);
        }
    }
    return nullptr;
}

}  // namespace automata
