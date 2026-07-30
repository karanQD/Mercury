#include <cassert>
#include <cstring>
#include <vector>

#include "mercury/parser/itch50_parser.hpp"

int main() {
    mercury::parser::Itch50Parser parser;
    std::vector<std::uint8_t> add(36U, 0U);
    add[0] = 'A';
    add[18] = 'B';
    add[22] = 5U;
    std::memcpy(add.data() + 23U, "MSFT    ", 8U);
    add[34] = 0x27U;
    add[35] = 0x10U;
    assert(parser.parse(add));
    const auto event = parser.get_last_event();
    assert(event.quantity == 5U);
    assert(event.symbol[0] == 'M');
    return 0;
}
