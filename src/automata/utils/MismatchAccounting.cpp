#include "automata/utils/MismatchAccounting.hpp"

#include <algorithm>

namespace automata {

std::size_t hammingDistance(const std::string& lhs, const std::string& rhs) {
    const auto limit = std::min(lhs.size(), rhs.size());
    std::size_t distance = 0;
    for (std::size_t i = 0; i < limit; ++i) {
        if (lhs[i] != rhs[i]) {
            ++distance;
        }
    }
    distance += lhs.size() > rhs.size() ? lhs.size() - rhs.size() : rhs.size() - lhs.size();
    return distance;
}

}  // namespace automata
