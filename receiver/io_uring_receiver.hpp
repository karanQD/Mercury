#pragma once

#include <cstdint>
#include <functional>
#include <span>
#include <string>

namespace mercury::receiver {

class IoUringReceiver {
  public:
    IoUringReceiver(std::string bind_ip, std::uint16_t port) : bind_ip_(std::move(bind_ip)), port_(port) {}

    [[nodiscard]] bool start();
    void stop();
    void set_on_data(std::function<void(std::span<const std::uint8_t>)> callback) { on_data_ = std::move(callback); }

  private:
    std::string bind_ip_;
    std::uint16_t port_{0};
    std::function<void(std::span<const std::uint8_t>)> on_data_{};
    bool running_{false};
};

} // namespace mercury::receiver
