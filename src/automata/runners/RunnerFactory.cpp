#include "automata/runners/Runners.hpp"
#include "parser/Parsers.hpp"

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
            
            // Calculate max depth from input sequences or RNA secondary structure
            DotBracketValidator validator;
            std::size_t maxDepth = 1;
            
            // Use RNA secondary structure if available, otherwise use datasets
            const std::string& dotBracket = plan.spec.rnaSecondaryStructure.empty() 
                ? (plan.spec.datasets.empty() ? "" : plan.spec.datasets[0])
                : plan.spec.rnaSecondaryStructure;
            
            if (!dotBracket.empty()) {
                maxDepth = validator.getMaxDepth(dotBracket);
            }
            
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
            
            auto pdaRunner = std::make_unique<PdaRunner>(std::move(pda), plan.spec.trace);
            
            // Set RNA validation if secondary structure provided
            if (!plan.spec.rnaSecondaryStructure.empty()) {
                pdaRunner->setRnaValidation(plan.spec.rnaSecondaryStructure);
            }
            
            return pdaRunner;
        }
    }
    return nullptr;
}

}  // namespace automata
