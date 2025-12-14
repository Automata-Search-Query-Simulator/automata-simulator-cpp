# Repository Guidelines

## Project Structure & Module Organization
Source lives in `src/`, split into focused modules: `automata/` implements the DFA/EFA/PDA cores, `parser/` handles regex, datasets, and dot-bracket logic, `modes/` chooses strategies, and `reporting/` summarizes runs. Shared headers are in `include/`, while `cli/` hosts `main.cpp` plus default CLI YAML, and produces `build/bin/automata_sim`. Tests mirror production code (`tests/unit`, `tests/integration`), with fixtures under `tests/data` and scratch files in `tmp/`. Reference inputs stay in `datasets/`; knobs and reproducibility notes are in `config/`, and helper automation lives in `scripts/`.

## Build, Test, and Development Commands
- `make` — CMake configure + compile, yielding `build/bin/automata_sim`.
- `make test` — builds everything and runs `build/bin/automata_tests` plus integration smoke tests.
- `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build` — use when you need non-default flags or generators.
- `./build/bin/automata_sim --pattern "A(CG|TT)*" --input datasets/dna/sample.fasta` — quick CLI verification; append `--trace` for debug output.
- `scripts/run_perf.sh` — reproducible latency sweep for NFA/EFA/PDA; run after major algorithmic changes.
- `python3 scripts/seed_datasets.py` — refresh bundled sample datasets when specs evolve.

## Coding Style & Naming Conventions
Target C++20, use 4-space indentation, and keep braces on the same line as declarations (`int main() {`). Types use PascalCase (`RegexParser`), functions/methods camelCase (`needsConcat`), constants SCREAMING_SNAKE_CASE, and private members gain a trailing underscore (`reports_`). Keep code inside `namespace automata`, favor standard containers/algorithms over custom loops, and isolate CLI-only code inside `cli/`. Run `clang-format` with LLVM defaults (or match existing files manually) before opening a PR.

## Testing Guidelines
Each test translation unit should expose a `run*Tests()` function invoked from `tests/test_main.cpp`; mirror the naming pattern (`runParserTests`, `runIntegrationTests`). Place new fixtures in `tests/data` and write integration tests that shell out to the built binary for real I/O paths. Temporary generated files should land in `tmp/` to avoid polluting version control. Every feature or bug fix must add at least one focused unit assertion plus, when user-facing behavior changes, an integration scenario exercising the CLI flags involved. Always run `make test` (and document the command) before submitting code.

## Commit & Pull Request Guidelines
Commit subjects should be short, imperative sentences (e.g., `Add PDA dot-bracket checks`) with optional bodies describing rationale and risks. Keep related source, header, and test files in the same commit, and avoid mixing refactors with feature work unless tightly coupled. Pull requests need: a concise summary, reproduction commands (`make`, CLI examples), linked issues if applicable, and screenshots or trace snippets when output changes. Highlight edits to `datasets/` or `config/` so reviewers can double-check generated artifacts.

## Security & Configuration Tips
Only keep anonymized sample sequences in the repo; regenerate new ones with `scripts/seed_datasets.py` instead of committing sensitive data. Store tunables in `config/defaults.yaml` and document overrides in `config/reproducibility.md` rather than hard-coding them. Before sharing logs or traces (e.g., in `docs/`), scrub raw input strings so downstream teams can safely reuse the artifacts.
