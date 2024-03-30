#include "strategy/ft_meta_strategy.hpp"
#include "strategy/ft_tracer.hpp"

BEGIN_FASTTEST_NAMESPACE

//============================================================================
struct MetaStrategyImpl {
  Vector<SharedPtr<StrategyAllocator>> child_strategies;
  Map<String, size_t> strategy_map;
  LinAlg::EigenMatrixXd weights;
  LinAlg::EigenVectorXd meta_weights;
  LinAlg::EigenVectorXd child_strategy_weights;
};

//============================================================================
MetaStrategy::MetaStrategy(String name, SharedPtr<Exchange> exchange,
                           StrategyAllocatorConfig config, double starting_cash,
                           Option<SharedPtr<StrategyAllocator>> parent) noexcept
    : StrategyAllocator(std::move(name), *exchange, std::move(config),
                        std::move(parent)) {
  getTracer().setStartingCash(starting_cash);
  m_impl = std::make_unique<MetaStrategyImpl>();
  m_impl->meta_weights.resize(getAssetCount());
  m_impl->meta_weights.setZero();
  enableMetaClass();
}

//============================================================================
MetaStrategy::~MetaStrategy() noexcept {}

//============================================================================
const Eigen::Ref<const Eigen::VectorXd>
MetaStrategy::getAllocationBuffer() const noexcept {
  return m_impl->meta_weights;
}

//============================================================================
void MetaStrategy::allocate() noexcept {
  // allocate the parent strategy based on meta impl, default to taking
  // the mean of the child strategy allocations not this operates in place
  // if allocate virtual step
  //
  // weights holds an M by N matrix of portfolio weights for the child
  // strategies where M is the number of assets and N is the number of child
  // strategies, to get the parent allocation, get the row-wise weighted mean
  assert(m_impl->weights.cols() == m_impl->child_strategy_weights.rows());
  assert(m_impl->meta_weights.rows() == m_impl->weights.rows());
  m_impl->meta_weights = (m_impl->weights.array().rowwise() *
                          m_impl->child_strategy_weights.array().transpose())
                             .rowwise()
                             .sum();
}

//============================================================================
void MetaStrategy::step() noexcept {
  if (!m_impl->child_strategies.size()) {
    return;
  }
  evaluate(m_impl->meta_weights);

#pragma omp parallel for
  for (int i = 0; i < m_impl->child_strategies.size(); i++) {
    auto target_weights = m_impl->weights.col(static_cast<int>(i));
    auto &strategy = m_impl->child_strategies[i];
    strategy->stepBase(target_weights);
    if (strategy->getIsMetaClass()) {
      auto meta_strategy = static_cast<MetaStrategy *>(strategy.get());
      meta_strategy->step();
    }
  }
  stepBase(m_impl->meta_weights);
}

//============================================================================
void MetaStrategy::step(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept {
  allocate();
}

//============================================================================
Option<SharedPtr<StrategyAllocator>>
MetaStrategy::addStrategy(SharedPtr<StrategyAllocator> allocator,
                          bool replace_if_exists) noexcept {
  if (m_impl->strategy_map.contains(allocator->getName())) {
    if (!replace_if_exists) {
      disable("Allocator already exists");
      return std::nullopt;
    }
    // find the Allocator and replace it in the vector
    auto idx = m_impl->strategy_map[allocator->getName()];
    allocator->load();
    m_impl->child_strategies[idx] = std::move(allocator);
    return m_impl->child_strategies[idx];
  }
  allocator->setID(m_impl->child_strategies.size());
  allocator->load();
  auto exception_opt = getException();
  if (exception_opt) {
    return std::nullopt;
  }
  // reshape matrix/vector containers holding sub strategy weightings
  m_impl->strategy_map[allocator->getName()] = m_impl->child_strategies.size();
  m_impl->child_strategies.push_back(std::move(allocator));
  m_impl->weights.resize(getAssetCount(), m_impl->child_strategies.size());
  m_impl->weights.setZero();
  m_impl->child_strategy_weights.resize(m_impl->child_strategies.size());
  int i = 0;
  for (auto const &strategy : m_impl->child_strategies) {
    m_impl->child_strategy_weights[i] = strategy->getAllocation();
    i++;
  }
  return m_impl->child_strategies.back();
}

//============================================================================
const Eigen::Ref<const Eigen::VectorXd> MetaStrategy::getAllocationBuffer(
    NonNullPtr<StrategyAllocator const> strategy) const noexcept {
  auto it = m_impl->strategy_map.find(strategy->getName());
  assert(it != m_impl->strategy_map.end());
  auto idx = it->second;
  return m_impl->weights.col(static_cast<int>(idx));
}

//============================================================================
void MetaStrategy::reset() noexcept {
  m_impl->weights.setZero();
  m_impl->meta_weights.setZero();
  for (auto &strategy : m_impl->child_strategies) {
    strategy->reset();
  }
}

//============================================================================
void MetaStrategy::load() noexcept {}

END_FASTTEST_NAMESPACE