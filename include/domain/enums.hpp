#pragma once

namespace tradeflow {

enum class Side {
    BUY,
    SELL
};

enum class OrderType {
    MARKET,
    LIMIT
};

enum class OrderStatus {
    NEW,
    ACCEPTED,
    REJECTED,
    CANCELLED,
    FILLED
};

inline const char* toString(Side side) {
    switch (side) {
        case Side::BUY: return "BUY";
        case Side::SELL: return "SELL";
    }
    return "UNKNOWN";
}

inline const char* toString(OrderType type) {
    switch (type) {
        case OrderType::MARKET: return "MARKET";
        case OrderType::LIMIT: return "LIMIT";
    }
    return "UNKNOWN";
}

inline const char* toString(OrderStatus status) {
    switch (status) {
        case OrderStatus::NEW: return "NEW";
        case OrderStatus::ACCEPTED: return "ACCEPTED";
        case OrderStatus::REJECTED: return "REJECTED";
        case OrderStatus::CANCELLED: return "CANCELLED";
        case OrderStatus::FILLED: return "FILLED";
    }
    return "UNKNOWN";
}

}  // namespace tradeflow
