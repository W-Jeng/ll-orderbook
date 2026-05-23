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
  using OrderBookT = ArrayInstrusiveOrderBook<256>;
  using BookRegistryT = BookRegistry<OrderBookT>;
  using InlineDispatcherT = InlineDispatcher<BookRegistryT>;

  // Initialization
  BookRegistryT book_registry;
  auto order_book = std::make_unique<OrderBookT>(0, 0, 1, 1024);
  book_registry.add(std::move(order_book));
  InlineDispatcherT d(book_registry); 
  OrderEngine<InlineDispatcherT> order_engine(d);

  // Creating order command to test
  NewOrderRequest nor_buy(0, Side::Buy, 100, 200);
  OrderCommand cmd_buy(std::move(nor_buy));
  order_engine.submit(cmd_buy);

  NewOrderRequest nor_sell(0, Side::Sell, 101, 100);
  OrderCommand cmd_sell(std::move(nor_sell));
  order_engine.submit(cmd_sell);

  return 0;
}
