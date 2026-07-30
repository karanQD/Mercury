#include "mercury/timestamp/timestamp_engine.hpp"

#include <ctime>
#include <thread>
#include <x86intrin.h>

namespace mercury::timestamp {

namespace {
[[nodiscard]] std::uint64_t clock_now_ns() noexcept {
    timespec ts{};
    ::clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return static_cast<std::uint64_t>(ts.tv_sec) * 1000000000ULL + static_cast<std::uint64_t>(ts.tv_nsec);
}
} // namespace

void TimestampEngine::calibrate() {
    base_ns_ = clock_now_ns();
    base_cycles_ = __rdtsc();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    const std::uint64_t ns2 = clock_now_ns();
    const std::uint64_t cycles2 = __rdtsc();
    const std::uint64_t delta_ns = ns2 - base_ns_;
    const std::uint64_t delta_cycles = cycles2 - base_cycles_;
    ns_per_cycle_ = delta_cycles == 0U ? 0.0 : static_cast<double>(delta_ns) / static_cast<double>(delta_cycles);
}

std::uint64_t TimestampEngine::now_cycles() const noexcept { return __rdtsc(); }

std::uint64_t TimestampEngine::cycles_to_ns(const std::uint64_t cycles) const noexcept {
    return static_cast<std::uint64_t>(static_cast<double>(cycles) * ns_per_cycle_);
}

std::uint64_t TimestampEngine::now_ns() const noexcept {
    const std::uint64_t current_cycles = now_cycles();
    const std::uint64_t delta_cycles = current_cycles - base_cycles_;
    return base_ns_ + cycles_to_ns(delta_cycles);
}

} // namespace mercury::timestamp
