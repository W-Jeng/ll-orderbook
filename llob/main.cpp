#include <fmt/format.h>
#include "llob/Types.h"
#include "llob/Order.h"
#include "llob/PriceLevel.h"
#include "llob/OrderBook.h"
#include "llob/Dispatcher.h"
#include "llob/BookRegistry.h"
#include "llob/OrderEngine.h"

int main() {
  fmt::println("Hello world!");
  using namespace llob;
  using OrderBookT = ClassicOrderBook<ClassicPriceLevel, 1024>;
  using BookRegistryT = BookRegistry<OrderBookT>;
  using InlineDispatcherT = InlineDispatcher<BookRegistryT>;

  // Initialization
  BookRegistryT book_registry;
  auto order_book = std::make_unique<OrderBookT>(0);
  book_registry.add(0, std::move(order_book));
  InlineDispatcherT d(book_registry); 
  OrderEngine<InlineDispatcherT> order_engine(d);

  // Creating order command to test
  NewOrderRequest nor(0, Side::Buy, 100.0, 200);
  OrderCommand cmd(std::move(nor));
  order_engine.submit(cmd);

  return 0;
}
