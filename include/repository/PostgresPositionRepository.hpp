#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <pqxx/pqxx>

#include "domain/Position.hpp"
#include "domain/RiskLimit.hpp"
#include "interfaces/IPositionRepository.hpp"

namespace tradeflow {

class PostgresPositionRepository : public IPositionRepository {
public:
    explicit PostgresPositionRepository(pqxx::connection& connection)
        : connection_(connection) {}

    std::optional<Position> getPosition(std::int64_t userId,
                                       const std::string& symbol) const override {
        std::string query = "SELECT user_id, symbol, quantity, average_price, realized_pnl, updated_at "
                            "FROM positions WHERE user_id = " + std::to_string(userId) +
                            " AND symbol = '" + escapeSql(symbol) + "' LIMIT 1";
        pqxx::work txn(connection_);
        pqxx::result result = txn.exec(query);
        if (result.empty()) {
            return std::nullopt;
        }
        return mapPosition(result[0]);
    }

    std::vector<Position> listPositions(std::int64_t userId) const override {
        std::string query = "SELECT user_id, symbol, quantity, average_price, realized_pnl, updated_at FROM positions";
        if (userId != -1) {
            query += " WHERE user_id = " + std::to_string(userId);
        }
        query += " ORDER BY symbol ASC";
        pqxx::work txn(connection_);
        auto result = txn.exec(query);
        std::vector<Position> positions;
        for (const auto& row : result) {
            positions.push_back(mapPosition(row));
        }
        return positions;
    }

    void upsertPosition(const Position& position) override {
        pqxx::work txn(connection_);
        txn.exec("INSERT INTO positions (user_id, symbol, quantity, average_price, realized_pnl, updated_at) VALUES (" +
                 std::to_string(position.userId) + ", '" + escapeSql(position.symbol) + "', " +
                 std::to_string(position.quantity) + ", " + std::to_string(position.averagePrice) + ", " +
                 std::to_string(position.realizedPnl) + ", NOW()) ON CONFLICT (user_id, symbol) DO UPDATE SET " +
                 "quantity = EXCLUDED.quantity, average_price = EXCLUDED.average_price, realized_pnl = EXCLUDED.realized_pnl, updated_at = NOW()");
        txn.commit();
    }

    RiskLimit getRiskLimit(std::int64_t userId) const override {
        std::string query = "SELECT user_id, max_order_quantity, max_position_quantity, max_notional FROM risk_limits WHERE user_id = " +
                            std::to_string(userId) + " LIMIT 1";
        pqxx::work txn(connection_);
        pqxx::result result = txn.exec(query);
        if (result.empty()) {
            return RiskLimit{userId, 1000, 1000, 1000000};
        }
        RiskLimit limit;
        limit.userId = result[0]["user_id"].as<std::int64_t>();
        limit.maxOrderQuantity = result[0]["max_order_quantity"].as<std::int64_t>();
        limit.maxPositionQuantity = result[0]["max_position_quantity"].as<std::int64_t>();
        limit.maxNotional = result[0]["max_notional"].as<std::int64_t>();
        return limit;
    }

    void upsertRiskLimit(const RiskLimit& riskLimit) override {
        pqxx::work txn(connection_);
        txn.exec("INSERT INTO risk_limits (user_id, max_order_quantity, max_position_quantity, max_notional) VALUES (" +
                 std::to_string(riskLimit.userId) + ", " + std::to_string(riskLimit.maxOrderQuantity) + ", " +
                 std::to_string(riskLimit.maxPositionQuantity) + ", " + std::to_string(riskLimit.maxNotional) + ") "
                 "ON CONFLICT (user_id) DO UPDATE SET max_order_quantity = EXCLUDED.max_order_quantity, max_position_quantity = EXCLUDED.max_position_quantity, max_notional = EXCLUDED.max_notional");
        txn.commit();
    }

    bool userExists(std::int64_t userId) const override {
        pqxx::work txn(connection_);
        auto result = txn.exec("SELECT 1 FROM users WHERE id = " + std::to_string(userId) + " LIMIT 1");
        return !result.empty();
    }

private:
    static std::string escapeSql(const std::string& value) {
        std::string escaped;
        for (char ch : value) {
            if (ch == '\'') {
                escaped += "''";
            } else {
                escaped += ch;
            }
        }
        return escaped;
    }

    static Position mapPosition(const pqxx::row& row) {
        Position position;
        position.userId = row["user_id"].as<std::int64_t>();
        position.symbol = row["symbol"].as<std::string>();
        position.quantity = row["quantity"].as<std::int64_t>();
        position.averagePrice = row["average_price"].as<std::int64_t>();
        position.realizedPnl = row["realized_pnl"].as<std::int64_t>();
        position.updatedAt = std::chrono::system_clock::now();
        return position;
    }

    pqxx::connection& connection_;
};

}  // namespace tradeflow
