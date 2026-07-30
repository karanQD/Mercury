#include "mercury/router/symbol_router.hpp"

namespace mercury::router {

std::size_t SymbolRouter::hash_symbol(const std::array<char, 8>& symbol) const noexcept {
    std::size_t hash = 1469598103934665603ULL;
    for (const char ch : symbol) {
        hash ^= static_cast<unsigned char>(ch);
        hash *= 1099511628211ULL;
    }
    return hash;
}

std::size_t SymbolRouter::get_worker_id(const std::array<char, 8>& symbol) const noexcept {
    return hash_symbol(symbol) % kWorkerCount;
}

void SymbolRouter::route(const mercury::normalizer::MarketEvent& event) const {
    const std::size_t worker_id = get_worker_id(event.symbol);
    if (callbacks_[worker_id]) {
        callbacks_[worker_id](event);
    }
}

void SymbolRouter::set_worker_callback(
    const std::size_t worker_id,
    std::function<void(const mercury::normalizer::MarketEvent&)> callback) {
    if (worker_id < kWorkerCount) {
        callbacks_[worker_id] = std::move(callback);
    }
}

} // namespace mercury::router
