#pragma once

#include <cstdint>
#include <string>

namespace tradeflow {

class IMarketPriceProvider {
public:
    virtual ~IMarketPriceProvider() = default;
    virtual std::int64_t getCurrentPrice(const std::string& symbol) const = 0;
};

}  // namespace tradeflow
