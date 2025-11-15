#include "parser/Parsers.hpp"

#include <stack>

namespace automata {

bool DotBracketValidator::validate(const std::string& sequence) const {
    std::size_t balance = 0;
    for (char c : sequence) {
        if (c == '(') {
            ++balance;
        } else if (c == ')') {
            if (balance == 0) {
                return false;
            }
            --balance;
        } else if (c != '.') {
            return false;
        }
    }
    return balance == 0;
}

}  // namespace automata
