# COMPLETE WORKFLOW: Automata Simulator

---

## Overview

This document provides a mode-by-mode, argument-to-output walkthrough for the Automata Simulator C++ project. For each mode (DFA, NFA, EFA, PDA), it traces every CLI argument, dataset, and sequence through all relevant files and functions, step by step, from start to output.

---

# Mode-by-Mode Argument-to-Output Walkthrough


## DFA Mode

**Command:**
```
./build/bin/automata_sim --mode dfa --pattern "A(CG|TT)*" --input datasets/dna/sample.txt
```

### Step 1: CLI Argument Parsing
- **File:** `cli/main.cpp`
  - `main(int argc, char** argv)`
    - Parses:
      - `--mode dfa` → `PatternSpec::requestedMode = Dfa`
      - `--pattern` → `PatternSpec::pattern`
      - `--input` → `inputPath`

### Step 2: Dataset Loading
- **File:** `src/evaluation/DatasetLoader.cpp`
  - `DatasetLoader::loadSequences(inputPath)`
    - Reads `datasets/dna/sample.txt` (extracts sequences)
    - Populates `PatternSpec::datasets`

### Step 3: Mode Decision
- **File:** `src/modes/ModeDispatcher.cpp`
  - `ModeDispatcher::decide(const PatternSpec&)`
    - Sets `AutomatonPlan::kind = Dfa`

### Step 4: Automaton Construction
- **File:** `src/automata/runners/RunnerFactory.cpp`
  - `RunnerFactory::create(plan, parser, snapshot)`
    - Calls:
      - `NfaBuilder::build(pattern)` in `src/automata/builders/NfaBuilder.cpp`
      - `DfaBuilder::build(nfa)` in `src/automata/builders/DfaBuilder.cpp`
    - Returns `DfaRunner`

### Step 5: Simulation
- **File:** `src/automata/runners/Runners.cpp`
  - `DfaRunner::run(const std::string&)`
    - Runs DFA on each sequence in `PatternSpec::datasets`
    - Collects matches, states visited, etc.

### Step 6: Reporting/Output
- **File:** `src/reporting/Reporting.cpp`
  - `MetricsAggregator`, `Reporter`, `TraceFormatter`
    - Aggregates and formats results
- **File:** `cli/main.cpp`
  - Prints output

---

## NFA Mode

**Command:**
```
./build/bin/automata_sim --mode nfa --pattern "A(CG|TT)*" --input datasets/dna/sample.txt
```

- Steps 1–3: Same as DFA mode, but `requestedMode = Nfa`, `AutomatonPlan::kind = Nfa`
- **Automaton Construction:**
  - `NfaBuilder::build(pattern)` in `src/automata/builders/NfaBuilder.cpp`
  - Returns `NfaRunner` in `src/automata/runners/Runners.cpp`
- **Simulation:**
  - `NfaRunner::run(const std::string&)`
- **Reporting/Output:** Same as DFA

---

## EFA Mode

**Command:**
```
./build/bin/automata_sim --mode efa --pattern ACGT --k 2 --input datasets/dna/sample.txt
```

### Step 1: CLI Argument Parsing
- `--mode efa` → `requestedMode = Efa`
- `--pattern` → `pattern`
- `--k 2` → `mismatchBudget = 2`
- `--input` → `inputPath`

### Step 2: Dataset Loading
- Same as above

### Step 3: Mode Decision
- `AutomatonPlan::kind = Efa`

### Step 4: Automaton Construction
- `EfaBuilder::build(pattern, k)` in `src/automata/builders/EfaBuilder.cpp`
- Returns `EfaRunner` in `src/automata/runners/Runners.cpp`

### Step 5: Simulation
- `EfaRunner::run(const std::string&)`

### Step 6: Reporting/Output
- Same as above

---

## PDA Mode

**Command:**
```
./build/bin/automata_sim --mode pda --dot-bracket --input datasets/rna/sample.txt
```

### Step 1: CLI Argument Parsing
- `--mode pda` → `requestedMode = Pda`
- `--dot-bracket` → `allowDotBracket = true`
- `--input` → `inputPath`

### Step 2: Dataset Loading
- `DatasetLoader::loadSequences(inputPath)`
  - Reads `datasets/rna/sample.txt`

### Step 3: Mode Decision
- `AutomatonPlan::kind = Pda`

### Step 4: Automaton Construction
- `PdaBuilder::build()` in `src/automata/builders/PdaBuilder.cpp`
- Returns `PdaRunner` in `src/automata/runners/Runners.cpp`

### Step 5: Simulation
- `PdaRunner::run(const std::string&)`

### Step 6: Reporting/Output
- Same as above

---

# Directory/File Map for All Modes

- **cli/main.cpp** — argument parsing, orchestration, output
- **src/evaluation/DatasetLoader.cpp** — dataset loading
- **src/modes/ModeDispatcher.cpp** — mode selection
- **src/automata/builders/NfaBuilder.cpp** — NFA construction
- **src/automata/builders/DfaBuilder.cpp** — DFA construction
- **src/automata/builders/EfaBuilder.cpp** — EFA construction
- **src/automata/builders/PdaBuilder.cpp** — PDA construction
- **src/automata/runners/RunnerFactory.cpp** — runner construction
- **src/automata/runners/Runners.cpp** — simulation logic
- **src/reporting/Reporting.cpp** — results aggregation, formatting

---

*Every CLI argument, dataset, and pattern flows through these steps and files, with each mode using its respective builder/runner. Output is always produced by the main.cpp reporting pipeline.*

    - NFA: `NfaBuilder::build(pattern)`
    - EFA: `EfaBuilder::build(pattern, k)`
    - PDA: `PdaBuilder::build()`
  - Returns a runner (`DfaRunner`, `NfaRunner`, etc.)

**Files:**
- `src/automata/builders/NfaBuilder.cpp` — `NfaBuilder::build(const std::string&)`
- `src/automata/builders/DfaBuilder.cpp` — `DfaBuilder::build(const Nfa&)`
- `src/automata/builders/EfaBuilder.cpp` — `EfaBuilder::build(const std::string&, std::size_t)`
- `src/automata/builders/PdaBuilder.cpp` — `PdaBuilder::build()`

---

## 4. Simulation (Pattern Matching/Validation)

**File:** `src/automata/runners/Runners.hpp` / `Runners.cpp`
- **Functions:**
  - `DfaRunner::run(const std::string&)`
  - `NfaRunner::run(const std::string&)`
  - `EfaRunner::run(const std::string&)`
  - `PdaRunner::run(const std::string&)`
- **What happens:**
  - For each sequence in `spec.datasets`, the runner simulates the automaton, collecting matches, states visited, etc.

---

## 5. Reporting and Output

**File:** `src/reporting/Reporting.hpp` / `Reporting.cpp`
- **Functions:**
  - `MetricsAggregator`, `Reporter`, `TraceFormatter`
- **What happens:**
  - Aggregates results from all runs, formats matches, traces, and summary statistics.
  - `main.cpp` prints the output to the user.

---

## 6. Example: DFA Mode Walkthrough

**Command:**
```
./build/bin/automata_sim --mode dfa --pattern "A(CG|TT)*" --input datasets/dna/sample.txt --dump-automaton tmp/dfa.json
```

**Dataset:**
```
>seq1
ACGTACGTACGT
```

**Output (abridged):**
```
Matches: [0,1) [0,3) [4,5) [4,7) [8,9) [8,11)
ACGTACGTACGT
States visited: 23
Runs: 1, Matches: 6, All accepted: no
```

### Match Explanation
- Each `[start,end)` shows the indices of a substring matching the pattern.
- For `ACGTACGTACGT`:
  - `[0,1)` = `A`
  - `[0,3)` = `ACG`
  - `[4,5)` = `A`
  - `[4,7)` = `ACG`
  - `[8,9)` = `A`
  - `[8,11)` = `ACG`

### Step-by-Step File/Function Flow
```
cli/main.cpp (main)
  └─> src/evaluation/DatasetLoader.cpp (DatasetLoader::loadSequences)
  └─> src/modes/ModeDispatcher.cpp (ModeDispatcher::decide)
  └─> src/automata/runners/RunnerFactory.cpp (RunnerFactory::create)
      └─> src/automata/builders/NfaBuilder.cpp (NfaBuilder::build)
      └─> src/automata/builders/DfaBuilder.cpp (DfaBuilder::build)
  └─> src/automata/runners/Runners.cpp (DfaRunner::run)
  └─> src/reporting/Reporting.cpp (Reporter, MetricsAggregator)
  └─> cli/main.cpp (prints output)
```

---

## 7. Mode-by-Mode Summary Table

| Mode | Key Builders & Runner | Used For |
|------|----------------------|----------|
| DFA  | NfaBuilder, DfaBuilder, DfaRunner | Exact regex search |
| NFA  | NfaBuilder, NfaRunner             | Regex search |
| EFA  | EfaBuilder, EfaRunner             | Approximate (Hamming) |
| PDA  | PdaBuilder, PdaRunner             | Dot-bracket validation |

**All modes:**
- Input is parsed and loaded in `main.cpp` (via `DatasetLoader`)
- Automaton is built and wrapped in the correct runner
- Each sequence is processed by the runner’s `run()`
- Results are aggregated and printed

---

## 8. Data Life Cycle
- **PatternSpec**: Grows as CLI is parsed (pattern, datasets, mode, etc.)
- **AutomatonPlan**: Combines mode and spec, passed to factory
- **Automaton**: Built from pattern (NFA, DFA, EFA, PDA)
- **Runner**: Runs simulation per input
- **RunResult**: Holds matches, trace, states visited, etc.
- **Reporter/MetricsAggregator**: Aggregate and format results

---

## 9. Visual Flow Diagram
```
main.cpp
  ↓
DatasetLoader.cpp (loadSequences)
  ↓
ModeDispatcher.cpp (decide)
  ↓
RunnerFactory.cpp (create)
  ↓
[NfaBuilder.cpp | DfaBuilder.cpp | EfaBuilder.cpp | PdaBuilder.cpp]
  ↓
Runners.cpp (run)
  ↓
Reporting.cpp (aggregate, print)
```

---

## 10. Appendix: File/Function Reference

- **cli/main.cpp**: Entry point, argument parsing, overall flow
- **src/evaluation/DatasetLoader.cpp**: Loads input sequences
- **src/modes/ModeDispatcher.cpp**: Decides automaton mode
- **src/automata/builders/**: Builds automata
- **src/automata/runners/**: Runs automata simulation
- **src/reporting/**: Aggregates and prints results

---

*Generated by Cascade AI for project defense and documentation.*
