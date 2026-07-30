#pragma once

#include <atomic>
#include <cstdint>
#include <string>

#include "mercury/normalizer/market_event.hpp"

namespace mercury::consumer {

class MetricsConsumer {
  public:
    void on_event(const mercury::normalizer::MarketEvent& event) noexcept;
    void on_invalid() noexcept;
    void on_duplicate() noexcept;
    void on_gap() noexcept;
    [[nodiscard]] std::string render_prometheus() const;

  private:
    std::atomic<std::uint64_t> messages_total_{0};
    std::atomic<std::uint64_t> invalid_total_{0};
    std::atomic<std::uint64_t> duplicate_total_{0};
    std::atomic<std::uint64_t> gap_total_{0};
    std::atomic<std::uint64_t> last_latency_ns_{0};
  };

} // namespace mercury::consumer
