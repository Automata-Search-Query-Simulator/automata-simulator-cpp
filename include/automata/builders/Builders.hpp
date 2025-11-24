#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "AutomatonPlan.hpp"
#include "PatternSpec.hpp"
#include "parser/Parsers.hpp"

namespace automata {

enum class EdgeType { Epsilon, Literal, Any, CharClass };

struct Edge {
    int to;
    EdgeType type;
    char literal;
    std::string charClass;
};

struct NfaState {
    std::vector<Edge> edges;
    bool accept{false};
};

struct Nfa {
    int start{0};
    int accept{0};
    std::vector<NfaState> states;
};

class NfaBuilder {
  public:
    explicit NfaBuilder(const RegexParser& parser);
    Nfa build(const std::string& pattern) const;

  private:
    const RegexParser& parser_;
};

struct DfaState {
    std::array<int, 256> next{};
    bool accept{false};
};

struct Dfa {
    int start{0};
    std::vector<DfaState> states;
};

class DfaBuilder {
  public:
    Dfa build(const Nfa& nfa) const;
};

struct Efa {
    Nfa automaton;
    std::size_t mismatchBudget{0};
    std::string pattern;
};

class EfaBuilder {
  public:
    explicit EfaBuilder(const RegexParser& parser);
    Efa build(const std::string& pattern, std::size_t mismatchBudget) const;

  private:
    const RegexParser& parser_;
};

struct PdaRule {
    char expected;
};

struct Pda {
    std::vector<PdaRule> rules;
};

class PdaBuilder {
  public:
    Pda build() const;
};

}  // namespace automata
