#include "automata/builders/Builders.hpp"

namespace automata {

Pda buildPdaStates(std::size_t maxDepth) {
    Pda pda;
    pda.start = 0;
    pda.rules = {PdaRule{'('}, PdaRule{')'}};
    
    for (std::size_t depth = 0; depth <= maxDepth; ++depth) {
        PdaState state;
        state.id = static_cast<int>(depth);
        state.stackDepth = static_cast<int>(depth);
        state.accept = (depth == 0);  // Accept state is at depth 0 (empty stack)
        
        // Push '(' transition (if not at max depth)
        if (depth < maxDepth) {
            state.transitions.push_back({
                '(',
                static_cast<int>('('),
                static_cast<int>(depth + 1),
                PdaOperation::Push
            });
        }
        
        // Pop ')' transition (if not at depth 0)
        if (depth > 0) {
            state.transitions.push_back({
                ')',
                static_cast<int>(')'),
                static_cast<int>(depth - 1),
                PdaOperation::Pop
            });
        }
        
        // Ignore '.' transition (always available)
        state.transitions.push_back({
            '.',
            static_cast<int>('.'),
            static_cast<int>(depth),
            PdaOperation::Ignore
        });
        
        pda.states.push_back(state);
    }
    
    return pda;
}

Pda PdaBuilder::build() const {
    // Build PDA with default support for up to depth 10
    return buildPdaStates(10);
}

Pda PdaBuilder::build(std::size_t maxDepth) const {
    // Build PDA with dynamic max depth
    return buildPdaStates(maxDepth);

}

}  // namespace automata
