#pragma once
#include <optional>
#include "llob/Dispatcher.h"
#include "llob/OrderEngine.h"
#include "llob/OrderIdGenerator.h"

namespace llob {

template<Dispatcher DispatcherT>
class OrderEngine {
public:
  explicit OrderEngine(DispatcherT& d)
    : dispatcher_(d)
    , order_id_gen_(OrderIdGenerator{}) {}

  std::optional<OrderId> submit(OrderCommand& cmd) {
    std::optional<OrderId> assigned_id;

    if (cmd.type == CommandType::New) {
      assigned_id = order_id_gen_.next();
      cmd.new_order_request.setOrderId(*assigned_id);
    }

    dispatcher_.submit(cmd);
    return assigned_id;
  }

private:
  OrderIdGenerator order_id_gen_;
  DispatcherT& dispatcher_;
};

} // namespace llob
