#pragma once
#include "llob/OrderCommand.h"
#include "llob/BookRegistry.h"

namespace llob {

template<typename T>
concept Dispatcher = requires(T d, OrderCommand cmd) {
  { T.submit(cmd) } -> std::same_as<void>;
};

template<typename BookT>
class InlineDispatcher {
public:
  explicit InlineDispatcher(BookRegistry<BookT>& registry)
    : registry_(registry) {}

  void submit(const OrderCommand& cmd) {
    registry_.get(cmd.instrument_id).process(cmd);
  }

private:
  BookRegistry<BookT>& registry_;
};

} // llob
