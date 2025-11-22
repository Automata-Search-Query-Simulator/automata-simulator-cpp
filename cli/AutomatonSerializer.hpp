#pragma once

#include <string>

#include "automata/runners/Runners.hpp"

namespace automata {

std::string serializeSnapshot(const RunnerFactory::Snapshot& snapshot);

}  // namespace automata

