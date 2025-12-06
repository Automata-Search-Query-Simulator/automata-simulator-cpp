#include <cassert>

#include "PatternSpec.hpp"
#include "automata/runners/Runners.hpp"
#include "modes/ModeDispatcher.hpp"
#include "parser/Parsers.hpp"

using namespace automata;

void runIntegrationTests() {
    RegexParser parser;
    RunnerFactory factory;
    ModeDispatcher dispatcher;

    PatternSpec spec;
    spec.pattern = "ACG";
    spec.datasets = {"ACGTTACG"};
    spec.requestedMode = ModePreference::Auto;
    auto plan = dispatcher.decide(spec);
    auto runner = factory.create(plan, parser);
    auto result = runner->run(spec.datasets.front());
    assert(!result.matches.empty());

    PatternSpec efaSpec = spec;
    efaSpec.pattern = "ACGT";
    efaSpec.mismatchBudget = 1;
    efaSpec.requestedMode = ModePreference::Efa;
    plan = dispatcher.decide(efaSpec);
    auto efaRunner = factory.create(plan, parser);
    auto efaResult = efaRunner->run("ACCT");
    assert(!efaResult.matches.empty());

    PatternSpec efaRegexSpec = spec;
    efaRegexSpec.pattern = "A(CG|TT)*";
    efaRegexSpec.mismatchBudget = 1;
    efaRegexSpec.requestedMode = ModePreference::Efa;
    plan = dispatcher.decide(efaRegexSpec);
    auto efaRegexRunner = factory.create(plan, parser);
    auto efaRegexResult = efaRegexRunner->run("ATGTTG");
    assert(!efaRegexResult.matches.empty());

    PatternSpec pdaSpec;
    pdaSpec.allowDotBracket = true;
    pdaSpec.requestedMode = ModePreference::Pda;
    plan = dispatcher.decide(pdaSpec);
    auto pdaRunner = factory.create(plan, parser);
    auto pdaResult = pdaRunner->run("(()())");
    assert(pdaResult.accepted);
}
