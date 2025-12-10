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

std::size_t DotBracketValidator::getMaxDepth(const std::string& sequence) const {
    std::size_t currentDepth = 0;
    std::size_t maxDepth = 0;
    for (char c : sequence) {
        if (c == '(') {
            ++currentDepth;
            if (currentDepth > maxDepth) {
                maxDepth = currentDepth;
            }
        } else if (c == ')') {
            if (currentDepth == 0) {
                return 0;  // Invalid: closing without opening
            }
            --currentDepth;
        }
        // Ignore '.' and other characters
    }
    return maxDepth;
}

}  // namespace automata
