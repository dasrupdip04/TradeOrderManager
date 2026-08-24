#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "execution/SimulatedExecutionEngine.hpp"
#include "infrastructure/Logger.hpp"
#include "repository/InMemoryOrderRepository.hpp"
#include "repository/InMemoryPositionRepository.hpp"
#include "risk/RiskManager.hpp"
#include "services/OrderService.hpp"

namespace {

class FixedPriceProvider : public tradeflow::IMarketPriceProvider {
public:
    std::int64_t getCurrentPrice(const std::string&) const override {
        return 1000;
    }
};

}  // namespace

int main() {
    tradeflow::InMemoryOrderRepository orderRepository;
    tradeflow::InMemoryPositionRepository positionRepository;
    positionRepository.addUser(1);
    positionRepository.upsertRiskLimit({1, 10000, 10000, 100000000});

    tradeflow::RiskManager riskManager;
    tradeflow::SimulatedExecutionEngine executionEngine(1000);
    tradeflow::PositionManager positionManager;
    tradeflow::Logger logger;
    FixedPriceProvider provider;
    tradeflow::PnLEngine pnlEngine(provider);
    tradeflow::OrderService orderService(orderRepository, positionRepository, riskManager, executionEngine, positionManager, pnlEngine, logger);

    const std::int64_t totalOperations = 5000;
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (std::int64_t i = 0; i < 8; ++i) {
        threads.emplace_back([&orderService, totalOperations]() {
            for (std::int64_t j = 0; j < totalOperations / 8; ++j) {
                tradeflow::OrderRequest request{1, "RELIANCE", tradeflow::Side::BUY, tradeflow::OrderType::MARKET,
                                              100, 1000, "bench-" + std::to_string(j) + "-" + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()))};
                orderService.createOrder(request);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
    const double opsPerSecond = static_cast<double>(totalOperations) * 1000.0 / static_cast<double>(std::max<int64_t>(1, elapsed));

    std::cout << "total_operations=" << totalOperations << " elapsed_ms=" << elapsed << " ops_per_second=" << opsPerSecond << std::endl;
    return 0;
}
