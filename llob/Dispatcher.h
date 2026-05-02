#pragma once
#include "llob/OrderCommand.h"
#include "llob/BookRegistry.h"

namespace llob {

template<typename T>
concept Dispatcher = requires(T d, const OrderCommand& cmd) {
  { d.submit(cmd) } -> std::same_as<void>;
};

template<typename BookRegistryT>
class InlineDispatcher {
public:
  explicit InlineDispatcher(BookRegistryT& registry)
    : registry_(registry) {}

  void submit(const OrderCommand& cmd) {
    auto instrument_id = cmd.type == CommandType::New 
                           ? cmd.new_order_request.instrument_id 
                           : cmd.order_cancel_request.instrument_id;
    registry_.get(instrument_id).process(cmd);
  }

private:
  BookRegistryT& registry_;
};

} // llob
