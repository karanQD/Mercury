#include "mercury/parser/message_validator.hpp"

#include <cctype>

namespace mercury::parser {

MessageValidator::MessageValidator() = default;

ValidationResult MessageValidator::validate_raw(const char message_type, const std::size_t bytes) noexcept {
    switch (message_type) {
        case 'S': return bytes == sizeof(itch::SystemEventMessage) ? ValidationResult{} : ValidationResult{false, ValidationCode::InvalidLength, "system length"};
        case 'A': return bytes == sizeof(itch::AddOrderMessage) ? ValidationResult{} : ValidationResult{false, ValidationCode::InvalidLength, "add length"};
        case 'E': return bytes == sizeof(itch::OrderExecutedMessage) ? ValidationResult{} : ValidationResult{false, ValidationCode::InvalidLength, "execute length"};
        case 'C': return bytes == sizeof(itch::OrderCancelMessage) ? ValidationResult{} : ValidationResult{false, ValidationCode::InvalidLength, "cancel length"};
        case 'D': return bytes == sizeof(itch::OrderDeleteMessage) ? ValidationResult{} : ValidationResult{false, ValidationCode::InvalidLength, "delete length"};
        default: return {false, ValidationCode::UnknownMessageType, "unknown type"};
    }
}

bool MessageValidator::is_valid_symbol(const std::string_view symbol) const noexcept {
    if (symbol.empty() || symbol.size() > 8U) {
        return false;
    }
    for (const char ch : symbol) {
        if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == ' ')) {
            return false;
        }
    }
    return true;
}

ValidationResult MessageValidator::validate_tracking(const std::uint16_t tracking_number) noexcept {
    if (!has_tracking_) {
        has_tracking_ = true;
        last_tracking_number_ = tracking_number;
        return {};
    }
    const std::uint16_t expected = static_cast<std::uint16_t>(last_tracking_number_ + 1U);
    if (tracking_number != expected) {
        ++gap_count_;
        last_tracking_number_ = tracking_number;
        return {false, ValidationCode::GapDetected, "tracking gap"};
    }
    last_tracking_number_ = tracking_number;
    return {};
}

bool MessageValidator::insert_order_ref(const std::uint64_t order_reference_number) noexcept {
    const std::size_t mask = kSeenCapacity - 1U;
    std::size_t idx = static_cast<std::size_t>(order_reference_number) & mask;
    for (std::size_t probe = 0; probe < kSeenCapacity; ++probe) {
        auto& slot = seen_order_refs_[(idx + probe) & mask];
        if (slot == 0U) {
            slot = order_reference_number;
            return true;
        }
        if (slot == order_reference_number) {
            return false;
        }
    }
    return false;
}

ValidationResult MessageValidator::validate(const itch::SystemEventMessage& msg) noexcept {
    return validate_tracking(itch::be16_to_cpu(msg.tracking_number));
}

ValidationResult MessageValidator::validate(const itch::AddOrderMessage& msg) noexcept {
    if (const auto raw = validate_raw(msg.message_type, sizeof(msg)); !raw.ok) {
        ++invalid_count_;
        return raw;
    }
    if (const auto tracking = validate_tracking(itch::be16_to_cpu(msg.tracking_number)); !tracking.ok) {
        return tracking;
    }
    if (!is_valid_symbol(itch::extract_symbol(msg.stock))) {
        ++invalid_count_;
        return {false, ValidationCode::InvalidSymbol, "invalid symbol"};
    }
    if (!insert_order_ref(itch::be64_to_cpu(msg.order_reference_number))) {
        ++duplicate_count_;
        return {false, ValidationCode::Duplicate, "duplicate order reference"};
    }
    return {};
}

ValidationResult MessageValidator::validate(const itch::OrderExecutedMessage& msg) noexcept {
    return validate_tracking(itch::be16_to_cpu(msg.tracking_number));
}

ValidationResult MessageValidator::validate(const itch::OrderCancelMessage& msg) noexcept {
    return validate_tracking(itch::be16_to_cpu(msg.tracking_number));
}

ValidationResult MessageValidator::validate(const itch::OrderDeleteMessage& msg) noexcept {
    return validate_tracking(itch::be16_to_cpu(msg.tracking_number));
}

} // namespace mercury::parser
