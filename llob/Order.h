#pragma once
#include <llob/Types.h>

namespace llob {

enum class Side {
  Buy,
  Sell
};

enum class OrderType {
  New,
  Cancel
};

struct OrderRequest {
  OrderType type;
  InstrumentId instrument_id;
  Side side;
  Price price;
  Quantity qty;
};

struct Order {
  OrderType type;
  InstrumentId instrument_id;
  Side side;
  Price price;
  Quantity qty;
  uint32_t book_slot;
  Quantity filled = 0;
};

}; //end of namespace llob
