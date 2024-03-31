#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_linalg.hpp"
#include "ft_types.hpp"
#include "ft_declare.hpp"

BEGIN_FASTTEST_NAMESPACE

struct ExchangeImpl;
class ExchangeMap;

//============================================================================
class Exchange {
  friend class ExchangeMap;
  friend class StrategyAllocator;

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
  void step(Int64 global_time) noexcept;
  void setExchangeOffset(size_t offset) noexcept;
  void registerAllocator(NonNullPtr<StrategyAllocator> allocator) noexcept;

public:
  Exchange(String name, String source, size_t id,
           Option<String> datetime_format) noexcept;
  ~Exchange() noexcept;

  [[nodiscard]] SharedPtr<AST::ObserverNode>
  registerObserver(SharedPtr<AST::ObserverNode> &&node) noexcept;
  [[nodiscard]] Option<size_t>
  getColumnIndex(String const &column) const noexcept;
  [[nodiscard]] size_t getAssetCount() const noexcept;
  [[nodiscard]] String const &getName() const noexcept { return m_name; }
  [[nodiscard]] String const &getSource() const noexcept { return m_source; }
  [[nodiscard]] size_t getId() const noexcept { return m_id; }
  [[nodiscard]] LinAlg::EigenMatrixXd const &getData() const noexcept;
  [[nodiscard]] LinAlg::EigenVectorXd const &getReturnsScalar() const noexcept;
  [[nodiscard]] LinAlg::EigenBlockView<double>
  getMarketReturnsBlock(size_t start_idex, size_t end_idx) const noexcept;
  [[nodiscard]] LinAlg::EigenConstColView<double>
  getSlice(size_t column, int row_offset) const noexcept;
  [[nodiscard]] LinAlg::EigenConstColView<double>
  getMarketReturns(int offset) const noexcept;
  [[nodiscard]] LinAlg::EigenConstRowView<double>
  getAssetSlice(size_t asset_index) const noexcept;
  
  [[nodiscard]] FASTTEST_API size_t getCurrentIdx();
  [[nodiscard]] FASTTEST_API Vector<Int64> const &
  getTimestamps() const noexcept;
  [[nodiscard]] FASTTEST_API Option<size_t>
  getAssetIndex(String const &asset) const noexcept;
  [[nodiscard]] FASTTEST_API Map<String, size_t> const &
  getAssetMap() const noexcept;
  [[nodiscard]] FASTTEST_API Option<SharedPtr<AST::ObserverNode>>
  getObserver(String const &name) const noexcept;
};

END_FASTTEST_NAMESPACE