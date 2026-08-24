#include "api/Routes.hpp"

#include <nlohmann/json.hpp>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "domain/enums.hpp"
#include "services/OrderService.hpp"

namespace tradeflow {
namespace {

crow::response jsonResponse(int status, const nlohmann::json& payload) {
    return crow::response(status, payload.dump());
}

Side parseSide(const std::string& value) {
    if (value == "BUY") {
        return Side::BUY;
    }
    if (value == "SELL") {
        return Side::SELL;
    }
    throw std::invalid_argument("Invalid side");
}

OrderType parseOrderType(const std::string& value) {
    if (value == "MARKET") {
        return OrderType::MARKET;
    }
    if (value == "LIMIT") {
        return OrderType::LIMIT;
    }
    throw std::invalid_argument("Invalid order type");
}

nlohmann::json toJson(const Order& order) {
    return {
        {"order_id", order.id},
        {"user_id", order.userId},
        {"symbol", order.symbol},
        {"side", toString(order.side)},
        {"order_type", toString(order.orderType)},
        {"quantity", order.quantity},
        {"price", order.price},
        {"status", toString(order.status)},
        {"idempotency_key", order.idempotencyKey},
        {"executed_quantity", order.quantity},
        {"execution_price", order.price}
    };
}

nlohmann::json toJson(const Position& position) {
    return {
        {"user_id", position.userId},
        {"symbol", position.symbol},
        {"quantity", position.quantity},
        {"average_price", position.averagePrice},
        {"realized_pnl", position.realizedPnl},
        {"updated_at", "now"}
    };
}

nlohmann::json toJson(const RiskLimit& limit) {
    return {
        {"user_id", limit.userId},
        {"max_order_quantity", limit.maxOrderQuantity},
        {"max_position_quantity", limit.maxPositionQuantity},
        {"max_notional", limit.maxNotional}
    };
}

}  // namespace

void registerRoutes(crow::SimpleApp& app, OrderService& orderService) {
    CROW_ROUTE(app, "/health")
        .methods(crow::HTTPMethod::GET)([&](const crow::request&) {
            return jsonResponse(200, { {"status", "ok"} });
        });

    CROW_ROUTE(app, "/api/orders")
        .methods(crow::HTTPMethod::POST)([&](const crow::request& req) {
            try {
                auto body = nlohmann::json::parse(req.body);
                OrderRequest request;
                request.userId = body.value("user_id", -1);
                request.symbol = body.value("symbol", "");
                request.side = parseSide(body.value("side", "BUY"));
                request.orderType = parseOrderType(body.value("order_type", "LIMIT"));
                request.quantity = body.value("quantity", 0);
                request.price = body.value("price", 0);
                request.idempotencyKey = body.value("idempotency_key", "");

                auto result = orderService.createOrder(request);
                if (!result.has_value()) {
                    return jsonResponse(422, {{"error", "BUSINESS_RULE"}, {"message", "Order rejected by business rules"}});
                }
                if (result->status == OrderStatus::REJECTED) {
                    return jsonResponse(422, {{"error", "RISK_LIMIT_EXCEEDED"}, {"message", "Order rejected by risk checks"}});
                }
                return jsonResponse(201, toJson(*result));
            } catch (const std::exception& exc) {
                return jsonResponse(400, {{"error", "INVALID_REQUEST"}, {"message", exc.what()}});
            }
        });

    CROW_ROUTE(app, "/api/orders")
        .methods(crow::HTTPMethod::GET)([&](const crow::request&) {
            auto orders = orderService.listOrders();
            nlohmann::json payload = nlohmann::json::array();
            for (const auto& order : orders) {
                payload.push_back(toJson(order));
            }
            return jsonResponse(200, payload);
        });

    CROW_ROUTE(app, "/api/orders/<int>")
        .methods(crow::HTTPMethod::GET)([&](const crow::request&, int orderId) {
            auto order = orderService.getOrder(orderId);
            if (!order.has_value()) {
                return jsonResponse(404, {{"error", "NOT_FOUND"}, {"message", "Order not found"}});
            }
            return jsonResponse(200, toJson(*order));
        });

    CROW_ROUTE(app, "/api/orders/<int>")
        .methods(crow::HTTPMethod::DELETE)([&](const crow::request&, int orderId) {
            bool ok = orderService.cancelOrder(orderId);
            if (!ok) {
                return jsonResponse(404, {{"error", "NOT_FOUND"}, {"message", "Order not found"}});
            }
            return jsonResponse(200, {{"status", "cancelled"}, {"order_id", orderId}});
        });

    CROW_ROUTE(app, "/api/positions")
        .methods(crow::HTTPMethod::GET)([&](const crow::request&) {
            auto positions = orderService.listPositions();
            nlohmann::json payload = nlohmann::json::array();
            for (const auto& position : positions) {
                payload.push_back(toJson(position));
            }
            return jsonResponse(200, payload);
        });

    CROW_ROUTE(app, "/api/positions/<string>")
        .methods(crow::HTTPMethod::GET)([&](const crow::request&, const std::string& symbol) {
            auto positions = orderService.listPositions();
            for (const auto& position : positions) {
                if (position.symbol == symbol) {
                    return jsonResponse(200, toJson(position));
                }
            }
            return jsonResponse(404, {{"error", "NOT_FOUND"}, {"message", "Position not found"}});
        });

    CROW_ROUTE(app, "/api/pnl")
        .methods(crow::HTTPMethod::GET)([&](const crow::request& req) {
            const auto query = req.url_params.get("user_id");
            const auto symbol = req.url_params.get("symbol");
            if (!query) {
                return jsonResponse(400, {{"error", "INVALID_REQUEST"}, {"message", "user_id query parameter is required"}});
            }
            std::int64_t userId = std::stoll(std::string(query));
            std::string symbolVal = symbol ? std::string(symbol) : "";
            auto report = orderService.getPnlReport(userId, symbolVal);
            return jsonResponse(200, {{"user_id", userId}, {"symbol", symbolVal}, {"realized", report.realized}, {"unrealized", report.unrealized}, {"total", report.total}});
        });

    CROW_ROUTE(app, "/api/risk")
        .methods(crow::HTTPMethod::GET)([&](const crow::request& req) {
            const auto userId = req.url_params.get("user_id");
            if (!userId) {
                return jsonResponse(400, {{"error", "INVALID_REQUEST"}, {"message", "user_id query parameter is required"}});
            }
            auto limit = orderService.getRiskLimit(std::stoll(std::string(userId)));
            return jsonResponse(200, toJson(limit));
        });

    CROW_ROUTE(app, "/api/risk/limits")
        .methods(crow::HTTPMethod::PUT)([&](const crow::request& req) {
            try {
                auto body = nlohmann::json::parse(req.body);
                RiskLimit limit;
                limit.userId = body.value("user_id", -1);
                limit.maxOrderQuantity = body.value("max_order_quantity", 0);
                limit.maxPositionQuantity = body.value("max_position_quantity", 0);
                limit.maxNotional = body.value("max_notional", 0);
                orderService.upsertRiskLimit(limit);
                return jsonResponse(200, toJson(limit));
            } catch (const std::exception& exc) {
                return jsonResponse(400, {{"error", "INVALID_REQUEST"}, {"message", exc.what()}});
            }
        });
}

}  // namespace tradeflow
