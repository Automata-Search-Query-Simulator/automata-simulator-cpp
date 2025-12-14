#pragma once

#include <unordered_set>
#include <vector>

#include "automata/builders/Builders.hpp"

namespace automata {

class StateSet {
  public:
    explicit StateSet(std::size_t size);
    void add(int state);
    bool contains(int state) const;
    const std::vector<int>& values() const { return values_; }

  private:
    std::vector<int> values_;
    std::vector<bool> visited_;
};

std::vector<int> epsilonClosure(const Nfa& nfa, const std::vector<int>& states);
std::vector<int> move(const Nfa& nfa, const std::vector<int>& states, char symbol);

}  // namespace automata
