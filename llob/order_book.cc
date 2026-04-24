#include "llob/order_book.h"

namespace llob {

OrderBook::OrderBook(InstrumentId instrument_id) : instrument_id_(instrument_id) {}

OrderBook::Process(const Order* order) {
  switch (order->type) {
    case OrderType::kNew:
      ProcessNewOrder(order);
      break;

    case OrderType::kCancel:
      ProcessCancelOrder(order);
      break;
  }
}

OrderBook::ProcessNewOrder(const Order* order) {
  
}

OrderBook::ProcessNewOrder(const Order* order) {

}

} // end of llob
