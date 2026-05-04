#pragma once
#include "llob/Types.h"

namespace llob {

class OrderIdGenerator {
public:
  OrderIdGenerator()
    : last_id_(OrderId{}) {}

  OrderId next() {
    return ++last_id_;
  }

private:
  OrderId last_id_;
};

} // namespace llob
