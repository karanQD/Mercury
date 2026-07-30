#pragma once

#include <array>
#include <cstdint>
#include <string_view>

namespace mercury::normalizer {

enum class Side : std::uint8_t { Buy = 0, Sell = 1 };
enum class EventType : std::uint8_t { Add = 0, Execute = 1, Cancel = 2, Delete = 3, System = 4 };

struct alignas(64) MarketEvent {
    std::array<char, 8> symbol{};
    Side side{Side::Buy};
    EventType event_type{EventType::Add};
    double price{0.0};
    std::uint64_t quantity{0};
    std::uint64_t timestamp_ns{0};
    std::uint64_t order_reference_number{0};
    std::uint64_t match_number{0};
    std::uint64_t packet_arrival_ns{0};
    std::uint64_t parser_complete_ns{0};
    std::uint64_t validator_complete_ns{0};
    std::uint64_t published_ns{0};
};

static_assert(alignof(MarketEvent) == 64, "MarketEvent must be cache-line aligned");

[[nodiscard]] inline std::string_view symbol_view(const std::array<char, 8>& symbol) noexcept {
    std::size_t end = symbol.size();
    while (end > 0U && symbol[end - 1U] == ' ') {
        --end;
    }
    return std::string_view(symbol.data(), end);
}

} // namespace mercury::normalizer
