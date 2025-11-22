#include "reporting/Reporting.hpp"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <sstream>
#include <vector>

#if defined(_WIN32)
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

namespace automata {
namespace {

const char* resetColorInternal(bool enabled) { return enabled ? "\033[0m" : ""; }

}  // namespace

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

const char* resetColor(bool enabled) {
    return resetColorInternal(enabled);
}

bool colorOutputEnabled() {
    static bool enabled = [] {
        if (std::getenv("NO_COLOR")) {
            return false;
        }
        return isatty(fileno(stdout)) != 0;
    }();
    return enabled;
}

std::string colorize(const std::string& text, const char* color, bool enabled) {
    if (!enabled) {
        return text;
    }

    std::string result(color);
    result.append(text);
    result.append(resetColorInternal(true));
    return result;
}

std::string highlightMatches(const std::string& sequence,
                             std::vector<std::pair<std::size_t, std::size_t>> matches,
                             bool colorizeOutput) {
    if (!colorizeOutput || matches.empty()) {
        return sequence;
    }
    std::sort(matches.begin(), matches.end());
    static constexpr std::array<const char*, 5> palette = {"\033[38;5;214m", "\033[38;5;45m", "\033[38;5;118m",
                                                           "\033[38;5;207m", "\033[38;5;33m"};
    std::string result;
    result.reserve(sequence.size() + matches.size() * 10);
    std::size_t cursor = 0;
    std::size_t paletteIndex = 0;
    for (const auto& match : matches) {
        std::size_t start = std::min(match.first, sequence.size());
        std::size_t end = std::min(match.second, sequence.size());
        if (end <= start || end <= cursor) {
            continue;
        }
        if (start > cursor) {
            result.append(sequence.substr(cursor, start - cursor));
        } else if (start < cursor) {
            start = cursor;
        }
        const char* color = palette[paletteIndex % palette.size()];
        ++paletteIndex;
        result.append(color);
        result.append(sequence.substr(start, end - start));
        result.append(resetColorInternal(true));
        cursor = end;
    }
    if (cursor < sequence.size()) {
        result.append(sequence.substr(cursor));
    }
    return result;
}

}  // namespace automata
