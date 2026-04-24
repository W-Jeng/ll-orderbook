#pragma once
#include <llob/types.h>

namespace llob {

enum class Side {
  kBuy,
  kSell
};

enum class OrderType {
  kNew,
  kCancel
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
};

}; //end of namespace llob
