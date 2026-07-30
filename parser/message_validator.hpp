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
