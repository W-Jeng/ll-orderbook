#pragma once
#include <llob/Types.h>

namespace llob {

enum class Side : uint8_t {
  Buy,
  Sell
};

enum class OrderState : uint8_t {
  New,
  PartiallyFilled,
  FullyFilled,
  Cancelled
}

struct NewOrderRequest {
  InstrumentId instrument_id;
  Side side;
  Price price;
  Quantity qty;
};

struct OrderCancelRequest {
  OrderId id; 
}

struct Order {
  OrderState state;
  OrderId id;
  InstrumentId instrument_id;
  Side side;
  Price price;
  Quantity qty;
  Quantity filled = 0;
  uint16_t book_slot;
};

}; //end of namespace llob
