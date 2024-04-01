#include "exchange/exchange.hpp"
#include "ft_test_base.hpp"
#include "manager/ft_manager.hpp"
#include "strategy/ft_meta_strategy.hpp"

BEGIN_FASTTEST_NAMESPACE

class SimpleExchangeTests : public ::testing::Test {
protected:
  SharedPtr<FTManager> manager;
  SharedPtr<Exchange> exchange_ptr;
  SharedPtr<MetaStrategy> root;
  String exchange_id = "test";
  String portfolio_id = "test_p";
  String strategy_id = "test_s";
  double initial_cash = 100.0f;
  size_t asset_id_1;
  size_t asset_id_2;

  void SetUp() override {
    manager = std::make_shared<FTManager>();
    exchange_ptr =
        manager->addExchange(exchange_id, EXCHANGE_PATH, DATETIME_FORMAT)
            .value();
    asset_id_1 = exchange_ptr->getAssetIndex(ASSET1_NAME).value();
    asset_id_2 = exchange_ptr->getAssetIndex(ASSET2_NAME).value();
    manager->build();

    auto config = StrategyAllocatorConfig();
    root =
        std::make_shared<MetaStrategy>("root", exchange_ptr, config, STARTING_CASH, std::nullopt);
    auto root_opt = manager->addStrategy(root, false);
    assert(root_opt);
    root = *root_opt;
  }
};

END_FASTTEST_NAMESPACE