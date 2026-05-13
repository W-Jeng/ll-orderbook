#pragma once
#include <limits>
#include "llob/Types.h"
#include "llob/PriceLevel.h"

namespace llob {

template<typename PriceLevelT, Side S>
class PriceArray {
public:
  PriceArray(Price min_price, Price tick_size, std::size_t N)
      : tick_size_(tick_size)
      , min_price_(min_price)
      , max_price_(min_price + (N-1) * tick_size)
      , best_idx(INVALID_IDX)
      , num_active_levels_(0)
  {}

  Price best() const {
    return priceAt(best_idx_);
  }

  PriceLevelT& bestPriceLevel() {
    return price_level_[best_idx_];
  }

  void updateBestOnInsertIdx(std::size_t idx) {
    if constexpr (S == Side::Buy) {
      if (idx > best_idx_ || best_idx_ == INVALID_IDX)
        best_idx_ = idx;
    } else {
      if (idx < best_idx_ || best_idx_ == INVALID_IDX)
        best_idx_ = idx;
    }
  }

  void updateBestOnEmptyIdx(std::size_t idx) {
    // does not influence best levels
    if (idx != best_idx_)
      return;

    if constexpr (S == Side::Buy) {
      // scan down
      while (best_idx_ > 0) {
        --best_idx_;
        if (!price_level_[best_idx_].empty())
          return;
      }

      if (price_level_[0].empty())
        best_idx_ = INVALID_IDX;
    } else {
      // scan up
      ++best_idx_;
      while (best_idx_ < price_level_.size()) {
        if (!price_level_[best_idx_].empty())
          return;
        ++best_idx_;
      }

      if (best_idx_ == price_level_.size())
        best_idx_ = INVALID_IDX;
    }
  }

  std::size_t index(Price p) const {
    return (p - min_price_)/tick_size_;
  }
  
  bool empty() const {
    return best_idx_ == INVALID_IDX;
  }

  Price priceAt(std::size_t i) const {
    return min_price_ + i * tick_size_;
  }

private:
  std::size_t best_idx_;
  Price min_price_;
  Price max_price_;
  Price tick_size_;
  std::vector<PriceLevel> price_levels_;
  static constexpr std::size_t INVALID_IDX = std::numeric_limits<std::size_t>::max();
};

};
