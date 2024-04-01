#include "simple_exchange.hpp"

#include "ast/ft_ast.hpp"
#include "ast/ft_node_factory.hpp"
#include "ast/ft_observer.hpp"

/*
BEGIN_FASTTEST_NAMESPACE

TEST_F(SimpleExchangeTests, TestSumObserver) {
  auto factory = AST::NodeFactory(exchange_ptr);
  auto read_op = factory.createReadOpNode("close", 0);
  auto read_op_prev = factory.createReadOpNode("close", -1);
  EXPECT_TRUE(read_op);
  EXPECT_TRUE(read_op_prev);
  auto diff = factory.createBinOpNode(read_op.value(), AST::BinOpType::SUB,
                                      read_op_prev.value());
  auto sum = factory.createSumObserverNode(diff.value(), 2, "sum");
  auto diff_ptr = diff.value();
  EXPECT_TRUE(sum);
  EXPECT_TRUE(diff);
  EXPECT_EQ(diff_ptr->getWarmup(), 1);
  EXPECT_EQ(sum->getWarmup(), 2);
  EXPECT_EQ(sum->getParentObserverCount(), 0);
  manager->build();

  for (int i = 0; i < 4; i++) {
    manager->step();
    double sum1 = 0.0;
    double sum2 = 0.0;
    Eigen::VectorXd temp = Eigen::VectorXd::Zero(2);
    sum->evaluate(temp);
    EXPECT_EQ(temp(0), 0);
    EXPECT_EQ(temp(1), 0);

    manager->step();
    sum2 += (ASSET2_CLOSE[1] - ASSET2_CLOSE[0]);
    sum->evaluate(temp);
    EXPECT_EQ(temp(asset_id_2), sum2);

    manager->step();
    sum1 += ASSET1_CLOSE[1] - ASSET1_CLOSE[0];
    sum2 += ASSET2_CLOSE[2] - ASSET2_CLOSE[1];
    sum->evaluate(temp);
    EXPECT_EQ(temp(asset_id_1), sum1);
    EXPECT_EQ(temp(asset_id_2), sum2);

    manager->step();
    sum1 += ASSET1_CLOSE[2] - ASSET1_CLOSE[1];
    sum2 += ASSET2_CLOSE[3] - ASSET2_CLOSE[2];
    sum2 -= ASSET2_CLOSE[1] - ASSET2_CLOSE[0];
    sum->evaluate(temp);
    EXPECT_EQ(temp(asset_id_1), sum1);
    EXPECT_EQ(temp(asset_id_2), sum2);
    manager->reset();
  }
}

TEST_F(SimpleExchangeTests, TestChainedObserver) {
  auto factory = AST::NodeFactory(exchange_ptr);
  auto read_op = factory.createReadOpNode("close", 0);
  auto read_op_prev = factory.createReadOpNode("open", 0);
  auto diff = factory.createBinOpNode(read_op.value(), AST::BinOpType::SUB,
                                      read_op_prev.value());
  auto sum = factory.createSumObserverNode(diff.value(), 2, "sum1");
  auto sum2 = factory.createSumObserverNode(diff.value(), 2, "sum2");
  auto diff2 = factory.createBinOpNode(sum, AST::BinOpType::ADD,
																			 sum2);
  auto sum3 = factory.createSumObserverNode(diff2.value(), 2, "sum3");
  manager->build();
  EXPECT_EQ(sum3->getParentObserverCount(), 2);
  for (int j = 0; j < 10; j++) {
    for (int i = 0; i < 4; i++) {
      Eigen::VectorXd temp = Eigen::VectorXd::Zero(2);
      sum->evaluate(temp);
      double sum_value = temp.sum();
      sum2->evaluate(temp);
      double sum2_value = temp.sum();
      sum3->evaluate(temp);
      double sum3_value = temp.sum();
      EXPECT_EQ(sum3_value, sum_value + sum2_value);
    }
    manager->reset();
  }
}

END_FASTTEST_NAMESPACE
*/