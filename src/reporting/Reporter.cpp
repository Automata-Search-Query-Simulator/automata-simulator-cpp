#include "reporting/Reporting.hpp"

#include <sstream>

namespace automata {

void Reporter::add(const SequenceReport& report) { reports_.push_back(report); }

std::string Reporter::summary(const MetricsAggregator& metrics) const {
    std::ostringstream oss;
    oss << "Runs: " << metrics.totalRuns() << ", Matches: " << metrics.totalMatches()
        << ", All accepted: " << (metrics.allAccepted() ? "yes" : "no");
    return oss.str();
}

}  // namespace automata
