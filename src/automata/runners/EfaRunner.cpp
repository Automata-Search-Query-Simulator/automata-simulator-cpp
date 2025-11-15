#include "automata/runners/Runners.hpp"

#include <sstream>

#include "automata/utils/MismatchAccounting.hpp"

namespace automata {

EfaRunner::EfaRunner(Efa efa, bool trace) : efa_(std::move(efa)), trace_(trace) {}

RunResult EfaRunner::run(const std::string& input) {
    RunResult result;
    const auto patternLength = efa_.literalPattern.size();
    if (patternLength == 0 || input.size() < patternLength) {
        return result;
    }
    for (std::size_t start = 0; start + patternLength <= input.size(); ++start) {
        const std::string window = input.substr(start, patternLength);
        auto distance = hammingDistance(window, efa_.literalPattern);
        result.statesVisited += patternLength;
        if (trace_) {
            std::ostringstream oss;
            oss << "start=" << start << " mismatches=" << distance;
            result.trace.push_back({start, oss.str()});
        }
        if (distance <= efa_.mismatchBudget) {
            result.matches.push_back({start, start + patternLength});
            if (start == 0 && patternLength == input.size()) {
                result.accepted = true;
            }
        }
    }
    return result;
}

}  // namespace automata
