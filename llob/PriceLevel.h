#pragma once
#include <optional>
#include <list>
#include <unordered_map>
#include <iostream>
#include <boost/unordered/unordered_flat_map.hpp>
#include "llob/Order.h"
#include "llob/PoolAllocator.h"

namespace llob {

template<typename T>
concept ListBasedPriceLevel = requires(T level, Order* o) {
  { level.size() } -> std::same_as<std::size_t>;
  { level.empty() } -> std::same_as<bool>;
  { level.add(o) } -> std::same_as<void>;
  { level.erase(o) } -> std::same_as<void>;
  { level.front() } -> std::same_as<Order*>;
};

class ClassicPriceLevel {
public:
  using Iterator = std::list<Order*>::iterator;

  explicit ClassicPriceLevel() {}
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
  std::list<Order*> orders_;
  std::unordered_map<OrderId, Iterator> order_location_;
};


template<typename T>
concept InstrusivePriceLevel = requires(T level, OrderNode* o) {
  { level.size() } -> std::same_as<std::size_t>;
  { level.empty() } -> std::same_as<bool>;
  { level.add(o) } -> std::same_as<void>;
  { level.erase(o) } -> std::same_as<void>;
  { level.front() } -> std::same_as<OrderNode*>;
};

class NodeBasedPriceLevel {
public:
  explicit NodeBasedPriceLevel()
    : first_node_(nullptr)
    , last_node_(nullptr)
    , num_orders_(0) {}

  std::size_t size() const { return num_orders_; }
  bool empty() const { return num_orders_ == 0; }
  
  void add(OrderNode* o) {
    o -> next = nullptr;
    o -> prev = last_node_;

    if (first_node_) [[likely]] {
      last_node_ -> next = o;
    } else {
      first_node_ = o;
    }
    
    last_node_ = o;
    ++num_orders_;
  }

  void erase(OrderNode* o) {
    /*
     * Handle 4 cases
     * 1) o is first node
     * 2) o is last node
     * 3) o is first and last node
     * 4) o is middle
     */
    if (o->prev) [[likely]] {
      o->prev->next = o->next;
    } else {
      first_node_ = o->next;
    }

    if (o->next) [[likely]] {
      o->next->prev = o->prev;
    } else {
      last_node_ = o->prev;
    }

    --num_orders_;
  } 

  OrderNode* front() const {
    return first_node_;
  }

private:
  std::size_t num_orders_;
  OrderNode* first_node_;
  OrderNode* last_node_;
};

// Use pool allocator for each price level
template<std::size_t PoolSize>
class DensePriceLevel {
public:
  DensePriceLevel()
    : order_node_pool_(PoolAllocator<OrderNode, PoolSize>())
    , first_node_(order_node_pool_.allocate())
    , last_node_(order_node_pool_.allocate())
    , num_orders_(0) 
  {
    // always pointing to a valid node makes it easier
    first_node_->next = last_node_;
    last_node_->prev = first_node_;
  }

  std::size_t size() const { return num_orders_; }
  bool empty() const { return num_orders_ == 0; }
  
  [[nodiscard]] OrderNode* add(const NewOrderRequest& nor) {
    OrderNode* o = order_node_pool_.allocate();
    
    if (!o) [[unlikely]]
      throw std::runtime_error("Order pool exhausted! Last nor="+nor.toString());

    o->order.setFrom(nor);
    OrderNode* temp = last_node_->prev;
    o->next = last_node_;
    last_node_->prev = o;
    temp->next = o;
    o->prev = temp;
    ++num_orders_;
    return o;
  }

  void erase(OrderNode* o) {
    OrderNode* front = o->prev;
    OrderNode* back = o->next;
    front->next = back;
    back->prev = front;
    order_node_pool_.release(o);
    --num_orders_;
  } 

  OrderNode* front() const {
    return first_node_->next;
  }

private:
  PoolAllocator<OrderNode, PoolSize> order_node_pool_;
  OrderNode* first_node_;
  OrderNode* last_node_;
  std::size_t num_orders_;
};

} // namespace llob
