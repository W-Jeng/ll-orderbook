#include <gtest/gtest.h>
#include <memory>
#include <iostream>
#include "llob/PriceLevel.h"

namespace llob {

class ClassicPriceLevelTest : public ::testing::Test {
protected:
  void SetUp() override {
    level = std::make_unique<ClassicPriceLevel>();
    order_storage.reserve(1024);
    internal_id = 0;
  }

  Order* makeOrder(OrderId id) {
    Order order{};
    NewOrderRequest nor(0, Side::Buy, 100.0, 10);
    nor.setOrderId(id);
    order.setFrom(nor);
    order_storage.push_back(order);
    return &order_storage.back();
  }

  void addOrders(int n) {
    for (int i = 0; i < n; ++i)
      level -> add(makeOrder(internal_id+i));
  }

  std::unique_ptr<ClassicPriceLevel> level;
  std::vector<Order> order_storage;
  std::size_t internal_id;
};

TEST_F(ClassicPriceLevelTest, EmptyOnConstruction) {
  EXPECT_EQ(level->size(), 0);
  EXPECT_TRUE(level->empty());
}

TEST_F(ClassicPriceLevelTest, AddOrders) {
  addOrders(3);
  EXPECT_EQ(level->size(), 3);
  EXPECT_FALSE(level->empty());
  EXPECT_EQ(level->front()->id, 0);
}

TEST_F(ClassicPriceLevelTest, EraseFrontOrders) {
  addOrders(3);
  level->erase(&order_storage[0]);
  EXPECT_EQ(level->size(), 2);
  EXPECT_EQ(level->front()->id, 1);
}

TEST_F(ClassicPriceLevelTest, EraseMiddleOrders) {
  addOrders(3);
  level->erase(&order_storage[1]);
  EXPECT_EQ(level->size(), 2);
  EXPECT_EQ(level->front()->id, 0);
}

TEST_F(ClassicPriceLevelTest, EraseLastOrders) {
  addOrders(3);
  level->erase(&order_storage[2]);
  EXPECT_EQ(level->size(), 2);
  EXPECT_EQ(level->front()->id, 0);
}

TEST_F(ClassicPriceLevelTest, EraseAllOrders) {
  addOrders(3);
  level->erase(&order_storage[0]);
  level->erase(&order_storage[1]);
  level->erase(&order_storage[2]);
  EXPECT_EQ(level->size(), 0);
  EXPECT_TRUE(level->empty());
  EXPECT_EQ(level->front(), nullptr);
}

TEST_F(ClassicPriceLevelTest, AddAfterEraseAppendsToBack) {
  addOrders(2);
  level->erase(&order_storage[0]);
  level->add(makeOrder(2));
  EXPECT_EQ(level->size(), 2);
  EXPECT_FALSE(level->empty());
  EXPECT_EQ(level->front()->id, 1);
  level->erase(&order_storage[1]);
  EXPECT_EQ(level->size(), 1);
  EXPECT_FALSE(level->empty());
  EXPECT_EQ(level->front()->id, 2);
}

class NodeBasedPriceLevelTest : public ::testing::Test {
protected:
  void SetUp() override {
    level = std::make_unique<NodeBasedPriceLevel>();
    order_storage.reserve(1024);
    internal_id = 0;
  }

  OrderNode* makeOrderNode(OrderId id) {
    Order order{};
    NewOrderRequest nor(0, Side::Buy, 100.0, 10);
    nor.setOrderId(id);
    order.setFrom(nor);
    OrderNode o_node;
    o_node.order = std::move(order);
    order_storage.push_back(o_node);
    return &order_storage.back();
  }

  void addOrders(int n) {
    for (int i = 0; i < n; ++i)
      level -> add(makeOrderNode(internal_id+i));
  }

  std::unique_ptr<NodeBasedPriceLevel> level;
  std::vector<OrderNode> order_storage;
  std::size_t internal_id;
};

TEST_F(NodeBasedPriceLevelTest, EmptyOnConsutrction) {
  EXPECT_EQ(level->size(), 0);
  EXPECT_TRUE(level->empty());
}

TEST_F(NodeBasedPriceLevelTest, AddOrders) {
  addOrders(3);
  EXPECT_EQ(level->size(), 3);
  EXPECT_FALSE(level->empty());
  EXPECT_EQ(level->front()->order.id, 0);
}

TEST_F(NodeBasedPriceLevelTest, EraseFrontOrders) {
  addOrders(3);
  level->erase(&order_storage[0]);
  EXPECT_EQ(level->size(), 2);
  EXPECT_EQ(level->front()->order.id, 1);
}

TEST_F(NodeBasedPriceLevelTest, EraseMiddleOrders) {
  addOrders(3);
  level->erase(&order_storage[1]);
  EXPECT_EQ(level->size(), 2);
  EXPECT_EQ(level->front()->order.id, 0);
}

TEST_F(NodeBasedPriceLevelTest, EraseLastOrders) {
  addOrders(3);
  level->erase(&order_storage[2]);
  EXPECT_EQ(level->size(), 2);
  EXPECT_EQ(level->front()->order.id, 0);
}

TEST_F(NodeBasedPriceLevelTest, EraseAllOrders) {
  addOrders(3);
  level->erase(&order_storage[0]);
  level->erase(&order_storage[1]);
  level->erase(&order_storage[2]);
  EXPECT_EQ(level->size(), 0);
  EXPECT_TRUE(level->empty());
  EXPECT_EQ(level->front(), nullptr);
}

TEST_F(NodeBasedPriceLevelTest, AddAfterEraseAppendsToBack) {
  addOrders(2);
  level->erase(&order_storage[0]);
  level->add(makeOrderNode(2));
  EXPECT_EQ(level->size(), 2);
  EXPECT_FALSE(level->empty());
  EXPECT_EQ(level->front()->order.id, 1);
  level->erase(&order_storage[1]);
  EXPECT_EQ(level->size(), 1);
  EXPECT_FALSE(level->empty());
  EXPECT_EQ(level->front()->order.id, 2);
}

} // namespace llob
