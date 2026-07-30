#include "mercury/consumer/latency_logger.hpp"

#include <algorithm>
#include <fstream>

namespace mercury::consumer {

void LatencyLogger::record(const mercury::normalizer::MarketEvent& event) {
    const auto latency = event.published_ns >= event.packet_arrival_ns ? event.published_ns - event.packet_arrival_ns : 0ULL;
    records_.push_back(LatencyRecord{event.timestamp_ns, latency, event.event_type,
                                     std::string(mercury::normalizer::symbol_view(event.symbol))});
}

void LatencyLogger::write_csv(const std::string& file_path) const {
    std::ofstream out(file_path);
    out << "timestamp_ns,latency_ns,event_type,symbol\n";
    for (const auto& record : records_) {
        out << record.timestamp_ns << ',' << record.latency_ns << ',' << static_cast<int>(record.event_type) << ','
            << record.symbol << '\n';
    }
}

std::uint64_t LatencyLogger::percentile(const double pct) const {
    if (records_.empty()) {
        return 0;
    }
    std::vector<std::uint64_t> values;
    values.reserve(records_.size());
    for (const auto& record : records_) {
        values.push_back(record.latency_ns);
    }
    std::sort(values.begin(), values.end());
    const auto idx = static_cast<std::size_t>(pct * static_cast<double>(values.size() - 1U));
    return values[idx];
}

} // namespace mercury::consumer
