#include <gtest/gtest.h>

#include "services/PnLEngine.hpp"

using namespace tradeflow;

class FixedPriceProvider : public IMarketPriceProvider {
public:
    std::int64_t getCurrentPrice(const std::string& symbol) const override {
        (void)symbol;
        return 1100;
    }
};

TEST(PnLEngineTest, UnrealizedPnlForLongPosition) {
    FixedPriceProvider provider;
    PnLEngine engine(provider);
    Position position{1, "RELIANCE", 100, 1000, 0, std::chrono::system_clock::now()};

    auto pnl = engine.calculateUnrealizedPnl(position, "RELIANCE");

    EXPECT_EQ(pnl, 10000);
}

TEST(PnLEngineTest, RealizedPnlAndTotalPnl) {
    FixedPriceProvider provider;
    PnLEngine engine(provider);
    Position position{1, "RELIANCE", 100, 1000, 5000, std::chrono::system_clock::now()};

    auto report = engine.calculateTotalPnl(position, "RELIANCE");

    EXPECT_EQ(report.realized, 5000);
    EXPECT_EQ(report.unrealized, 10000);
    EXPECT_EQ(report.total, 15000);
}
