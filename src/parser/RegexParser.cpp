#include "parser/Parsers.hpp"

#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <string>
#include <vector>

namespace automata {
namespace {

enum class RawTokenType {
    Literal,
    CharClass,
    Any,
    Union,
    Star,
    Plus,
    Question,
    LParen,
    RParen,
    Concat
};

struct RawToken {
    RawTokenType type;
    std::string text;
};

bool isOperandType(RawTokenType type) {
    return type == RawTokenType::Literal || type == RawTokenType::CharClass || type == RawTokenType::Any;
}

bool isUnaryOperator(RawTokenType type) {
    return type == RawTokenType::Star || type == RawTokenType::Plus || type == RawTokenType::Question;
}

bool needsConcat(RawTokenType prev, RawTokenType next) {
    auto beginsOperand = [](RawTokenType type) {
        return type == RawTokenType::Literal || type == RawTokenType::CharClass || type == RawTokenType::Any ||
               type == RawTokenType::LParen;
    };
    auto endsOperand = [&](RawTokenType type) {
        if (isOperandType(type)) {
            return true;
        }
        if (type == RawTokenType::RParen) {
            return true;
        }
        return isUnaryOperator(type);
    };
    return endsOperand(prev) && beginsOperand(next);
}

int precedence(RawTokenType type) {
    switch (type) {
        case RawTokenType::Star:
        case RawTokenType::Plus:
        case RawTokenType::Question:
            return 3;
        case RawTokenType::Concat:
            return 2;
        case RawTokenType::Union:
            return 1;
        default:
            return 0;
    }
}

bool isOperator(RawTokenType type) {
    return type == RawTokenType::Union || type == RawTokenType::Concat || type == RawTokenType::Star ||
           type == RawTokenType::Plus || type == RawTokenType::Question;
}

std::string parseCharClass(const std::string& pattern, std::size_t& index) {
    std::string cls;
    bool closed = false;
    for (std::size_t i = index + 1; i < pattern.size(); ++i) {
        if (pattern[i] == ']' && !cls.empty()) {
            index = i;
            closed = true;
            break;
        }
        if (pattern[i] == '-' && !cls.empty() && i + 1 < pattern.size() && pattern[i + 1] != ']') {
            char start = cls.back();
            char end = pattern[i + 1];
            if (start > end) {
                std::swap(start, end);
            }
            cls.pop_back();
            for (char c = start; c <= end; ++c) {
                cls.push_back(c);
            }
            ++i;
            continue;
        }
        cls.push_back(pattern[i]);
    }
    if (!closed) {
        throw std::runtime_error("Unterminated character class in regex pattern.");
    }
    return cls;
}

std::vector<RawToken> tokenize(const std::string& pattern) {
    std::vector<RawToken> tokens;
    for (std::size_t i = 0; i < pattern.size(); ++i) {
        char c = pattern[i];
        if (std::isspace(static_cast<unsigned char>(c))) {
            continue;
        }
        switch (c) {
            case '\\':
                if (i + 1 >= pattern.size()) {
                    throw std::runtime_error("Dangling escape in regex pattern.");
                }
                tokens.push_back({RawTokenType::Literal, std::string(1, pattern[i + 1])});
                ++i;
                break;
            case '.':
                tokens.push_back({RawTokenType::Any, {}});
                break;
            case '|':
                tokens.push_back({RawTokenType::Union, {}});
                break;
            case '*':
                tokens.push_back({RawTokenType::Star, {}});
                break;
            case '+':
                tokens.push_back({RawTokenType::Plus, {}});
                break;
            case '?':
                tokens.push_back({RawTokenType::Question, {}});
                break;
            case '(':
                tokens.push_back({RawTokenType::LParen, {}});
                break;
            case ')':
                tokens.push_back({RawTokenType::RParen, {}});
                break;
            case '[': {
                std::string cls = parseCharClass(pattern, i);
                tokens.push_back({RawTokenType::CharClass, cls});
                break;
            }
            default:
                tokens.push_back({RawTokenType::Literal, std::string(1, c)});
                break;
        }
    }

    std::vector<RawToken> enhanced;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        enhanced.push_back(tokens[i]);
        if (i + 1 < tokens.size() && needsConcat(tokens[i].type, tokens[i + 1].type)) {
            enhanced.push_back({RawTokenType::Concat, {}});
        }
    }
    return enhanced;
}

RegexToken toRegexToken(const RawToken& token) {
    switch (token.type) {
        case RawTokenType::Literal:
            return {RegexTokenType::Literal, token.text};
        case RawTokenType::CharClass:
            return {RegexTokenType::CharClass, token.text};
        case RawTokenType::Any:
            return {RegexTokenType::Any, {}};
        case RawTokenType::Union:
            return {RegexTokenType::Union, {}};
        case RawTokenType::Concat:
            return {RegexTokenType::Concat, {}};
        case RawTokenType::Star:
            return {RegexTokenType::Star, {}};
        case RawTokenType::Plus:
            return {RegexTokenType::Plus, {}};
        case RawTokenType::Question:
            return {RegexTokenType::Question, {}};
        default:
            return {RegexTokenType::Literal, ""};
    }
}

}  // namespace

std::vector<RegexToken> RegexParser::parseToPostfix(const std::string& pattern) const {
    auto tokens = tokenize(pattern);
    std::vector<RegexToken> output;
    std::vector<RawToken> opStack;

    for (const auto& token : tokens) {
        switch (token.type) {
            case RawTokenType::Literal:
            case RawTokenType::CharClass:
            case RawTokenType::Any:
                output.push_back(toRegexToken(token));
                break;
            case RawTokenType::LParen:
                opStack.push_back(token);
                break;
            case RawTokenType::RParen: {
                bool matched = false;
                while (!opStack.empty()) {
                    auto top = opStack.back();
                    opStack.pop_back();
                    if (top.type == RawTokenType::LParen) {
                        matched = true;
                        break;
                    }
                    output.push_back(toRegexToken(top));
                }
                if (!matched) {
                    throw std::runtime_error("Mismatched parentheses in regex pattern.");
                }
                break;
            }
            default: {
                while (!opStack.empty() && isOperator(opStack.back().type) &&
                       precedence(opStack.back().type) >= precedence(token.type)) {
                    output.push_back(toRegexToken(opStack.back()));
                    opStack.pop_back();
                }
                opStack.push_back(token);
                break;
            }
        }
    }

    while (!opStack.empty()) {
        auto top = opStack.back();
        opStack.pop_back();
        if (top.type == RawTokenType::LParen || top.type == RawTokenType::RParen) {
            throw std::runtime_error("Mismatched parentheses in regex pattern.");
        }
        output.push_back(toRegexToken(top));
    }

    return output;
}

}  // namespace automata
