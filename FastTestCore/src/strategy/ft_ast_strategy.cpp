#include "strategy/ft_ast_strategy.hpp"
#include "strategy/ft_meta_strategy.hpp"
#include "ast/ft_allocation.hpp"

BEGIN_FASTTEST_NAMESPACE

//============================================================================
struct ASTStrategyImpl {
  SharedPtr<AST::AllocationNode> allocation_node;

  ASTStrategyImpl(SharedPtr<AST::AllocationNode> allocation_node) noexcept
			: allocation_node(allocation_node) {}
};

//============================================================================
ASTStrategy::ASTStrategy(
    String name,
    SharedPtr<StrategyAllocator> parent, StrategyAllocatorConfig config,
    SharedPtr<AST::AllocationNode> allocation_node) noexcept
    : StrategyAllocator(StrategyType::AST_STRATEGY, name,
                        allocation_node->getExchange(), config, parent) {
  m_impl = std::make_unique<ASTStrategyImpl>(allocation_node);
}

//============================================================================
ASTStrategy::~ASTStrategy() noexcept {}

//============================================================================
size_t ASTStrategy::getWarmup() const noexcept {
  return m_impl->allocation_node->getWarmup();
}

//============================================================================
void ASTStrategy::step(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept {
  m_impl->allocation_node->evaluate(target_weights_buffer);
}

//============================================================================
const Eigen::Ref<const Eigen::VectorXd>
ASTStrategy::getAllocationBuffer() const noexcept {
  auto parent = getParent();
  assert(parent);
  auto meta_strategy = static_cast<MetaStrategy *>(parent.value().get());
  return meta_strategy->getAllocationBuffer(this);
}


//============================================================================
void ASTStrategy::reset() noexcept {}

//============================================================================
void ASTStrategy::load() noexcept {}

END_FASTTEST_NAMESPACE
