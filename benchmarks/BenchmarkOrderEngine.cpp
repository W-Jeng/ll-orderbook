#include <vector>
#include <memory>
#include <exception>
#include <cmath>
#include <random>
#include <string>
#include <chrono>
#include <thread>
#include "llob/Types.h"
#include "llob/OrderIdGenerator.h"
#include "llob/OrderBook.h"
#include "llob/OrderEngine.h"
#include "llob/Order.h"
#include "llob/OrderCommand.h"
#include "llob/BookRegistry.h"
#include "llob/SPSCQueue.h"


namespace llob {

Price roundPrice(Price p, int decimal) {
  double val = std::pow(10, decimal);
  return std::round(p * val)/val;
}

std::vector<std::vector<OrderCommand>> makeWorkloadNoMatch(
    std::size_t n_commands,
    std::size_t max_live,
    std::uint32_t seed,
    uint16_t n_instruments) {
  std::uniform_real_distribution<Price> buy_dist(99.0, 99.99);
  std::uniform_real_distribution<Price> sell_dist(100.0, 100.99);
  std::uniform_int_distribution<Quantity> qty_dist(1, 100);
  std::bernoulli_distribution side_dist(0.5);
  std::bernoulli_distribution is_new(0.5);

  OrderIdGenerator id_generator;
  std::mt19937 rng(seed);

  std::vector<std::vector<OrderCommand>> cmds(n_instruments);
  for (auto& v : cmds) v.reserve(n_commands);

  std::vector<std::vector<OrderId>> active(n_instruments);

  for (std::size_t e = 0; e < n_commands; ++e) {
    for (uint16_t inst = 0; inst < n_instruments; ++inst) {
      auto& active_ids = active[inst];
      bool must_cancel = (active_ids.size() >= max_live);
      bool must_new = active_ids.empty();
      bool submit_new = must_new || (!must_cancel && is_new(rng));

      if (submit_new) {
        Side s = side_dist(rng) ? Side::Buy : Side::Sell;
        Price p = (s == Side::Buy) ? buy_dist(rng) : sell_dist(rng);
        p = roundPrice(p, 2);
        Quantity q = qty_dist(rng);
        NewOrderRequest nor(inst, s, p, q);
        cmds[inst].emplace_back(std::move(nor));
        active_ids.push_back(id_generator.next());
      } else {
        std::uniform_int_distribution<std::size_t> cancel_idx_dist(0, active_ids.size() - 1);
        std::size_t cancel_idx = cancel_idx_dist(rng);
        OrderCancelRequest ocq(active_ids[cancel_idx], inst);
        cmds[inst].emplace_back(std::move(ocq));
        active_ids[cancel_idx] = active_ids.back();
        active_ids.pop_back();
      }
    }
  }
  return cmds;
}

} // namespace llob

template<typename OrderEngineT>
void submitAllOrdersToEngine(OrderEngineT& order_engine, std::size_t n_cmds,
    std::vector<std::vector<llob::OrderCommand>>& cmds_vec, std::uint16_t n_instruments) {
  if (n_instruments > 1) {
    /*
     * submit to all instruments in every event round
     * array access-pattern is not cache friendly
     * but it simulates how real-life event submission works
     */
    for (std::size_t cmd_i = 0; cmd_i < n_cmds; ++cmd_i) {
      for (std::uint16_t inst_i = 0; inst_i < n_instruments; ++inst_i) {
        order_engine.submit(cmds_vec[inst_i][cmd_i]);     
      }
    }
  } else {
    for (std::size_t cmd_i = 0; cmd_i < n_cmds; ++cmd_i)
      order_engine.submit(cmds_vec[0][cmd_i]);     
  }
}

void summarizeRuntime(const std::string& name, auto& start, auto& end,
    std::size_t n_cmds, std::uint16_t n_instruments) {
  double secs = std::chrono::duration<double>(end-start).count();
  std::size_t total_commands = n_instruments * n_cmds;
  std::size_t commands_per_second = total_commands / secs;
  std::cout << "Benchmark for " << name << " completed in " 
    << std::round(secs*pow(10,5))/pow(10,5) << " seconds. " 
    << "Total operations/sec: " << commands_per_second << "\n";
}

int main(int argc, char** argv) {
  /*
   * Args:
   *  1) which: OrderEngine algorithm (classic or node) and (single or multithreaded)
   *  2) n_cmds: num of commands per instrument
   *  3) live: how many events an order can live through
   *  4) n_instruments: how many instruments we are distributing to
   *  5) n_workers: only for multithreaded usage
   */

  std::string which = (argc > 1) ? argv[1] : "all";
  std::size_t n_cmds = (argc > 2) ? std::stoul(argv[2]) : 1'000;
  std::size_t live = (argc > 3) ? std::stoul(argv[3]) : 500;
  std::uint16_t n_instruments = (argc > 4) ? std::stoul(argv[4]) : 1;
  std::uint8_t n_workers = (argc > 5) ? std::stoul(argv[5]) : 1;
  std::uint32_t seed = 42;

  if (live >= 1024)
    throw std::runtime_error("Order cannot live through more than 1024 events");

  auto cmds_vec = llob::makeWorkloadNoMatch(n_cmds, live, seed, n_instruments);

  if (which == "classic_s" || which == "all") {
    // Single threaded unoptimized version (classic)
    using OrderBookT = llob::ClassicOrderBook<llob::ClassicPriceLevel, 1024>;
    using BookRegistryT = llob::BookRegistry<OrderBookT>;
    using InlineDispatcherT = llob::InlineDispatcher<BookRegistryT>;
    BookRegistryT book_registry;

    for (std::uint16_t i = 0; i < n_instruments; ++i)
      book_registry.add(std::make_unique<OrderBookT>(i));

    InlineDispatcherT d(book_registry);
    llob::OrderEngine<InlineDispatcherT> order_engine(d);

    auto t0 = std::chrono::steady_clock::now();
    submitAllOrdersToEngine(order_engine, n_cmds, cmds_vec, n_instruments); 
    auto t1 = std::chrono::steady_clock::now();
    summarizeRuntime("Single threaded Classic Order Engine", t0, t1, n_cmds, n_instruments);
  }

  if (which == "node_s" || which == "all") {
    // Single threaded node-based order engine
    using OrderBookT = llob::NodeBasedOrderBook<llob::NodeBasedPriceLevel, 1024>;
    using BookRegistryT = llob::BookRegistry<OrderBookT>;
    using InlineDispatcherT = llob::InlineDispatcher<BookRegistryT>;
    BookRegistryT book_registry;

    for (std::uint16_t i = 0; i < n_instruments; ++i)
      book_registry.add(std::make_unique<OrderBookT>(i));

    InlineDispatcherT d(book_registry);
    llob::OrderEngine<InlineDispatcherT> order_engine(d);

    auto t0 = std::chrono::steady_clock::now();
    submitAllOrdersToEngine(order_engine, n_cmds, cmds_vec, n_instruments); 
    auto t1 = std::chrono::steady_clock::now();
    summarizeRuntime("Single threaded Node-based Order Engine", t0, t1, n_cmds, n_instruments);
  }

  if (which == "classic_m" || which == "all") {
    using namespace llob;
    using OrderBookT = ClassicOrderBook<ClassicPriceLevel, 1024>;
    using BookRegistryT = BookRegistry<OrderBookT>;
    using SPSCQueueT = SPSCQueue<OrderCommand, 2048>;
    using WorkerT = SPSCWorker<BookRegistryT, SPSCQueueT>;
    using WorkerManagerT = WorkerManager<OrderBookT, WorkerT>;
    using DispatcherT = ConcurrentDispatcher<WorkerManagerT>;

    // Initialization
    WorkerManagerT worker_manager(n_workers);
    
    // rotate, split evenly
    for (std::uint16_t i = 0; i < n_instruments; ++i)
      worker_manager.add(i % n_workers, std::make_unique<OrderBookT>(i));

    DispatcherT dispatcher(worker_manager);
    dispatcher.start();
    OrderEngine<DispatcherT> order_engine(dispatcher);
    
    // wait for thread to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto t0 = std::chrono::steady_clock::now();
    submitAllOrdersToEngine(order_engine, n_cmds, cmds_vec, n_instruments); 
    dispatcher.stop();
    auto t1 = std::chrono::steady_clock::now();
    summarizeRuntime("Multi Threaded Classic Order Engine", t0, t1, n_cmds, n_instruments);
    // std::cout << "Report:\n";
    // std::cout << order_engine.report() << "\n";
  }
  
  if (which == "node_m" || which == "all") {
    using namespace llob;
    using OrderBookT = NodeBasedOrderBook<NodeBasedPriceLevel, 1024>;
    using BookRegistryT = BookRegistry<OrderBookT>;
    using SPSCQueueT = SPSCQueue<OrderCommand, 2048>;
    using WorkerT = SPSCWorker<BookRegistryT, SPSCQueueT>;
    using WorkerManagerT = WorkerManager<OrderBookT, WorkerT>;
    using DispatcherT = ConcurrentDispatcher<WorkerManagerT>;

    // Initialization
    WorkerManagerT worker_manager(n_workers);
    
    // rotate, split evenly
    for (std::uint16_t i = 0; i < n_instruments; ++i)
      worker_manager.add(i % n_workers, std::make_unique<OrderBookT>(i));

    DispatcherT dispatcher(worker_manager);
    dispatcher.start();
    OrderEngine<DispatcherT> order_engine(dispatcher);
    
    // wait for thread to start
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    auto t0 = std::chrono::steady_clock::now();
    submitAllOrdersToEngine(order_engine, n_cmds, cmds_vec, n_instruments); 
    dispatcher.stop();
    auto t1 = std::chrono::steady_clock::now();
    summarizeRuntime("Multi Threaded Node-based Order Engine", t0, t1, n_cmds, n_instruments);
    // std::cout << "Report:\n";
    // std::cout << order_engine.report() << "\n";
  }
  
  return 0;
}
