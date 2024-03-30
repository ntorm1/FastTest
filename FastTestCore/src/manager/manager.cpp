#include "ft_macros.hpp"
#include "manager/ft_manager.hpp"
#include "exchange/exchange_map.hpp"
#include "strategy/ft_meta_strategy.hpp"

BEGIN_FASTTEST_NAMESPACE



struct FTManagerImpl {
  ExchangeMap exchange_map;
  Vector<FastTestException> exceptions;
  Vector<SharedPtr<MetaStrategy>> m_strategies;
  Map<String, size_t> m_strategy_map;
};

//============================================================================
FTManager::FTManager() noexcept { m_impl = std::make_unique<FTManagerImpl>(); }

//============================================================================
FTManager::~FTManager() noexcept {}

//============================================================================
FastTestResult<SharedPtr<Exchange>>
FTManager::addExchange(String name, String source,
                       Option<String> datetime_format) noexcept {
  if (m_state != FTManagerState::INIT && m_state != FTManagerState::BUILT) {
    return Err("Hydra must be in init state to add exchange");
  }
  auto res = m_impl->exchange_map.addExchange(
      std::move(name), std::move(source), std::move(datetime_format));
  if (!res) {
    return res;
  }
  m_state = FTManagerState::INIT;
  return std::move(res);
}

//============================================================================
FastTestResult<SharedPtr<Exchange>>
FTManager::getExchange(String const &name) const noexcept {
  return m_impl->exchange_map.getExchange(name);
}

//============================================================================
Option<SharedPtr<MetaStrategy>>
FTManager::addStrategy(SharedPtr<MetaStrategy> allocator,
                       bool replace_if_exists) noexcept {
  if (m_state != FTManagerState::BUILT && m_state != FTManagerState::FINISHED) {
    ADD_EXCEPTION_TO_IMPL("Hydra must be in build or finished state to add Allocator");
    return std::nullopt;
  }
  if (m_impl->m_strategy_map.contains(allocator->getName())) {
    if (!replace_if_exists) {
      ADD_EXCEPTION_TO_IMPL("Allocator with name " + allocator->getName() +
                 " already exists");
      return std::nullopt;
    }
    // find the Allocator and replace it in the vector
    auto idx = m_impl->m_strategy_map[allocator->getName()];
    allocator->load();
    m_impl->m_strategies[idx] = std::move(allocator);
    return m_impl->m_strategies[idx];
  }
  allocator->setID(m_impl->m_strategies.size());
  allocator->load();
  m_impl->m_strategy_map[allocator->getName()] = m_impl->m_strategies.size();
  m_impl->m_strategies.push_back(std::move(allocator));
  return m_impl->m_strategies.back();
}

//============================================================================
Int64 FTManager::getGlobalTime() const noexcept {
  return m_impl->exchange_map.getGlobalTime();
}

//============================================================================
void FTManager::step() noexcept {
  assert(m_state == FTManagerState::BUILT || m_state == FTManagerState::RUNING);
  m_impl->exchange_map.step();
  m_state = FTManagerState::RUNING;
}

//============================================================================
void FTManager::reset() noexcept {
  m_impl->exchange_map.reset();
  m_state = FTManagerState::BUILT;
}

//============================================================================
FastTestResult<bool> FTManager::build() noexcept {
  m_impl->exchange_map.build();
  if (m_impl->exchange_map.getTimestamps().size() == 0) {
    return Err("{}", "No timestamps found");
  }
  m_state = FTManagerState::BUILT;
  return true;
}

END_FASTTEST_NAMESPACE