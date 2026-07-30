#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "mercury/normalizer/market_event.hpp"

namespace mercury::consumer {

struct LatencyRecord {
    std::uint64_t timestamp_ns{0};
    std::uint64_t latency_ns{0};
    mercury::normalizer::EventType event_type{mercury::normalizer::EventType::Add};
    std::string symbol;
};

class LatencyLogger {
  public:
    void record(const mercury::normalizer::MarketEvent& event);
    void write_csv(const std::string& file_path) const;
    [[nodiscard]] std::uint64_t percentile(double pct) const;

  private:
    std::vector<LatencyRecord> records_{};
};

} // namespace mercury::consumer
