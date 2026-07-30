#pragma once

#include <array>
#include <atomic>
#include <bit>
#include <cstddef>
#include <new>
#include <type_traits>

namespace mercury::queue {

template <typename T, std::size_t Capacity>
class alignas(64) SpscRingBuffer {
    static_assert(Capacity >= 2, "Capacity must be at least 2");
    static_assert(std::has_single_bit(Capacity), "Capacity must be a power of two");

  public:
    [[nodiscard]] bool push(const T& item) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        const std::size_t tail = tail_.load(std::memory_order_relaxed);
        const std::size_t next = tail + 1U;
        const std::size_t head_snapshot = head_.load(std::memory_order_acquire);
        if ((next - head_snapshot) > Capacity) {
            return false;
        }
        storage_[tail & mask_] = item;
        tail_.store(next, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool pop(T& item) noexcept(std::is_nothrow_copy_assignable_v<T>) {
        const std::size_t head = head_.load(std::memory_order_relaxed);
        const std::size_t tail_snapshot = tail_.load(std::memory_order_acquire);
        if (head == tail_snapshot) {
            return false;
        }
        item = storage_[head & mask_];
        head_.store(head + 1U, std::memory_order_release);
        return true;
    }

    [[nodiscard]] bool empty() const noexcept {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

    [[nodiscard]] std::size_t size() const noexcept {
        const std::size_t head = head_.load(std::memory_order_acquire);
        const std::size_t tail = tail_.load(std::memory_order_acquire);
        return tail - head;
    }

    [[nodiscard]] constexpr std::size_t capacity() const noexcept { return Capacity; }

  private:
    static constexpr std::size_t mask_ = Capacity - 1U;

    alignas(64) std::array<T, Capacity> storage_{};
    alignas(64) std::atomic<std::size_t> head_{0};
    alignas(64) std::atomic<std::size_t> tail_{0};
};

} // namespace mercury::queu
