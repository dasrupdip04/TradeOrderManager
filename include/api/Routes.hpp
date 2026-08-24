#pragma once

#include <string>

#include "crow.h"
#include "services/OrderService.hpp"

namespace tradeflow {

void registerRoutes(crow::SimpleApp& app, OrderService& orderService);

}  // namespace tradeflow
