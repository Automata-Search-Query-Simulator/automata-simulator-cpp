#include "automata/builders/Builders.hpp"

#include <algorithm>
#include <array>
#include <queue>
#include <string>
#include <unordered_map>

#include "automata/utils/StateSet.hpp"

namespace automata {
namespace {

std::vector<int> normalize(std::vector<int> states) {
    std::sort(states.begin(), states.end());
    states.erase(std::unique(states.begin(), states.end()), states.end());
    return states;
}

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
    std::unordered_map<std::string, int> subsetToId;
    std::queue<std::vector<int>> pending;

    auto startClosure = normalize(epsilonClosure(nfa, {nfa.start}));
    auto startKey = makeKey(startClosure);
    subsetToId[startKey] = 0;
    pending.push(startClosure);
    dfaStates.push_back(DfaState{});
    dfaStates[0].next.fill(-1);
    for (int s : startClosure) {
        if (nfa.states[s].accept) {
            dfaStates[0].accept = true;
            break;
        }
    }

    while (!pending.empty()) {
        auto currentStates = pending.front();
        pending.pop();
        int currentId = subsetToId[makeKey(currentStates)];
        for (int c = 0; c < 256; ++c) {
            auto moved = move(nfa, currentStates, static_cast<char>(c));
            if (moved.empty()) {
                continue;
            }
            auto closure = normalize(epsilonClosure(nfa, moved));
            if (closure.empty()) {
                continue;
            }
            auto key = makeKey(closure);
            int targetId = 0;
            auto it = subsetToId.find(key);
            if (it == subsetToId.end()) {
                targetId = static_cast<int>(dfaStates.size());
                subsetToId[key] = targetId;
                pending.push(closure);
                DfaState state{};
                state.next.fill(-1);
                for (int s : closure) {
                    if (nfa.states[s].accept) {
                        state.accept = true;
                        break;
                    }
                }
                dfaStates.push_back(state);
            } else {
                targetId = it->second;
            }
            dfaStates[currentId].next[c] = targetId;
        }
    }

    dfa.start = 0;
    dfa.states = std::move(dfaStates);
    return dfa;
}

}  // namespace automata
