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
