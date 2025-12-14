#pragma once

#include <cstddef>

namespace automata {

bool shouldDeterminize(std::size_t patternLength, std::size_t inputLength);

}  // namespace automata
