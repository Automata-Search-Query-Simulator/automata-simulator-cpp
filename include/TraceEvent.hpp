#pragma once

#include <cstddef>
#include <string>

namespace automata {

struct TraceEvent {
    std::size_t index{0};
    std::string detail;
};

}  // namespace automata
