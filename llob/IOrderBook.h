#pragma once
#include "llob/Order.h"

namespace llob {

class IOrderBook {
public:
  virtual process(const Order*) = 0;
  virtual ~IOrderBook() {}
};

} // end of llob
