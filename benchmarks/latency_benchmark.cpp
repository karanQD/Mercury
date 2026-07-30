#include <iostream>

#include "mercury/consumer/latency_logger.hpp"

int main() {
    mercury::consumer::LatencyLogger logger;
    for (std::uint64_t i = 0; i < 1000; ++i) {
        mercury::normalizer::MarketEvent event{};
        event.timestamp_ns = i;
        event.packet_arrival_ns = i;
        event.published_ns = i + (i % 50U);
        logger.record(event);
    }
    std::cout << "p50=" << logger.percentile(0.50) << '\n';
    std::cout << "p99=" << logger.percentile(0.99) << '\n';
    return 0;
}
