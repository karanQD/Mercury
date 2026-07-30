#include "mercury/parser/itch50_parser.hpp"

#include <algorithm>
#include <cstring>

namespace mercury::parser {

namespace {
[[nodiscard]] std::uint64_t normalize_order_ref(std::uint64_t value) noexcept {
    return itch::be64_to_cpu(value);
}
} // namespace

void Itch50Parser::record_add(const itch::AddOrderMessage& msg) noexcept {
    const std::uint64_t order_ref = normalize_order_ref(msg.order_reference_number);
    const std::size_t mask = kOrderCacheCapacity - 1U;
    std::size_t idx = static_cast<std::size_t>(order_ref) & mask;
    for (std::size_t probe = 0; probe < kOrderCacheCapacity; ++probe) {
        auto& slot = order_cache_[(idx + probe) & mask];
        if (!slot.occupied || slot.order_reference_number == order_ref) {
            slot.occupied = true;
            slot.order_reference_number = order_ref;
            std::copy_n(msg.stock, 8, slot.symbol.begin());
            slot.price = itch::price_to_double(itch::be32_to_cpu(msg.price));
            slot.side = (msg.buy_sell_indicator == 'S') ? mercury::normalizer::Side::Sell
                                                        : mercury::normalizer::Side::Buy;
            return;
        }
    }
}

const Itch50Parser::OrderCacheEntry* Itch50Parser::find_order(const std::uint64_t order_reference_number) const noexcept {
    const std::size_t mask = kOrderCacheCapacity - 1U;
    std::size_t idx = static_cast<std::size_t>(order_reference_number) & mask;
    for (std::size_t probe = 0; probe < kOrderCacheCapacity; ++probe) {
        const auto& slot = order_cache_[(idx + probe) & mask];
        if (!slot.occupied) {
            return nullptr;
        }
        if (slot.order_reference_number == order_reference_number) {
            return &slot;
        }
    }
    return nullptr;
}

Itch50Parser::OrderCacheEntry* Itch50Parser::find_order_mut(const std::uint64_t order_reference_number) noexcept {
    const std::size_t mask = kOrderCacheCapacity - 1U;
    std::size_t idx = static_cast<std::size_t>(order_reference_number) & mask;
    for (std::size_t probe = 0; probe < kOrderCacheCapacity; ++probe) {
        auto& slot = order_cache_[(idx + probe) & mask];
        if (!slot.occupied) {
            return nullptr;
        }
        if (slot.order_reference_number == order_reference_number) {
            return &slot;
        }
    }
    return nullptr;
}

void Itch50Parser::clear_order(const std::uint64_t order_reference_number) noexcept {
    if (auto* slot = find_order_mut(order_reference_number); slot != nullptr) {
        slot->occupied = false;
        slot->order_reference_number = 0;
    }
}

bool Itch50Parser::parse(std::span<const std::uint8_t> bytes) noexcept {
    if (bytes.empty()) {
        return false;
    }

    const char type = static_cast<char>(bytes[0]);
    switch (type) {
        case 'S': {
            if (bytes.size() < sizeof(itch::SystemEventMessage)) {
                return false;
            }
            itch::SystemEventMessage msg{};
            std::memcpy(&msg, bytes.data(), sizeof(msg));
            last_event_ = {};
            last_event_.event_type = mercury::normalizer::EventType::System;
            last_event_.timestamp_ns = itch::parse_itch_timestamp(msg.timestamp);
            return true;
        }
        case 'A': {
            if (bytes.size() < sizeof(itch::AddOrderMessage)) {
                return false;
            }
            itch::AddOrderMessage msg{};
            std::memcpy(&msg, bytes.data(), sizeof(msg));
            record_add(msg);
            last_event_ = {};
            std::copy_n(msg.stock, 8, last_event_.symbol.begin());
            last_event_.event_type = mercury::normalizer::EventType::Add;
            last_event_.side = (msg.buy_sell_indicator == 'S') ? mercury::normalizer::Side::Sell
                                                               : mercury::normalizer::Side::Buy;
            last_event_.price = itch::price_to_double(itch::be32_to_cpu(msg.price));
            last_event_.quantity = itch::be32_to_cpu(msg.shares);
            last_event_.timestamp_ns = itch::parse_itch_timestamp(msg.timestamp);
            last_event_.order_reference_number = normalize_order_ref(msg.order_reference_number);
            return true;
        }
        case 'E': {
            if (bytes.size() < sizeof(itch::OrderExecutedMessage)) {
                return false;
            }
            itch::OrderExecutedMessage msg{};
            std::memcpy(&msg, bytes.data(), sizeof(msg));
            const std::uint64_t order_ref = normalize_order_ref(msg.order_reference_number);
            last_event_ = {};
            if (const auto* slot = find_order(order_ref); slot != nullptr) {
                last_event_.symbol = slot->symbol;
                last_event_.side = slot->side;
                last_event_.price = slot->price;
            }
            last_event_.event_type = mercury::normalizer::EventType::Execute;
            last_event_.quantity = itch::be32_to_cpu(msg.executed_shares);
            last_event_.timestamp_ns = itch::parse_itch_timestamp(msg.timestamp);
            last_event_.order_reference_number = order_ref;
            last_event_.match_number = normalize_order_ref(msg.match_number);
            return true;
        }
        case 'C': {
            if (bytes.size() < sizeof(itch::OrderCancelMessage)) {
                return false;
            }
            itch::OrderCancelMessage msg{};
            std::memcpy(&msg, bytes.data(), sizeof(msg));
            const std::uint64_t order_ref = normalize_order_ref(msg.order_reference_number);
            last_event_ = {};
            if (const auto* slot = find_order(order_ref); slot != nullptr) {
                last_event_.symbol = slot->symbol;
                last_event_.side = slot->side;
                last_event_.price = slot->price;
            }
            last_event_.event_type = mercury::normalizer::EventType::Cancel;
            last_event_.quantity = itch::be32_to_cpu(msg.canceled_shares);
            last_event_.timestamp_ns = itch::parse_itch_timestamp(msg.timestamp);
            last_event_.order_reference_number = order_ref;
            return true;
        }
        case 'D': {
            if (bytes.size() < sizeof(itch::OrderDeleteMessage)) {
                return false;
            }
            itch::OrderDeleteMessage msg{};
            std::memcpy(&msg, bytes.data(), sizeof(msg));
            const std::uint64_t order_ref = normalize_order_ref(msg.order_reference_number);
            last_event_ = {};
            if (const auto* slot = find_order(order_ref); slot != nullptr) {
                last_event_.symbol = slot->symbol;
                last_event_.side = slot->side;
                last_event_.price = slot->price;
            }
            last_event_.event_type = mercury::normalizer::EventType::Delete;
            last_event_.timestamp_ns = itch::parse_itch_timestamp(msg.timestamp);
            last_event_.order_reference_number = order_ref;
            clear_order(order_ref);
            return true;
        }
        default:
            return false;
    }
}

} // namespace mercury::parser
