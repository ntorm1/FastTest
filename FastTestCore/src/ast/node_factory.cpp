#include "ast/ft_node_factory.hpp"
#include "ast/ft_asset_node.hpp"
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
  return std::make_shared<ReadOpNode>(m_impl->m_exchange, col_opt.value(), row_offset);
}

END_AST_NAMESPACE