
#include <benchmark/benchmark.h>
#include <vector>
#include <memory>
#include <exception>
#include <random>

namespace llob {
  
std::vector<OrderCommand> makeWorkloadNoMatch(
  std::size_t n_commands,
  std::size_t max_live,
  std::uint32_t seed = 42) {
  // have to design commands that wont live more than N turns in an orderbook
  std::uniform_real_distribution<Price> buy_dist(99.0, 99.99);
  std::uniform_real_distribution<Price> sell_dist(100.0, 100.99);
  std::uniform_int_distribution<Quantity> qty_dist(1, 100);
  std::bernoulli_distribution side_dist(0.5);
  std::bernoulli_distribution is_new(0.5);
  std::vector<OrderCommand> cmds;
  cmds.reserve(n_commands);
  std::vector<OrderId> activeOrderIds;
  
  for (std::size_t i = 0; i < n_commands; ++i) {
    bool must_cancel = (activeOrderIds.size() >= max_live);
    bool must_new = activeOrderIds.empty();
    bool submit_new = must_new || (!must_cancel && is_new(rng));

    if (submit_new) {
      Side s = side_dist(rng) ? Side::Buy : Side::Sell;
      Price p = (s == Side::Buy) ? buy_dist(rng) : sell_dist(rng)
      Quantity q = qty_dist(rng);
      

    } else {

    }
  }
}

template<typename BookT>
static void BM_OrderEngineSingleNoMatch(benchmark::State& state) {
  for (auto _ : state) {
    int x = 1;
    ++x;
    benchmark::DoNotOptimize(x);
  }
}

BENCHMARK(BM_Classic);

} // namespace llob
