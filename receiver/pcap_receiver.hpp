#pragma once

#include <cstdint>
#include <functional>
#include <span>
#include <string>
#include <vector>

namespace mercury::receiver {

#pragma pack(push, 1)
struct PcapGlobalHeader {
    std::uint32_t magic_number;
    std::uint16_t version_major;
    std::uint16_t version_minor;
    std::int32_t thiszone;
    std::uint32_t sigfigs;
    std::uint32_t snaplen;
    std::uint32_t network;
};

struct PcapPacketHeader {
    std::uint32_t ts_sec;
    std::uint32_t ts_usec;
    std::uint32_t incl_len;
    std::uint32_t orig_len;
};

struct SllHeader {
    std::uint16_t packet_type;
    std::uint16_t link_layer_type;
    std::uint16_t link_layer_length;
    std::uint64_t source_address;
};
#pragma pack(pop)

class PcapReplayReceiver {
  public:
    explicit PcapReplayReceiver(std::string file_path);

    [[nodiscard]] bool start();
    void stop();
    void set_on_data(std::function<void(std::span<const std::uint8_t>)> callback);

  private:
    [[nodiscard]] bool load();
    [[nodiscard]] std::size_t link_offset() const noexcept;
    [[nodiscard]] bool is_swapped_endian() const noexcept;
    [[nodiscard]] std::uint32_t maybe_swap32(std::uint32_t value) const noexcept;

    std::string file_path_;
    std::vector<std::uint8_t> bytes_{};
    std::function<void(std::span<const std::uint8_t>)> on_data_{};
    bool running_{false};
    PcapGlobalHeader global_header_{};
};

} // namespace mercury::receiver
