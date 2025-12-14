#include "automata/builders/Builders.hpp"

#include <stack>
#include <stdexcept>

namespace automata {
namespace {

struct Fragment {
    int start;
    int accept;
};

// make transition
// cls is used to represent character class (e.g [a-z]) 
Edge makeEdge(int to, EdgeType type, char literal, const std::string& cls = {}) {
    return Edge{to, type, literal, cls};
}

}  // namespace

NfaBuilder::NfaBuilder(const RegexParser& parser) : parser_(parser) {}

Nfa NfaBuilder::build(const std::string& pattern) const {
    std::vector<NfaState> states;

    // lambda function to create a new NfaState
    auto newState = [&states]() {
        states.push_back(NfaState{});
        return static_cast<int>(states.size() - 1);
    };

    // lambda function to create a new edge between two states
    auto addEdge = [&states](int from, int to, EdgeType type, char literal = '\0', const std::string& cls = std::string()) {
        states[from].edges.push_back(makeEdge(to, type, literal, cls));
    };

    // as we parse every token, we build sub-NFAs and push them on to the stack

    std::vector<Fragment> stack;
    
    const auto postfix = parser_.parseToPostfix(pattern);

    // if the pattern is empty, return a single state NFA
    if (postfix.empty()) {
        int start = newState();
        int accept = newState();
        addEdge(start, accept, EdgeType::Epsilon);
        states[accept].accept = true;
        return Nfa{start, accept, states};
    }

    for (const auto& token : postfix) {
        switch (token.type) {
            // if token is literal, create a new state and add an edge from the start state to the accept state
            case RegexTokenType::Literal: {
                int start = newState();
                int accept = newState();
                addEdge(start, accept, EdgeType::Literal, token.text.empty() ? '\0' : token.text.front());
                stack.push_back({start, accept});
                break;
            }
            // if token is char class, create a new state and add an edge from the start state to the accept state
            case RegexTokenType::CharClass: {
                int start = newState();
                int accept = newState();
                addEdge(start, accept, EdgeType::CharClass, '\0', token.text);
                stack.push_back({start, accept});
                break;
            }
            // if token is any, create a new state and add an edge from the start state to the accept state
            case RegexTokenType::Any: {
                int start = newState();
                int accept = newState();
                addEdge(start, accept, EdgeType::Any);
                stack.push_back({start, accept});
                break;
            }
            // if token is concatenation, 
            case RegexTokenType::Concat: {
                // stack should have atleast 2 fragments
                if (stack.size() < 2) {
                    throw std::runtime_error("Invalid regex: concatenation missing operand.");
                }

                // pop right fragment and store in right then remove from stack
                auto right = stack.back();
                stack.pop_back();
                // pop left fragment and store in left then remove from stack
                auto left = stack.back();
                stack.pop_back();

                // add an epsilon edge from the accept state of the left fragment to the start state of the right fragment
                addEdge(left.accept, right.start, EdgeType::Epsilon);
                stack.push_back({left.start, right.accept});
                break;
            }
            case RegexTokenType::Union: {
                // stack should have atleast 2 fragments
                if (stack.size() < 2) {
                    throw std::runtime_error("Invalid regex: union missing operand.");
                }

                // pop right fragment and store in right then remove from stack
                auto right = stack.back();
                stack.pop_back();
                // pop left fragment and store in left then remove from stack
                auto left = stack.back();
                stack.pop_back();

                // create a new start state and accept state
                int start = newState();
                int accept = newState();

                // add epsilon edges from the start state to the start states of the left and right fragments
                // nfa can begin by choosing left or right fragment first
                addEdge(start, left.start, EdgeType::Epsilon);
                addEdge(start, right.start, EdgeType::Epsilon);

                // add epsilon edges from the accept states of the left and right fragments to the accept state
                addEdge(left.accept, accept, EdgeType::Epsilon);
                addEdge(right.accept, accept, EdgeType::Epsilon);

                // push combined fragment on to the stack
                stack.push_back({start, accept});
                break;
            }
            case RegexTokenType::Star: {

                // stack should have atleast 1 fragment
                if (stack.empty()) {
                    throw std::runtime_error("Invalid regex: star missing operand.");
                }
                // pop fragment and store in frag then remove from stack
                auto frag = stack.back();
                stack.pop_back();
                // create a new start state and accept state
                int start = newState();
                int accept = newState();

                // add epsilon edges from the start state to the start state of the fragment
                addEdge(start, frag.start, EdgeType::Epsilon);

                // add epsilon edges from the start state to the accept state
                // allows zero occurences
                addEdge(start, accept, EdgeType::Epsilon);

                // add epsilon edges from the accept state of the fragment to the start state and accept state
                addEdge(frag.accept, frag.start, EdgeType::Epsilon);

                // add epsilon edges from the accept state of the fragment to the accept state
                addEdge(frag.accept, accept, EdgeType::Epsilon);
                stack.push_back({start, accept});
                break;
            }
            case RegexTokenType::Plus: {
                if (stack.empty()) {
                    throw std::runtime_error("Invalid regex: plus missing operand.");
                }

                // pop fragment and store in frag then remove from stack
                auto frag = stack.back();
                stack.pop_back();
                // create a new start state and accept state
                int start = newState();
                int accept = newState();

                // add epsilon edges from the start state to the start state of the fragment
                addEdge(start, frag.start, EdgeType::Epsilon);

                // add epsilon edges from the accept state of the fragment to the start state and accept state
                addEdge(frag.accept, frag.start, EdgeType::Epsilon);
                addEdge(frag.accept, accept, EdgeType::Epsilon);

                // theres no addEdge that goes from start -> accept since it needs to occure atleast once

                stack.push_back({start, accept});
                break;
            }
            case RegexTokenType::Question: {
                if (stack.empty()) {
                    throw std::runtime_error("Invalid regex: question missing operand.");
                }
                // pop fragment and store in frag then remove from stack
                auto frag = stack.back();
                stack.pop_back();

                // create a new start state and accept state
                int start = newState();
                int accept = newState();

                // add epsilon edges from the start state to the start state of the fragment
                addEdge(start, frag.start, EdgeType::Epsilon);
                // add epsilon edges from the start state to the accept state
                // allows no occurences
                addEdge(start, accept, EdgeType::Epsilon);
                // add epsilon edges from the accept state of the fragment to the accept state
                addEdge(frag.accept, accept, EdgeType::Epsilon);
                stack.push_back({start, accept});
                break;
            }
        }
    }

    // should have exactly 1 fragment on the stack
    // represented by a single NFA fragment after processing every token
    if (stack.size() != 1) {
        throw std::runtime_error("Invalid regex pattern produced multiple fragments.");
    }

    // set the accept state of the fragment to true
    // mark the accepting fragment state on the nfa states as true
    auto fragment = stack.back();
    states[fragment.accept].accept = true;
    return Nfa{fragment.start, fragment.accept, states};
}

}  // namespace automata
