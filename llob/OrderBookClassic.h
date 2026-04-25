#pragma once
#include <map>
#include <vector>
#include "llob/Types.h"
#include "llob/IOrderBook.h"

namespace llob {

class OrderBookClassic final : public IOrderBook {
public:
  OrderBook(InstrumentId)
  void process(const Order*);

private:
  const InstrumentId instrument_id_;
  std::map<Price, std::vector

  void processNewOrder(const Order*);
  void processCancelOrder(const Order*);
};

}; // end of namespace llob
