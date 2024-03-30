#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_types.hpp"

BEGIN_FASTTEST_NAMESPACE

struct FTManagerImpl;
class Exchange;

enum class FTManagerState { INIT = 0, BUILT = 1, RUNING = 2, FINISHED = 3 };



//============================================================================
class FTManager {
private:
  UniquePtr<FTManagerImpl> m_impl;
  FTManagerState m_state = FTManagerState::INIT;


public:
  FASTTEST_API FTManager() noexcept;
  FASTTEST_API  ~FTManager() noexcept;

  [[nodiscard]] FASTTEST_API FastTestResult<SharedPtr<Exchange>>
  addExchange(String name, String source,
              Option<String> datetime_format = std::nullopt) noexcept;
  [[nodiscard]] FASTTEST_API FastTestResult<SharedPtr<Exchange>>
  getExchange(String const &name) const noexcept;
};

END_FASTTEST_NAMESPACE