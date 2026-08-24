#include <gtest/gtest.h>

#include "domain/Order.hpp"
#include "domain/Position.hpp"
#include "domain/RiskLimit.hpp"
#include "risk/RiskManager.hpp"

using namespace tradeflow;

TEST(RiskManagerTest, AcceptsValidBuyOrder) {
    RiskManager manager;
    RiskLimit limit{1, 1000, 1000, 1000000};
    Position current{1, "RELIANCE", 100, 1500, 0, std::chrono::system_clock::now()};
    Order order{1, 1, "RELIANCE", Side::BUY, OrderType::LIMIT, 100, 145050, OrderStatus::NEW, "id-1", std::chrono::system_clock::now(), std::chrono::system_clock::now()};

    auto result = manager.validate(order, limit, current, true);

    EXPECT_TRUE(result.allowed);
}

TEST(RiskManagerTest, RejectsZeroQuantity) {
    RiskManager manager;
    RiskLimit limit{1, 1000, 1000, 1000000};
    Position current{1, "RELIANCE", 0, 0, 0, std::chrono::system_clock::now()};
    Order order{1, 1, "RELIANCE", Side::BUY, OrderType::LIMIT, 0, 145050, OrderStatus::NEW, "id-2", std::chrono::system_clock::now(), std::chrono::system_clock::now()};

    auto result = manager.validate(order, limit, current, true);

    EXPECT_FALSE(result.allowed);
    EXPECT_EQ(result.reason, "QUANTITY_MUST_BE_POSITIVE");
}

TEST(RiskManagerTest, RejectsQuantityExceededLimit) {
    RiskManager manager;
    RiskLimit limit{1, 100, 1000, 1000000};
    Position current{1, "RELIANCE", 0, 0, 0, std::chrono::system_clock::now()};
    Order order{1, 1, "RELIANCE", Side::BUY, OrderType::LIMIT, 101, 145050, OrderStatus::NEW, "id-3", std::chrono::system_clock::now(), std::chrono::system_clock::now()};

    auto result = manager.validate(order, limit, current, true);

    EXPECT_FALSE(result.allowed);
}

TEST(RiskManagerTest, RejectsPositionExceededLimit) {
    RiskManager manager;
    RiskLimit limit{1, 1000, 500, 1000000};
    Position current{1, "RELIANCE", 400, 1000, 0, std::chrono::system_clock::now()};
    Order order{1, 1, "RELIANCE", Side::BUY, OrderType::LIMIT, 200, 1000, OrderStatus::NEW, "id-4", std::chrono::system_clock::now(), std::chrono::system_clock::now()};

    auto result = manager.validate(order, limit, current, true);

    EXPECT_FALSE(result.allowed);
    EXPECT_EQ(result.reason, "RESULTING_POSITION_EXCEEDS_LIMIT");
}

TEST(RiskManagerTest, RejectsSellExceedingPosition) {
    RiskManager manager;
    RiskLimit limit{1, 1000, 1000, 1000000};
    Position current{1, "RELIANCE", 100, 1000, 0, std::chrono::system_clock::now()};
    Order order{1, 1, "RELIANCE", Side::SELL, OrderType::LIMIT, 200, 1000, OrderStatus::NEW, "id-5", std::chrono::system_clock::now(), std::chrono::system_clock::now()};

    auto result = manager.validate(order, limit, current, true);

    EXPECT_FALSE(result.allowed);
    EXPECT_EQ(result.reason, "SELL_QUANTITY_EXCEEDS_CURRENT_POSITION");
}

TEST(RiskManagerTest, RejectsNotionalExceededLimit) {
    RiskManager manager;
    RiskLimit limit{1, 1000, 1000, 100000};
    Position current{1, "RELIANCE", 0, 0, 0, std::chrono::system_clock::now()};
    Order order{1, 1, "RELIANCE", Side::BUY, OrderType::LIMIT, 1000, 200000, OrderStatus::NEW, "id-6", std::chrono::system_clock::now(), std::chrono::system_clock::now()};

    auto result = manager.validate(order, limit, current, true);

    EXPECT_FALSE(result.allowed);
    EXPECT_EQ(result.reason, "NOTIONAL_EXCEEDS_LIMIT");
}
