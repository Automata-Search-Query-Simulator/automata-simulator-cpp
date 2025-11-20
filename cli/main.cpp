#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>
#if defined(_WIN32)
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

#include "AutomatonPlan.hpp"
#include "PatternSpec.hpp"
#include "automata/runners/Runners.hpp"
#include "evaluation/EvaluationHarness.hpp"
#include "modes/ModeDispatcher.hpp"
#include "parser/Parsers.hpp"
#include "reporting/Reporting.hpp"

using namespace automata;

namespace {

ModePreference parseMode(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
    if (value == "nfa") {
        return ModePreference::Nfa;
    }
    if (value == "dfa") {
        return ModePreference::Dfa;
    }
    if (value == "efa") {
        return ModePreference::Efa;
    }
    if (value == "pda") {
        return ModePreference::Pda;
    }
    return ModePreference::Auto;
}

std::string kindToString(AutomatonKind kind) {
    switch (kind) {
        case AutomatonKind::Nfa:
            return "NFA";
        case AutomatonKind::Dfa:
            return "DFA";
        case AutomatonKind::Efa:
            return "EFA";
        case AutomatonKind::Pda:
            return "PDA";
    }
    return "Unknown";
}

const char* resetColor(bool enabled) { return enabled ? "\033[0m" : ""; }

std::string colorize(const std::string& text, const char* color, bool enabled) {
    if (!enabled) {
        return text;
    }
    return std::string(color) + text + resetColor(true);
}

bool colorOutputEnabled() {
    static bool enabled = [] {
        if (std::getenv("NO_COLOR")) {
            return false;
        }
        return isatty(fileno(stdout)) != 0;
    }();
    return enabled;
}

std::string highlightMatches(const std::string& sequence,
                             std::vector<std::pair<std::size_t, std::size_t>> matches,
                             bool colorizeOutput) {
    if (!colorizeOutput || matches.empty()) {
        return sequence;
    }
    std::sort(matches.begin(), matches.end());
    static constexpr std::array<const char*, 5> palette = {"\033[38;5;214m", "\033[38;5;45m", "\033[38;5;118m",
                                                           "\033[38;5;207m", "\033[38;5;33m"};
    std::string result;
    result.reserve(sequence.size() + matches.size() * 10);
    std::size_t cursor = 0;
    std::size_t paletteIndex = 0;
    for (const auto& match : matches) {
        std::size_t start = std::min(match.first, sequence.size());
        std::size_t end = std::min(match.second, sequence.size());
        if (end <= start || end <= cursor) {
            continue;
        }
        if (start > cursor) {
            result.append(sequence.substr(cursor, start - cursor));
        } else if (start < cursor) {
            start = cursor;
        }
        const char* color = palette[paletteIndex % palette.size()];
        ++paletteIndex;
        result.append(color);
        result.append(sequence.substr(start, end - start));
        result.append(resetColor(true));
        cursor = end;
    }
    if (cursor < sequence.size()) {
        result.append(sequence.substr(cursor));
    }
    return result;
}

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
        << efa.mismatchBudget << "}";
    return out.str();
}

std::string serializePda(const Pda& pda) {
    std::ostringstream out;
    out << "{\"kind\":\"PDA\",\"rules\":[";
    for (std::size_t i = 0; i < pda.rules.size(); ++i) {
        out << "{\"expected\":\"" << charToJson(pda.rules[i].expected) << "\"}";
        if (i + 1 < pda.rules.size()) {
            out << ",";
        }
    }
    out << "]}";
    return out.str();
}

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

}  // namespace

int main(int argc, char** argv) {
    PatternSpec spec;
    std::string inputPath;
    std::string dumpAutomatonPath;
    const bool colorEnabled = colorOutputEnabled();

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--pattern" && i + 1 < argc) {
            spec.pattern = argv[++i];
        } else if (arg == "--input" && i + 1 < argc) {
            inputPath = argv[++i];
        } else if (arg == "--k" && i + 1 < argc) {
            spec.mismatchBudget = std::stoul(argv[++i]);
        } else if (arg == "--trace") {
            spec.trace = true;
        } else if (arg == "--mode" && i + 1 < argc) {
            spec.requestedMode = parseMode(argv[++i]);
        } else if (arg == "--dot-bracket") {
            spec.allowDotBracket = true;
        } else if (arg == "--dump-automaton" && i + 1 < argc) {
            dumpAutomatonPath = argv[++i];
        } else {
            std::cerr << "Unknown or incomplete argument: " << arg << "\n";
            return EXIT_FAILURE;
        }
    }

    try {
        DatasetLoader loader;
        if (!inputPath.empty()) {
            spec.datasetPath = inputPath;
            spec.datasets = loader.loadSequences(inputPath);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Failed to load dataset: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    if (spec.datasets.empty()) {
        EvaluationHarness harness;
        spec.datasets = spec.allowDotBracket ? harness.rnaSmokeSet() : harness.dnaSmokeSet();
    }

    try {
        ModeDispatcher dispatcher;
        auto plan = dispatcher.decide(spec);
        const char* cyan = "\033[96m";
        const char* magenta = "\033[95m";
        const char* bold = "\033[1m";
        const auto bannerColor = colorEnabled ? cyan : "";
        const auto reset = resetColor(colorEnabled);
        std::cout << bannerColor << "\n╔══════════ Automata Simulator ══════════╗\n" << reset;
        std::cout << (colorEnabled ? bold : "") << "Pattern: " << reset << spec.pattern << "\n";
        std::cout << (colorEnabled ? bold : "") << "Datasets: " << reset << spec.datasets.size() << " sequence(s)\n";
        std::cout << (colorEnabled ? bold : "") << "Automaton Mode: " << reset
                  << colorize(kindToString(plan.kind), magenta, colorEnabled) << "\n\n";

        RegexParser parser;
        RunnerFactory factory;
        RunnerFactory::Snapshot snapshot;
        RunnerFactory::Snapshot* snapshotPtr = dumpAutomatonPath.empty() ? nullptr : &snapshot;
        auto runner = factory.create(plan, parser, snapshotPtr);

        if (!dumpAutomatonPath.empty()) {
            auto json = serializeSnapshot(snapshot);
            if (dumpAutomatonPath == "-") {
                std::cout << json << "\n";
            } else {
                std::ofstream out(dumpAutomatonPath);
                if (!out) {
                    std::cerr << "Failed to open automaton dump path: " << dumpAutomatonPath << "\n";
                    return EXIT_FAILURE;
                }
                out << json;
            }
        }
        Reporter reporter;
        MetricsAggregator metrics;
        TraceFormatter formatter;

        for (std::size_t index = 0; index < plan.spec.datasets.size(); ++index) {
            const auto& sequence = plan.spec.datasets[index];
            const auto seqLabel = "Sequence #" + std::to_string(index + 1);
            std::cout << colorize(seqLabel, "\033[93m", colorEnabled) << " (len=" << sequence.size() << ")\n";
            auto result = runner->run(sequence);
            metrics.record(result);
            reporter.add({sequence.substr(0, std::min<std::size_t>(sequence.size(), 40)), result});
            if (spec.trace) {
                std::cout << formatter.format(result);
            }
            if (!result.matches.empty()) {
                std::cout << colorize("  Matches: ", "\033[32m", colorEnabled);
                for (const auto& match : result.matches) {
                    std::cout << "[" << match.first << "," << match.second << ") ";
                }
                std::cout << "\n";
                std::cout << "  " << highlightMatches(sequence, result.matches, colorEnabled) << "\n";
            } else {
                std::cout << colorize("  No matches found.", "\033[31m", colorEnabled) << "\n";
            }
            std::cout << "  States visited: " << result.statesVisited;
            if (plan.kind == AutomatonKind::Pda) {
                std::cout << " | Max stack depth: " << result.stackDepth;
            }
            std::cout << "\n\n";
        }

        const bool accepted = metrics.allAccepted();
        const char* summaryColor = accepted ? "\033[32m" : "\033[31m";
        std::cout << colorize(reporter.summary(metrics), summaryColor, colorEnabled) << "\n";
    } catch (const std::exception& ex) {
        std::cerr << "Simulation error: " << ex.what() << "\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
