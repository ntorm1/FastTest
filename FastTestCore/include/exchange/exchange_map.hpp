#pragma once
#include "ft_types.hpp"


BEGIN_FASTTEST_NAMESPACE

class Exchange;
struct ExchangeMapImpl;

//============================================================================
class ExchangeMap {
  friend class FTManager;

private:
  UniquePtr<ExchangeMapImpl> m_impl;

  void build() noexcept;
  void reset() noexcept;
  void step() noexcept;
  void cleanup() noexcept;

  FastTestResult<SharedPtr<Exchange>>
  addExchange(String name, String source,
              Option<String> datetime_format = std::nullopt) noexcept;
  FastTestResult<SharedPtr<Exchange>>
  getExchange(String const &name) const noexcept;

public:
  ExchangeMap() noexcept;
  ~ExchangeMap() noexcept;
  ExchangeMap(const ExchangeMap &) = delete;
  ExchangeMap(ExchangeMap &&) = delete;
  ExchangeMap &operator=(const ExchangeMap &) = delete;
  ExchangeMap &operator=(ExchangeMap &&) = delete;
};

END_FASTTEST_NAMESPACE