#!/usr/bin/env bash
set -euo pipefail
./build/parser_benchmark
./build/queue_comparison
./build/latency_benchmark
./build/routing_benchmark
