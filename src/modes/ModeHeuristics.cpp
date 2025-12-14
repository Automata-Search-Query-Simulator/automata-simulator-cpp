#include "modes/ModeHeuristics.hpp"

namespace automata {

bool shouldDeterminize(std::size_t patternLength, std::size_t inputLength) {
    if (patternLength == 0) {
        return false;
    }
    if (patternLength < 6) {
        return true;
    }
    return inputLength > patternLength * 10;
}

}  // namespace automata
