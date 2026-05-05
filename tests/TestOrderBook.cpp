#include <gtest/gtest.h>
#include <memory>
#include <iostream>
#include "llob/PriceLevel.h"
#include "llob/OrderBook.h"
#include "llob/OrderCommand.h"

namespace llob {

class ClassicOrderBookTest : public ::testing::Test {
protected:
  ClassicOrderBookTest()
    : book(0) {}

  void SetUp() override {
    order_cmd_storage.reserve(1024);
  }

  OrderCommand* makeNewOrderCommand(OrderId id, Side s, Price p, Quantity q) {
    NewOrderRequest nor(0, s, p, q);
    nor.setOrderId(id);
    OrderCommand cmd(std::move(nor));
    order_cmd_storage.push_back(std::move(cmd));
    return &order_cmd_storage.back();
  }

  OrderCommand* makeCancelOrderCommand(OrderId id) {
    OrderCancelRequest ocq(0, id);
    OrderCommand cmd(std::move(ocq));
    order_cmd_storage.push_back(std::move(cmd));
    return &order_cmd_storage.back();
  }

  ClassicOrderBook<ClassicPriceLevel, 1024> book;
  std::vector<OrderCommand> order_cmd_storage;
};

TEST_F(ClassicOrderBookTest, EmptyOnConstruction) {
  EXPECT_FALSE(book.bestBid().has_value());
  EXPECT_FALSE(book.bestAsk().has_value());
  EXPECT_EQ(book.numLevels(Side::Buy), 0);
  EXPECT_EQ(book.numLevels(Side::Sell), 0);
}

TEST_F(ClassicOrderBookTest, NewBuyOrderUpdatesBestBid) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 10));
  EXPECT_EQ(book.bestBid(), 100.0);
  EXPECT_EQ(book.sizeAtPrice(Side::Buy, 100.0), 1);
}

TEST_F(ClassicOrderBookTest, MultiplePriceLevelsRanked) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 10));
  book.process(*makeNewOrderCommand(1, Side::Buy, 101.0, 10));
  book.process(*makeNewOrderCommand(2, Side::Buy, 99.0, 10));
  book.process(*makeNewOrderCommand(3, Side::Buy, 99.0, 20));
  EXPECT_EQ(book.bestBid(), 101.0);
  EXPECT_EQ(book.sizeAtPrice(Side::Buy, 99.0), 2);
  EXPECT_EQ(book.numLevels(Side::Buy), 3);
}

TEST_F(ClassicOrderBookTest, CancelRemovesOrder) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 10));
  EXPECT_TRUE(book.hasOrder(0));
  
  book.process(*makeCancelOrderCommand(0));
  EXPECT_FALSE(book.hasOrder(0));
  EXPECT_EQ(book.sizeAtPrice(Side::Buy, 100.0), 0);
}

TEST_F(ClassicOrderBookTest, CancelOneOfManyAtSamePrice) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 10));
  book.process(*makeNewOrderCommand(1, Side::Buy, 100.0, 20));
  book.process(*makeCancelOrderCommand(0));
  EXPECT_EQ(book.sizeAtPrice(Side::Buy, 100.0), 1);
  EXPECT_EQ(book.numLevels(Side::Buy), 1);
}

TEST_F(ClassicOrderBookTest, BidsAndAsks) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 10));
  book.process(*makeNewOrderCommand(1, Side::Sell, 101.0, 20));
  EXPECT_EQ(book.bestBid(), 100.0);
  EXPECT_EQ(book.bestAsk(), 101.0);
}

TEST_F(ClassicOrderBookTest, BidsAndAsksMultipleMatch) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 20));
  book.process(*makeNewOrderCommand(1, Side::Buy, 101.0, 50));
  book.process(*makeNewOrderCommand(2, Side::Sell, 99.0, 60));
  EXPECT_EQ(book.bestBid(), 100.0);
  EXPECT_FALSE(book.bestAsk().has_value());
}

TEST_F(ClassicOrderBookTest, BidsAndAsksMultipleMatchAndClear) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 20));
  book.process(*makeNewOrderCommand(1, Side::Sell, 99.0, 20));
  EXPECT_FALSE(book.bestBid().has_value());
  EXPECT_FALSE(book.bestAsk().has_value());
}

TEST_F(ClassicOrderBookTest, CancelUnknownOrder) {
  book.process(*makeCancelOrderCommand(0));
  EXPECT_EQ(book.numLevels(Side::Buy), 0);
}

class NodeBasedOrderBookTest : public ::testing::Test {
protected:
  NodeBasedOrderBookTest()
    : book(0) {}

  void SetUp() override {
    order_cmd_storage.reserve(1024);
  }

  OrderCommand* makeNewOrderCommand(OrderId id, Side s, Price p, Quantity q) {
    NewOrderRequest nor(0, s, p, q);
    nor.setOrderId(id);
    OrderCommand cmd(std::move(nor));
    order_cmd_storage.push_back(std::move(cmd));
    return &order_cmd_storage.back();
  }

  OrderCommand* makeCancelOrderCommand(OrderId id) {
    OrderCancelRequest ocq(0, id);
    OrderCommand cmd(std::move(ocq));
    order_cmd_storage.push_back(std::move(cmd));
    return &order_cmd_storage.back();
  }

  NodeBasedOrderBook<NodeBasedPriceLevel, 1024> book;
  std::vector<OrderCommand> order_cmd_storage;
};

TEST_F(NodeBasedOrderBookTest, EmptyOnConstruction) {
  EXPECT_FALSE(book.bestBid().has_value());
  EXPECT_FALSE(book.bestAsk().has_value());
  EXPECT_EQ(book.numLevels(Side::Buy), 0);
  EXPECT_EQ(book.numLevels(Side::Sell), 0);
}

TEST_F(NodeBasedOrderBookTest, NewBuyOrderUpdatesBestBid) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 10));
  EXPECT_EQ(book.bestBid(), 100.0);
  EXPECT_EQ(book.sizeAtPrice(Side::Buy, 100.0), 1);
}

TEST_F(NodeBasedOrderBookTest, MultiplePriceLevelsRanked) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 10));
  book.process(*makeNewOrderCommand(1, Side::Buy, 101.0, 10));
  book.process(*makeNewOrderCommand(2, Side::Buy, 99.0, 10));
  book.process(*makeNewOrderCommand(3, Side::Buy, 99.0, 20));
  EXPECT_EQ(book.bestBid(), 101.0);
  EXPECT_EQ(book.sizeAtPrice(Side::Buy, 99.0), 2);
  EXPECT_EQ(book.numLevels(Side::Buy), 3);
}

TEST_F(NodeBasedOrderBookTest, CancelRemovesOrder) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 10));
  EXPECT_TRUE(book.hasOrder(0));
  
  book.process(*makeCancelOrderCommand(0));
  EXPECT_FALSE(book.hasOrder(0));
  EXPECT_EQ(book.sizeAtPrice(Side::Buy, 100.0), 0);
}

TEST_F(NodeBasedOrderBookTest, CancelOneOfManyAtSamePrice) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 10));
  book.process(*makeNewOrderCommand(1, Side::Buy, 100.0, 20));
  book.process(*makeCancelOrderCommand(0));
  EXPECT_EQ(book.sizeAtPrice(Side::Buy, 100.0), 1);
  EXPECT_EQ(book.numLevels(Side::Buy), 1);
}

TEST_F(NodeBasedOrderBookTest, BidsAndAsks) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 10));
  book.process(*makeNewOrderCommand(1, Side::Sell, 101.0, 20));
  EXPECT_EQ(book.bestBid(), 100.0);
  EXPECT_EQ(book.bestAsk(), 101.0);
}

TEST_F(NodeBasedOrderBookTest, BidsAndAsksMultipleMatch) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 20));
  book.process(*makeNewOrderCommand(1, Side::Buy, 101.0, 50));
  book.process(*makeNewOrderCommand(2, Side::Sell, 99.0, 60));
  EXPECT_EQ(book.bestBid(), 100.0);
  EXPECT_FALSE(book.bestAsk().has_value());
}

TEST_F(NodeBasedOrderBookTest, BidsAndAsksMultipleMatchAndClear) {
  book.process(*makeNewOrderCommand(0, Side::Buy, 100.0, 20));
  book.process(*makeNewOrderCommand(1, Side::Sell, 99.0, 20));
  EXPECT_FALSE(book.bestBid().has_value());
  EXPECT_FALSE(book.bestAsk().has_value());
}

TEST_F(NodeBasedOrderBookTest, CancelUnknownOrder) {
  book.process(*makeCancelOrderCommand(0));
  EXPECT_EQ(book.numLevels(Side::Buy), 0);
}

} // namespace llob
