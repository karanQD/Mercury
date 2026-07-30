#include "mercury/receiver/udp_receiver.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace mercury::receiver {

UdpReceiver::UdpReceiver(std::string bind_ip, const std::uint16_t port)
    : bind_ip_(std::move(bind_ip)), port_(port) {}

UdpReceiver::~UdpReceiver() { stop(); }

bool UdpReceiver::start() {
    if (running_.exchange(true)) {
        return true;
    }

    fd_ = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd_ < 0) {
        running_ = false;
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = bind_ip_.empty() ? INADDR_ANY : inet_addr(bind_ip_.c_str());
    if (::bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        ::close(fd_);
        fd_ = -1;
        running_ = false;
        return false;
    }

    worker_ = std::thread(&UdpReceiver::run_loop, this);
    return true;
}

void UdpReceiver::run_loop() {
    while (running_) {
        const auto received = ::recv(fd_, buffer_.data(), buffer_.size(), 0);
        if (received <= 0) {
            continue;
        }
        if (on_data_) {
            on_data_(std::span<const std::uint8_t>(buffer_.data(), static_cast<std::size_t>(received)));
        }
    }
}

void UdpReceiver::stop() {
    if (!running_.exchange(false)) {
        return;
    }
    if (fd_ >= 0) {
        ::shutdown(fd_, SHUT_RDWR);
    }
    if (worker_.joinable()) {
        worker_.join();
    }
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}

void UdpReceiver::set_on_data(std::function<void(std::span<const std::uint8_t>)> callback) {
    on_data_ = std::move(callback);
}

} // namespace mercury::receiver
