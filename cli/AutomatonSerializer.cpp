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
    out << "{\"kind\":\"EFA\",\"pattern\":\"" << jsonEscape(efa.pattern) << "\",\"mismatchBudget\":"
        << efa.mismatchBudget << ",\"nfa\":" << serializeNfa(efa.automaton) << "}";
    return out.str();
}

std::string serializePda(const Pda& pda) {
    std::ostringstream out;
    out << "{\"kind\":\"PDA\",\"start\":" << pda.start << ",\"states\":[";

    for (std::size_t i = 0; i < pda.states.size(); ++i) {
        const auto& state = pda.states[i];
        out << "{\"id\":" << state.id << ",\"accept\":" << (state.accept ? "true" : "false")
            << ",\"stackDepth\":" << state.stackDepth << ",\"transitions\":[";

        for (std::size_t j = 0; j < state.transitions.size(); ++j) {
            const auto& trans = state.transitions[j];
            std::string opStr;
            switch (trans.operation) {
                case PdaOperation::Push:
                    opStr = "push";
                    break;
                case PdaOperation::Pop:
                    opStr = "pop";
                    break;
                case PdaOperation::Ignore:
                    opStr = "ignore";
                    break;
            }
            out << "{\"code\":" << trans.code << ",\"symbol\":\"" << charToJson(trans.symbol)
                << "\",\"to\":" << trans.to << ",\"operation\":\"" << opStr << "\"}";
            if (j + 1 < state.transitions.size()) {
                out << ",";
            }
        }
        out << "]}";
        if (i + 1 < pda.states.size()) {
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

