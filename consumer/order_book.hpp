#pragma once

#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "mercury/normalizer/market_event.hpp"

namespace mercury::consumer {

class OrderBook {
  public:
    void on_event(const mercury::normalizer::MarketEvent& event);
    [[nodiscard]] std::vector<std::pair<double, std::uint64_t>> top_bids(const std::string& symbol, std::size_t depth = 10) const;
    [[nodiscard]] std::vector<std::pair<double, std::uint64_t>> top_asks(const std::string& symbol, std::size_t depth = 10) const;

  private:
    struct Levels {
        std::map<double, std::uint64_t, std::greater<>> bids;
        std::map<double, std::uint64_t, std::less<>> asks;
    };

    std::unordered_map<std::string, Levels> books_{};
};

} // namespace mercury::consumer
