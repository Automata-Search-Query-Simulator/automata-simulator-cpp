#include "automata/builders/Builders.hpp"

#include <stdexcept>

namespace automata {

EfaBuilder::EfaBuilder(const RegexParser& parser) : parser_(parser) {}

Efa EfaBuilder::build(const std::string& pattern, std::size_t mismatchBudget) const {
    if (pattern.empty()) {
        throw std::runtime_error("Approximate matching requires a non-empty pattern.");
    }

    // create Nfabuilder object then pass regex parser from the Efabuilder
    // RegexParser -> EfaBuilder(parser) -> NfaBuilder(parser)
    NfaBuilder builder(parser_);

    // builds the NFA for the pattern
    auto nfa = builder.build(pattern);

    // Efa constructor
    // std::move(nfa) = make nfa into an r-value expression (temporary) since nfa is a BIG object
    return Efa{std::move(nfa), mismatchBudget, pattern};
}

}  // namespace automata
