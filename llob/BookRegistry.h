#pragma once
#include <memory>

namespace llob {

template<typename BookT>
class BookRegistry {
public:
  void add(InstrumentId id, std::unique_ptr<BookT> book) {
    if (books_.size() <= id) books_.resize(id+1);
    books_[id] = std::move(book);
  }

  BookT& get(InstrumentId id) { return *books_[id]; }

private:
  std::vector<std::unique_ptr<BookT>> books_;    
};

} // namespace llob 
