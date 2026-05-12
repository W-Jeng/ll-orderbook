#pragma once
#include <limits>
#include "llob/Types.h"
#include "llob/PriceLevel.h"

namespace llob {

template<typename PriceLevelT, Side S>
class PriceArray {
public:
  PriceArray(Price mid, Price tick_size, std::size_t N)
      : tick_size_(tick_size)
      , min_price_(mid - (N/2) * tick_size)
      , max_price_(mid + (N/2-1) * tick_size)
      , best_idx(INVALID_IDX)
      , num_active_levels_(0)
  {}

  Price best() const {
    return priceAt(best_idx_);
  }

  PriceLevelT& bestPriceLevel() {
    return price_level_[best_idx_];
  }
  
  bool empty() const {
    return best_idx_ == INVALID_IDX;
  }

private:
  std::size_t index(Price p) const {
    return (p - min_price_)/tick_size_;
  }

  void rescanBest() {
    if (best_idx_ == INVALID_IDX || price_level_[best_idx_].empty()) {
      
    }
  }

  Price priceAt(std::size_t i) const {
    return min_price_ + i * tick_size_;
  }

  std::size_t best_idx_;
  Price min_price_;
  Price max_price_;
  Price tick_size_;
  std::vector<PriceLevel> price_levels_;
  static constexpr std::size_t INVALID_IDX = std::numeric_limits<std::size_t>::max();
};

};
