#pragma once
#include <optional>
#include <list>
#include <unordered_map>
#include "llob/Order.h"

namespace llob {

template<typename T>
concept PriceLevel = requires(T level, Order* o) {
  { level.size() } -> std::same_as<std::size_t>;
  { level.empty() } -> std::same_as<bool>;
  { level.add(o) } -> std::same_as<void>;
  { level.erase(o) } -> std::same_as<void>;
  { level.front() } -> std::same_as<Order*>;
};

class ClassicPriceLevel {
public:
  using Iterator = std::list<Order*>::iterator;

  explicit ClassicPriceLevel(Price p) : price_(p) {}
  std::size_t size() const { return orders_.size(); }
  bool empty() const { return orders_.empty(); }

  void add(Order* o) {
    orders_.push_back(o);
    order_location_[o->id] = std::prev(orders_.end());
  }

  void erase(Order* o) {
    auto it = order_location_.find(o->id);
    if (it == order_location_.end()) return;
    orders_.erase(it->second);
    order_location_.erase(it);
  }

  Order* front() const {
    return orders_.empty() ? nullptr : orders_.front();
  }

private:
  Price price_;
  std::list<Order*> orders_;
  std::unordered_map<OrderId, Iterator> order_location_;
};

} // namespace llob
