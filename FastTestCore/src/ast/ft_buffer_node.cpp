#include "ast/ft_buffer_node.hpp"
#include "ast/ft_reduce.hpp"
#include "exchange/exchange.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
BufferOpNode::BufferOpNode(Exchange &exchange, NodeType t, size_t warmup,
                           Option<NonNullPtr<ASTNode>> parent) noexcept
    : OpperationNode<void, LinAlg::EigenRef<LinAlg::EigenVectorXd>>(
          exchange, t, warmup, std::move(parent)) {}

//============================================================================
BufferOpNode::BufferOpNode(Exchange &exchange, NodeType t, size_t warmup,
                           Vector<NonNullPtr<ASTNode>> parent) noexcept 
  : OpperationNode<void, LinAlg::EigenRef<LinAlg::EigenVectorXd>>(
    exchange, t, warmup, std::move(parent)) {}

//============================================================================
BufferOpNode::~BufferOpNode() noexcept {}

//============================================================================
void BufferOpNode::enableCache(bool v) noexcept {
  size_t rows = m_exchange.getAssetCount();
  size_t cols = m_exchange.getTimestamps().size();
  if (v && m_cache.cols() != cols) {
    m_cache.resize(rows, cols);
    m_cache.setZero();
  } else if (!v && m_cache.cols() > 1) {
    m_cache.resize(0, 0);
  }
}

//============================================================================
LinAlg::EigenRef<LinAlg::EigenVectorXd>
BufferOpNode::cacheColumn(Option<size_t> col) noexcept {
  if (col) {
    assert(col.value() < static_cast<size_t>(m_cache.cols()));
    return m_cache.col(col.value());
  }
  if (m_cache.cols() > 1) {
    size_t col_idx = m_exchange.getCurrentIdx();
    return m_cache.col(col_idx);
  }
  if (m_cache.cols() == 0) {
    size_t rows = m_exchange.getAssetCount();
    m_cache.resize(rows, 1);
    m_cache.setZero();
    return m_cache.col(0);
  }
  return m_cache.col(0);
}


//============================================================================
size_t BufferOpNode::getAssetCount() const noexcept {
	return m_exchange.getAssetCount();
}

END_AST_NAMESPACE