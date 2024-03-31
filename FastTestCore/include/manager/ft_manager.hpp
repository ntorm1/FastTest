#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "standard/ft_types.hpp"

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
  FASTTEST_API ~FTManager() noexcept;

  [[nodiscard]] FASTTEST_API Option<SharedPtr<Exchange>>
  addExchange(String name, String source,
              Option<String> datetime_format = std::nullopt) noexcept;
  [[nodiscard]] FASTTEST_API Option<SharedPtr<Exchange>>
  getExchange(String const &name) const noexcept;

  [[nodiscard]] FASTTEST_API Option<SharedPtr<MetaStrategy>>
  addStrategy(SharedPtr<MetaStrategy> Allocator,
              bool replace_if_exists = false) noexcept;
  FASTTEST_API Vector<FastTestException> getExceptions(bool take = false) const noexcept;
  FASTTEST_API Int64 getGlobalTime() const noexcept;
  FASTTEST_API void step() noexcept;
  FASTTEST_API void reset() noexcept;
  FASTTEST_API FTManagerState getState() const noexcept { return m_state;}
  FASTTEST_API [[nodiscard]] bool run() noexcept;
  FASTTEST_API [[nodiscard]] bool build() noexcept;
};

END_FASTTEST_NAMESPACE