#include "ast/ft_observer_base.hpp"
#include "exchange/exchange.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
ObserverNode::ObserverNode(SharedPtr<BufferOpNode> parent,
                           ObserverType observer_type, size_t window,
                           Option<String> name) noexcept
    : BufferOpNode(parent->getExchange(), NodeType::ASSET_OBSERVER, window,
                   parent.get()),
      m_window(window), m_observer_warmup(window), m_name(name),
      m_observer_type(observer_type) {
  m_observer_warmup = parent->getWarmup();
  setWarmup(parent->getWarmup() + window - 1);
  m_parent = parent;
  m_buffer_matrix.resize(getAssetCount(), window);
  m_buffer_matrix.setZero();
  m_signal.resize(getAssetCount());
  m_signal.setZero();
  m_signal_copy.resize(getAssetCount());
  m_signal_copy.setZero();
}

//============================================================================
void ObserverNode::resetBase() noexcept {
  auto count = m_parent_observer_count.fetch_add(1, std::memory_order_relaxed);
  if (count < m_parent_observer_max) {
    return;
  }
  m_parent_observer_count.store(0, std::memory_order_relaxed);
  m_buffer_matrix.setZero();
  m_signal.setZero();
  m_signal_copy.setZero();
  m_buffer_idx = 0;
  m_parent_observer_count.store(0, std::memory_order_relaxed);
  reset();
  for (auto &child : m_children) {
    child->resetBase();
  }
}

//============================================================================
void ObserverNode::cacheBase() noexcept {
  // do an atomic increment on the parent observer count
  auto count = m_parent_observer_count.fetch_add(1, std::memory_order_relaxed);
  if (count < m_parent_observer_max) {
    return;
  }
  m_parent_observer_count.store(0, std::memory_order_relaxed);

  // check if the observer is in warmup
  if (m_exchange.getCurrentIdx() < m_observer_warmup) {
    return;
  }

  // evaluate parent of the observer into the cache buffer
  auto buffer_ref = buffer();
  m_parent->evaluate(buffer_ref);

  // execute observer logic on the buffer
  cacheObserver();

  // copy the buffer to the signal so that signal value is preserved before
  // out of range is called
  if (hasCache() && m_exchange.getCurrentIdx() >= (m_window - 1))
    cacheColumn() = m_signal;

  // call on out of range on the data that is about to be overwritten on the
  // next step
  m_buffer_idx = (m_buffer_idx + 1) % m_window;
  m_signal_copy = m_signal;
  if (m_exchange.getCurrentIdx() >= (m_window - 1))
    onOutOfRange(m_buffer_matrix.col(m_buffer_idx));

  for (auto &child : m_children) {
    child->cacheBase();
  }
}

//============================================================================
LinAlg::EigenRef<LinAlg::EigenVectorXd> ObserverNode::buffer() noexcept {
  size_t col = 0;
  assert(m_buffer_idx < static_cast<size_t>(m_buffer_matrix.cols()));
  return m_buffer_matrix.col(m_buffer_idx);
}

//============================================================================
bool ObserverNode::isSame(NonNullPtr<BufferOpNode const> other) const noexcept {
  if (other->getType() != getType()) {
    return false;
  }
  auto other_observer = static_cast<ObserverNode const *>(other.get());
  if (m_observer_type != other_observer->getObserverType()) {
    return false;
  }
  if (m_window != other_observer->getWindow()) {
    return false;
  }
  auto const &other_parents = other_observer->getParents();
  if (m_parent->getParents().size() != other_parents.size()) {
    return false;
  }
  for (size_t i = 0; i < m_parent->getParents().size(); ++i) {
    if (m_parent->getParents()[i] != other_parents[i]) {
      return false;
    }
  }
  return true;
}

//============================================================================
void ObserverNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  assert(target.size() == m_signal_copy.size());
  target = m_signal_copy;
}

END_AST_NAMESPACE