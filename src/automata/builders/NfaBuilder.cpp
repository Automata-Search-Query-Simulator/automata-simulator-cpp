#include "automata/builders/Builders.hpp"

#include <stack>
#include <stdexcept>

namespace automata {
namespace {

struct Fragment {
    int start;
    int accept;
};

Edge makeEdge(int to, EdgeType type, char literal, const std::string& cls = {}) {
    return Edge{to, type, literal, cls};
}

}  // namespace

NfaBuilder::NfaBuilder(const RegexParser& parser) : parser_(parser) {}

Nfa NfaBuilder::build(const std::string& pattern) const {
    std::vector<NfaState> states;
    auto newState = [&states]() {
        states.push_back(NfaState{});
        return static_cast<int>(states.size() - 1);
    };
    auto addEdge = [&states](int from, int to, EdgeType type, char literal = '\0', const std::string& cls = std::string()) {
        states[from].edges.push_back(makeEdge(to, type, literal, cls));
    };

    std::vector<Fragment> stack;
    const auto postfix = parser_.parseToPostfix(pattern);
    if (postfix.empty()) {
        int start = newState();
        int accept = newState();
        addEdge(start, accept, EdgeType::Epsilon);
        states[accept].accept = true;
        return Nfa{start, accept, states};
    }
    for (const auto& token : postfix) {
        switch (token.type) {
            case RegexTokenType::Literal: {
                int start = newState();
                int accept = newState();
                addEdge(start, accept, EdgeType::Literal, token.text.empty() ? '\0' : token.text.front());
                stack.push_back({start, accept});
                break;
            }
            case RegexTokenType::CharClass: {
                int start = newState();
                int accept = newState();
                addEdge(start, accept, EdgeType::CharClass, '\0', token.text);
                stack.push_back({start, accept});
                break;
            }
            case RegexTokenType::Any: {
                int start = newState();
                int accept = newState();
                addEdge(start, accept, EdgeType::Any);
                stack.push_back({start, accept});
                break;
            }
            case RegexTokenType::Concat: {
                if (stack.size() < 2) {
                    throw std::runtime_error("Invalid regex: concatenation missing operand.");
                }
                auto right = stack.back();
                stack.pop_back();
                auto left = stack.back();
                stack.pop_back();
                addEdge(left.accept, right.start, EdgeType::Epsilon);
                stack.push_back({left.start, right.accept});
                break;
            }
            case RegexTokenType::Union: {
                if (stack.size() < 2) {
                    throw std::runtime_error("Invalid regex: union missing operand.");
                }
                auto right = stack.back();
                stack.pop_back();
                auto left = stack.back();
                stack.pop_back();
                int start = newState();
                int accept = newState();
                addEdge(start, left.start, EdgeType::Epsilon);
                addEdge(start, right.start, EdgeType::Epsilon);
                addEdge(left.accept, accept, EdgeType::Epsilon);
                addEdge(right.accept, accept, EdgeType::Epsilon);
                stack.push_back({start, accept});
                break;
            }
            case RegexTokenType::Star: {
                if (stack.empty()) {
                    throw std::runtime_error("Invalid regex: star missing operand.");
                }
                auto frag = stack.back();
                stack.pop_back();
                int start = newState();
                int accept = newState();
                addEdge(start, frag.start, EdgeType::Epsilon);
                addEdge(start, accept, EdgeType::Epsilon);
                addEdge(frag.accept, frag.start, EdgeType::Epsilon);
                addEdge(frag.accept, accept, EdgeType::Epsilon);
                stack.push_back({start, accept});
                break;
            }
            case RegexTokenType::Plus: {
                if (stack.empty()) {
                    throw std::runtime_error("Invalid regex: plus missing operand.");
                }
                auto frag = stack.back();
                stack.pop_back();
                int start = newState();
                int accept = newState();
                addEdge(start, frag.start, EdgeType::Epsilon);
                addEdge(frag.accept, frag.start, EdgeType::Epsilon);
                addEdge(frag.accept, accept, EdgeType::Epsilon);
                stack.push_back({start, accept});
                break;
            }
            case RegexTokenType::Question: {
                if (stack.empty()) {
                    throw std::runtime_error("Invalid regex: question missing operand.");
                }
                auto frag = stack.back();
                stack.pop_back();
                int start = newState();
                int accept = newState();
                addEdge(start, frag.start, EdgeType::Epsilon);
                addEdge(start, accept, EdgeType::Epsilon);
                addEdge(frag.accept, accept, EdgeType::Epsilon);
                stack.push_back({start, accept});
                break;
            }
        }
    }
    if (stack.size() != 1) {
        throw std::runtime_error("Invalid regex pattern produced multiple fragments.");
    }
    auto fragment = stack.back();
    states[fragment.accept].accept = true;
    return Nfa{fragment.start, fragment.accept, states};
}

}  // namespace automata
