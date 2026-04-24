#pragma once
#include <map>
#include <vector>
#include "llob/types.h"

namespace llob {

class OrderBook {
 public:
  OrderBook(InstrumentId)
  void Process(const Order*);

 private:
  const InstrumentId instrument_id_;
  std::map<Price, std::vector

  void ProcessNewOrder(const Order*);
  void ProcessCancelOrder(const Order*);
};

}; // end of namespace llob
