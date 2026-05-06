#pragma once
#include <vector>
#include <memory>
#include "llob/Types.h"

namespace llob {

template<typename BookT>
class BookRegistry {
public:
  void add(std::unique_ptr<BookT> book) {
    InstrumentId id = book->getInstrumentId();
    if (books_.size() <= id) books_.resize(id+1);
    books_[id] = std::move(book);
  }

  BookT& get(InstrumentId id) { return *books_[id]; }

private:
  std::vector<std::unique_ptr<BookT>> books_;    
};

} // namespace llob 
