#!/usr/bin/env bash
set -euo pipefail
perf record -F 999 -g -- ./build/parser_benchmark
perf script > out.perf
cat <<MSG
Use Brendan Gregg's FlameGraph tools:
  stackcollapse-perf.pl out.perf > out.folded
  flamegraph.pl out.folded > flamegraph.svg
MSG
