#pragma once

#include <string>
#include <utility>
#include <vector>

namespace automata {

enum class RegexTokenType { Literal, CharClass, Any, Concat, Union, Star, Plus, Question };

struct RegexToken {
    RegexTokenType type;
    std::string text;
};

class RegexParser {
  public:
    std::vector<RegexToken> parseToPostfix(const std::string& pattern) const;
};

class DatasetLoader {
  public:
    std::vector<std::string> loadSequences(const std::string& path) const;
};

class DotBracketValidator {
  public:
    bool validate(const std::string& sequence) const;
};

}  // namespace automata
