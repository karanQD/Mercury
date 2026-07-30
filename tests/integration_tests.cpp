#include <cassert>
#include <cstring>
#include <vector>

#include "mercury/consumer/order_book.hpp"
#include "mercury/parser/itch50_parser.hpp"
#include "mercury/router/symbol_router.hpp"

int main() {
    mercury::parser::Itch50Parser parser;
    mercury::router::SymbolRouter router;
    mercury::consumer::OrderBook book;

    bool called = false;
    router.set_worker_callback(0, [&](const mercury::normalizer::MarketEvent& event) {
        called = true;
        book.on_event(event);
    });
    router.set_worker_callback(1, [&](const mercury::normalizer::MarketEvent& event) {
        called = true;
        book.on_event(event);
    });
    router.set_worker_callback(2, [&](const mercury::normalizer::MarketEvent& event) {
        called = true;
        book.on_event(event);
    });

    std::vector<std::uint8_t> add(36U, 0U);
    add[0] = 'A';
    add[18] = 'B';
    add[22] = 10U;
    std::memcpy(add.data() + 23U, "NVDA    ", 8U);
    add[34] = 0x27U;
    add[35] = 0x10U;
    assert(parser.parse(add));
    router.route(parser.get_last_event());
    assert(called);
    return 0;
}
