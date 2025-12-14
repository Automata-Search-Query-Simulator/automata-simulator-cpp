#!/usr/bin/env bash
set -euo pipefail

if [[ ! -x build/bin/automata_sim ]]; then
  echo "Simulator binary not found. Run 'make' first." >&2
  exit 1
fi

echo "== NFA mode =="
time build/bin/automata_sim --pattern "ACGT" --input datasets/dna/sample.txt || true

echo "== EFA mode =="
time build/bin/automata_sim --pattern "ACGT" --k 1 --mode efa --input datasets/dna/sample.txt || true

echo "== PDA mode =="
time build/bin/automata_sim --mode pda --dot-bracket --input datasets/rna/sample.txt || true
