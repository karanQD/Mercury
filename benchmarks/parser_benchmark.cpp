#include <chrono>
#include <cstring>
#include <iostream>
#include <vector>

#include "mercury/parser/itch50_parser.hpp"

int main() {
    mercury::parser::Itch50Parser parser;
    std::vector<std::uint8_t> add(36U, 0U);
    add[0] = 'A';
    std::memcpy(add.data() + 23U, "AAPL    ", 8U);
    constexpr std::size_t iterations = 1000000;
    const auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < iterations; ++i) {
        add[22] = static_cast<std::uint8_t>(i & 0xFFU);
        parser.parse(add);
    }
    const auto end = std::chrono::steady_clock::now();
    const auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "parser_iterations=" << iterations << " total_ns=" << ns << '\n';
    return 0;
}
