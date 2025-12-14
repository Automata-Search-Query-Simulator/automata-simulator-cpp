#include "automata/builders/Builders.hpp"

#include <algorithm>
#include <array>
#include <queue>
#include <string>
#include <unordered_map>

#include "automata/utils/StateSet.hpp"

namespace automata {
namespace {

// sorting then removing duplicates on the end side
std::vector<int> normalize(std::vector<int> states) {
    std::sort(states.begin(), states.end());
    states.erase(std::unique(states.begin(), states.end()), states.end());
    return states;
}

// like converting the vector in to a string with commas delimit
std::string makeKey(const std::vector<int>& states) {
    std::string key;
    for (int s : states) {
        key.append(std::to_string(s));
        key.push_back(',');
    }
    return key;
}

}  // namespace

Dfa DfaBuilder::build(const Nfa& nfa) const {
    Dfa dfa;
    std::vector<DfaState> dfaStates;
    // each unique subset of NFA states gets assigned a unique ID
    std::unordered_map<std::string, int> subsetToId;
    // todo list of subsets that need to be processed
    std::queue<std::vector<int>> pending;

    // epsilon closure = find all states reachable from start state by epsilon
    auto startClosure = normalize(epsilonClosure(nfa, {nfa.start}));

    // string rep
    auto startKey = makeKey(startClosure);
    
    // first nfa subset is 0
    subsetToId[startKey] = 0;
    // add first subset to queue
    pending.push(startClosure);

    // append new dfa state , assures each unique set of NFA state gets its own corresponding dfa state in order.
    // we add new dfa state when we encounter a new subset of NFA states that hasnt been represented yet
    dfaStates.push_back(DfaState{});

    // initial set of all transition to -1
    dfaStates[0].next.fill(-1);

    // go through each state in start closure
    for (int s : startClosure) {
        if (nfa.states[s].accept) {
            // if any of the states in the start closure are accepting, set the accepting flag for the first dfa state
            dfaStates[0].accept = true;
            break;
        }
    }

    while (!pending.empty()) {
        auto currentStates = pending.front();
        // remove front subset from queue
        pending.pop();
        int currentId = subsetToId[makeKey(currentStates)];
        
        
        for (int c = 0; c < 256; ++c) {
            // which nfa states we reach from the current set of NFA states were in
            auto moved = move(nfa, currentStates, static_cast<char>(c));
            if (moved.empty()) {
                continue;
            }
            // epsilon closure = find all states reachable from the current set of NFA states by epsilon
            auto closure = normalize(epsilonClosure(nfa, moved));
            // if the epsilon closure is empty, there are no reachable states from the current set of NFA states by epsilon
            if (closure.empty()) {
                continue;
            }
            
            // string rep
            auto key = makeKey(closure);
            
            int targetId = 0;
            auto it = subsetToId.find(key);

            // if the key is not existing, then it = subsetToId.end()
            if (it == subsetToId.end()) {
                targetId = static_cast<int>(dfaStates.size());

                // add new NFA subset to subsetToId
                subsetToId[key] = targetId;

                // add new NFA subset to queue
                pending.push(closure);

                // append new dfa state
                DfaState state{};

                // initial set of all transition to -1
                state.next.fill(-1);
                for (int s : closure) {

                    // if any of the states in the NFA subset are accepting, set the accepting flag for the new dfa state
                    // if any of the nfa state is accepting, then the whole dfa state is accepting
                    if (nfa.states[s].accept) {
                        state.accept = true;
                        break;
                    }
                }
                dfaStates.push_back(state);
            } else {
                // if the key is existing, then it = subsetToId.find(key)
                // it->second = the id of the NFA subset
                targetId = it->second;
            }

            // set the transition for the current dfa state to the target dfa state
            // next = transition
            dfaStates[currentId].next[c] = targetId;
        }
    }

    // set the start state of the dfa to the first dfa state
    dfa.start = 0;

    // set the states of the dfa to the dfa states
    dfa.states = std::move(dfaStates);
    return dfa;
}

}  // namespace automata
