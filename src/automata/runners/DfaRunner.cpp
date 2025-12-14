#include "automata/runners/Runners.hpp"

#include <sstream>

namespace automata {

DfaRunner::DfaRunner(Dfa dfa, bool trace) : dfa_(std::move(dfa)), trace_(trace) {}

RunResult DfaRunner::run(const std::string& input) {
    RunResult result;
    // shortcut: empty input matches when the start state itself is accepting
    if (input.empty() && dfa_.states[dfa_.start].accept) {
        result.accepted = true;
        result.matches.emplace_back(0, 0);
        return result;
    }
    // try running the DFA from each possible start index
    for (std::size_t start = 0; start < input.size(); ++start) {
        int state = dfa_.start;
        // advance through the input until the automaton can no longer transition
        for (std::size_t pos = start; pos < input.size(); ++pos) {
            ++result.statesVisited;
            unsigned char c = static_cast<unsigned char>(input[pos]);
            int next = dfa_.states[state].next[c];
            if (trace_) {
                std::ostringstream oss;
                oss << "start=" << start << " pos=" << pos << " state=" << state << " next=" << next;
                result.trace.push_back({pos, oss.str()});
            }
            if (next == -1) {
                break;
            }
            state = next;
            // add any accepting matches reached after following the transition
            if (dfa_.states[state].accept) {
                result.matches.push_back({start, pos + 1});
                if (start == 0 && pos + 1 == input.size()) {
                    result.accepted = true;
                }
            }
        }
    }
    return result;
}

}  // namespace automata
