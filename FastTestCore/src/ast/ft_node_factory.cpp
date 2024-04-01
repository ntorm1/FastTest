#include "standard/ft_macros.hpp"
#include "ast/ft_node_factory.hpp"
#include "ast/ft_allocation.hpp"
#include "ast/ft_asset_node.hpp"
#include "ast/ft_observer.hpp"
#include "ast/ft_reduce.hpp"
#include "exchange/exchange.hpp"
#include "strategy/ft_ast_strategy.hpp"

BEGIN_AST_NAMESPACE

struct NodeFactoryImpl {
  Exchange &m_exchange;
  ASTStrategy &m_allocator;
  Vector<FastTestException> exceptions;
  NodeFactoryImpl(Exchange &exchange, ASTStrategy &allocator)
      : m_exchange(exchange), m_allocator(allocator) {}
};

//============================================================================
NodeFactory::NodeFactory(NonNullPtr<ASTStrategy> strategy) noexcept {
  m_impl =
      std::make_unique<NodeFactoryImpl>(strategy->getExchange(), *strategy);
}

//============================================================================
NodeFactory::~NodeFactory() noexcept {}

//============================================================================
Option<SharedPtr<ReadOpNode>>
NodeFactory::createReadOpNode(String const &column, int row_offset) noexcept {
  CHECK_EXCEPTIONS
  auto col_opt = m_impl->m_exchange.getColumnIndex(column);
  if (!col_opt) {
    ADD_FORMATED_EXCEPTION("column not found: {}", column);
    return std::nullopt;
  }
  if (row_offset > 0) {
    ADD_FORMATED_EXCEPTION("row offset greater than 0: {}", row_offset);
    return std::nullopt;
  }
  return std::make_shared<ReadOpNode>(m_impl->m_exchange, col_opt.value(),
                                      row_offset);
}

//============================================================================
Option<SharedPtr<BinOpNode>>
NodeFactory::createBinOpNode(SharedPtr<BufferOpNode> left, BinOpType op,
                             SharedPtr<BufferOpNode> right) noexcept {
  CHECK_EXCEPTIONS
  if (!left || !right) {
    ADD_FORMATED_EXCEPTION("{}", "left or right arg in bin op is null");
    return std::nullopt;
  }
  return std::make_shared<BinOpNode>(m_impl->m_exchange, left, op, right);
}

//============================================================================
FASTTEST_API Option<SharedPtr<UnaryOpNode>>
NodeFactory::createUnaryOpNode(SharedPtr<BufferOpNode> parent, UnaryOpType op,
                               Option<double> op_param) noexcept {
  CHECK_EXCEPTIONS
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

//============================================================================
Option<SharedPtr<AllocationNode>>
NodeFactory::createAllocationNode(SharedPtr<BufferOpNode> parent,
                                  AllocationType alloc_type, double epsilon,
                                  Option<double> alloc_param) noexcept {
  CHECK_EXCEPTIONS
  switch (alloc_type) {
  case AllocationType::CONDITIONAL_SPLIT: {
    if (!alloc_param) {
      ADD_FORMATED_EXCEPTION("{}", "conditional split missing alloc param");
      return std::nullopt;
    }
  }
  default:
    break;
  }
  auto &tracer = m_impl->m_allocator.getMutTracer();
  auto allocation =  std::make_shared<AllocationNode>(parent, &tracer, alloc_type, epsilon,
                                          alloc_param);
  m_impl->m_allocator.loadAST(allocation);
  return allocation;
}

END_AST_NAMESPACE