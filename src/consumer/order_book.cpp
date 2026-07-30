#include "mercury/consumer/order_book.hpp"

namespace mercury::consumer {

void OrderBook::on_event(const mercury::normalizer::MarketEvent& event) {
    auto& levels = books_[std::string(mercury::normalizer::symbol_view(event.symbol))];
    auto& side_map = (event.side == mercury::normalizer::Side::Buy) ? levels.bids : levels.asks;

    switch (event.event_type) {
        case mercury::normalizer::EventType::Add:
            side_map[event.price] += event.quantity;
            break;
        case mercury::normalizer::EventType::Execute:
        case mercury::normalizer::EventType::Cancel: {
            auto it = side_map.find(event.price);
            if (it != side_map.end()) {
                if (it->second <= event.quantity) {
                    side_map.erase(it);
                } else {
                    it->second -= event.quantity;
                }
            }
            break;
        }
        case mercury::normalizer::EventType::Delete:
            side_map.erase(event.price);
            break;
        default:
            break;
    }
}

std::vector<std::pair<double, std::uint64_t>> OrderBook::top_bids(const std::string& symbol, const std::size_t depth) const {
    std::vector<std::pair<double, std::uint64_t>> out;
    if (const auto it = books_.find(symbol); it != books_.end()) {
        for (const auto& level : it->second.bids) {
            out.push_back(level);
            if (out.size() == depth) {
                break;
            }
        }
    }
    return out;
}

std::vector<std::pair<double, std::uint64_t>> OrderBook::top_asks(const std::string& symbol, const std::size_t depth) const {
    std::vector<std::pair<double, std::uint64_t>> out;
    if (const auto it = books_.find(symbol); it != books_.end()) {
        for (const auto& level : it->second.asks) {
            out.push_back(level);
            if (out.size() == depth) {
                break;
            }
        }
    }
    return out;
}

} // namespace mercury::consumer
