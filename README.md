# ll-orderbook

A low-latency order book engine written in C++20, designed to explore the cost model of order book implementations from naive to highly optimized. Ships with multiple engine variants, an SPSC-based dispatcher for multi-threaded execution, and a benchmarking harness for direct comparison.

## Core Features

- **Zero allocation in the hot path.** All order nodes and price levels are pre-allocated via pool allocators at startup (Node and Array Intrusive Order Book). No `new`/`delete` occurs during order submission, cancellation, or matching. That eliminates allocator contention and giving predictable latency.
- **Multi-threaded with SPSC queues.** Orders are sharded across worker threads by instrument id, each owning its own bounded single-producer single-consumer queue. Lock-free, cache-line aligned, with cached head/tail to minimize cross-thread atomic loads.
- **Scales linearly with consumer threads.** Each worker is pinned to a distinct physical core. Throughput grows close to linearly with the number of consumers (measured up to 2 additional threads on a 6-core machine).
- **Three engine implementations for comparison:**
  - `ClassicOrderBook` — `std::map<Price, PriceLevel>` with `std::list` of orders. Baseline.
  - `NodeBasedOrderBook` — `boost::flat_map` with intrusive doubly-linked list nodes on the order book level.
  - `ArrayIntrusiveOrderBook` — Dense price array with pool-allocated intrusive nodes within the price level itself. Fastest.

## Dependencies

Install via your system package manager or [vcpkg](https://github.com/microsoft/vcpkg):

- CMake 3.20 or later
- A C++20-capable compiler (GCC 11+, Clang 13+)
- [fmt](https://github.com/fmtlib/fmt)
- [GoogleTest](https://github.com/google/googletest)
- [Google Benchmark](https://github.com/google/benchmark)
- [Boost](https://www.boost.org/) (components: `container`, `unordered`)

If using vcpkg:

```bash
vcpkg install fmt gtest benchmark boost-container boost-unordered
```

## Build

Debug build:

```bash
mkdir build && cd build
cmake -S .. -B . -G "Unix Makefiles" \
    -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

Release build (recommended for benchmarking):

```bash
mkdir build && cd build
cmake -S .. -B . -G "Unix Makefiles" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

## Linux Performance Settings

For consistent benchmark numbers, set the CPU governor to `performance` and relax `perf_event_paranoid` so `perf stat` can read hardware counters from userspace:

```bash
sudo cpupower frequency-set -g performance
sudo sysctl -w kernel.perf_event_paranoid=1
```

Optionally, make the perf setting persistent across reboots:

```bash
echo 'kernel.perf_event_paranoid=1' | sudo tee /etc/sysctl.d/99-perf.conf
```

## Running Tests

Tests are written with GoogleTest and cover the order book engines (matching, cancellation, edge cases) and the price level data structures:

```bash
cd build
./tests/llob_tests
```

## Running Benchmarks

```bash
./benchmarks/llob_bm <engine> <n_cmds> <live> <n_instruments> <n_workers>
```

Arguments:

- `engine` — which engine to run. Options: `classic_s`, `node_s`, `array_s` (single-threaded variants), `classic_m`, `node_m`, `array_m` (multi-threaded variants), or `all`
- `n_cmds` — number of order commands per instrument
- `live` — maximum simultaneously-live orders per instrument
- `n_instruments` — number of instruments to spread orders across
- `n_workers` — number of consumer threads (multi-threaded engines only)

Example:

```bash
./benchmarks/llob_bm array_m 50000000 500 4 2
```

With perf counters:

```bash
perf stat -d -d -d ./benchmarks/llob_bm array_m 50000000 500 4 2
```

## Performance

Benchmarks generate a workload of roughly **50% new orders and 50% cancellations** per instrument (capped by a configurable max-live ceiling). Buys are priced 9900–10010 and sells 9990–10099. The overlapping price range means roughly **20% of new orders exercise the matching engine** with potential fills.

The single-threaded variants (`*_s`) run the entire pipeline on one thread (no queueing). The multi-threaded variants (`*_m`) use **1 producer thread + N consumer threads**, communicating via per-worker SPSC queues. `n_workers=1` means 1 producer + 1 consumer (2 threads total).

Run with 50 million commands per instrument across 4 instruments (200 M total operations):

| Engine                  | Single-threaded (`*_s`) | Multi (1 consumer) | Multi (2 consumers) |
|-------------------------|------------------------:|-------------------:|--------------------:|
| Classic                 | 8.0 M ops/sec           | 6.7 M ops/sec      | 13.7 M ops/sec      |
| Node-based              | 16.2 M ops/sec          | 11.9 M ops/sec     | 24.6 M ops/sec      |
| Array-Intrusive         | 30.4 M ops/sec          | 19.4 M ops/sec     | 45.9 M ops/sec      |

**Scaling from 1 to 2 consumer threads** (multi-threaded):

- Classic: 6.7 → 13.7 M ops/sec (**2.06×**)
- Node-based: 11.9 → 24.6 M ops/sec (**2.07×**)
- Array-Intrusive: 19.4 → 45.9 M ops/sec (**2.46x**)

Near-linear scaling from 1 to 2 consumers across all engines, confirming the SPSC fan-out design holds up. The 1-consumer multi-threaded variant is slower than single-threaded because of the producer/consumer cross-core cache coherence cost; with 2 consumers, the added parallelism overcomes that overhead.

Correctness is verified by the GoogleTest suite (`./tests/llob_tests`), which exercises both single-threaded and multi-threaded paths across all three engine implementations. Benchmarks are run on a release build with CPU governor set to `performance`, consumer threads pinned to dedicated physical cores, and the producer pinned to a separate physical core to eliminate scheduling noise.

## Afterword
This is my second time writing a low-latency order book implementation. I made several design flaws in the first one, making it extremely hard to extend and test new features. The previous version also had a very obvious issue with scaling, i.e. adding 2x consumers didn't increase the throughput much at all. This project achieved everything that I wanted it to do; obviously not the perfect implementation, but a very good lesson. Till next time!