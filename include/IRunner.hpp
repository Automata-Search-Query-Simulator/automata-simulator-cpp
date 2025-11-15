#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "TraceEvent.hpp"

namespace automata {

struct RunResult {
    bool accepted{false};
    std::vector<std::pair<std::size_t, std::size_t>> matches;
    std::vector<TraceEvent> trace;
    std::size_t statesVisited{0};
    std::size_t stackDepth{0};
};

class IRunner {
  public:
    virtual ~IRunner() = default;
    virtual RunResult run(const std::string& input) = 0;
};

using RunnerPtr = std::unique_ptr<IRunner>;

}  // namespace automata
