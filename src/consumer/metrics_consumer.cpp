#include "mercury/consumer/metrics_consumer.hpp"

#include <sstream>

namespace mercury::consumer {

void MetricsConsumer::on_event(const mercury::normalizer::MarketEvent& event) noexcept {
    messages_total_.fetch_add(1U, std::memory_order_relaxed);
    const auto latency = event.published_ns >= event.packet_arrival_ns ? event.published_ns - event.packet_arrival_ns : 0ULL;
    last_latency_ns_.store(latency, std::memory_order_relaxed);
}

void MetricsConsumer::on_invalid() noexcept { invalid_total_.fetch_add(1U, std::memory_order_relaxed); }
void MetricsConsumer::on_duplicate() noexcept { duplicate_total_.fetch_add(1U, std::memory_order_relaxed); }
void MetricsConsumer::on_gap() noexcept { gap_total_.fetch_add(1U, std::memory_order_relaxed); }

std::string MetricsConsumer::render_prometheus() const {
    std::ostringstream out;
    out << "mercury_messages_total " << messages_total_.load(std::memory_order_relaxed) << '\n';
    out << "mercury_invalid_messages_total " << invalid_total_.load(std::memory_order_relaxed) << '\n';
    out << "mercury_duplicate_messages_total " << duplicate_total_.load(std::memory_order_relaxed) << '\n';
    out << "mercury_gap_events_total " << gap_total_.load(std::memory_order_relaxed) << '\n';
    out << "mercury_last_latency_ns " << last_latency_ns_.load(std::memory_order_relaxed) << '\n';
    return out.str();
}

} // namespace mercury::consumer
