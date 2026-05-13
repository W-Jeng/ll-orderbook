#pragma once
#include <thread>
#include <string>
#include <pthread.h>
#include <sched.h>
#include "llob/OrderCommand.h"
#include "llob/BookRegistry.h"

namespace llob {

template<typename T>
concept Dispatcher = requires(T d, const OrderCommand& cmd) {
  { d.submit(cmd) } -> std::same_as<void>;
  { d.report() } -> std::same_as<std::string>;
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
  
  std::string report() {
    return registry_.report();
  }

private:
  BookRegistryT& registry_;
};

template<typename BookRegistryT, typename SPSCQueueT>
struct alignas(64) SPSCWorker {
  SPSCQueueT command_q;
  BookRegistryT registry;
  uint8_t worker_id;
  std::thread t;
  std::atomic<bool> running{true};

  SPSCWorker(uint8_t w_id)
    : registry(BookRegistryT{})
    , worker_id(w_id) {}
  
  void submit(const OrderCommand& cmd) {
    while (!command_q.push(cmd))
      continue;
  }
  
  std::string report() {
    return registry.report();
  }
  
  void pin_thread() {
    if (!t.joinable())
      return;

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET((int) worker_id, &cpuset);
    int rc = pthread_setaffinity_np(t.native_handle(), sizeof(cpu_set_t), &cpuset);

    if (rc != 0)
      std::cout << "Unable to pin thread to a core\n";
  }

  void start() {
    running.store(true, std::memory_order_release);
    t = std::thread([this] { run(); });
    pin_thread();
  }

  void stop() {
    running.store(false, std::memory_order_release);
    if (t.joinable())
      t.join();
  }

  void run() {
    while (running.load(std::memory_order_acquire)) {
      while (true) {
        OrderCommand* cmd = command_q.front();
        if (!cmd) break;
        auto instrument_id = (cmd->type == CommandType::New)
             ? cmd->new_order_request.instrument_id 
             : cmd->order_cancel_request.instrument_id;
        registry.get(instrument_id).process(*cmd);
        command_q.pop();
      }
    }
  }

};

template<typename BookT, typename WorkerT>
class WorkerManager {
public:
  WorkerManager(uint8_t n_worker)
      : num_workers_(n_worker) 
  {
    for (uint8_t i = 0; i < n_worker; ++i)
      workers_.push_back(std::make_unique<WorkerT>(i));
  }

  void add(uint8_t worker_id, std::unique_ptr<BookT> book) {
    if (worker_id >= num_workers_)
      throw std::out_of_range("Worker id out of bound num_workers");
    
    InstrumentId id = book->getInstrumentId();
    workers_[worker_id]->registry.add(std::move(book));

    if (worker_lookup_.size() <= id)
      worker_lookup_.resize(id+1);
    
    worker_lookup_[id] = workers_[worker_id].get();
  }
  
  void submit(const OrderCommand& cmd) {
    auto instrument_id = (cmd.type == CommandType::New)
         ? cmd.new_order_request.instrument_id 
         : cmd.order_cancel_request.instrument_id;
    
    assert(instrument_id < worker_lookup_.size() &&
        "Invalid instrument id that exceeds lookup size");
    WorkerT* w = worker_lookup_[instrument_id];
    assert(w && "Null pointer for instrument id specified");
    w->submit(cmd);
  }

  void start() {
    for (auto& w: workers_)
      w->start();
  }

  void stop() {
    for (auto& w: workers_)
      w->stop();
  }
  
  std::string report() {
    std::string res = "";
    for (auto& w: workers_)
      res += w->report();
    return res;
  }

private:
  // index (instrument id) -> Worker* 
  uint8_t num_workers_;
  std::vector<WorkerT*> worker_lookup_;
  std::vector<std::unique_ptr<WorkerT>> workers_;
};

template<typename WorkerManagerT>
class ConcurrentDispatcher {
public:
  ConcurrentDispatcher(WorkerManagerT& w_manager)
    : worker_manager_(w_manager) {}

  void submit(const OrderCommand& cmd) {
    worker_manager_.submit(cmd);
  }

  std::string report() {
    return worker_manager_.report();
  }

  void start() {
    worker_manager_.start();
  }

  void stop() {
    worker_manager_.stop();
  }

private:
  WorkerManagerT& worker_manager_;
};

} // llob
