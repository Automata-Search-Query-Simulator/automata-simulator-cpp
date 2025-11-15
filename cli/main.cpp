#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
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
        if (start > cursor) {
            result.append(sequence.substr(cursor, start - cursor));
        }
        if (end > start) {
            const char* color = palette[paletteIndex % palette.size()];
            ++paletteIndex;
            result.append(color);
            result.append(sequence.substr(start, end - start));
            result.append(resetColor(true));
        }
        cursor = end;
    }
    if (cursor < sequence.size()) {
        result.append(sequence.substr(cursor));
    }
    return result;
}

}  // namespace

int main(int argc, char** argv) {
    PatternSpec spec;
    std::string inputPath;
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
        auto runner = factory.create(plan, parser);
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
