#pragma once
#include <new>
#include <atomic>

static constexpr std::size_t CACHE_LINE_SIZE = 64;

namespace llob {

template<typename T, std::size_t Size>

class SPSCQueue3 {
static_assert(Size > 1, "Size must be larger than 1");
static_assert((Size & (Size-1)) == 0, "Size must be a power of 2 factor");

public:
  SPSCQueue3()
    : head_(0)
    , tail_(0) {}

  SPSCQueue3(const SPSCQueue3&) = delete;
  SPSCQueue3(SPSCQueue3&&) = delete;
  SPSCQueue3& operator=(const SPSCQueue3&) = delete;
  SPSCQueue3& operator=(SPSCQueue3&&) = delete;

  ~SPSCQueue3() {
    while (front())
      pop();
  }

  template<typename... Args>
  bool emplace(Args&&... args) {
    const std::size_t head = head_.load(std::memory_order_relaxed);
    std::size_t next = increment(head);
    
    // double check 
    if (next == cached_tail_) {
      cached_tail_ = tail_.load(std::memory_order_acquire);
      if (next == cached_tail_)
        return false;
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

  T* front() {
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
  std::size_t cached_head_{0};
  char pad0_[
    CACHE_LINE_SIZE
    -sizeof(std::atomic<std::size_t>)
    -sizeof(std::size_t)];

  alignas(CACHE_LINE_SIZE) std::atomic<std::size_t> tail_;
  std::size_t cached_tail_{0};
  char pad1_[
    CACHE_LINE_SIZE
    -sizeof(std::atomic<std::size_t>)
    -sizeof(std::size_t)];
  
  alignas(CACHE_LINE_SIZE) alignas(T) std::byte storage_[Size * sizeof(T)];

  std::size_t increment(std::size_t idx) {
    return (idx+1) & (Size-1);
  }

  T* slot(std::size_t idx) {
    return std::launder(reinterpret_cast<T*>(storage_ + idx*sizeof(T)));
  }
};

} // namespace llob
