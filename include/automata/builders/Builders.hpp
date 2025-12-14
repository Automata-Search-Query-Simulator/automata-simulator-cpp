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

enum class PdaOperation { Push, Pop, Ignore };

struct PdaTransition {
    char symbol;
    int code;  // ASCII code
    int to;
    PdaOperation operation;
};

struct PdaState {
    int id;
    int stackDepth;
    bool accept;
    std::vector<PdaTransition> transitions;
};

struct Pda {
    int start{0};
    std::vector<PdaRule> rules;
    std::vector<PdaState> states;
};

class PdaBuilder {
  public:
    Pda build() const;
    Pda build(std::size_t maxDepth) const;
};

// Helper to build PDA states up to a given max depth
Pda buildPdaStates(std::size_t maxDepth);

}  // namespace automata
