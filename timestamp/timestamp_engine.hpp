#pragma once

#include <cstdint>

namespace mercury::timestamp {

class TimestampEngine {
  public:
    void calibrate();
    [[nodiscard]] std::uint64_t now_ns() const noexcept;
    [[nodiscard]] std::uint64_t now_cycles() const noexcept;
    [[nodiscard]] std::uint64_t cycles_to_ns(std::uint64_t cycles) const noexcept;

  private:
    double ns_per_cycle_{0.0};
    std::uint64_t base_cycles_{0};
    std::uint64_t base_ns_{0};
};

} // namespace mercury::timestamp
