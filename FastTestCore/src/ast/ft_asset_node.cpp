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
bool ReadOpNode::isSame(NonNullPtr<BufferOpNode const> other) const noexcept {
  if (other->getType() != NodeType::ASSET_READ) {
    return false;
  }
  auto other_read = static_cast<ReadOpNode const *>(other.get());
  return other_read->m_column == m_column &&
         other_read->m_row_offset == m_row_offset;
}

//============================================================================
BinOpNode::BinOpNode(Exchange &exchange, SharedPtr<BufferOpNode> left,
                     BinOpType op_type, SharedPtr<BufferOpNode> right) noexcept
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
bool BinOpNode::isSame(NonNullPtr<BufferOpNode const> other) const noexcept {
  if (other->getType() != NodeType::BIN_OP) {
    return false;
  }
  auto other_bin = static_cast<BinOpNode const *>(other.get());
  return m_op_type == other_bin->m_op_type &&
         m_asset_op_left->isSame(other_bin->left()) &&
         m_asset_op_right->isSame(other_bin->right());
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

//============================================================================
UnaryOpNode::UnaryOpNode(Exchange &exchange, SharedPtr<BufferOpNode> parent,
                         UnaryOpType op_type,
                         Option<double> func_param) noexcept
    : BufferOpNode(exchange, NodeType::UNARY_OP, parent->getWarmup(),
                   Vector<NonNullPtr<ASTNode>>({parent.get()})),
      m_op_type(op_type), m_parent(parent), m_func_param(func_param) {}

//============================================================================
UnaryOpNode::~UnaryOpNode() noexcept {}

//============================================================================
void UnaryOpNode::reset() noexcept { m_parent->reset(); }

//============================================================================
bool UnaryOpNode::isSame(NonNullPtr<BufferOpNode const> other) const noexcept {
  if (other->getType() != NodeType::UNARY_OP) {
    return false;
  }
  auto other_unary = static_cast<UnaryOpNode const *>(other.get());
  return m_op_type == other_unary->getOpType() &&
         m_parent->isSame(other_unary->getParent());
}

//============================================================================
void UnaryOpNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  m_parent->evaluate(target);
  switch (m_op_type) {
  case UnaryOpType::ABS:
    target = target.array().abs();
    break;
  case UnaryOpType::SIGN:
    target = target.array().sign();
    break;
  case UnaryOpType::POWER:
    assert(m_func_param);
    target = target.array().pow(m_func_param.value());
    break;
  case UnaryOpType::SCALAR:
    assert(m_func_param);
    target = target.array() * m_func_param.value();
    break;
  case UnaryOpType::LOG:
    target = target.array().log();
    break;
  }
  if (hasCache())
    cacheColumn() = target;
}

END_AST_NAMESPACE
