#pragma once
#include <string>
#include "llob/Order.h"


namespace llob {

enum class CommandType : uint8_t {
  New,
  Cancel
};

/*
 * Only for performance's sake, can consider std::variant
 */
struct OrderCommand {
  CommandType type;
  union {
    NewOrderRequest new_order_request;
    OrderCancelRequest order_cancel_request;
  };

  explicit OrderCommand(NewOrderRequest&& nor) noexcept
    : type(CommandType::New)
    , new_order_request(std::move(nor)) {}

  explicit OrderCommand(OrderCancelRequest&& ocq) noexcept
    : type(CommandType::Cancel)
    , order_cancel_request(std::move(ocq)) {}

  inline std::string toString() const {
    switch (type) {
      case CommandType::New:
        return new_order_request.toString();
      case CommandType::Cancel:
        return order_cancel_request.toString();
    }
    return "";
  }
};

};
