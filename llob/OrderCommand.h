#pragma once
#include "llob/Order.h"

namespace llob {

enum class CommandType : uint8_t {
  New,
  Cancel
}

/*
 * Only for performance's sake, can consider std::variant
 */
struct alignas(64) OrderCommand {
  CommandType type;
  union {
    NewOrderRequest new_order_request;
    OrderCancelRequest order_cancel_request;
  }
}

};
