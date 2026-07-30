#pragma once

#include <array>
#include <cstdint>
#include <cstring>
#include <string_view>

namespace mercury::parser::itch {

[[nodiscard]] constexpr std::uint16_t be16_to_cpu(const std::uint16_t value) noexcept {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return __builtin_bswap16(value);
#else
    return value;
#endif
}

[[nodiscard]] constexpr std::uint32_t be32_to_cpu(const std::uint32_t value) noexcept {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return __builtin_bswap32(value);
#else
    return value;
#endif
}

[[nodiscard]] constexpr std::uint64_t be64_to_cpu(const std::uint64_t value) noexcept {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return __builtin_bswap64(value);
#else
    return value;
#endif
}

[[nodiscard]] inline std::uint64_t parse_itch_timestamp(const std::uint8_t ts[6]) noexcept {
    return (static_cast<std::uint64_t>(ts[0]) << 40U) |
           (static_cast<std::uint64_t>(ts[1]) << 32U) |
           (static_cast<std::uint64_t>(ts[2]) << 24U) |
           (static_cast<std::uint64_t>(ts[3]) << 16U) |
           (static_cast<std::uint64_t>(ts[4]) << 8U) |
           static_cast<std::uint64_t>(ts[5]);
}

[[nodiscard]] inline double price_to_double(const std::uint32_t price) noexcept {
    return static_cast<double>(price) / 10000.0;
}

[[nodiscard]] inline std::string_view extract_symbol(const char stock[8]) noexcept {
    std::size_t end = 8;
    while (end > 0U && stock[end - 1U] == ' ') {
        --end;
    }
    return std::string_view(stock, end);
}

#pragma pack(push, 1)
struct SystemEventMessage {
    char message_type;
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint8_t timestamp[6];
};
static_assert(sizeof(SystemEventMessage) == 11, "SystemEventMessage size mismatch");

struct AddOrderMessage {
    char message_type;
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint8_t timestamp[6];
    std::uint64_t order_reference_number;
    char buy_sell_indicator;
    std::uint32_t shares;
    char stock[8];
    std::uint32_t price;
};
static_assert(sizeof(AddOrderMessage) == 36, "AddOrderMessage size mismatch");

struct OrderExecutedMessage {
    char message_type;
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint8_t timestamp[6];
    std::uint64_t order_reference_number;
    std::uint32_t executed_shares;
    std::uint64_t match_number;
};
static_assert(sizeof(OrderExecutedMessage) == 31, "OrderExecutedMessage size mismatch");

struct OrderCancelMessage {
    char message_type;
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint8_t timestamp[6];
    std::uint64_t order_reference_number;
    std::uint32_t canceled_shares;
    std::uint16_t reserved;
};
static_assert(sizeof(OrderCancelMessage) == 25, "OrderCancelMessage size mismatch");

struct OrderDeleteMessage {
    char message_type;
    std::uint16_t stock_locate;
    std::uint16_t tracking_number;
    std::uint8_t timestamp[6];
    std::uint64_t order_reference_number;
    std::uint8_t reserved[3];
};
static_assert(sizeof(OrderDeleteMessage) == 22, "OrderDeleteMessage size mismatch");
#pragma pack(pop)

} // namespace mercury::parser::itch
