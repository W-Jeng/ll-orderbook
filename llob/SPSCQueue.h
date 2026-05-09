#pragma once
#include <atomic>
#include <optional>
#include <new>

namespace llob {

static constexpr std::size_t CACHE_LINE_SIZE = 64;

template<typename T, std::size_t Capacity>
alignas(CACHE_LINE_SIZE)
class SPSCQueue {
  static_assert((Capacity & (Capacity-1)) == 0, "Must be power of 2");
  static_assert(Capacity > 1, "Capacity must be larger than 1");
public:
  SPSCQueue()
    : head_(0)
    , tail_(0) {}
  
  ~SPSCQueue() {
    while (pop().has_value())
      continue;
  }

  SPSCQueue(const SPSCQueue&) = delete;
  SPSCQueue(SPSCQueue&&) = delete;
  SPSCQueue& operator=(const SPSCQueue&) = delete;
  SPSCQueue& operator=(SPSCQueue&&) = delete;

  template <typename... Args>
  bool emplace(Args&&... args) {
    const std::size_t head = head_.load(std::memory_order_relaxed);
    const std::size_t next = increment(head);

    if (next == cached_tail_) {
      cached_tail_ =  tail_.load(std::memory_order_acquire);
      if (next == cached_tail_) {
        return false;
      }
    }

    ::new(slot(head)) T(std::forward<Args>(args)...);
    head_.store(next, std::memory_order_release);
    return true;
  }

  bool push(T&& val) {
    return emplace(std::move(val));
  }

  bool push(const T& val) {
    return emplace(val);
  }

  std::optional<T> pop() {
    const std::size_t tail = tail_.load(std::memory_order_relaxed);

    if (tail == cached_head_) {
      cached_head_ = head_.load(std::memory_order_acquire);

      if (tail == cached_head_)
        return std::nullopt;
    }

    T val = std::move(*slot(tail));
    slot(tail) -> ~T();
    tail_.store(increment(tail), std::memory_order_release);
    return val;
  }

private:
  static constexpr std::size_t MASK = Capacity-1;

  alignas(CACHE_LINE_SIZE) std::atomic<std::size_t> head_;
  std::size_t cached_head_{0};
  char pad0_[CACHE_LINE_SIZE
    - sizeof(std::atomic<std::size_t>)
    - sizeof(std::size_t)];

  alignas(CACHE_LINE_SIZE) std::atomic<std::size_t> tail_;
  std::size_t cached_tail_{0};
  char pad1_[CACHE_LINE_SIZE
    - sizeof(std::atomic<std::size_t>)
    - sizeof(std::size_t)
  ];

  alignas(CACHE_LINE_SIZE) alignas(T) std::byte storage_[sizeof(T)*Capacity];

  std::size_t increment(std::size_t idx) {
    return (idx+1) % Capacity;
  }

  T* slot(std::size_t idx) {
    return std::launder(reinterpret_cast<T*>(storage_ + idx * sizeof(T)));
  }
};

} // namespace llob
