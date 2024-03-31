#include "ast/ft_observer.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
SumObserverNode::SumObserverNode(SharedPtr<BufferOpNode> parent, size_t window,
                                 Option<String> name) noexcept
    : ObserverNode(parent, ObserverType::SUM, window, name) {}

//============================================================================
SumObserverNode::~SumObserverNode() noexcept {}

//============================================================================
void SumObserverNode::onOutOfRange(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept {
  m_signal -= (buffer_old.array().isNaN()).select(0, buffer_old);
}

//============================================================================
void SumObserverNode::cacheObserver() noexcept {
  m_signal += (buffer().array().isNaN()).select(0, buffer());
  assert(!m_signal.array().isNaN().any());
}

//============================================================================
void SumObserverNode::reset() noexcept {}

END_AST_NAMESPACE
