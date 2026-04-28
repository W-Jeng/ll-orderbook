#pragma once
#include <map>
#include <vector>
#include "llob/Types.h"
#include "llob/OrderCommand.h"
#include "llob/PriceLevel.h"

namespace llob {

template<typename T>
concept OrderBook = requires(T book, OrderCommand oc) {
  { book.process(oc) } -> std::same_as<void>;
};

template<PriceLevel PriceLevelT>
class ClassicOrderBook {
public:
  ClassicOrderBook(InstrumentId id):
      instrument_id_(id) {}

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

private:
  struct OrderInfo {
    Side side;
    Price price;
  };

  const InstrumentId instrument_id_;
  std::map<Price, PriceLevelT, std::greater<Price>> bids_;
  std::map<Price, PriceLevelT, std::less<Price>> asks_;
  std::unordered_map<OrderId, OrderInfo> order_price_loc_;

  void processNewOrder(const NewOrderRequest& nor) {
       
  }

  void processCancelOrder(const OrderCancelRequest& ocr) {
    auto it = order_price_loc_.find(ocr.id);
    if (it == order_price_loc_.end()) return;
    auto& [side, price] = it -> second;
    auto& book_side = (nor.side == Side::Buy) ? bids_ : asks;
    


  }
};

}; // end of namespace llob
