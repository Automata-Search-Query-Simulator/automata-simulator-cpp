#include "automata/utils/StateSet.hpp"

#include <queue>

namespace automata {

StateSet::StateSet(std::size_t size) : visited_(size, false) {}

void StateSet::add(int state) {
    if (state < 0 || static_cast<std::size_t>(state) >= visited_.size()) {
        return;
    }
    if (!visited_[state]) {
        visited_[state] = true;
        values_.push_back(state);
    }
}

bool StateSet::contains(int state) const {
    return state >= 0 && static_cast<std::size_t>(state) < visited_.size() && visited_[state];
}

std::vector<int> epsilonClosure(const Nfa& nfa, const std::vector<int>& states) {
    StateSet closure(nfa.states.size());
    std::queue<int> q;
    for (int state : states) {
        closure.add(state);
        q.push(state);
    }
    while (!q.empty()) {
        int state = q.front();
        q.pop();
        for (const auto& edge : nfa.states[state].edges) {
            if (edge.type == EdgeType::Epsilon && !closure.contains(edge.to)) {
                closure.add(edge.to);
                q.push(edge.to);
            }
        }
    }
    auto values = closure.values();
    return values;
}

std::vector<int> move(const Nfa& nfa, const std::vector<int>& states, char symbol) {
    StateSet destination(nfa.states.size());
    for (int state : states) {
        for (const auto& edge : nfa.states[state].edges) {
            bool matches = false;
            switch (edge.type) {
                case EdgeType::Literal:
                    matches = edge.literal == symbol;
                    break;
                case EdgeType::Any:
                    matches = true;
                    break;
                case EdgeType::CharClass:
                    matches = edge.charClass.find(symbol) != std::string::npos;
                    break;
                default:
                    break;
            }
            if (matches) {
                destination.add(edge.to);
            }
        }
    }
    return destination.values();
}

}  // namespace automata
