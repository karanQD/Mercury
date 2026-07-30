#include "mercury/receiver/pcap_receiver.hpp"

#include <cstring>
#include <fstream>

namespace mercury::receiver {

PcapReplayReceiver::PcapReplayReceiver(std::string file_path) : file_path_(std::move(file_path)) {}

bool PcapReplayReceiver::load() {
    std::ifstream input(file_path_, std::ios::binary);
    if (!input) {
        return false;
    }
    bytes_ = std::vector<std::uint8_t>(std::istreambuf_iterator<char>(input), {});
    if (bytes_.size() < sizeof(PcapGlobalHeader)) {
        return false;
    }
    std::memcpy(&global_header_, bytes_.data(), sizeof(global_header_));
    return true;
}

bool PcapReplayReceiver::is_swapped_endian() const noexcept {
    return global_header_.magic_number == 0xd4c3b2a1U;
}

std::uint32_t PcapReplayReceiver::maybe_swap32(std::uint32_t value) const noexcept {
    return is_swapped_endian() ? __builtin_bswap32(value) : value;
}

std::size_t PcapReplayReceiver::link_offset() const noexcept {
    const std::uint32_t network = maybe_swap32(global_header_.network);
    if (network == 1U) {
        return 14U;
    }
    if (network == 113U) {
        return 16U;
    }
    return 14U;
}

bool PcapReplayReceiver::start() {
    if (!load()) {
        return false;
    }
    running_ = true;

    std::size_t offset = sizeof(PcapGlobalHeader);
    const std::size_t l2 = link_offset();
    constexpr std::size_t l3_l4 = 20U + 8U;

    while (running_ && offset + sizeof(PcapPacketHeader) <= bytes_.size()) {
        PcapPacketHeader packet{};
        std::memcpy(&packet, bytes_.data() + offset, sizeof(packet));
        offset += sizeof(packet);
        const std::size_t incl_len = maybe_swap32(packet.incl_len);
        if (offset + incl_len > bytes_.size()) {
            break;
        }
        if (incl_len > l2 + l3_l4) {
            const auto* payload = bytes_.data() + offset + l2 + l3_l4;
            const auto payload_len = incl_len - l2 - l3_l4;
            if (on_data_) {
                on_data_(std::span<const std::uint8_t>(payload, payload_len));
            }
        }
        offset += incl_len;
    }

    return true;
}

void PcapReplayReceiver::stop() { running_ = false; }

void PcapReplayReceiver::set_on_data(std::function<void(std::span<const std::uint8_t>)> callback) {
    on_data_ = std::move(callback);
}

} // namespace mercury::receiver
