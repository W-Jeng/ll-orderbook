#pragma once
#include <memory>
#include <type_traits>
#include <array>
#include "llob/Order.h"

namespace llob {

template<typename T, size_t N>
  requires std::is_default_constructible_v<T>
class PoolAllocator {
public:
  PoolAllocator()
      : storage_(std::make_unique<std::array<T,N>>())
  {
    free_list_.reserve(N);
    size_t i = N-1;
    while (true) {
      free_list_.push_back(i);
      if (i == 0)
          break;
      --i;
    }
  }

  PoolAllocator(PoolAllocator&&)=delete;
  PoolAllocator& operator=(PoolAllocator&&)=delete;

  [[nodiscard]] T* allocate() noexcept {
    if (free_list_.empty()) return nullptr;
    size_t idx = free_list_.back();
    free_list_.pop_back();
    return &(*storage_[idx]);
  }

  void release(T* p) noexcept {
    assert(p != nullptr);
    const T* base = storage_->data();
    assert(p >= base && p < base+N);
    const std::size_t idx = static_cast<std::size_t>(p-base);
    free_list_.push_back(idx);
  }

private:
  std::unique_ptr<std::array<T,N>> storage_;
  std::vector<std::size_t> free_list_;
};

} // namespace llob
