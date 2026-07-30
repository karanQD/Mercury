#include <concepts>
#include <cstdint>
#include <functional>
#include <span>

namespace mercury::concepts {

template <typename T>
concept MarketDataReceiver = requires(T receiver, std::function<void(std::span<const std::uint8_t>)> cb) {
    { receiver.start() } -> std::same_as<bool>;
    { receiver.stop() } -> std::same_as<void>;
    { receiver.set_on_data(cb) } -> std::same_as<void>;
};

} // namespace mercury::concepts
EOF
cat > /home/user/mercury/include/mercury/normalizer/market_event.hpp <<'EOF'
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
EOF
cat > /home/user/mercury/include/mercury/concepts/parser_concepts.hpp <<'EOF'
#pragma once

#include <concepts>
#include <cstdint>
#include <span>

#include "mercury/normalizer/market_event.hpp"

namespace mercury::concepts {

template <typename T>
concept ProtocolParser = requires(T parser, std::span<const std::uint8_t> bytes) {
    { parser.parse(bytes) } -> std::same_as<bool>;
    { parser.get_last_event() } -> std::same_as<mercury::normalizer::MarketEvent>;
};

} // namespace mercury::concepts
EOF
cat > /home/user/mercury/include/mercury/parser/itch_messages.hpp <<'EOF'
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
EOF
cat > /home/user/mercury/include/mercury/parser/message_validator.hpp <<'EOF'
#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "mercury/parser/itch_messages.hpp"

namespace mercury::parser {

enum class ValidationCode : std::uint8_t {
    Ok = 0,
    InvalidLength,
    UnknownMessageType,
    Duplicate,
    GapDetected,
    InvalidSymbol
};

struct ValidationResult {
    bool ok{true};
    ValidationCode code{ValidationCode::Ok};
    const char* reason{"ok"};
};

class MessageValidator {
  public:
    MessageValidator();

    [[nodiscard]] ValidationResult validate(const itch::SystemEventMessage& msg) noexcept;
    [[nodiscard]] ValidationResult validate(const itch::AddOrderMessage& msg) noexcept;
    [[nodiscard]] ValidationResult validate(const itch::OrderExecutedMessage& msg) noexcept;
    [[nodiscard]] ValidationResult validate(const itch::OrderCancelMessage& msg) noexcept;
    [[nodiscard]] ValidationResult validate(const itch::OrderDeleteMessage& msg) noexcept;
    [[nodiscard]] ValidationResult validate_raw(char message_type, std::size_t bytes) noexcept;

    [[nodiscard]] std::uint64_t get_invalid_count() const noexcept { return invalid_count_; }
    [[nodiscard]] std::uint64_t get_duplicate_count() const noexcept { return duplicate_count_; }
    [[nodiscard]] std::uint64_t get_gap_count() const noexcept { return gap_count_; }

  private:
    static constexpr std::size_t kSeenCapacity = 1U << 20U;

    [[nodiscard]] bool is_valid_symbol(std::string_view symbol) const noexcept;
    [[nodiscard]] ValidationResult validate_tracking(std::uint16_t tracking_number) noexcept;
    [[nodiscard]] bool insert_order_ref(std::uint64_t order_reference_number) noexcept;

    std::array<std::uint64_t, kSeenCapacity> seen_order_refs_{};
    std::uint16_t last_tracking_number_{0};
    bool has_tracking_{false};
    std::uint64_t invalid_count_{0};
    std::uint64_t duplicate_count_{0};
    std::uint64_t gap_count_{0};
};

} // namespace mercury::parser
EOF
cat > /home/user/mercury/include/mercury/queue/spsc_ring_buffer.hpp <<'EOF'
#pragma once

#include <array>
#include <atomic>
#include <bit>
#include <cstddef>
#include <new>
#include <type_traits>

namespace mercury::queue {

template <typename T, std::size_t Capacity>
class alignas(64) SpscRingBuffer {
    static_assert(Capacity >= 2, "Capacity must be at least 2");
    static_assert(std::has_single_bit(Capacity), "Capacity must be a power of two");

  public:
    [[nodiscard]] bool push(const T& item) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        const std::size_t tail = tail_.load(std::memory_order_relaxed);
        const std::size_t next = tail + 1U;
        const std::size_t head_snapshot = head_.load(std::memory_order_acquire);
        if ((next - head_snapshot) > Capacity) {
            return false;
        }
        storage_[tail & mask_] = item;
        tail_.store(next, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool pop(T& item) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t tail_snapshot = tail_.load(std::memory_order_acquire);
        if (head == tail_snapshot) {
            return false;
        }
        item = storage_[head & mask_];
        head_.store(head + 1U, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool empty() const noexcept {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

    [[nodiscard]] std::size_t size() const noexcept {
        const std::size_t head = head_.load(std::memory_order_acquire);
        const std::size_t tail = tail_.load(std::memory_order_acquire);
        return tail - head;
    }

    [[nodiscard]] constexpr std::size_t capacity() const noexcept { return Capacity; }

  private:
    static constexpr std::size_t mask_ = Capacity - 1U;

    alignas(64) std::array<T, Capacity> storage_{};
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
};

} // namespace mercury::queue
