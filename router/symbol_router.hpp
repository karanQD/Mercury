#pragma once

#include <array>
#include <cstddef>
#include <functional>

#include "mercury/normalizer/market_event.hpp"

namespace mercury::router {

class SymbolRouter {
  public:
    static constexpr std::size_t kWorkerCount = 3;

    void route(const mercury::normalizer::MarketEvent& event) const;
    void set_worker_callback(std::size_t worker_id,
                             std::function<void(const mercury::normalizer::MarketEvent&)> callback);

  private:
    [[nodiscard]] std::size_t hash_symbol(const std::array<char, 8>& symbol) const noexcept;
    [[nodiscard]] std::size_t get_worker_id(const std::array<char, 8>& symbol) const noexcept;

    std::array<std::function<void(const mercury::normalizer::MarketEvent&)>, kWorkerCount> callbacks_{};
};

} // namespace mercury::router
