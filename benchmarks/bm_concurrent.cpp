#include <fmt/format.h>
#include <chrono>
#include <thread>
#include "llob/Types.h"
#include "llob/Order.h"
#include "llob/PriceLevel.h"
#include "llob/OrderBook.h"
#include "llob/Dispatcher.h"
#include "llob/BookRegistry.h"
#include "llob/OrderEngine.h"
#include "llob/SPSCQueue.h"

int main() {
  fmt::println("Hello world!");
  using namespace llob;
  using OrderBookT = NodeBasedOrderBook<NodeBasedPriceLevel, 1024>;
  using BookRegistryT = BookRegistry<OrderBookT>;
  using SPSCQueueT = SPSCQueue<OrderCommand, 2048>;
  using WorkerT = SPSCWorker<BookRegistryT, SPSCQueueT>;
  using WorkerManagerT = WorkerManager<OrderBookT, WorkerT>;
  using DispatcherT = ConcurrentDispatcher<WorkerManagerT>;

  // Initialization
  WorkerManagerT worker_manager(1);
  auto order_book0 = std::make_unique<OrderBookT>(0);
  auto order_book1 = std::make_unique<OrderBookT>(1);
  worker_manager.add(0, std::move(order_book0));
  worker_manager.add(0, std::move(order_book1));
  DispatcherT dispatcher(worker_manager);
  dispatcher.start();
  OrderEngine order_engine(dispatcher);

  // Creating order command to test
  NewOrderRequest nor_buy(0, Side::Buy, 100.0, 200);
  OrderCommand cmd_buy(std::move(nor_buy));
  order_engine.submit(cmd_buy);

  NewOrderRequest nor_sell(1, Side::Sell, 100.0, 100);
  OrderCommand cmd_sell(std::move(nor_sell));
  order_engine.submit(cmd_sell);

  std::this_thread::sleep_for(std::chrono::seconds(1));
  dispatcher.stop();
  return 0;
}
