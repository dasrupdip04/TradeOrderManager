#include <iostream>
#include <memory>

#include <crow.h>

#include "api/Routes.hpp"
#include "infrastructure/Database.hpp"
#include "execution/SimulatedExecutionEngine.hpp"
#include "infrastructure/Logger.hpp"
#include "repository/InMemoryOrderRepository.hpp"
#include "repository/InMemoryPositionRepository.hpp"
#include "repository/PostgresOrderRepository.hpp"
#include "repository/PostgresPositionRepository.hpp"
#include "risk/RiskManager.hpp"
#include "services/OrderService.hpp"
#include "services/PnLEngine.hpp"

namespace {

class SimpleMarketPriceProvider : public tradeflow::IMarketPriceProvider {
public:
    std::int64_t getCurrentPrice(const std::string& symbol) const override {
        if (symbol == "RELIANCE") {
            return 145050;
        }
        if (symbol == "TCS") {
            return 320000;
        }
        return 100000;
    }
};

}  // namespace

int main() {
    tradeflow::Logger logger;
    SimpleMarketPriceProvider marketProvider;
    tradeflow::PnLEngine pnlEngine(marketProvider);

    std::unique_ptr<tradeflow::IOrderRepository> orderRepository;
    std::unique_ptr<tradeflow::IPositionRepository> positionRepository;
    std::unique_ptr<pqxx::connection> databaseConnection;

    try {
        databaseConnection = std::make_unique<pqxx::connection>(tradeflow::Database::buildConnectionString());
        if (!databaseConnection->is_open()) {
            throw std::runtime_error("Unable to open PostgreSQL connection");
        }
        orderRepository = std::make_unique<tradeflow::PostgresOrderRepository>(*databaseConnection);
        positionRepository = std::make_unique<tradeflow::PostgresPositionRepository>(*databaseConnection);
        std::cout << "Connected to PostgreSQL." << std::endl;
    } catch (const std::exception& exc) {
        std::cout << "PostgreSQL unavailable, using in-memory repositories: " << exc.what() << std::endl;
        orderRepository = std::make_unique<tradeflow::InMemoryOrderRepository>();
        auto memoryPositionRepo = std::make_unique<tradeflow::InMemoryPositionRepository>();
        memoryPositionRepo->addUser(1);
        memoryPositionRepo->addUser(2);
        memoryPositionRepo->upsertRiskLimit({1, 1000, 1000, 1000000});
        memoryPositionRepo->upsertRiskLimit({2, 1000, 1000, 1000000});
        positionRepository = std::move(memoryPositionRepo);
    }

    tradeflow::RiskManager riskManager;
    tradeflow::SimulatedExecutionEngine executionEngine(145050);
    tradeflow::PositionManager positionManager;
    tradeflow::OrderService orderService(*orderRepository, *positionRepository, riskManager, executionEngine, positionManager, pnlEngine, logger);

    crow::SimpleApp app;
    tradeflow::registerRoutes(app, orderService);

    app.port(18080).multithreaded().run();
    return 0;
}
