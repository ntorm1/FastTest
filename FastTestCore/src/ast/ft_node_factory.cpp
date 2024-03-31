#include "ast/ft_node_factory.hpp"
#include "ast/ft_asset_node.hpp"
#include "ast/ft_observer.hpp"
#include "ast/ft_reduce.hpp"
#include "exchange/exchange.hpp"

BEGIN_AST_NAMESPACE

struct NodeFactoryImpl {
  Exchange &m_exchange;
  NodeFactoryImpl(Exchange &exchange) noexcept : m_exchange(exchange) {}
};

//============================================================================
NodeFactory::NodeFactory(SharedPtr<Exchange> exchange) noexcept {
  assert(exchange);
  m_impl = std::make_unique<NodeFactoryImpl>(*exchange);
}

//============================================================================
NodeFactory::~NodeFactory() noexcept {}

//============================================================================
Option<SharedPtr<ReadOpNode>>
NodeFactory::createReadOpNode(String const &column, int row_offset) noexcept {
  auto col_opt = m_impl->m_exchange.getColumnIndex(column);
  if (!col_opt) {
    return std::nullopt;
  }
  if (row_offset > 0) {
    return std::nullopt;
  }
  return std::make_shared<ReadOpNode>(m_impl->m_exchange, col_opt.value(),
                                      row_offset);
}

//============================================================================
Option<SharedPtr<BinOpNode>>
NodeFactory::createBinOpNode(SharedPtr<BufferOpNode> left, BinOpType op,
                             SharedPtr<BufferOpNode> right) noexcept {
  if (!left || !right) {
    return std::nullopt;
  }
  return std::make_shared<BinOpNode>(m_impl->m_exchange, left, op, right);
}

//============================================================================
FASTTEST_API Option<SharedPtr<UnaryOpNode>>
NodeFactory::createUnaryOpNode(SharedPtr<BufferOpNode> parent, UnaryOpType op,
                               Option<double> op_param) noexcept {
  switch (op) {
  case UnaryOpType::SCALAR:
  case UnaryOpType::POWER:
    if (!op_param) {
      return std::nullopt;
    }
    break;
  default:
    break;
  }
  return std::make_shared<UnaryOpNode>(m_impl->m_exchange, parent, op,
                                       op_param);
}

//============================================================================
FASTTEST_API SharedPtr<ReduceOpNode> NodeFactory::createReduceOp(
    SharedPtr<BufferOpNode> node,
    Vector<std::pair<ReduceOpType, double>> filters) noexcept {
  return std::make_shared<ReduceOpNode>(node, std::move(filters));
}

//============================================================================
SharedPtr<ObserverNode>
NodeFactory::createSumObserverNode(SharedPtr<BufferOpNode> node, size_t window,
                                   Option<String> name) noexcept {
  return m_impl->m_exchange.registerObserver(
      std::make_shared<SumObserverNode>(node, window));
}

END_AST_NAMESPACE