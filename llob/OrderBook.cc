#include "llob/OrderBook.h"

namespace llob {

OrderBook::OrderBook(InstrumentId instrument_id) : instrument_id_(instrument_id) {}

OrderBook::process(const Order* order) {
  switch (order->type) {
    case OrderType::New:
      processNewOrder(order);
      break;

    case OrderType::Cancel:
      processCancelOrder(order);
      break;
  }
}

void OrderBook::processNewOrder(const Order* order) {
  
}

void OrderBook::processCancelOrder(const Order* order) {

}

} // end of llob
