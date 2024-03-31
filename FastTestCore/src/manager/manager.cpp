#include "exchange/exchange_map.hpp"
#include "manager/ft_manager.hpp"
#include "standard/ft_macros.hpp"
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
Option<SharedPtr<Exchange>>
FTManager::addExchange(String name, String source,
                       Option<String> datetime_format) noexcept {
  if (m_state != FTManagerState::INIT && m_state != FTManagerState::BUILT) {
    ADD_EXCEPTION_TO_IMPL("Hydra must be in init state to add exchange");
    return std::nullopt;
  }
  auto res = m_impl->exchange_map.addExchange(
      std::move(name), std::move(source), std::move(datetime_format));
  if (!res) {
    ADD_EXCEPTION_TO_IMPL(
        std::format("Failed to add exchange: {}", res.error().what()).c_str());
    return std::nullopt;
  }
  m_state = FTManagerState::INIT;
  return std::move(*res);
}

//============================================================================
Option<SharedPtr<Exchange>>
FTManager::getExchange(String const &name) const noexcept {
  auto res = m_impl->exchange_map.getExchange(name);
  if (!res) {
    ADD_EXCEPTION_TO_IMPL(
        std::format("error getting exchange: {}", res.error().what()).c_str());
    return std::nullopt;
  }
  return *res;
}

//============================================================================
Option<SharedPtr<MetaStrategy>>
FTManager::addStrategy(SharedPtr<MetaStrategy> allocator,
                       bool replace_if_exists) noexcept {
  if (m_state != FTManagerState::BUILT && m_state != FTManagerState::FINISHED) {
    ADD_EXCEPTION_TO_IMPL(
        "Hydra must be in build or finished state to add Allocator");
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
Vector<FastTestException> const &FTManager::getExceptions() const noexcept {
  return m_impl->exceptions;
}

//============================================================================
Int64 FTManager::getGlobalTime() const noexcept {
  return m_impl->exchange_map.getGlobalTime();
}

//============================================================================
void FTManager::step() noexcept {
  assert(m_state == FTManagerState::BUILT || m_state == FTManagerState::RUNING);
  m_impl->exchange_map.step();
#pragma omp parallel
  for (int i = 0; i < m_impl->m_strategies.size(); i++) {
    auto &allocator = m_impl->m_strategies[i];
    allocator->step();
  }
  m_state = FTManagerState::RUNING;
}

//============================================================================
void FTManager::reset() noexcept {
  m_impl->exchange_map.reset();
  for (auto &allocator : m_impl->m_strategies) {
    allocator->reset();
  }
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