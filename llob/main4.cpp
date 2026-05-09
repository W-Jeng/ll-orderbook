#include <thread>
#include <atomic>
#include <iostream>
#include <string>
#include <chrono>
#include "llob/SPSCQueue.h"

struct A {
  const int b;

  std::string toString() const {
    return "A(b=" + std::to_string(b) + ")";
  }
};

std::atomic<bool> running = false;

void worker(llob::SPSCQueue<A, 16>& q) {
  while (running) {
    std::optional<A> popped = q.pop();

    if (popped.has_value())
      std::cout << (*popped).toString() << "\n";
  }
}

int main() {
  std::cout << "Start main\n";
  running.store(true);
  llob::SPSCQueue<A, 16> q;
  std::thread t([&q](){ 
      worker(q);
  });

  for (int i = 0; i < 100 ; ++i) {
    A a{i};
    while (!q.push(a))
      continue;
  }

  std::this_thread::sleep_for(std::chrono::seconds(1));
  running.store(false);
  t.join();
  std::cout << "End main\n";
}
