#include "parser/Parsers.hpp"

#include <stack>

namespace automata {

// validates if sequence is balanced by incremending and decrementing balance variable and checking
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
