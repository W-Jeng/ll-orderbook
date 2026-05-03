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

    o->setFrom(nor);
    order_indexer_[o->id] = o;
    auto add_to = [](auto& book, Order* order) {
      auto [it, _] = book.try_emplace(order->price, PriceLevelT(order->price));
      it->second.add(order);
    };

    if (o->side == Side::Buy)
      add_to(bids_, o);
    else
      add_to(asks_, o);

    tryMatchOrders();
  }

  void processCancelOrder(const OrderCancelRequest& ocr) {
    auto it = order_indexer_.find(ocr.order_id);

    if (it == order_indexer_.end())
      return;

    auto* o = it -> second;
    auto cancel_from = [](auto& book, Order* order) {
      auto price_level_it = book.find(order->price);
      
      if (price_level_it == book.end())
        return;

      price_level_it->second.erase(order);

      if (price_level_it->second.empty()) 
        book.erase(price_level_it);
    };

    if (o->side == Side::Buy)
      cancel_from(bids_, o);
    else
      cancel_from(asks_, o);

    order_indexer_.erase(it);
    order_pool_.release(o);
  }

  void tryMatchOrders() {
    auto match = [](Order* bid_o, Order* ask_o) {
      Quantity to_fill_qty = std::min(
        bid_o->qty - bid_o->filled,
        ask_o->qty - ask_o->filled);
      bid_o->fillQty(to_fill_qty);
      ask_o->fillQty(to_fill_qty);
    };

    auto popOrderIfFullyFilled = [](PriceLevelT& plevel, Order* o) {
      if (!o->isFullyFilled())
        return;

      plevel.erase(o);
    };

    auto popPriceLevelIfEmpty = [](auto& plevel_map) {
      auto best_plevel = plevel_map.begin();

      if (best_plevel->second.empty())
        plevel_map.erase(best_plevel);
    };

    auto releaseIfFilled = [&](Order* o) {
      if (!o->isFullyFilled())
        return;

      order_indexer_.erase(o->id);
      order_pool_.release(o);
    };

    while (true) {
      if (bids_.empty() || asks_.empty())
        return;
      
      auto bbid_plevel = bids_.begin();
      auto bask_plevel = asks_.begin();

      if (bbid_plevel->first < bask_plevel->first)
        return;

      // match happens here
      auto bbid_o = bbid_plevel->second.front();
      auto bask_o = bask_plevel->second.front();
      match(bbid_o, bask_o);

      // remove from price level, if fully filled
      popOrderIfFullyFilled(bbid_plevel->second, bbid_o);
      popOrderIfFullyFilled(bask_plevel->second, bask_o);
      
      // remove top level if empty
      popPriceLevelIfEmpty(bids_);
      popPriceLevelIfEmpty(asks_);

      // release usage
      releaseIfFilled(bbid_o);
      releaseIfFilled(bask_o);
    }
  }
};

}; // namespace llob
