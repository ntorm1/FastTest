#include "ft_test_base.hpp"

#include "exchange/Exchange.hpp"
#include "manager/ft_manager.hpp"

#include "ast/ft_ast.hpp"

BEGIN_FASTTEST_NAMESPACE

class SimpleExchangeTests : public ::testing::Test {
protected:
  SharedPtr<FTManager> manager;
  SharedPtr<Exchange> exchange_ptr;
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
  }
};

TEST_F(SimpleExchangeTests, BuildTest) {
  auto exchange_res = manager->getExchange("test");
  EXPECT_TRUE(exchange_res.has_value());
  auto exchange = exchange_res.value();
  auto const &timestamps = exchange->getTimestamps();
  for (size_t i = 0; i < timestamps.size(); ++i) {
    EXPECT_EQ(timestamps[i], TIMESTAMPS[i]);
  }
}

TEST_F(SimpleExchangeTests, ReadTest) {
  manager->build();
  auto factory = std::make_shared<AST::NodeFactory>(exchange_ptr);
  auto read_op = factory->createReadOpNode("close", 0);
  EXPECT_TRUE(read_op);
  manager->step();
  EXPECT_EQ(manager->getGlobalTime(), TIMESTAMPS[0]);
  Eigen::VectorXd temp = Eigen::VectorXd::Zero(2);
  read_op.value()->evaluate(temp);
  EXPECT_EQ(temp[asset_id_2], ASSET2_CLOSE[0]);
  EXPECT_NE(temp[asset_id_1],temp[asset_id_1]);

}

END_FASTTEST_NAMESPACE