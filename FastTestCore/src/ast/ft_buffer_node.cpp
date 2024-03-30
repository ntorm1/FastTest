#include "ast/ft_buffer_node.hpp"
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
size_t BufferOpNode::getAssetCount() const noexcept {
	return m_exchange.getAssetCount();
}

END_AST_NAMESPACE