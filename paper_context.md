# Project Summary — Automata-Based Pattern Search Simulator (C++)

## Context

- Course: **Automata & Formal Languages**; finals project + paper/case study.
- Requirement: **One main user task** (search/validate a sequence given a pattern); system auto-chooses the appropriate automaton.
- Scope bridges **text/bioinformatics** and **formal language classes**: Regular (NFA/DFA), extended finite automata for approximate matching, and PDA for nested structure validation.

---

## Core Idea (Single Task, Multiple Models)

**User story:** “Given a pattern and a sequence/dataset, find matches or validate structure and show the automaton behavior.”

**Automaton choice (internal):**

- If regex exact → **NFA**, optionally determinize to **DFA** for speed.
- If regex with ≤ _k_ mismatches (Hamming) → **Extended FA (EFA)** layered by mismatch count.
- If nested structure (RNA dot–bracket / XML-like) → **PDA** for balanced pairing.

---

## NEW VERSION — 4 Modes (for documentation; UI still one task)

| Mode                 | Automaton              | Purpose                                      | Language           | Domain |
| -------------------- | ---------------------- | -------------------------------------------- | ------------------ | ------ |
| 1. DNA Exact (Regex) | NFA (Thompson)         | Build from regex; exact motif search         | Regular            | DNA    |
| 2. DNA Deterministic | DFA (subset of Mode 1) | Faster exact matching on large inputs        | Regular            | DNA    |
| 3. DNA Approximate   | EFA (≤k mismatches)    | Handle Hamming mismatches                    | Regular (extended) | DNA    |
| 4. RNA Structural    | PDA                    | Validate balanced base-pairing (dot–bracket) | Context-free       | RNA    |

> UI remains: **one Search/Validate action**; mode decides under the hood.

---

## Datasets (minimal & reproducible)

- **DNA**: FASTA-like strings over Σ = {A,C,G,T}; synthetic + small public snippets.
- **RNA**: Dot–bracket strings `(` `)` `.` with curated valid/invalid cases.
- **Controls**: Edge/negative cases (empty, homopolymers “AAAA…”, alternating patterns, deep nesting).

---

## Pattern Specs

- **Regex subset**: union `|`, concatenation, `* + ?`, character classes `[ACGT]` (no backrefs).
- **Approximate search**: integer **k ≥ 0** (Hamming; indel-free baseline).
- **RNA validation**: dot–bracket balance; optional “search motif **inside** valid spans.”

---

## Construction & Execution Pipeline

1. Parse pattern/constraints.
2. **Select automaton** (NFA / DFA / EFA / PDA).
3. Build (Thompson; subset/minimize; layered EFA; PDA push/pop).
4. Run on sequences (streamed).
5. Report: match intervals, counts, pass/fail (PDA), and automaton stats (#states, #transitions, stack depth).
6. Optional traces: NFA/EFA active-state sets, DFA state path, PDA stack snapshots.

---

## Evaluation Metrics (compact narrative)

**Correctness.** Embed known motifs at fixed positions and verify exact (Modes 1/2) and ≤k-mismatch hits (Mode 3) by start–end indices and match counts. For RNA, test paired valid/malformed dot–bracket strings and check PDA accept/reject (Mode 4). Cross-validate: **k=0** in Mode 3 must reproduce Mode 1/2 on same inputs.

**Edge-case coverage.** Include no-match inputs, overlapping matches, homopolymers, alternating patterns, very short/long patterns, and deeply nested RNA to stress the PDA stack. Reject or sanitize mixed/invalid symbols per preprocessing; fix random seeds for synthetic data.

**Performance.** Measure NFA build/sim time, DFA build/minimize + run time, EFA scaling vs pattern length _m_ and mismatch budget _k_, and PDA time per symbol + max stack depth as sequence length _N_ scales (e.g., 10³–10⁶). Report NFA→DFA crossover and 95% CI over multiple runs.

**Usability (optional).** Small peer survey on clarity of traces and stats; iterate default visualization accordingly.

---

## Implementation (C++ brief)

- **Language/Build:** C++ (CMake, `-O3`).
- **Core structures:** adjacency maps; bitsets for state sets; cached ε-closures.
- **EFA:** layers indexed by mismatches `e = 0…k`; windowed scan.
- **PDA:** simple char stack with deterministic transitions.
- **CLI:** `--mode auto|nfa|dfa|efa|pda --pattern P --k K --input file --trace`
- **Outputs:** JSON/CSV results; optional ASCII diagrams/traces.

---

## Paper / Case Study Outline (already prepared)

- Title, Abstract, Keywords
- Introduction (motivation; problem)
- Theoretical Framework (RE→NFA→DFA; EFA; PDA)
- **Methodology** (datasets; modes; pipeline; evaluation)
- Case Scenarios (log/regex; DNA approx; RNA PDA)
- Discussion (model-language fit; trade-offs)
- Conclusion & Future Work (GUI, Levenshtein automata, web demo)
- Related Literature (2020+) — **mix of search, bioinformatics, education** (to be inserted)
- References

---

## Demo Patterns (examples)

- **Exact DNA:** `A(T|G)*C`, `ATG[AC]T{2}`
- **Approx DNA:** Pattern `ACGT`, k = 1..3 (Hamming)
- **RNA PDA:** `(((...)))`, `(()())`, invalid: `())(`, `(.)` (reject)

---

## Acceptance Criteria (succinct)

- Correct detection per ground truth (including **k=0 → exact** equivalence).
- Determinization improves run time for long inputs (document crossover).
- PDA accepts all valid and rejects malformed dot–bracket cases.
- Reproducible runs (seeded), clear traces, and summarized stats.

---

## Risks & Mitigations

- **State blow-up (DFA/EFA):** keep regex subset; cap _k_; prefer NFA sim when determinization is large.
- **Scope creep:** focus on Hamming (no indels); keep UI single-task.
- **Performance variance:** average multiple runs; report CI; use fixed hardware profile.

---

## Open Item

- Insert **2020+ related studies** (automata in search, bioinformatics, and educational simulators) with citation-ready statements.
