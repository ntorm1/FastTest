#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_types.hpp"

BEGIN_FASTTEST_NAMESPACE

struct ExchangeImpl;
class ExchangeMap;

//============================================================================
class Exchange {
  friend class ExchangeMap;
private:
  UniquePtr<ExchangeImpl> m_impl;
  String m_name;
  String m_source;
  size_t m_id;

  [[nodiscard]] FastTestResult<bool> init() noexcept;
  [[nodiscard]] FastTestResult<bool> initDir() noexcept;
  [[nodiscard]] FastTestResult<bool> validate() noexcept;
  [[nodiscard]] FastTestResult<bool> build() noexcept;

  void reset() noexcept;
  void setExchangeOffset(size_t offset) noexcept;

public:
  Exchange(String name, String source, size_t id,
           Option<String> datetime_format) noexcept;
  ~Exchange() noexcept;

  String const &getName() const noexcept { return m_name; }
  String const &getSource() const noexcept { return m_source; }
  size_t getId() const noexcept { return m_id; }
  [[nodiscard]] FASTTEST_API Vector<Int64> const &
  getTimestamps() const noexcept;
  [[nodiscard]] FASTTEST_API Option<size_t>
  getAssetIndex(String const &asset) const noexcept;
  [[nodiscard]] FASTTEST_API Map<String, size_t> const &
  getAssetMap() const noexcept;
};

END_FASTTEST_NAMESPACE