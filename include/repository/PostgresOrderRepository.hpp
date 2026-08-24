#pragma once

#include <chrono>
#include <cstdint>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <pqxx/pqxx>

#include "domain/Execution.hpp"
#include "domain/Order.hpp"
#include "interfaces/IOrderRepository.hpp"

namespace tradeflow {

class PostgresOrderRepository : public IOrderRepository {
public:
    explicit PostgresOrderRepository(pqxx::connection& connection)
        : connection_(connection) {}

    std::optional<Order> findById(std::int64_t orderId) const override {
        std::string query = "SELECT id, user_id, symbol, side, order_type, quantity, price, status, idempotency_key, created_at, updated_at "
                            "FROM orders WHERE id = " + std::to_string(orderId);
        pqxx::work txn(connection_);
        pqxx::result result = txn.exec(query);
        if (result.empty()) {
            return std::nullopt;
        }
        return mapOrder(result[0]);
    }

    std::optional<Order> findByIdempotencyKey(std::int64_t userId,
                                             const std::string& key) const override {
        std::string query = "SELECT id, user_id, symbol, side, order_type, quantity, price, status, idempotency_key, created_at, updated_at "
                            "FROM orders WHERE user_id = " + std::to_string(userId) +
                            " AND idempotency_key = '" + escapeSql(key) + "' LIMIT 1";
        pqxx::work txn(connection_);
        pqxx::result result = txn.exec(query);
        if (result.empty()) {
            return std::nullopt;
        }
        return mapOrder(result[0]);
    }

    std::vector<Order> listOrders(std::int64_t userId,
                                 const std::string& symbol) const override {
        std::string query = "SELECT id, user_id, symbol, side, order_type, quantity, price, status, idempotency_key, created_at, updated_at "
                            "FROM orders";
        if (userId != -1 || !symbol.empty()) {
            query += " WHERE ";
            bool needAnd = false;
            if (userId != -1) {
                query += "user_id = " + std::to_string(userId);
                needAnd = true;
            }
            if (!symbol.empty()) {
                if (needAnd) {
                    query += " AND ";
                }
                query += "symbol = '" + escapeSql(symbol) + "'";
            }
        }
        query += " ORDER BY created_at DESC";
        pqxx::work txn(connection_);
        pqxx::result result = txn.exec(query);
        std::vector<Order> orders;
        for (const auto& row : result) {
            orders.push_back(mapOrder(row));
        }
        return orders;
    }

    std::vector<Execution> listExecutionsForOrder(std::int64_t orderId) const override {
        std::string query = "SELECT id, order_id, executed_quantity, execution_price, executed_at "
                            "FROM executions WHERE order_id = " + std::to_string(orderId) +
                            " ORDER BY executed_at ASC";
        pqxx::work txn(connection_);
        pqxx::result result = txn.exec(query);
        std::vector<Execution> executions;
        for (const auto& row : result) {
            Execution execution;
            execution.id = row["id"].as<std::int64_t>();
            execution.orderId = row["order_id"].as<std::int64_t>();
            execution.executedQuantity = row["executed_quantity"].as<std::int64_t>();
            execution.executionPrice = row["execution_price"].as<std::int64_t>();
            execution.executedAt = parseTimestamp(row["executed_at"].as<std::string>());
            executions.push_back(execution);
        }
        return executions;
    }

    std::optional<Order> insert(const Order& order) override {
        pqxx::work txn(connection_);
        pqxx::result result = txn.exec(
            "INSERT INTO orders (user_id, symbol, side, order_type, quantity, price, status, idempotency_key, created_at, updated_at) "
            "VALUES (" + std::to_string(order.userId) + ", '" + escapeSql(order.symbol) + "', '" + toString(order.side) + "', '" + toString(order.orderType) + "', " +
            std::to_string(order.quantity) + ", " + std::to_string(order.price) + ", '" + toString(order.status) + "', '" + escapeSql(order.idempotencyKey) + "', NOW(), NOW()) "
            "RETURNING id, user_id, symbol, side, order_type, quantity, price, status, idempotency_key, created_at, updated_at");
        if (result.empty()) {
            return std::nullopt;
        }
        auto persisted = mapOrder(result[0]);
        txn.commit();
        return persisted;
    }

    bool insertExecution(const Execution& execution) override {
        pqxx::work txn(connection_);
        txn.exec("INSERT INTO executions (order_id, executed_quantity, execution_price, executed_at) VALUES (" +
                 std::to_string(execution.orderId) + ", " + std::to_string(execution.executedQuantity) + ", " +
                 std::to_string(execution.executionPrice) + ", NOW())");
        txn.commit();
        return true;
    }

    bool updateStatus(std::int64_t orderId, OrderStatus status) override {
        pqxx::work txn(connection_);
        pqxx::result result = txn.exec("UPDATE orders SET status = '" + std::string(toString(status)) + "', updated_at = NOW() WHERE id = " +
                                      std::to_string(orderId));
        txn.commit();
        return result.affected_rows() > 0;
    }

    bool cancel(std::int64_t orderId) override {
        return updateStatus(orderId, OrderStatus::CANCELLED);
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

    static std::chrono::system_clock::time_point parseTimestamp(const std::string& value) {
        // PostgreSQL timestamps are returned in ISO 8601 form.
        std::tm tm{};
        std::istringstream stream(value);
        stream >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
        if (!stream.fail()) {
            auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
            return tp;
        }
        return std::chrono::system_clock::now();
    }

    static Order mapOrder(const pqxx::row& row) {
        Order order;
        order.id = row["id"].as<std::int64_t>();
        order.userId = row["user_id"].as<std::int64_t>();
        order.symbol = row["symbol"].as<std::string>();
        std::string side = row["side"].as<std::string>();
        if (side == "BUY") {
            order.side = Side::BUY;
        } else {
            order.side = Side::SELL;
        }
        std::string type = row["order_type"].as<std::string>();
        if (type == "MARKET") {
            order.orderType = OrderType::MARKET;
        } else {
            order.orderType = OrderType::LIMIT;
        }
        order.quantity = row["quantity"].as<std::int64_t>();
        order.price = row["price"].as<std::int64_t>();
        std::string status = row["status"].as<std::string>();
        if (status == "NEW") {
            order.status = OrderStatus::NEW;
        } else if (status == "ACCEPTED") {
            order.status = OrderStatus::ACCEPTED;
        } else if (status == "REJECTED") {
            order.status = OrderStatus::REJECTED;
        } else if (status == "CANCELLED") {
            order.status = OrderStatus::CANCELLED;
        } else {
            order.status = OrderStatus::FILLED;
        }
        order.idempotencyKey = row["idempotency_key"].as<std::string>();
        order.createdAt = parseTimestamp(row["created_at"].as<std::string>());
        order.updatedAt = parseTimestamp(row["updated_at"].as<std::string>());
        return order;
    }

    pqxx::connection& connection_;
};

}  // namespace tradeflow
