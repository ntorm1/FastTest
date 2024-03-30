#include "manager/ft_manager.hpp"

#include "exchange/exchange_map.hpp"

BEGIN_FASTTEST_NAMESPACE

struct FTManagerImpl {
  ExchangeMap m_exchange_map;

};

//============================================================================
FTManager::FTManager() noexcept {
  m_impl = std::make_unique<FTManagerImpl>();
}

//============================================================================
FTManager::~FTManager() noexcept {}

//============================================================================
FastTestResult<SharedPtr<Exchange>>
FTManager::addExchange(String name, String source,
                       Option<String> datetime_format) noexcept {
  if (m_state != FTManagerState::INIT && m_state != FTManagerState::BUILT) {
    return Err("Hydra must be in init state to add exchange");
  }
  auto res = m_impl->m_exchange_map.addExchange(
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
  return m_impl->m_exchange_map.getExchange(name);
}

END_FASTTEST_NAMESPACE