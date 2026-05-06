#include <vector>
#include <memory>
#include <exception>
#include <cmath>
#include <random>
#include <string>
#include "llob/Types.h"
#include "llob/OrderIdGenerator.h"
#include "llob/OrderBook.h"
#include "llob/OrderEngine.h"
#include "llob/Order.h"
#include "llob/OrderCommand.h"
#include "llob/BookRegistry.h"


namespace llob {

Price roundPrice(Price p, int decimal) {
  double val = std::pow(10, decimal);
  return std::round(p * val)/val;
}

std::vector<OrderCommand> makeWorkloadNoMatchSingle(
    std::size_t n_commands,
    std::size_t max_live,
    std::uint32_t seed,
    InstrumentId inst_id,
    OrderIdGenerator& id_generator) {
  // use the OrderIdGenerator that orderbook is using
  std::mt19937 rng(seed);
  std::uniform_real_distribution<Price> buy_dist(99.0, 99.99);
  std::uniform_real_distribution<Price> sell_dist(100.0, 100.99);
  std::uniform_int_distribution<Quantity> qty_dist(1, 100);
  std::bernoulli_distribution side_dist(0.5);
  std::bernoulli_distribution is_new(0.5);
  std::vector<OrderCommand> cmds;
  cmds.reserve(n_commands);
  std::vector<OrderId> active_order_ids;
  
  for (std::size_t i = 0; i < n_commands; ++i) {
    bool must_cancel = (active_order_ids.size() >= max_live);
    bool must_new = active_order_ids.empty();
    bool submit_new = must_new || (!must_cancel && is_new(rng));

    if (submit_new) {
      Side s = side_dist(rng) ? Side::Buy : Side::Sell;
      Price p = (s == Side::Buy) ? buy_dist(rng) : sell_dist(rng);
      p = roundPrice(p, 2);
      Quantity q = qty_dist(rng);
      NewOrderRequest nor(inst_id, s, p, q);
      OrderCommand cmd(std::move(nor));
      cmds.push_back(cmd);
      active_order_ids.push_back(id_generator.next());
    } else {
      std::uniform_int_distribution<std::size_t> cancel_idx_dist(0, active_order_ids.size()-1);
      std::size_t cancel_idx = cancel_idx_dist(rng);
      OrderCancelRequest ocq(active_order_ids[cancel_idx], inst_id);
      OrderCommand cmd(std::move(ocq));
      cmds.push_back(cmd);
      // swap with last, then pop last
      active_order_ids[cancel_idx] = active_order_ids.back();
      active_order_ids.pop_back();
    }
  }

  return cmds;
}

std::vector<std::vector<OrderCommand>> makeWorkloadNoMatch(
    std::size_t n_commands,
    std::size_t max_live,
    std::uint32_t seed,
    uint16_t n_instruments) {
  std::vector<std::vector<OrderCommand>> cmds;
  cmds.reserve(n_instruments);
  OrderIdGenerator id_generator;

  for (uint16_t i = 0; i < n_instruments; ++i)
    cmds.push_back(makeWorkloadNoMatchSingle(n_commands, max_live, seed, i, id_generator));

  return cmds;
}

} // namespace llob

int main(int argc, char** argv) {
  /*
   * Args:
   *  1) which: OrderBook construction algorithm (classic or node)
   *  2) n_cmds: num of commands per instrument
   *  3) live: how many events an order can live through
   *  4) n_instruments: how many instruments we are distributing to
   */

  std::string which = (argc > 1) ? argv[1] : "all";
  std::size_t n_cmds = (argc > 2) ? std::stoul(argv[2]) : 1'000;
  std::size_t live = (argc > 3) ? std::stoul(argv[3]) : 500;
  std::uint16_t n_instruments = (argc > 4) ? std::stoul(argv[4]) : 1;
  std::uint32_t seed = 42;

  if (live >= 1024)
    throw std::runtime_error("Order cannot live through more than 1024 events");

  auto cmds_vec = llob::makeWorkloadNoMatch(n_cmds, live, seed, n_instruments);

  if (which == "classic" || which == "all") {
    using OrderBookT = llob::ClassicOrderBook<llob::ClassicPriceLevel, 1024>;
    using BookRegistryT = llob::BookRegistry<OrderBookT>;
    using InlineDispatcherT = llob::InlineDispatcher<BookRegistryT>;
    BookRegistryT book_registry;

    for (std::uint16_t i = 0; i < n_instruments; ++i)
      book_registry.add(std::make_unique<OrderBookT>(i));

    InlineDispatcherT d(book_registry);
    llob::OrderEngine<InlineDispatcherT> order_engine(d);
    
    if (n_instruments > 1) {
      /*
       * submit to all instruments in every event round
       * array access-pattern is not cache friendly
       * but it simulates how real-life event submission works
       */
      for (std::size_t cmd_i; cmd_i < n_cmds; ++cmd_i) {
        for (std::uint16_t inst_i; inst_i < n_instruments; ++inst_i) {
          order_engine.submit(cmds_vec[inst_i][cmd_i]);     
        }
      }
    } else {
      for (std::size_t cmd_i; cmd_i < n_cmds; ++cmd_i)
        order_engine.submit(cmds_vec[0][cmd_i]);     
    }
  }

  if (which == "node" || which == "all") {

  }

  
  return 0;
}
