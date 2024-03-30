#include "ast/ft_asset_node.hpp"

#include "exchange/exchange.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
ReadOpNode::ReadOpNode(Exchange &exchange, size_t column,
                       int row_offset) noexcept
    : BufferOpNode(exchange, NodeType::ASSET_READ,
                   static_cast<size_t>(abs(row_offset)), std::nullopt),
      m_column(column), m_row_offset(row_offset) {}

//============================================================================
ReadOpNode::~ReadOpNode() noexcept {}

//============================================================================
void ReadOpNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  auto slice = m_exchange.getSlice(m_column, m_row_offset);
  assert(static_cast<size_t>(slice.rows()) == m_exchange.getAssetCount());
  assert(slice.rows() == target.rows());
  assert(static_cast<size_t>(slice.cols()) == 1);
  assert(static_cast<size_t>(target.cols()) == 1);
  target = slice;
}

END_AST_NAMESPACE