#include "automata/runners/Runners.hpp"

#include <sstream>

#include "automata/utils/StateSet.hpp"

namespace automata {
namespace {

bool isAccepting(const Nfa& nfa, const std::vector<int>& states) {
    for (int state : states) {
        if (nfa.states[state].accept) {
            return true;
        }
    }
    return false;
}

}  // namespace

NfaRunner::NfaRunner(Nfa nfa, bool trace) : nfa_(std::move(nfa)), trace_(trace) {}

RunResult NfaRunner::run(const std::string& input) {
    RunResult result;
    const auto startClosure = epsilonClosure(nfa_, {nfa_.start});
    if (input.empty()) {
        if (isAccepting(nfa_, startClosure)) {
            result.accepted = true;
            result.matches.emplace_back(0, 0);
        }
        return result;
    }
    bool entireMatch = false;
    for (std::size_t start = 0; start < input.size(); ++start) {
        auto current = startClosure;
        if (current.empty()) {
            break;
        }
        for (std::size_t pos = start; pos < input.size(); ++pos) {
            auto moved = move(nfa_, current, input[pos]);
            current = epsilonClosure(nfa_, moved);
            result.statesVisited += current.size();
            if (trace_) {
                std::ostringstream oss;
                oss << "start=" << start << " pos=" << pos << " states=" << current.size();
                result.trace.push_back({pos, oss.str()});
            }
            if (current.empty()) {
                break;
            }
            if (isAccepting(nfa_, current)) {
                result.matches.push_back({start, pos + 1});
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
