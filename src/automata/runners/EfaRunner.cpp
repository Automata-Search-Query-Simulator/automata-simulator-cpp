#include "automata/runners/Runners.hpp"

#include <queue>
#include <sstream>
#include <vector>

namespace automata {
namespace {

// helper returns a mismatch cost greater than the allowed budget to mark unreachable states
std::size_t unreachableCost(std::size_t budget) {
    return budget + 1;
}

// alias for the vector that tracks mismatch cost per state
using CostVector = std::vector<std::size_t>;

// compute epsilon closure using the current mismatch costs and budget; needed because epsilon transitions
// do not consume symbols but still affect the set of reachable states and their mismatch costs.
CostVector epsilonClosureWithCosts(const Nfa& nfa, CostVector costs, std::size_t budget) {
    std::queue<int> q;
    std::vector<bool> inQueue(nfa.states.size(), false);
    for (std::size_t i = 0; i < costs.size(); ++i) {
        if (costs[i] <= budget) {
            q.push(static_cast<int>(i));
            inQueue[i] = true;
        }
    }
    while (!q.empty()) {
        const int state = q.front();
        q.pop();
        inQueue[state] = false;
        const auto cost = costs[state];
        for (const auto& edge : nfa.states[state].edges) {
            if (edge.type != EdgeType::Epsilon) {
                continue;
            }
            if (cost < costs[edge.to]) {
                costs[edge.to] = cost;
                if (!inQueue[edge.to]) {
                    q.push(edge.to);
                    inQueue[edge.to] = true;
                }
            }
        }
    }
    return costs;
}

// helpers that describe how edge types consume input symbols (used in mismatch accounting)
bool consumesSymbol(EdgeType type) {
    return type == EdgeType::Literal || type == EdgeType::Any || type == EdgeType::CharClass;
}

// determine if the current edge accepts the symbol (Literal/Any/CharClass are relevant)
bool matchesEdge(const Edge& edge, char symbol) {
    switch (edge.type) {
        case EdgeType::Literal:
            return edge.literal == symbol;
        case EdgeType::Any:
            return true;
        case EdgeType::CharClass:
            return edge.charClass.find(symbol) != std::string::npos;
        default:
            break;
    }
    return false;
}

// count how many states are still under the mismatch budget so we can stop early if none remain
std::size_t activeCount(const CostVector& costs, std::size_t budget) {
    std::size_t count = 0;
    for (auto cost : costs) {
        if (cost <= budget) {
            ++count;
        }
    }
    return count;
}

// check whether any active state is accepting, used for match detection
bool hasAccepting(const Nfa& nfa, const CostVector& costs, std::size_t budget) {
    for (std::size_t i = 0; i < costs.size(); ++i) {
        if (costs[i] <= budget && nfa.states[i].accept) {
            return true;
        }
    }
    return false;
}

// find the best (lowest) mismatch cost among active states, primarily for trace output detail
std::size_t minMismatch(const CostVector& costs, std::size_t budget) {
    std::size_t best = budget + 1;
    for (auto cost : costs) {
        if (cost <= budget && cost < best) {
            best = cost;
        }
    }
    return best;
}

}  // namespace

EfaRunner::EfaRunner(Efa efa, bool trace) : efa_(std::move(efa)), trace_(trace) {}

RunResult EfaRunner::run(const std::string& input) {
    RunResult result;
    // simulate against the stored NFA underlying the EFA
    const auto& nfa = efa_.automaton;
    if (nfa.states.empty()) {
        return result;
    }
    const auto budget = efa_.mismatchBudget;
    const auto unreachable = unreachableCost(budget);

    // initialize every state to unreachable, except the start state
    CostVector startCosts(nfa.states.size(), unreachable);
    startCosts[nfa.start] = 0;
    startCosts = epsilonClosureWithCosts(nfa, std::move(startCosts), budget);

    // handle the empty input case using only the epsilon closure of the start
    // (e.g., a pattern like "(|a)" can accept immediately without consuming characters)
    if (input.empty()) {
        if (hasAccepting(nfa, startCosts, budget)) {
            result.accepted = true;
            result.matches.emplace_back(0, 0);
        }
        return result;
    }

    bool entireMatch = false;
    const auto baseActive = activeCount(startCosts, budget);
    if (baseActive == 0) {
        return result;
    }
    // try each possible start position and track mismatch-aware costs; budget replays per start
    // mismatches are limited by the budget, so characters that push costs beyond it are skipped
    for (std::size_t start = 0; start < input.size(); ++start) {
        auto current = startCosts;
        for (std::size_t pos = start; pos < input.size(); ++pos) {
            CostVector next(nfa.states.size(), unreachable);
            const char symbol = input[pos];
            // consider every active state for the current character, building the next cost vector
            for (std::size_t state = 0; state < current.size(); ++state) {
                const auto cost = current[state];
                if (cost > budget) {
                    continue;
                }
                for (const auto& edge : nfa.states[state].edges) {
                    if (!consumesSymbol(edge.type)) {
                        continue;
                    }
                    const bool matches = matchesEdge(edge, symbol);
                    const auto newCost = cost + (matches ? 0 : 1);
                    if (newCost > budget) {
                        continue;
                    }
                    const auto to = static_cast<std::size_t>(edge.to);
                    // only update if this path gives a lower mismatch count
                    if (newCost >= next[to]) {
                        continue;
                    }
                    next[to] = newCost;
                }
            }
            current = epsilonClosureWithCosts(nfa, std::move(next), budget);
            const auto active = activeCount(current, budget);
            // stop consuming when no states remain within the budget
            if (active == 0) {
                break;
            }
            // record effort for diagnostics
            result.statesVisited += active;
            if (trace_) {
                std::ostringstream oss;
                const auto best = minMismatch(current, budget);
                oss << "start=" << start << " pos=" << pos << " states=" << active;
                if (best <= budget) {
                    oss << " bestMismatch=" << best;
                }
                result.trace.push_back({pos, oss.str()});
            }
            // if any active state is accepting, record the match interval
            if (hasAccepting(nfa, current, budget)) {
                result.matches.emplace_back(start, pos + 1);
                if (start == 0 && pos + 1 == input.size()) {
                    entireMatch = true;
                }
            }
        }
    }
    result.accepted = entireMatch;
    return result;
}

}  // namespace automata
