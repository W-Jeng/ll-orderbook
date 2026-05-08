#pragma once
#include <map>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <optional>
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/container/flat_map.hpp>
#include "llob/Types.h"
#include "llob/Order.h"
#include "llob/OrderCommand.h"
#include "llob/PriceLevel.h"
#include "llob/PoolAllocator.h"

namespace llob {

template<typename T>
concept OrderBook = requires(T book, OrderCommand oc, Side s, Price p, OrderId id) {
  { book.process(oc) } -> std::same_as<void>;
  { book.getInstrumentId() } -> std::same_as<InstrumentId>;
  { book.bestBid() } -> std::same_as<std::optional<Price>>;
  { book.bestAsk() } -> std::same_as<std::optional<Price>>;
  { book.sizeAtPrice(s, p) } -> std::same_as<std::size_t>;
  { book.hasOrders(id) } -> std::same_as<bool>;
  { book.numLevels(s) } -> std::same_as<std::size_t>;
};

template<ListBasedPriceLevel PriceLevelT, size_t PoolSize>
class ClassicOrderBook {
public:
  explicit ClassicOrderBook(InstrumentId id)
      : instrument_id_(id) { }
  
  void process(const OrderCommand& command) {
    switch (command.type) {
      case CommandType::New:
        processNewOrder(command.new_order_request);
        break;

      case CommandType::Cancel:
        processCancelOrder(command.order_cancel_request);
        break;
    }
  }
  
  InstrumentId getInstrumentId() const {
    return instrument_id_;
  }

  std::optional<Price> bestBid() const {
    return bids_.empty() ? std::nullopt : std::optional{bids_.begin()->first};
  }
  
  std::optional<Price> bestAsk() const {
    return asks_.empty() ? std::nullopt : std::optional{asks_.begin()->first};
  }
  
  template<typename Book>
  static std::size_t sizeAtPriceImpl(const Book& book, Price p) {
    auto it = book.find(p);
    return (it == book.end()) ? 0 : it->second.size();
  }

  std::size_t sizeAtPrice(Side side, Price p) const {
    return (side == Side::Buy) ? sizeAtPriceImpl(bids_, p) : sizeAtPriceImpl(asks_, p);
  }

  bool hasOrder(OrderId id) const {
    return order_indexer_.contains(id);
  }

  std::size_t numLevels(Side side) const {
    return (side == Side::Buy) ? bids_.size() : asks_.size();
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
    order_indexer_.insert({o->id, o});
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

template<InstrusivePriceLevel PriceLevelT, size_t PoolSize>
class NodeBasedOrderBook {
public:
  explicit NodeBasedOrderBook(InstrumentId id)
      : instrument_id_(id) { }

  void process(const OrderCommand& command) {
    switch (command.type) {
      case CommandType::New:
        processNewOrder(command.new_order_request);
        break;

      case CommandType::Cancel:
        processCancelOrder(command.order_cancel_request);
        break;
    }
  }

  InstrumentId getInstrumentId() const {
    return instrument_id_;
  }

  std::optional<Price> bestBid() const {
    return bids_.empty() ? std::nullopt : std::optional{bids_.begin()->first};
  }

  std::optional<Price> bestAsk() const {
    return asks_.empty() ? std::nullopt : std::optional{asks_.begin()->first};
  }

  template<typename Book>
  static std::size_t sizeAtPriceImpl(const Book& book, Price p) {
    auto it = book.find(p);
    return (it == book.end()) ? 0 : it->second.size();
  }

  std::size_t sizeAtPrice(Side side, Price p) const {
    return (side == Side::Buy) ? sizeAtPriceImpl(bids_, p) : sizeAtPriceImpl(asks_, p);
  }

  bool hasOrder(OrderId id) const {
    return order_node_indexer_.contains(id);
  }

  std::size_t numLevels(Side side) const {
    return (side == Side::Buy) ? bids_.size() : asks_.size();
  }

private:
  const InstrumentId instrument_id_;
  boost::container::flat_map<Price, PriceLevelT, std::greater<Price>> bids_;
  boost::container::flat_map<Price, PriceLevelT, std::less<Price>> asks_;
  boost::unordered_flat_map<OrderId, OrderNode*> order_node_indexer_;
  PoolAllocator<OrderNode, PoolSize> order_pool_;

  void processNewOrder(const NewOrderRequest& nor) {
    OrderNode* o_node = order_pool_.allocate();

    if (!o_node) 
      throw std::runtime_error("Order pool exhausted! Last nor="+nor.toString());

    o_node->order.setFrom(nor);
    order_node_indexer_[o_node->order.id] = o_node;
    auto add_to = [](auto& book, OrderNode* order_node) {
      auto [it, _] = book.try_emplace(order_node->order.price,
                                      PriceLevelT(order_node->order.price));
      it->second.add(order_node);
    };

    if (o_node->order.side == Side::Buy)
      add_to(bids_, o_node);
    else
      add_to(asks_, o_node);

    tryMatchOrders();
  }

  void processCancelOrder(const OrderCancelRequest& ocr) {
    auto it = order_node_indexer_.find(ocr.order_id);

    if (it == order_node_indexer_.end())
      return;

    auto* o_node = it -> second;
    auto cancel_from = [](auto& book, OrderNode* order_node) {
      auto price_level_it = book.find(order_node->order.price);
      
      if (price_level_it == book.end())
        return;

      price_level_it->second.erase(order_node);

      if (price_level_it->second.empty()) 
        book.erase(price_level_it);
    };

    if (o_node->order.side == Side::Buy)
      cancel_from(bids_, o_node);
    else
      cancel_from(asks_, o_node);

    order_node_indexer_.erase(it);
    order_pool_.release(o_node);
  }

  void tryMatchOrders() {
    auto match = [](OrderNode* bid_o_node, OrderNode* ask_o_node) {
      Quantity to_fill_qty = std::min(
        bid_o_node->order.qty - bid_o_node->order.filled,
        ask_o_node->order.qty - ask_o_node->order.filled);
      bid_o_node->order.fillQty(to_fill_qty);
      ask_o_node->order.fillQty(to_fill_qty);
    };

    auto popOrderIfFullyFilled = [](PriceLevelT& plevel, OrderNode* o_node) {
      if (!o_node->order.isFullyFilled())
        return;

      plevel.erase(o_node);
    };

    auto popPriceLevelIfEmpty = [](auto& plevel_map) {
      auto best_plevel = plevel_map.begin();

      if (best_plevel->second.empty())
        plevel_map.erase(best_plevel);
    };

    auto releaseIfFilled = [&](OrderNode* o_node) {
      if (!o_node->order.isFullyFilled())
        return;

      order_node_indexer_.erase(o_node->order.id);
      order_pool_.release(o_node);
    };

    while (true) {
      if (bids_.empty() || asks_.empty())
        return;
      
      auto bbid_plevel = bids_.begin();
      auto bask_plevel = asks_.begin();

      if (bbid_plevel->first < bask_plevel->first)
        return;

      // match happens here
      auto bbid_o_node = bbid_plevel->second.front();
      auto bask_o_node = bask_plevel->second.front();
      match(bbid_o_node, bask_o_node);

      // remove from price level, if fully filled
      popOrderIfFullyFilled(bbid_plevel->second, bbid_o_node);
      popOrderIfFullyFilled(bask_plevel->second, bask_o_node);
      
      // remove top level if empty
      popPriceLevelIfEmpty(bids_);
      popPriceLevelIfEmpty(asks_);

      // release usage
      releaseIfFilled(bbid_o_node);
      releaseIfFilled(bask_o_node);
    }
  }
};

}; // namespace llob
