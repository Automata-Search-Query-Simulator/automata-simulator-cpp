#include "AutomatonSerializer.hpp"

#include <cstdio>
#include <set>
#include <sstream>
#include <string>

namespace automata {
namespace {

std::string jsonEscape(const std::string& value) {
    std::string escaped;
    escaped.reserve(value.size() + 8);
    for (unsigned char c : value) {
        switch (c) {
            case '\"':
                escaped.append("\\\"");
                break;
            case '\\':
                escaped.append("\\\\");
                break;
            case '\b':
                escaped.append("\\b");
                break;
            case '\f':
                escaped.append("\\f");
                break;
            case '\n':
                escaped.append("\\n");
                break;
            case '\r':
                escaped.append("\\r");
                break;
            case '\t':
                escaped.append("\\t");
                break;
            default:
                if (c < 0x20 || c > 0x7E) {
                    char buffer[7];
                    std::snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                    escaped.append(buffer);
                } else {
                    escaped.push_back(static_cast<char>(c));
                }
        }
    }
    return escaped;
}

std::string charToJson(char c) {
    return jsonEscape(std::string(1, c));
}

std::string edgeTypeToString(EdgeType type) {
    switch (type) {
        case EdgeType::Epsilon:
            return "epsilon";
        case EdgeType::Literal:
            return "literal";
        case EdgeType::Any:
            return "any";
        case EdgeType::CharClass:
            return "char_class";
    }
    return "unknown";
}

std::string serializeNfa(const Nfa& nfa) {
    std::ostringstream out;
    out << "{\"kind\":\"NFA\",\"start\":" << nfa.start << ",\"accept\":" << nfa.accept << ",\"states\":[";
    for (std::size_t i = 0; i < nfa.states.size(); ++i) {
        const auto& state = nfa.states[i];
        out << "{\"id\":" << i << ",\"accept\":" << (state.accept ? "true" : "false") << ",\"edges\":[";
        for (std::size_t j = 0; j < state.edges.size(); ++j) {
            const auto& edge = state.edges[j];
            out << "{\"to\":" << edge.to << ",\"type\":\"" << edgeTypeToString(edge.type) << "\"";
            if (edge.type == EdgeType::Literal) {
                out << ",\"literal\":\"" << charToJson(edge.literal) << "\"";
            } else if (edge.type == EdgeType::CharClass) {
                out << ",\"charClass\":\"" << jsonEscape(edge.charClass) << "\"";
            }
            out << "}";
            if (j + 1 < state.edges.size()) {
                out << ",";
            }
        }
        out << "]}";
        if (i + 1 < nfa.states.size()) {
            out << ",";
        }
    }
    out << "]}";
    return out.str();
}

std::string serializeDfa(const Dfa& dfa) {
    std::ostringstream out;
    out << "{\"kind\":\"DFA\",\"start\":" << dfa.start << ",\"states\":[";
    for (std::size_t i = 0; i < dfa.states.size(); ++i) {
        const auto& state = dfa.states[i];
        out << "{\"id\":" << i << ",\"accept\":" << (state.accept ? "true" : "false") << ",\"transitions\":[";
        bool first = true;
        for (int c = 0; c < 256; ++c) {
            int target = state.next[c];
            if (target < 0) {
                continue;
            }
            if (!first) {
                out << ",";
            }
            first = false;
            out << "{\"code\":" << c << ",\"symbol\":\"" << charToJson(static_cast<char>(c)) << "\",\"to\":" << target
                << "}";
        }
        out << "]}";
        if (i + 1 < dfa.states.size()) {
            out << ",";
        }
    }
    out << "]}";
    return out.str();
}

std::string serializeEfa(const Efa& efa) {
    std::ostringstream out;
    out << "{\"kind\":\"EFA\",\"pattern\":\"" << jsonEscape(efa.literalPattern) << "\",\"mismatchBudget\":"
        << efa.mismatchBudget << ",\"start\":0,\"states\":[";

    const std::size_t patternLength = efa.literalPattern.size();
    const std::size_t maxStates = (patternLength + 1) * (efa.mismatchBudget + 1);

    for (std::size_t stateId = 0; stateId < maxStates; ++stateId) {
        const std::size_t position = stateId / (efa.mismatchBudget + 1);
        const std::size_t mismatches = stateId % (efa.mismatchBudget + 1);
        const bool isAccept = (position == patternLength && mismatches <= efa.mismatchBudget);

        out << "{\"id\":" << stateId << ",\"accept\":" << (isAccept ? "true" : "false")
            << ",\"position\":" << position << ",\"mismatches\":" << mismatches << ",\"transitions\":[";

        if (position < patternLength) {
            bool first = true;
            const char expectedChar = efa.literalPattern[position];
            const std::size_t matchStateId = (position + 1) * (efa.mismatchBudget + 1) + mismatches;
            out << "{\"code\":" << static_cast<int>(static_cast<unsigned char>(expectedChar))
                << ",\"symbol\":\"" << charToJson(expectedChar) << "\",\"to\":" << matchStateId
                << ",\"type\":\"match\"}";
            first = false;

            if (mismatches < efa.mismatchBudget) {
                const std::size_t mismatchStateId = (position + 1) * (efa.mismatchBudget + 1) + (mismatches + 1);
                std::set<char> alphabet;
                for (char c : efa.literalPattern) {
                    alphabet.insert(c);
                }
                for (char c : {'A', 'C', 'G', 'T', 'U'}) {
                    alphabet.insert(c);
                }

                for (char c : alphabet) {
                    if (c != expectedChar) {
                        if (!first) {
                            out << ",";
                        }
                        first = false;
                        out << "{\"code\":" << static_cast<int>(static_cast<unsigned char>(c))
                            << ",\"symbol\":\"" << charToJson(c) << "\",\"to\":" << mismatchStateId
                            << ",\"type\":\"mismatch\"}";
                    }
                }
                if (!first) {
                    out << ",";
                }
                out << "{\"code\":-1,\"symbol\":\"*\",\"to\":" << mismatchStateId
                    << ",\"type\":\"mismatch\",\"wildcard\":true}";
            }
        }

        out << "]}";
        if (stateId + 1 < maxStates) {
            out << ",";
        }
    }

    out << "]}";
    return out.str();
}

std::string serializePda(const Pda& pda) {
    std::ostringstream out;
    out << "{\"kind\":\"PDA\",\"start\":0,\"states\":[";

    const std::size_t maxDepth = 10;
    for (std::size_t depth = 0; depth <= maxDepth; ++depth) {
        const bool isAccept = (depth == 0);
        out << "{\"id\":" << depth << ",\"accept\":" << (isAccept ? "true" : "false")
            << ",\"stackDepth\":" << depth << ",\"transitions\":[";

        bool first = true;
        if (depth < maxDepth) {
            out << "{\"code\":" << static_cast<int>('(') << ",\"symbol\":\"(\",\"to\":" << (depth + 1)
                << ",\"operation\":\"push\"}";
            first = false;
        }
        if (depth > 0) {
            if (!first) {
                out << ",";
            }
            out << "{\"code\":" << static_cast<int>(')') << ",\"symbol\":\")\",\"to\":" << (depth - 1)
                << ",\"operation\":\"pop\"}";
            first = false;
        }
        if (!first) {
            out << ",";
        }
        out << "{\"code\":" << static_cast<int>('.') << ",\"symbol\":\".\",\"to\":" << depth
            << ",\"operation\":\"ignore\"}";

        out << "]}";
        if (depth < maxDepth) {
            out << ",";
        }
    }

    out << "],\"rules\":[";
    for (std::size_t i = 0; i < pda.rules.size(); ++i) {
        out << "{\"expected\":\"" << charToJson(pda.rules[i].expected) << "\"}";
        if (i + 1 < pda.rules.size()) {
            out << ",";
        }
    }
    out << "]}";
    return out.str();
}

}  // namespace

std::string serializeSnapshot(const RunnerFactory::Snapshot& snapshot) {
    switch (snapshot.kind) {
        case AutomatonKind::Nfa:
            return serializeNfa(std::get<Nfa>(snapshot.automaton));
        case AutomatonKind::Dfa:
            return serializeDfa(std::get<Dfa>(snapshot.automaton));
        case AutomatonKind::Efa:
            return serializeEfa(std::get<Efa>(snapshot.automaton));
        case AutomatonKind::Pda:
            return serializePda(std::get<Pda>(snapshot.automaton));
    }
    return "{}";
}

}  // namespace automata

