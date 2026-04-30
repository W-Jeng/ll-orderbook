#pragma once
#include "llob/Dispatcher.h"

namespace llob {

template<Dispatcher DispatcherT>
class OrderEngine {
public:
  explicit OrderEngine(DispatcherT& d)
    : dispatcher(d) {}

  void submit(const OrderCommand& cmd) {
    dispatcher.submit(cmd);
  }

private:
  DispatcherT& dispatcher;
};

} // namespace llob
