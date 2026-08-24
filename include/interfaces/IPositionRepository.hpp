#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "domain/Position.hpp"
#include "domain/RiskLimit.hpp"

namespace tradeflow {

class IPositionRepository {
public:
    virtual ~IPositionRepository() = default;

    virtual std::optional<Position> getPosition(std::int64_t userId,
                                               const std::string& symbol) const = 0;
    virtual std::vector<Position> listPositions(std::int64_t userId = -1) const = 0;
    virtual void upsertPosition(const Position& position) = 0;
    virtual RiskLimit getRiskLimit(std::int64_t userId) const = 0;
    virtual void upsertRiskLimit(const RiskLimit& riskLimit) = 0;
    virtual bool userExists(std::int64_t userId) const = 0;
};

}  // namespace tradeflow
