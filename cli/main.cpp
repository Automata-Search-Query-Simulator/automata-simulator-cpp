#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "AutomatonSerializer.hpp"
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
