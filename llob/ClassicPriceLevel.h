#pragma once 
#include <array>
#include "llob/Order.h"
#include "llob/Types.h"

namespace llob {
 
class ClassicPriceLevel {
public:
  ClassicPriceLevel() = default;
  ~ClassicPriceLevel() = default;
  void addOrder(Order*);
  void cancelOrder(OrderId);
  void matchFront(Quantity qty);
  const Order* getFrontOrder();

private:
  std::list<Order*> orders_;
};

};
