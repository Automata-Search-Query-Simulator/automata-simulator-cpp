#include "automata/runners/Runners.hpp"
#include "parser/Parsers.hpp"

namespace automata {

RunnerPtr RunnerFactory::create(const AutomatonPlan& plan, const RegexParser& parser, Snapshot* snapshot) const {
    NfaBuilder nfaBuilder(parser);
    switch (plan.kind) {
        case AutomatonKind::Nfa: {
            auto nfa = nfaBuilder.build(plan.spec.pattern);
            if (snapshot) {
                snapshot->kind = AutomatonKind::Nfa;
                snapshot->automaton = nfa;
            }
            return std::make_unique<NfaRunner>(std::move(nfa), plan.spec.trace);
        }
        case AutomatonKind::Dfa: {
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
            EfaBuilder builder(parser);
            auto efa = builder.build(plan.spec.pattern, plan.spec.mismatchBudget);
            if (snapshot) {
                snapshot->kind = AutomatonKind::Efa;
                snapshot->automaton = efa;
            }
            return std::make_unique<EfaRunner>(std::move(efa), plan.spec.trace);
        }
        case AutomatonKind::Pda: {
            PdaBuilder builder;
            
            // Calculate max depth from input sequences
            DotBracketValidator validator;
            std::size_t maxDepth = 1;  // Minimum depth
            for (const auto& seq : plan.spec.datasets) {
                std::size_t seqDepth = validator.getMaxDepth(seq);
                if (seqDepth > maxDepth) {
                    maxDepth = seqDepth;
                }
            }
            
            auto pda = builder.build(maxDepth);
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
