#include <cassert>
#include <cstring>

#include "mercury/parser/message_validator.hpp"

int main() {
    mercury::parser::MessageValidator validator;
    mercury::parser::itch::AddOrderMessage add{};
    add.message_type = 'A';
    add.tracking_number = mercury::parser::itch::be16_to_cpu(1U);
    add.order_reference_number = mercury::parser::itch::be64_to_cpu(42U);
    std::memcpy(add.stock, "AAPL    ", 8U);
    const auto r1 = validator.validate(add);
    assert(r1.ok);
    const auto r2 = validator.validate(add);
    assert(!r2.ok);
    assert(validator.get_duplicate_count() == 1U);
    return 0;
}
