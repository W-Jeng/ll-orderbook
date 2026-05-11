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

std::atomic<bool> running = true;

void worker(llob::SPSCQueue<A, 16>& q) {
  while (running) {
    A* a = q.front();
    if (a) {
      std::cout << a -> toString() << "\n";
      q.pop();
    }
  }
}

int main() {
  std::cout << "start of main!\n";
  llob::SPSCQueue<A, 16> q;
  running = true;
  std::thread t([&q]{
    worker(q);
  });

  for (int i = 0; i < 100; ++i) {
    while (!q.push(A{i}))
      continue;
  }
  
  std::this_thread::sleep_for(std::chrono::seconds(1));
  running = false;
  t.join();
  std::cout << "end of main!\n";
}
