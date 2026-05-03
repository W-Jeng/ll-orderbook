#pragma once
#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include "llob/Types.h"
#include "llob/Order.h"
#include "llob/OrderCommand.h"
#include "llob/PriceLevel.h"
#include "llob/PoolAllocator.h"

namespace llob {

template<typename T>
concept OrderBook = requires(T book, OrderCommand oc) {
  { book.process(oc) } -> std::same_as<void>;
};

template<PriceLevel PriceLevelT, size_t PoolSize>
class ClassicOrderBook {
public:
  explicit ClassicOrderBook(InstrumentId id)
      : instrument_id_(id) { }

  void process(const OrderCommand& command) {
    std::cout << "Processing command: " << command.toString() << "\n";
    switch (command.type) {
      case CommandType::New:
        processNewOrder(command.new_order_request);
        break;

      case CommandType::Cancel:
        processCancelOrder(command.order_cancel_request);
        break;
    }
  }

private:
  const InstrumentId instrument_id_;
  std::map<Price, PriceLevelT, std::greater<Price>> bids_;
  std::map<Price, PriceLevelT, std::less<Price>> asks_;
  std::unordered_map<OrderId, Order*> order_indexer_;
  PoolAllocator<Order, PoolSize> order_pool_;

  void processNewOrder(const NewOrderRequest& nor) {
    Order* o = order_pool_.allocate();

    if (!o) 
      throw std::runtime_error("Order pool exhausted! Last nor="+nor.toString());

    o->set_from(nor);
    order_indexer_[o->id] = o;

    if (o->side == Side::Buy) {
      auto [it, _] = bids_.try_emplace(o->price, PriceLevelT(o->price));
      it->second.add(o);
    } else {
      auto [it, _] = asks_.try_emplace(o->price, PriceLevelT(o->price));
      it->second.add(o);
    }
  }

  void processCancelOrder(const OrderCancelRequest& ocr) {
    auto it = order_indexer_.find(ocr.order_id);
    if (it == order_indexer_.end()) return;
    auto* o = it -> second;

    if (o->side == Side::Buy) {
      auto price_level_it = bids_.find(o->price);
      if (price_level_it != bids_.end()) 
        price_level_it->second.erase(o);
    } else {
      auto price_level_it = asks_.find(o->price);
      if (price_level_it != asks_.end()) 
        price_level_it->second.erase(o);
    }

    order_indexer_.erase(it);
    order_pool_.release(o);
  }
};

}; // namespace llob
