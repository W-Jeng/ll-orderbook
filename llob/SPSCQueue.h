#pragma once
#include <atomic>
#include <new>

static constexpr std::size_t CACHE_LINE_SIZE = 64;

namespace llob {

template<typename T, std::size_t N>
class SPSCQueue {
  static_assert((N & (N-1)) == 0, "N must be power of 2");
  static_assert(N>1, "N must be more than 1");
public:
  SPSCQueue()
    : head_(0)
    , tail_(0) {}

  ~SPSCQueue() {
    while (front())
      pop();
  }

  SPSCQueue(SPSCQueue&&) = delete;
  SPSCQueue(const SPSCQueue&) = delete;
  SPSCQueue& operator=(SPSCQueue&&) = delete;
  SPSCQueue& operator=(const SPSCQueue&) = delete;
  
  template<typename... Args>
  [[nodiscard]] bool emplace(Args&&... args) {
    const std::size_t head = head_.load(std::memory_order_relaxed);
    std::size_t next = increment(head);

    if (next == cached_tail_) {
      cached_tail_ = tail_.load(std::memory_order_acquire);
      if (next == cached_tail_)
        return false;
    }

    ::new(slot(head)) T(std::forward<Args>(args)...);
    head_.store(next, std::memory_order_release);
    return true;
  }

  [[nodiscard]] bool push(T&& a) {
    return emplace(std::move(a));
  }

  [[nodiscard]] bool push(const T& a) {
    return emplace(a);
  }

  [[nodiscard]] T* front() {
    const std::size_t tail = tail_.load(std::memory_order_relaxed);

    if (tail == cached_head_) {
      cached_head_ = head_.load(std::memory_order_acquire);
      if (tail == cached_head_)
        return nullptr;
    }
    
    return slot(tail);
  }

  void pop() {
    const std::size_t tail = tail_.load(std::memory_order_relaxed);
    slot(tail) -> ~T();
    tail_.store(increment(tail), std::memory_order_release);
  }

private:
  alignas(CACHE_LINE_SIZE) std::atomic<std::size_t> head_;
  std::size_t cached_tail_{0};
  char pad0_[
    CACHE_LINE_SIZE-
    sizeof(std::atomic<std::size_t>)-
    sizeof(std::size_t)];

  alignas(CACHE_LINE_SIZE) std::atomic<std::size_t> tail_;
  std::size_t cached_head_{0};
  char pad1_[
    CACHE_LINE_SIZE-
    sizeof(std::atomic<std::size_t>)-
    sizeof(std::size_t)];

  alignas(CACHE_LINE_SIZE) alignas(T) std::byte storage_[sizeof(T)*N];

  std::size_t increment(std::size_t idx) {
    return (idx+1) & (N-1);
  }

  T* slot(std::size_t idx) {
    return std::launder(reinterpret_cast<T*>(storage_+sizeof(T)*idx));
  }
};

} //namespace llob
