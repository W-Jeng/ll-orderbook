#include <thread>
#include <atomic>
#include <iostream>
#include <string>
#include <chrono>
#include "llob/PoolAllocator2.h"

struct A {
  A()
    : b(0) {}

  const int b;

  std::string toString() const {
    return "A(b=" + std::to_string(b) + ")";
  }
};

int main() {
  std::cout << "Start main\n";

  llob::Allocator<A, 10> allocator;
  std::vector<A*> mem_storage;
  
  for (int i = 0; i < 15; ++i) {
    A* a = allocator.allocate();
    if (a) {
      std::cout << "i: " << i << ", allocated mem address: " << a << std::endl;
      mem_storage.push_back(a);
    } else {
      std::cout << "i: " << i << ", nullptr u dummy!\n";
      if (!mem_storage.empty()) {
        allocator.release(mem_storage.back());
        mem_storage.pop_back();
      }
    }
  }


  std::cout << "End main\n";
}
