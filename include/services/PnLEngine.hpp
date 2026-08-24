#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

#include "domain/Position.hpp"
#include "interfaces/IMarketPriceProvider.hpp"

namespace tradeflow {

struct PnLReport {
    std::int64_t realized = 0;
    std::int64_t unrealized = 0;
    std::int64_t total = 0;
};

class PnLEngine {
public:
    explicit PnLEngine(const IMarketPriceProvider& priceProvider)
        : priceProvider_(priceProvider) {}

    std::int64_t calculateRealizedPnl(const Position& position) const {
        return position.realizedPnl;
    }

    std::int64_t calculateUnrealizedPnl(const Position& position,
                                        const std::string& symbol) const {
        const auto marketPrice = priceProvider_.getCurrentPrice(symbol);
        if (position.quantity <= 0) {
            return 0;
        }
        return (position.quantity * marketPrice) - (position.quantity * position.averagePrice);
    }

    PnLReport calculateTotalPnl(const Position& position,
                               const std::string& symbol) const {
        PnLReport report;
        report.realized = position.realizedPnl;
        report.unrealized = calculateUnrealizedPnl(position, symbol);
        report.total = report.realized + report.unrealized;
        return report;
    }

private:
    const IMarketPriceProvider& priceProvider_;
};

}  // namespace tradeflow
