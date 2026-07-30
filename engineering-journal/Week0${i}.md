# Week 0${i}

## What I built
- Advanced the Mercury feed-handler pipeline toward a fully pinned, bounded-memory architecture.
- Extended tests and benchmarks so each major subsystem could be measured in isolation.

## What failed
- At least one design assumption proved too expensive under tail-latency measurement and had to be replaced or simplified.
- Early convenience abstractions caused branchy code or hidden allocations that were visible in flamegraphs.

## Design decisions
- Prefer fixed-capacity data structures and cache-line alignment over flexible containers in the hot path.
- Keep stage interfaces explicit and measurable so each queue boundary has a clear owner and latency budget.

## Benchmarks
- Throughput and percentile latency improved week-over-week after removing dynamic allocation, reducing symbol copies, and flattening control flow.
- Each benchmark run was treated as invalid unless CPU pinning and release builds were enabled.

## Next week's plan
- Tighten the parser and validator contract.
- Reduce branch misses in the mixed-message benchmark.
- Improve documentation and reproducibility.
