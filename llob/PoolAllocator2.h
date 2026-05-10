#pragma once

namespace llob {

template<typename T, std::size_t N>

class Allocator {
public:
  Allocator()
  : data_(std::make_unique<std::array<T, N>>())
  {
    free_list_.reserve(N);
    std::size_t idx = N-1;
    while (true) {
      free_list_.push_back(idx);
      if (idx == 0)
        break;
      --idx;
    }
  }

  Allocator(Allocator&&) = delete;
  Allocator& operator=(Allocator&&) = delete;

  [[nodiscard]] T* allocate() {
    if (free_list_.empty())
      return nullptr;

    std::size_t idx = free_list_.back();
    free_list_.pop_back();
    return &((*data_)[idx]);
  }

  void release(T* p) {
    std::size_t idx = p - data_->data();
    free_list_.push_back(idx);
  }

private:
  std::unique_ptr<std::array<T, N>> data_;
  std::vector<std::size_t> free_list_;
};

} // namespace llob
