#include "automata/runners/Runners.hpp"

#include <sstream>
#include <vector>

namespace automata {

PdaRunner::PdaRunner(Pda pda, bool trace) : pda_(std::move(pda)), trace_(trace) {}

RunResult PdaRunner::run(const std::string& input) {
    RunResult result;
    std::vector<char> stack;
    for (std::size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        if (c == '(') {
            stack.push_back(c);
        } else if (c == ')') {
            if (stack.empty()) {
                return result;
            }
            stack.pop_back();
        } else if (c == '.') {
            // ignore
        } else {
            return result;
        }
        if (trace_) {
            std::ostringstream oss;
            oss << "pos=" << i << " stack=" << stack.size();
            result.trace.push_back({i, oss.str()});
        }
        if (stack.size() > result.stackDepth) {
            result.stackDepth = stack.size();
        }
        ++result.statesVisited;
    }
    if (stack.empty()) {
        result.accepted = true;
        result.matches.push_back({0, input.size()});
    }
    return result;
}

}  // namespace automata
