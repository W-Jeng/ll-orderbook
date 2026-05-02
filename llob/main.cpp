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
  using BookRegistryT = BookRegistry<ClassicOrderBook<ClassicPriceLevel, 1024>>;
  BookRegistryT book_registry;
  using InlineDispatcherT = InlineDispatcher<BookRegistryT>;
  InlineDispatcherT d(book_registry); 
  OrderEngine<InlineDispatcherT> order_engine(d);
  return 0;
}
