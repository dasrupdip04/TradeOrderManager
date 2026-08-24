#include <gtest/gtest.h>

#include "services/PositionManager.hpp"

using namespace tradeflow;

TEST(PositionManagerTest, FirstBuyCreatesPosition) {
    PositionManager manager;
    Position current{1, "RELIANCE", 0, 0, 0, std::chrono::system_clock::now()};

    auto updated = manager.applyBuy(current, 100, 1500);

    EXPECT_EQ(updated.quantity, 100);
    EXPECT_EQ(updated.averagePrice, 1500);
}

TEST(PositionManagerTest, MultipleBuysAveragePrice) {
    PositionManager manager;
    Position current{1, "RELIANCE", 100, 1500, 0, std::chrono::system_clock::now()};

    auto updated = manager.applyBuy(current, 100, 1600);

    EXPECT_EQ(updated.quantity, 200);
    EXPECT_EQ(updated.averagePrice, 1550);
}

TEST(PositionManagerTest, SellRealizesPnL) {
    PositionManager manager;
    Position current{1, "RELIANCE", 100, 1000, 0, std::chrono::system_clock::now()};

    auto updated = manager.applySell(current, 50, 1100);

    EXPECT_EQ(updated.quantity, 50);
    EXPECT_EQ(updated.realizedPnl, 5000);
}
