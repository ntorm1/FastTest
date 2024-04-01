#include "strategy/ft_ast_strategy.hpp"
#include "ast/ft_allocation.hpp"
#include "ast/ft_node_factory.hpp"
#include "strategy/ft_meta_strategy.hpp"

BEGIN_FASTTEST_NAMESPACE

//============================================================================
struct ASTStrategyImpl {
  SharedPtr<AST::AllocationNode> allocation_node;
  SharedPtr<AST::NodeFactory> factory;

  ASTStrategyImpl(NonNullPtr<ASTStrategy> strategy,
                  SharedPtr<AST::AllocationNode> allocation_node) noexcept
      : allocation_node(allocation_node) {
    factory = std::make_shared<AST::NodeFactory>(strategy);
  }
};

//============================================================================
ASTStrategy::ASTStrategy(String name, SharedPtr<StrategyAllocator> parent,
                         StrategyAllocatorConfig config) noexcept
    : StrategyAllocator(StrategyType::AST_STRATEGY, name, parent->getExchange(),
                        config, parent) {
  m_impl = std::make_unique<ASTStrategyImpl>(this, nullptr);
}

//============================================================================
ASTStrategy::~ASTStrategy() noexcept {}

//============================================================================
size_t ASTStrategy::getWarmup() const noexcept {
  if (!m_impl->allocation_node) {
    return 0;
  }
  return m_impl->allocation_node->getWarmup();
}

//============================================================================
void ASTStrategy::step(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept {
  if (!m_impl->allocation_node) {
    disable(std::format("no ast loaded for strategy: {} ", getName()).c_str());
  }
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
SharedPtr<AST::NodeFactory> ASTStrategy::getNodeFactory() const noexcept {
  return m_impl->factory;
}

//============================================================================
void ASTStrategy::reset() noexcept {}

//============================================================================
bool ASTStrategy::load() noexcept {
  if (!m_impl->allocation_node) {
    return false;
  }
  return true;
}

//============================================================================
void ASTStrategy::loadAST(SharedPtr<AST::AllocationNode> allocation) noexcept {
  m_impl->allocation_node = allocation;
}

END_FASTTEST_NAMESPACE
