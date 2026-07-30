#include <chrono>
#include <cstring>
#include <iostream>

#include "mercury/router/symbol_router.hpp"

int main() {
    mercury::router::SymbolRouter router;
    std::size_t hits = 0;
    for (std::size_t i = 0; i < mercury::router::SymbolRouter::kWorkerCount; ++i) {
        router.set_worker_callback(i, [&](const mercury::normalizer::MarketEvent&) { ++hits; });
    }
    mercury::normalizer::MarketEvent event{};
    std::memcpy(event.symbol.data(), "GOOG    ", 8U);
    constexpr std::size_t iterations = 1000000;
    const auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < iterations; ++i) {
        router.route(event);
    }
    const auto end = std::chrono::steady_clock::now();
    std::cout << "routing_ns=" << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
              << " hits=" << hits << '\n';
    return 0;
}
