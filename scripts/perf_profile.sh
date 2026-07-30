#!/usr/bin/env bash
set -euo pipefail
perf stat -d ./build/parser_benchmark
perf stat -d ./build/queue_comparison
