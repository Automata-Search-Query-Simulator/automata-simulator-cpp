#include "automata/builders/Builders.hpp"

namespace automata {

Pda PdaBuilder::build() const {
    return Pda{{PdaRule{'('}, PdaRule{')'}}};
}

}  // namespace automata
