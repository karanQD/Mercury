#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <span>

#include "mercury/normalizer/market_event.hpp"
#include "mercury/parser/itch_messages.hpp"

namespace mercury::parser {

class Itch50Parser {
  public:
    [[nodiscard]] bool parse(std::span<const std::uint8_t> bytes) noexcept;
    [[nodiscard]] mercury::normalizer::MarketEvent get_last_event() const noexcept { return last_event_; }

  private:
    struct OrderCacheEntry {
        std::uint64_t order_reference_number{0};
        std::array<char, 8> symbol{' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
        double price{0.0};
        mercury::normalizer::Side side{mercury::normalizer::Side::Buy};
        bool occupied{false};
    };

    static constexpr std::size_t kOrderCacheCapacity = 1U << 16U;

    void record_add(const itch::AddOrderMessage& msg) noexcept;
    [[nodiscard]] const OrderCacheEntry* find_order(std::uint64_t order_reference_number) const noexcept;
    [[nodiscard]] OrderCacheEntry* find_order_mut(std::uint64_t order_reference_number) noexcept;
    void clear_order(std::uint64_t order_reference_number) noexcept;

    std::array<OrderCacheEntry, kOrderCacheCapacity> order_cache_{};
    mercury::normalizer::MarketEvent last_event_{};
};

} // namespace mercury::parser
