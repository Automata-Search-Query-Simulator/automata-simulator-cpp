#include "automata/runners/Runners.hpp"

namespace automata {

RunnerPtr RunnerFactory::create(const AutomatonPlan& plan, const RegexParser& parser, Snapshot* snapshot) const {
    // reuse a parser-backed NFA builder for any automaton that starts with regex conversion
    NfaBuilder nfaBuilder(parser);
    switch (plan.kind) {
        case AutomatonKind::Nfa: {
            // convert the pattern directly into an NFA runner-ready structure
            auto nfa = nfaBuilder.build(plan.spec.pattern);
            if (snapshot) {
                snapshot->kind = AutomatonKind::Nfa;
                snapshot->automaton = nfa;
            }
            return std::make_unique<NfaRunner>(std::move(nfa), plan.spec.trace);
        }
        case AutomatonKind::Dfa: {
            // build the DFA via intermediate NFA and dedicated builder logic
            auto nfa = nfaBuilder.build(plan.spec.pattern);
            DfaBuilder dfaBuilder;
            auto dfa = dfaBuilder.build(nfa);
            if (snapshot) {
                snapshot->kind = AutomatonKind::Dfa;
                snapshot->automaton = dfa;
            }
            return std::make_unique<DfaRunner>(std::move(dfa), plan.spec.trace);
        }
        case AutomatonKind::Efa: {
            // mismatch-tolerant sampler derived directly from regex
            EfaBuilder builder(parser);
            auto efa = builder.build(plan.spec.pattern, plan.spec.mismatchBudget);
            if (snapshot) {
                snapshot->kind = AutomatonKind::Efa;
                snapshot->automaton = efa;
            }
            return std::make_unique<EfaRunner>(std::move(efa), plan.spec.trace);
        }
        case AutomatonKind::Pda: {
            // balanced-parentheses detector living in PDA land
            PdaBuilder builder;
            auto pda = builder.build();
            if (snapshot) {
                snapshot->kind = AutomatonKind::Pda;
                snapshot->automaton = pda;
            }
            return std::make_unique<PdaRunner>(std::move(pda), plan.spec.trace);
        }
    }
    return nullptr;
}

}  // namespace automata
