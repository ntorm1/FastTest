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
  for (int i = 0; i <= 2; i++) {

    manager->build();
    auto factory = std::make_shared<AST::NodeFactory>(exchange_ptr);
    auto read_op = factory->createReadOpNode("close", 0);
    EXPECT_TRUE(read_op);
    manager->step();
    EXPECT_EQ(manager->getGlobalTime(), TIMESTAMPS[0]);
    Eigen::VectorXd temp = Eigen::VectorXd::Zero(2);
    read_op.value()->evaluate(temp);
    EXPECT_EQ(temp[asset_id_2], ASSET2_CLOSE[0]);
    EXPECT_NE(temp[asset_id_1], temp[asset_id_1]);
    manager->reset();
  }
}

TEST_F(SimpleExchangeTests, OpTest) {
  manager->build();
  auto factory = std::make_shared<AST::NodeFactory>(exchange_ptr);
  auto read_op = factory->createReadOpNode("close", 0).value();
  auto read_prev_op = factory->createReadOpNode("close", -1).value();
  auto delta_op =
      factory->createBinOpNode(read_op, AST::BinOpType::SUB, read_prev_op);
  EXPECT_TRUE(delta_op);
  auto &op = delta_op.value();
  EXPECT_EQ(op->getWarmup(), 1);
  for (int i = 0; i <= 3; i++) {
    manager->step();
  }
  Eigen::VectorXd temp = Eigen::VectorXd::Zero(2);
  op->evaluate(temp);
  EXPECT_EQ(temp[asset_id_2], ASSET2_CLOSE[3] - ASSET2_CLOSE[2]);
  EXPECT_EQ(temp[asset_id_1], ASSET1_CLOSE[2] - ASSET1_CLOSE[1]);
}

END_FASTTEST_NAMESPACE