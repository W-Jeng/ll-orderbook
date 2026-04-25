#include "llob/PriceLevelClassic.h"

namespace llob {

void ClassicPriceLevel::addOrder(Order* order) {
  orders_.push_back(order);
}

void ClassicPriceLevel::cancelOrder(OrderId order_id) {

}

void ClassicPriceLevel::matchFront(Quantity qty) {

}

const Order* getFrontOrder() {

};

} // end of namespace llob
