#include "strategy/ft_benchmark.hpp"
#include "exchange/exchange.hpp"
#include "strategy/ft_meta_strategy.hpp"

BEGIN_FASTTEST_NAMESPACE

//============================================================================
struct BenchMarkStrategyImpl {
  bool rebalance = false;
  bool is_first_step = true;
  Vector<std::pair<String, double>> allocations_str;
  Vector<std::pair<size_t, double>> allocations;

  BenchMarkStrategyImpl(Vector<std::pair<String, double>> const &in_allocs,
                        bool rebalance = false)
      : allocations_str(in_allocs), rebalance(rebalance) {}
};

//============================================================================
BenchMarkStrategy::BenchMarkStrategy(
    String name, SharedPtr<Exchange> exchange,
    SharedPtr<StrategyAllocator> parent, StrategyAllocatorConfig config,
    Vector<std::pair<String, double>> const &allocations,
    bool rebalance) noexcept
    : StrategyAllocator(StrategyType::BENCHMARK_STRATEGY, std::move(name),
                        *exchange, std::move(config), std::move(parent)) {
  m_impl = std::make_unique<BenchMarkStrategyImpl>(allocations, rebalance);
}

//============================================================================
BenchMarkStrategy::~BenchMarkStrategy() noexcept {}

//============================================================================
void BenchMarkStrategy::load() noexcept {
  m_impl->allocations.clear();
  auto const &exchange = getExchange();
  for (auto const &[asset, weight] : m_impl->allocations_str) {
    auto index_opt = exchange.getAssetIndex(asset);
    if (!index_opt) {
      disable(std::format("Asset {} not found", asset));
    }
    m_impl->allocations.push_back({*index_opt, weight});
  }
}

//============================================================================
const Eigen::Ref<const Eigen::VectorXd>
BenchMarkStrategy::getAllocationBuffer() const noexcept {
  auto parent = getParent();
  assert(parent);
  auto meta_strategy = static_cast<MetaStrategy *>(parent.value().get());
  return meta_strategy->getAllocationBuffer(this);
}

//============================================================================
void BenchMarkStrategy::reset() noexcept { m_impl->is_first_step = true; }

//============================================================================
void BenchMarkStrategy::step(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept {
  if (!m_impl->is_first_step && !m_impl->rebalance) {
    return;
  }
  assert(target_weights_buffer.size() == m_impl->allocations.size());
  if (m_impl->is_first_step || m_impl->rebalance) {
    m_impl->is_first_step = false;
    for (size_t i = 0; i < m_impl->allocations.size(); ++i) {
      target_weights_buffer(i) = m_impl->allocations[i].second;
    }
  }
}

END_FASTTEST_NAMESPACE