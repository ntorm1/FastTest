#include "ast/ft_ast.hpp"
#include "simple_exchange.hpp"
#include "strategy/ft_meta_strategy.hpp"

BEGIN_FASTTEST_NAMESPACE

TEST_F(SimpleExchangeTests, NewStrategyTest) {
  manager->build();
  auto config = StrategyAllocatorConfig();
  auto root = std::make_shared<MetaStrategy>("root", exchange_ptr, config,
                                             STARTING_CASH, std::nullopt);
  auto res = manager->addStrategy(std::move(root));
  EXPECT_TRUE(res.has_value());
}

END_FASTTEST_NAMESPACE