#include "automata/builders/Builders.hpp"

#include <stdexcept>

namespace automata {

Efa EfaBuilder::build(const std::string& pattern, std::size_t mismatchBudget) const {
    if (pattern.empty()) {
        throw std::runtime_error("Approximate matching requires a non-empty literal pattern.");
    }
    for (char c : pattern) {
        if (c == '(' || c == ')' || c == '[' || c == ']' || c == '*' || c == '+' || c == '?' || c == '|') {
            throw std::runtime_error("EFA builder currently supports literal patterns only.");
        }
    }
    return Efa{pattern, mismatchBudget};
}

}  // namespace automata
