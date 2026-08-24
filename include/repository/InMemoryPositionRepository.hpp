#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "domain/Position.hpp"
#include "domain/RiskLimit.hpp"
#include "interfaces/IPositionRepository.hpp"

namespace tradeflow {

class InMemoryPositionRepository : public IPositionRepository {
public:
    std::optional<Position> getPosition(std::int64_t userId,
                                       const std::string& symbol) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto key = std::make_pair(userId, symbol);
        auto found = positions_.find(key);
        if (found == positions_.end()) {
            return std::nullopt;
        }
        return found->second;
    }

    std::vector<Position> listPositions(std::int64_t userId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        std::vector<Position> results;
        for (const auto& [key, value] : positions_) {
            if (userId != -1 && key.first != userId) {
                continue;
            }
            results.push_back(value);
        }
        return results;
    }

    void upsertPosition(const Position& position) override {
        std::lock_guard<std::mutex> lock(mutex_);
        positions_[std::make_pair(position.userId, position.symbol)] = position;
    }

    RiskLimit getRiskLimit(std::int64_t userId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        auto found = riskLimits_.find(userId);
        if (found == riskLimits_.end()) {
            return RiskLimit{userId, 1000, 1000, 1000000};
        }
        return found->second;
    }

    void upsertRiskLimit(const RiskLimit& riskLimit) override {
        std::lock_guard<std::mutex> lock(mutex_);
        riskLimits_[riskLimit.userId] = riskLimit;
    }

    bool userExists(std::int64_t userId) const override {
        std::lock_guard<std::mutex> lock(mutex_);
        return users_.find(userId) != users_.end();
    }

    void addUser(std::int64_t userId) {
        std::lock_guard<std::mutex> lock(mutex_);
        users_.insert(userId);
    }

private:
    mutable std::mutex mutex_;
    std::map<std::pair<std::int64_t, std::string>, Position> positions_;
    std::map<std::int64_t, RiskLimit> riskLimits_;
    std::set<std::int64_t> users_;
};

}  // namespace tradeflow
