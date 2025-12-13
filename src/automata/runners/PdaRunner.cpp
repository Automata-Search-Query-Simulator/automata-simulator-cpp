#include "automata/runners/Runners.hpp"

#include <sstream>
#include <vector>
#include <stack>
#include <cctype>

namespace automata {

PdaRunner::PdaRunner(Pda pda, bool trace) : pda_(std::move(pda)), trace_(trace) {}

bool isValidBasePair(char b1, char b2) {
    b1 = std::toupper(b1);
    b2 = std::toupper(b2);
    return (b1 == 'A' && b2 == 'U') || (b1 == 'U' && b2 == 'A') ||
           (b1 == 'G' && b2 == 'C') || (b1 == 'C' && b2 == 'G');
}

// Validate that sequence contains only RNA bases: A, U, C, G
bool isValidRnaSequence(const std::string& seq) {
    for (char c : seq) {
        char upper = std::toupper(c);
        if (upper != 'A' && upper != 'U' && upper != 'C' && upper != 'G') {
            return false;  // Invalid: contains T, numbers, or other characters
        }
    }
    return true;
}

RunResult PdaRunner::run(const std::string& input) {
    RunResult result;
    
    // RNA validation mode
    if (!rnaSecondary_.empty() && input.size() == rnaSecondary_.size()) {
        result.isRnaValidation = true;
        
        // First, validate that this is actual RNA (only A, U, C, G)
        if (!isValidRnaSequence(input)) {
            result.accepted = false;
            result.rnaParenthesesValid = false;
            // Add error info
            result.basePairs.clear();
            return result;
        }
        
        // Validate parentheses balance
        int balance = 0;
        for (char c : rnaSecondary_) {
            if (c == '(') balance++;
            else if (c == ')') {
                balance--;
                if (balance < 0) {
                    result.accepted = false;
                    result.rnaParenthesesValid = false;
                    return result;
                }
            }
        }
        result.rnaParenthesesValid = (balance == 0);
        
        if (!result.rnaParenthesesValid) {
            result.accepted = false;
            return result;
        }
        
        // Find matching pairs and validate base pairing
        std::stack<std::size_t> st;
        bool allValid = true;
        
        for (std::size_t i = 0; i < rnaSecondary_.size(); ++i) {
            if (rnaSecondary_[i] == '(') {
                st.push(i);
            } else if (rnaSecondary_[i] == ')') {
                if (!st.empty()) {
                    std::size_t j = st.top();
                    st.pop();
                    
                    bool valid = isValidBasePair(input[j], input[i]);
                    result.basePairs.push_back({j, i, static_cast<char>(std::toupper(input[j])), 
                                              static_cast<char>(std::toupper(input[i])), valid});
                    
                    if (!valid) allValid = false;
                }
            }
        }
        
        result.accepted = allValid && result.rnaParenthesesValid;
        if (result.accepted) {
            result.matches.push_back({0, input.size()});
        }
        result.statesVisited = 1;
        return result;
    }
    
    // Standard PDA mode
    int currentState = pda_.start;
    std::size_t stackDepth = 0;
    
    for (std::size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        ++result.statesVisited;
        
        if (currentState < 0 || currentState >= static_cast<int>(pda_.states.size())) {
            return result;
        }
        
        const auto& state = pda_.states[currentState];
        
        const PdaTransition* matchingTransition = nullptr;
        for (const auto& trans : state.transitions) {
            if (trans.symbol == c) {
                matchingTransition = &trans;
                break;
            }
        }
        
        if (!matchingTransition) {
            return result;
        }
        
        if (matchingTransition->operation == PdaOperation::Push) {
            ++stackDepth;
        } else if (matchingTransition->operation == PdaOperation::Pop) {
            if (stackDepth == 0) {
                return result;
            }
            --stackDepth;
        }
        
        currentState = matchingTransition->to;
        
        if (stackDepth > result.stackDepth) {
            result.stackDepth = stackDepth;
        }
        
        if (trace_) {
            std::ostringstream oss;
            oss << "pos=" << i << " state=" << currentState << " stack=" << stackDepth;
            result.trace.push_back({i, oss.str()});
        }
    }
    
    if (pda_.states[currentState].accept && stackDepth == 0) {
        result.accepted = true;
        result.matches.push_back({0, input.size()});
    }
    
    return result;
}

}  // namespace automata
