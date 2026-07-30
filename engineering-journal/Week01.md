# Week 01

## What I built
- Set up the CMake project, include tree, and first-pass ITCH 5.0 message structs.
- Implemented endian helpers and timestamp decoding.
- Added the first parser loop for Add / Execute / Cancel / Delete messages.

## What failed
- A naïve symbol path used `std::string` and quietly allocated on every parse.
- That single choice pushed observed P99 to roughly 2.1us in an early prototype.

## Design decisions
- Replaced dynamic symbol ownership with fixed-width padded storage and `std::array<char, 8>`.
- Moved normalization directly into a cache-line-sized `MarketEvent`.

## Benchmarks
- Initial prototype: functional but latency-unstable.
- After symbol fix: throughput improved and allocator noise vanished from profiles.

## Next week's plan
- Finish PCAP replay.
- Add a proper validator.
- Introduce the first SPSC queue boundary.
