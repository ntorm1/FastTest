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

//============================================================================
BinOpNode::BinOpNode(Exchange &exchange, SharedPtr<BufferOpNode> left,
                     BinOpType op_type,
                     SharedPtr<BufferOpNode> right) noexcept
    : BufferOpNode(exchange, NodeType::BIN_OP,
                   std::max(left->getWarmup(), right->getWarmup()),
                   Vector<NonNullPtr<ASTNode>>({left.get(), right.get()})),
      m_asset_op_left(left), m_asset_op_right(right), m_op_type(op_type) {
  m_right_buffer.resize(getAssetCount());
  m_right_buffer.setZero();
}

//============================================================================
BinOpNode::~BinOpNode() noexcept {}

//============================================================================
void BinOpNode::reset() noexcept {
  m_asset_op_left->reset();
  m_asset_op_right->reset();
  m_right_buffer.setZero();
}

//============================================================================
void BinOpNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
#ifdef _DEBUG
  assert(target.rows() == static_cast<int>(getAssetCount()));
  assert(static_cast<size_t>(target.cols()) == 1);
#endif
  m_asset_op_left->evaluate(target);
  m_asset_op_right->evaluate(m_right_buffer);
  assert(target.size() == m_right_buffer.size());
  switch (m_op_type) {
  case BinOpType::ADD:
    target = target + m_right_buffer;
    break;
  case BinOpType::SUB:
    target = target - m_right_buffer;
    break;
  case BinOpType::MUL:
    target = target.cwiseProduct(m_right_buffer);
    break;
  case BinOpType::DIV:
    target = target.cwiseQuotient(m_right_buffer);
    break;
  }
}

END_AST_NAMESPACE
