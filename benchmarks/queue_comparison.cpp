#include <chrono>
#include <iostream>
#include <mutex>
#include <queue>

#include "mercury/queue/spsc_ring_buffer.hpp"

int main() {
    constexpr std::size_t iterations = 100000;
    mercury::queue::SpscRingBuffer<int, 1024> spsc;
    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < iterations; ++i) {
        while (!spsc.push(static_cast<int>(i))) {}
        int out = 0;
        while (!spsc.pop(out)) {}
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "spsc_ns=" << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << '\n';

    std::queue<int> q;
    std::mutex m;
    start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < iterations; ++i) {
        {
            std::lock_guard<std::mutex> lock(m);
            q.push(static_cast<int>(i));
            q.pop();
        }
    }
    end = std::chrono::steady_clock::now();
    std::cout << "mutex_queue_ns=" << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << '\n';
    return 0;
}
