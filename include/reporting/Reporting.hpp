#pragma once

#include <string>
#include <utility>
#include <vector>

#include "IRunner.hpp"

namespace automata {

struct SequenceReport {
    std::string sequenceSummary;
    RunResult result;
};

class MetricsAggregator {
  public:
    void record(const RunResult& result);
    std::size_t totalMatches() const;
    std::size_t totalRuns() const { return runs_; }
    bool allAccepted() const { return accepts_ == runs_; }

  private:
    std::size_t matches_{0};
    std::size_t runs_{0};
    std::size_t accepts_{0};
};

class TraceFormatter {
  public:
    std::string format(const RunResult& result) const;
};

class Reporter {
  public:
    void add(const SequenceReport& report);
    std::string summary(const MetricsAggregator& metrics) const;
    const std::vector<SequenceReport>& reports() const { return reports_; }

  private:
    std::vector<SequenceReport> reports_;
};

const char* resetColor(bool enabled);
bool colorOutputEnabled();
std::string colorize(const std::string& text, const char* color, bool enabled);
std::string highlightMatches(const std::string& sequence,
                             std::vector<std::pair<std::size_t, std::size_t>> matches,
                             bool colorizeOutput);

}  // namespace automata
