#include "reporting/Reporting.hpp"

namespace automata {

void MetricsAggregator::record(const RunResult& result) {
    ++runs_;
    matches_ += result.matches.size();
    if (result.accepted) {
        ++accepts_;
    }
}

std::size_t MetricsAggregator::totalMatches() const { return matches_; }

}  // namespace automata
