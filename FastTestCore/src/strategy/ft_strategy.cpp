#include "strategy/ft_strategy.hpp"
#include "strategy/ft_meta_strategy.hpp"

BEGIN_FASTTEST_NAMESPACE

//============================================================================
struct StrategyImpl {};

//============================================================================
Strategy::Strategy(String name, SharedPtr<Exchange> exchange,
                   StrategyAllocatorConfig config,
                   SharedPtr<StrategyAllocator> parent) noexcept
    : StrategyAllocator(StrategyType::STRATEGY, std::move(name), *exchange,
                        std::move(config), std::move(parent)) {
  m_impl = std::make_unique<StrategyImpl>();
}

//============================================================================
Strategy::~Strategy() noexcept {}

//============================================================================
void Strategy::reset() noexcept {}

//============================================================================
bool Strategy::load() noexcept { return true; }

//============================================================================
const Eigen::Ref<const Eigen::VectorXd>
Strategy::getAllocationBuffer() const noexcept {
  auto parent = getParent();
  assert(parent);
  auto meta_strategy = static_cast<MetaStrategy *>(parent.value().get());
  return meta_strategy->getAllocationBuffer(this);
}

END_FASTTEST_NAMESPACE
