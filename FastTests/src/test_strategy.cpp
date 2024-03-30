#include "ast/ft_ast.hpp"
#include "simple_exchange.hpp"
#include "strategy/ft_benchmark.hpp"
#include "strategy/ft_meta_strategy.hpp"
#include "strategy/ft_tracer.hpp"

BEGIN_FASTTEST_NAMESPACE

TEST_F(SimpleExchangeTests, NewStrategyTest) {
  manager->build();
  auto config = StrategyAllocatorConfig();
  auto root = std::make_shared<MetaStrategy>("root", exchange_ptr, config,
                                             STARTING_CASH, std::nullopt);
  auto res = manager->addStrategy(std::move(root));
  EXPECT_TRUE(res.has_value());
}

TEST_F(SimpleExchangeTests, TestBenchMarkStrategy) {
  manager->build();
  auto config = StrategyAllocatorConfig();
  auto root = std::make_shared<MetaStrategy>("root", exchange_ptr, config,
                                             STARTING_CASH, std::nullopt);
  std::vector<std::pair<std::string, double>> allocations = {
      {ASSET1_NAME, 0.4}, {ASSET2_NAME, 0.6}};
  auto benchmark = std::make_shared<BenchMarkStrategy>(
      "benchmark", exchange_ptr, root, config, allocations, true);
  auto res = root->addStrategy(std::move(benchmark));
  EXPECT_TRUE(res.has_value());
  auto strategy = res.value();
  auto const& tracer = strategy->getTracer();
  manager->addStrategy(std::move(root));
  manager->step();
  auto const &buffer = strategy->getAllocationBuffer();
  EXPECT_EQ(buffer.size(), 2);
  EXPECT_EQ(buffer[asset_id_1], 0.4);
  EXPECT_EQ(buffer[asset_id_2], 0.6);
  EXPECT_EQ(tracer.getNLV(), STARTING_CASH);
  manager->step();
  auto returns = .6 * (ASSET2_CLOSE[1] - ASSET2_CLOSE[0]) / ASSET2_CLOSE[0];
  auto nlv = STARTING_CASH * (1 + returns);
  EXPECT_EQ(tracer.getNLV(), nlv);
  manager->step();
  returns = .4 * (ASSET1_CLOSE[1] - ASSET1_CLOSE[0]) / ASSET1_CLOSE[0] +
						.6 * (ASSET2_CLOSE[2] - ASSET2_CLOSE[1]) / ASSET2_CLOSE[1];
  nlv = nlv * (1 + returns);
  EXPECT_EQ(tracer.getNLV(), nlv);
}

END_FASTTEST_NAMESPACE