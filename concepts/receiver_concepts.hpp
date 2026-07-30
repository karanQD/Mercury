#pragma once

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
