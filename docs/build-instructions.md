# Build Instructions

## Dependencies

- Linux x86_64
- CMake 3.20+
- GCC 13+ or Clang 17+
- POSIX threads

## Configure and build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Run tests

```bash
ctest --test-dir build --output-on-failure
```

## Run benchmarks

```bash
./build/parser_benchmark
./build/queue_comparison
./build/latency_benchmark
./build/routing_benchmark
```
