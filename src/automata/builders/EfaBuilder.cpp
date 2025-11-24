#include "automata/builders/Builders.hpp"

#include <stdexcept>

namespace automata {

EfaBuilder::EfaBuilder(const RegexParser& parser) : parser_(parser) {}

Efa EfaBuilder::build(const std::string& pattern, std::size_t mismatchBudget) const {
    if (pattern.empty()) {
        throw std::runtime_error("Approximate matching requires a non-empty pattern.");
    }
    NfaBuilder builder(parser_);
    auto nfa = builder.build(pattern);
    return Efa{std::move(nfa), mismatchBudget, pattern};
}

}  // namespace automata
