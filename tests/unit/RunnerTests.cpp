#include <cassert>

#include "automata/builders/Builders.hpp"
#include "automata/runners/Runners.hpp"
#include "parser/Parsers.hpp"

using namespace automata;

void runRunnerTests() {
    RegexParser parser;
    NfaBuilder nfaBuilder(parser);
    {
        auto nfa = nfaBuilder.build("ACG");
        NfaRunner runner(std::move(nfa), false);
        auto result = runner.run("ACGTTACG");
        assert(!result.matches.empty());
    }
    {
        auto nfa = nfaBuilder.build("ACG");
        DfaBuilder dfaBuilder;
        auto dfa = dfaBuilder.build(nfa);
        DfaRunner runner(std::move(dfa), false);
        auto result = runner.run("ACGTTACG");
        assert(!result.matches.empty());
    }
    {
        EfaBuilder builder;
        auto efa = builder.build("ACG", 1);
        EfaRunner runner(std::move(efa), false);
        auto result = runner.run("ACCAT");
        assert(!result.matches.empty());
    }
    {
        PdaBuilder builder;
        auto pda = builder.build();
        PdaRunner runner(std::move(pda), false);
        auto result = runner.run("(()())");
        assert(result.accepted);
    }
}
