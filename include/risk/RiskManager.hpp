#pragma once

#include <algorithm>
#include <cstdint>
#include <string>

#include "interfaces/IRiskManager.hpp"

namespace tradeflow {

class RiskManager : public IRiskManager {
public:
    RiskCheckResult validate(const Order& order,
                             const RiskLimit& limit,
                             const Position& currentPosition,
                             bool userExists) const override {
        if (order.quantity <= 0) {
            return {false, "QUANTITY_MUST_BE_POSITIVE"};
        }
        if (order.orderType == OrderType::LIMIT && order.price <= 0) {
            return {false, "LIMIT_PRICE_MUST_BE_POSITIVE"};
        }
        if (order.symbol.empty()) {
            return {false, "SYMBOL_CANNOT_BE_EMPTY"};
        }
        if (!userExists) {
            return {false, "USER_DOES_NOT_EXIST"};
        }
        if (order.quantity > limit.maxOrderQuantity) {
            return {false, "ORDER_QUANTITY_EXCEEDS_LIMIT"};
        }

        const std::int64_t notional = order.quantity * order.price;
        if (notional > limit.maxNotional) {
            return {false, "NOTIONAL_EXCEEDS_LIMIT"};
        }

        if (order.side == Side::SELL) {
            if (order.quantity > currentPosition.quantity) {
                return {false, "SELL_QUANTITY_EXCEEDS_CURRENT_POSITION"};
            }
        } else if (order.side == Side::BUY) {
            const std::int64_t resultingPosition = currentPosition.quantity + order.quantity;
            if (resultingPosition > limit.maxPositionQuantity) {
                return {false, "RESULTING_POSITION_EXCEEDS_LIMIT"};
            }
        }

        if (order.side == Side::SELL && currentPosition.quantity == 0) {
            return {false, "SHORT_SELLING_NOT_ALLOWED"};
        }

        return {true, "OK"};
    }
};

}  // namespace tradeflow
