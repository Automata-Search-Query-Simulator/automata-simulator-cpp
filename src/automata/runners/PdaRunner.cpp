#include "automata/runners/Runners.hpp"

#include <sstream>
#include <vector>

namespace automata {

PdaRunner::PdaRunner(Pda pda, bool trace) : pda_(std::move(pda)), trace_(trace) {}

RunResult PdaRunner::run(const std::string& input) {
    RunResult result;
    int currentState = pda_.start;
    std::size_t stackDepth = 0;
    
    for (std::size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        ++result.statesVisited;
        
        // Find the current PDA state
        if (currentState < 0 || currentState >= static_cast<int>(pda_.states.size())) {
            return result;  // Invalid state, reject
        }
        
        const auto& state = pda_.states[currentState];
        
        // Find matching transition for this character
        const PdaTransition* matchingTransition = nullptr;
        for (const auto& trans : state.transitions) {
            if (trans.symbol == c) {
                matchingTransition = &trans;
                break;
            }
        }
        
        if (!matchingTransition) {
            return result;  // No transition, reject
        }
        
        // Execute the transition
        if (matchingTransition->operation == PdaOperation::Push) {
            ++stackDepth;
        } else if (matchingTransition->operation == PdaOperation::Pop) {
            if (stackDepth == 0) {
                return result;  // Can't pop from empty stack, reject
            }
            --stackDepth;
        }
        // Ignore does nothing
        
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
    
    // Accept if we end in an accepting state with empty stack
    if (pda_.states[currentState].accept && stackDepth == 0) {
        result.accepted = true;
        result.matches.push_back({0, input.size()});
    }
    
    return result;
}

}  // namespace automata
