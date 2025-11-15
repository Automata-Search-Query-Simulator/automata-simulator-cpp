#include "reporting/Reporting.hpp"

#include <sstream>

namespace automata {

std::string TraceFormatter::format(const RunResult& result) const {
    if (result.trace.empty()) {
        return "Tracing disabled.";
    }
    std::ostringstream oss;
    for (const auto& event : result.trace) {
        oss << "[" << event.index << "] " << event.detail << "\n";
    }
    return oss.str();
}

}  // namespace automata
