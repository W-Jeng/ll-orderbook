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
};

struct NewOrderRequest {
  InstrumentId instrument_id;
  Side side;
  Price price;
  Quantity qty;
  OrderId allocated_order_id; // let OrderEngine fill this

  NewOrderRequest(InstrumentId iid, Side s,
                  Price p, Quantity q)
    : instrument_id(iid)
    , side(s)
    , price(p)
    , qty(q)
    , allocated_order_id(OrderId{}) { }
};

struct OrderCancelRequest {
  OrderId order_id; 
  InstrumentId instrument_id;
};

struct Order {
  OrderState state;
  OrderId id;
  InstrumentId instrument_id;
  Side side;
  Price price;
  Quantity qty;
  Quantity filled;
  uint16_t book_slot;

  Order()
    : state(OrderState::New)
    , id(0)
    , instrument_id(0)
    , side(Side::Buy)
    , price(Price{})
    , qty(Quantity{})
    , filled(Quantity{})
    , book_slot(0) {}

  void set_from(const NewOrderRequest& nor) {
    state = OrderState::New;
    id = nor.allocated_order_id;
    instrument_id = nor.instrument_id;
    side = nor.side;
    price = nor.price;
    qty = nor.qty;
    filled = 0;
  }
};

}; //end of namespace llob
