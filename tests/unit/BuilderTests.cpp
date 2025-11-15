#include <cassert>

#include "automata/builders/Builders.hpp"
#include "parser/Parsers.hpp"

using namespace automata;

void runBuilderTests() {
    RegexParser parser;
    NfaBuilder nfaBuilder(parser);
    auto nfa = nfaBuilder.build("A|C");
    assert(!nfa.states.empty());
    DfaBuilder dfaBuilder;
    auto dfa = dfaBuilder.build(nfa);
    assert(!dfa.states.empty());

    EfaBuilder efaBuilder;
    auto efa = efaBuilder.build("ACGT", 2);
    assert(efa.literalPattern == "ACGT");
    assert(efa.mismatchBudget == 2);

    PdaBuilder pdaBuilder;
    auto pda = pdaBuilder.build();
    assert(!pda.rules.empty());
}
