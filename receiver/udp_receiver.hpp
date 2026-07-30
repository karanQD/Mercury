#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <thread>

namespace mercury::receiver {

class UdpReceiver {
  public:
    UdpReceiver(std::string bind_ip, std::uint16_t port);
    ~UdpReceiver();

    [[nodiscard]] bool start();
    void stop();
    void set_on_data(std::function<void(std::span<const std::uint8_t>)> callback);

  private:
    void run_loop();

    std::string bind_ip_;
    std::uint16_t port_{0};
    int fd_{-1};
    std::array<std::uint8_t, 2048> buffer_{};
    std::function<void(std::span<const std::uint8_t>)> on_data_{};
    std::atomic<bool> running_{false};
    std::thread worker_{};
};

} // namespace mercury::receiver
