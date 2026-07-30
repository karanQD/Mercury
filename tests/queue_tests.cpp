#include <cassert>

#include "mercury/queue/spsc_ring_buffer.hpp"

int main() {
    mercury::queue::SpscRingBuffer<int, 8> q;
    assert(q.empty());
    for (int i = 0; i < 8; ++i) {
        assert(q.push(i));
    }
    assert(!q.push(9));
    for (int i = 0; i < 8; ++i) {
        int value = -1;
        assert(q.pop(value));
        assert(value == i);
    }
    assert(q.empty());
    return 0;
}
