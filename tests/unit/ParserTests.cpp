#include <cassert>
#include <fstream>

#include "parser/Parsers.hpp"

using namespace automata;

void runParserTests() {
    RegexParser parser;
    auto tokens = parser.parseToPostfix("A(B|C)*D");
    assert(!tokens.empty());
    assert(tokens.back().type == RegexTokenType::Concat || tokens.back().type == RegexTokenType::Union ||
           tokens.back().type == RegexTokenType::Star);

    DotBracketValidator validator;
    assert(validator.validate("(()())"));
    assert(!validator.validate("(()"));
    assert(!validator.validate("())("));

    std::ofstream file("tmp/test_dataset.txt");
    file << ">seq\nACGT\n";
    file.close();

    DatasetLoader loader;
    auto sequences = loader.loadSequences("tmp/test_dataset.txt");
    assert(sequences.size() == 1);
    assert(sequences[0] == "ACGT");
}
