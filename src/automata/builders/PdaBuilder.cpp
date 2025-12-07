#include "automata/builders/Builders.hpp"

namespace automata {

Pda PdaBuilder::build() const {

    // PdaRule takes argument of expected character (e.g '(' or ')')
    // Pda object accepts argument of rules
    return Pda{{PdaRule{'('}, PdaRule{')'}}};
}

}  // namespace automata
