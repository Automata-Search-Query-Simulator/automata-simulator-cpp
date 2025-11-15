#include "parser/Parsers.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace automata {

std::vector<std::string> DatasetLoader::loadSequences(const std::string& path) const {
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("Failed to open dataset: " + path);
    }
    std::vector<std::string> sequences;
    std::ostringstream builder;
    bool seenHeader = false;
    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        if (!line.empty() && line.front() == '>') {
            seenHeader = true;
            if (!builder.str().empty()) {
                sequences.push_back(builder.str());
                builder.str("");
                builder.clear();
            }
            continue;
        }
        if (seenHeader) {
            builder << line;
        } else {
            sequences.push_back(line);
        }
    }
    if (!builder.str().empty()) {
        sequences.push_back(builder.str());
    }
    if (sequences.empty()) {
        throw std::runtime_error("Dataset " + path + " did not contain any sequences.");
    }
    return sequences;
}

}  // namespace automata
