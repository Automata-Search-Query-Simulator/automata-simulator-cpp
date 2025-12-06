# Automata Simulator

Automata-based pattern validation/search playground built for the Automata & Formal Languages case study. The CLI automatically chooses among:

- **NFA/DFA** for exact regex search over DNA-like alphabets (Thompson + subset construction)
- **EFA** layers for Hamming-distance approximate matching
- **PDA** stack machine for dot-bracket (RNA) structural validation

Tracing, metrics, and reporting are unified so Mode 1–4 behavior is observable from a single entry point.

## Building & Testing

```bash
make            # build release binary at build/bin/automata_sim
make test       # build + run unit/integration tests (build/bin/automata_tests)
make clean      # remove build artifacts
```

## Running via Make

Set `SIM_ARGS` to control CLI flags; defaults target `datasets/dna/sample.txt`.

```bash
make run                                               # uses default args (ACGT vs sample DNA file)
make run SIM_ARGS='--pattern "A(CG|TT)*" --trace'      # override args (auto-mode search with traces)
make run SIM_ARGS='--mode efa --pattern ACGT --k 2 \
                   --input datasets/dna/sample.txt'    # approximate matching (regex supported)
make run SIM_ARGS='--mode pda --dot-bracket \
                   --input datasets/rna/sample.txt'    # dot-bracket validation
```

`NO_COLOR=1 make run …` disables ANSI colors when needed.

## Manual CLI (optional)

```bash
./build/bin/automata_sim --pattern "A(CG|TT)*" --input datasets/dna/sample.txt
./build/bin/automata_sim --pattern ACGT --k 1 --mode efa --input datasets/dna/sample.txt
./build/bin/automata_sim --mode pda --dot-bracket --input datasets/rna/sample.txt
```

Use `--trace` for step-by-step execution logs. Without `--input`, smoke-test sequences from `EvaluationHarness` are used.

## Repository Layout

```
.
├── cli/                    # CLI entry + config (argument parsing, logging, mode wiring)
├── include/                # Public headers (PatternSpec, runners, builders, reporting)
├── src/
│   ├── parser/             # Regex parser, dataset loader, dot-bracket validator
│   ├── modes/              # Dispatcher + heuristics for automaton selection
│   ├── automata/           # Builders, runners, and shared utilities (NFA/DFA/EFA/PDA)
│   ├── reporting/          # Metrics, trace formatting, user-facing summaries
│   └── evaluation/         # Smoke datasets + performance harness scaffolding
├── tests/                  # Unit + integration suites (invoked via make test)
│   ├── unit/
│   ├── integration/
│   └── test_main.cpp
├── datasets/               # Sample DNA/RNA/control corpora for reproducible runs
├── docs/                   # Paper drafts, figures, notes
├── scripts/                # Utility scripts (performance runner, dataset seeding)
├── config/                 # Defaults, reproducibility notes
├── build/                  # Generated build artifacts (ignored)
└── tmp/                    # Scratch space for transient logs
```

Refer to `paper_context.md` for the full theoretical motivation, evaluation plan, and paper outline tied to this implementation.
