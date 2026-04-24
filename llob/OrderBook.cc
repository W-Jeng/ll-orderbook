#include "llob/OrderBook.h"

namespace llob {

OrderBook::OrderBook(InstrumentId instrument_id) : instrument_id_(instrument_id) {}

OrderBook::process(const Order* order) {
  switch (order->type) {
    case OrderType::kNew:
      ProcessNewOrder(order);
      break;

    case OrderType::kCancel:
      ProcessCancelOrder(order);
      break;
  }
}

OrderBook::processNewOrder(const Order* order) {
  
}

OrderBook::processNewOrder(const Order* order) {

}

} // end of llob
