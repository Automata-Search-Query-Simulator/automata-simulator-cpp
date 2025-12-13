# Automata Simulator Pipeline Notes

This note captures the end-to-end flow from CLI intent through automaton execution and reporting, along with concrete inputs, datasets, and commands that exercise each mode.

## 1. CLI quick-start

- Build the simulator via `make` (CMake config + Release build) to produce `build/bin/automata_sim`.
- Run the default DNA search: `make run` (wraps `./build/bin/automata_sim --pattern "A(CG|TT)*" --input datasets/dna/sample.txt`).
- Override flags: `make run SIM_ARGS='--mode efa --pattern ACGT --k 2 --input datasets/dna/sample.txt'` for approximate matching or `make run SIM_ARGS='--mode pda --dot-bracket --input datasets/rna/sample.txt'` for RNA structure.
- Direct invocation works too: `./build/bin/automata_sim --pattern "A(CG|TT)*" --input datasets/dna/sample.txt --trace`.
- Use `NO_COLOR=1` to disable ANSI coloring when piping or capturing output.

## 2. Argument parsing and input loading

- `cli/main.cpp` parses positional flags into a `PatternSpec` (`include/PatternSpec.hpp`), which stores the regex pattern, dataset path/vector, mismatch budget `--k`, `--trace`, `--dot-bracket`, and `ModePreference` (`Auto`, `Nfa`, `Dfa`, `Efa`, `Pda`, `PdaOnly`).
- `DatasetLoader::loadSequences` (`src/parser/DatasetLoader.cpp`) accepts FASTA-like files: it trims `\r`, ignores blank lines, treats lines starting with `>` as headers, concatenates multi-line sequences per header, and ensures at least one sequence exists (throws otherwise). Example file:
  ```text
  >seq1
  TAGTAACGTCGTAAGTCGG
  ```
- If no `--input` is provided, `EvaluationHarness` (`src/evaluation/EvaluationHarness.cpp`) supplies hard-coded smoke sequences: DNA → `["ACGTACGT", "TTTTACGT", "GGGGCCCC"]`, RNA → `["((..))", "(()())", "(.)"]`. The CLI toggles RNA mode when `spec.allowDotBracket` is set via `--dot-bracket`.

## 3. Mode selection heuristics

- `ModeDispatcher::decide` (`src/modes/ModeDispatcher.cpp`) applies simple rules:
  - Explicit `--mode` values override everything (`nfa`, `dfa`, `efa`, `pda`).
  - `--dot-bracket` or `ModePreference::PdaOnly` forces PDA mode (RNA stack validation).
  - `--k` (mismatch budget) greater than zero selects EFA (`AutomatonKind::Efa`).
  - Otherwise default to the NFA pipeline.
- `PatternSpec.requestedMode` stores the parsed `ModePreference`, while `PatternSpec.allowDotBracket` controls PDA fallback for evaluation harness datasets.
- `AutomatonPlan` (`include/AutomatonPlan.hpp`) pairs the resolved `AutomatonKind` with the populated `PatternSpec` and is fed into `RunnerFactory`.

## 4. Pattern parsing and automaton construction

### Regex parsing

- `RegexParser::parseToPostfix` (`src/parser/RegexParser.cpp`) tokenizes the pattern using a Shunting Yard approach. Supported constructs: literal characters (escaped via `\`), `.`, character classes (`[A-Z]`), `|`, `*`, `+`, `?`, implicit concatenation, and parentheses. The parser also inserts explicit `Concat` tokens where needed.

### NFA builder

- `NfaBuilder::build` consumes the postfix tokens, applies Thompson construction, and handles fragments for concatenation, union, Kleene star/plus/question, and literal/character class/any transitions. It returns an `Nfa` with `states`, a `start`, and an `accept` index, marking the accept state with `NfaState::accept = true`.
- Empty patterns produce a two-state NFA with a single epsilon transition (useful for testing `""`).

### DFA conversion

- `DfaBuilder::build` performs subset construction (`src/automata/builders/DfaBuilder.cpp`). It tracks unique sets of NFA states via string keys, computes epsilon closures (via `epsilonClosure` in `src/automata/utils/StateSet.cpp`), and builds deterministic transition tables (256 entries per `DfaState`). Accepting DFA states are those containing any accepting NFA state.

### EFA assembly

- `EfaBuilder` wraps the NFA plus a mismatch budget `k` in an `Efa` structure (`src/automata/builders/EfaBuilder.cpp`). The same regex parser/NFA builder is reused, so any regex supported by the NFA path is permitted.

### PDA setup

- `PdaBuilder` returns a minimal pushdown automaton for dot-bracket notation: rules expect `(` to push, `)` to pop, and `.` to ignore (`src/automata/builders/PdaBuilder.cpp`). `DotBracketValidator` (`src/parser/DotBracketValidator.cpp`) quickly rejects invalid characters or unbalanced parentheses before the PDA runs.

## 5. Mode-specific execution

| Mode    | Pattern intent                                            | Builder → Runner                          | Key behavior                                                                                                                                                                                                                               |
| ------- | --------------------------------------------------------- | ----------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | ---------------------------------------------------------------------------------------------------------------- |
| **NFA** | Exact regex matching (`"A(CG                              | TT)\*"`)                                  | `NfaBuilder` → `NfaRunner`                                                                                                                                                                                                                 | Maintains epsilon-closure state sets; simulates every prefix; records `[start,end)` matches and `statesVisited`. |
| **DFA** | Deterministic exact match (`--mode dfa`)                  | `NfaBuilder` + `DfaBuilder` → `DfaRunner` | Uses deterministic transition table; tracks only one active state per symbol; still records matches per prefix but with fewer states visited.                                                                                              |
| **EFA** | Approximate matching with mismatches (`--mode efa --k 1`) | `NfaBuilder` + `EfaBuilder` → `EfaRunner` | Tracks mismatch costs per NFA state (`CostVector`); consumes mismatches until budget exhausted; uses `epsilonClosureWithCosts`, `activeCount`, `minMismatch` helpers; records matches when any accepting state is reachable within budget. |
| **PDA** | RNA dot-bracket validation (`--dot-bracket`)              | `PdaBuilder` → `PdaRunner`                | Stack machine increments on `(`, decrements on `)`, ignores `.`; rejects on unexpected characters or premature pops; tracks `stackDepth` and accepts only when stack empties at end.                                                       |

- All runners inherit `IRunner` (`include/IRunner.hpp`) and return a `RunResult` containing `accepted`, `matches`, `trace`, `statesVisited`, and `stackDepth`. Tracing information comprises `TraceEvent` objects (`include/TraceEvent.hpp`).
- `RunnerFactory::create` (`src/automata/runners/RunnerFactory.cpp`) centralizes builder selection based on `AutomatonPlan`. It also fills an optional `RunnerFactory::Snapshot` (variant of `Nfa`, `Dfa`, `Efa`, `Pda`) so `cli/AutomatonSerializer` can emit JSON for `--dump-automaton`.

## 6. Reporting, tracing, and coloring

- Each sequence run writes to `MetricsAggregator` (`src/reporting/MetricsAggregator.cpp`), which counts runs, matches, and accepted sequences. `Reporter::summary` (`src/reporting/Reporter.cpp`) prints `Runs`, `Matches`, and `All accepted: yes/no`.
- `TraceFormatter` (`src/reporting/TraceFormatter.cpp`) renders the `RunResult.trace` vector when `--trace` is enabled, printing each `TraceEvent` index/detail.
- `colorize`, `resetColor`, and `highlightMatches` help produce a colored CLI experience; `NO_COLOR` disables colors, and `isatty` detects terminals.
- Matches are highlighted within sequences using a rotating palette when colors are enabled.

## 7. Observability: `--trace` and `--dump-automaton`

- `--trace` turns on per-symbol logging in each runner, which emits `TraceEvent` entries describing `start`, `pos`, `state`, and (for EFAs) the best mismatch count. `TraceFormatter` turns those entries into multi-line text under the matches list.
- `--dump-automaton PATH` requests a JSON serialization of the constructed automaton via `RunnerFactory::Snapshot`. Pass `-` to print to stdout. The serializer (`cli/AutomatonSerializer.cpp`) emits format-specific objects for NFA, DFA, EFA (including wrapped NFA and mismatch budget), or PDA (states for stack depths and rules).
- Temporary dumps (e.g., for tests) can be written to `tmp/` to avoid polluting version control.

## 8. Example flows with patterns and data

### Exact DNA matching (NFA)

Command:

```bash
./build/bin/automata_sim --pattern "A(CG|TT)*" --input datasets/dna/sample.txt
```

Dataset: `datasets/dna/sample.txt`

```text
>seq1
TAGTAACGTCGTAAGTCGG
```

The NFA path parses the pattern, builds a Thompson NFA, and slides over the sequence to report match intervals such as `[2,6)` for `ACGT`. Metrics show how many states were visited while `Reporter` prints the per-sequence summary and colored match highlights.

### Deterministic DNA matching (DFA)

Command:

```bash
./build/bin/automata_sim --mode dfa --pattern "A(CG|TT)*" --input datasets/dna/sample.txt
```

The DFA path converts the same NFA into a deterministic table. Runtime instrumentation is similar, but `statesVisited` counts transitions instead of whole state sets, making repeated runs faster at the cost of larger transition tables.

### Approximate matching (EFA)

Command:

```bash
./build/bin/automata_sim --mode efa --pattern "ACGT" --k 1 --input datasets/dna/sample.txt
```

The EFA runner allows one mismatch, so `"ACCT"` would still register as a match, whereas the DFA path would reject it. Traces report `bestMismatch` values per step thanks to `minMismatch`.

### RNA dot-bracket validation (PDA)

Command:

```bash
./build/bin/automata_sim --mode pda --dot-bracket --input datasets/rna/sample.txt
```

Dataset:

```text
(()())
((..))
```

The PDA ignores `.` and checks nested parentheses; both sequences in `datasets/rna/sample.txt` are accepted. A trace records stack depth per position, and the summary reports `stackDepth` in `RunResult`.

## 9. Dataset/test/support matrix

- `datasets/dna/sample.txt` and `datasets/rna/sample.txt` feed the CLI examples above; `datasets/controls` stores future stress cases (TODO in README).
- Unit tests cover parsers/builders/runners:
  - `tests/unit/ParserTests.cpp` writes `tmp/test_dataset.txt` to validate `DatasetLoader` and `DotBracketValidator`.
  - `tests/unit/BuilderTests.cpp` ensures `Nfa`, `Dfa`, `Efa`, and `Pda` constructors populate expected fields.
  - `tests/unit/RunnerTests.cpp` exercises each runner with representative input strings.
- Integration tests (`tests/integration/EndToEndTests.cpp`) reproduce CLI flows (NFA, EFA with regex, PDA) by combining `PatternSpec`, `ModeDispatcher`, `RunnerFactory`, and `RegexParser`.
- `test_main.cpp` simply calls `runParserTests`, `runBuilderTests`, `runRunnerTests`, and `runIntegrationTests`.
- `scripts/run_perf.sh` executes NFA/EFA/PDA commands with timing for performance sweeps; run this after algorithmic changes.
- `scripts/seed_datasets.py` (TODO stub) is intended for regenerating anonymized samples if specs evolve.

## 10. Next references

- For low-level implementation details (per-file responsibilities, additional heuristics, etc.), refer to `docs/WORKFLOW/workflow.md` and `docs/COMPLETE_WORKFLOW.md`.
- For reproducibility notes, consult `config/reproducibility.md` and update `config/defaults.yaml`/`cli/config.yaml` when defaults change.
