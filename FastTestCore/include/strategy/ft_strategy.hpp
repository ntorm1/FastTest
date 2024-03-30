#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_allocator.hpp"
#include "ft_declare.hpp"
#include "ft_types.hpp"

BEGIN_FASTTEST_NAMESPACE

struct StrategyImpl;

class Strategy final : public StrategyAllocator {
private:
  UniquePtr<StrategyImpl> m_impl;

public:
  FASTTEST_API Strategy(String name, SharedPtr<Exchange> exchange,
                        StrategyAllocatorConfig config,
                        SharedPtr<StrategyAllocator> parent) noexcept;
  ~Strategy() noexcept;

  /// <summary>
  /// Get the meta strategy's target weights buffer.
  /// </summary>
  /// <returns></returns>
  [[nodiscard]] const LinAlg::EigenRef<const LinAlg::EigenVectorXd>
  getAllocationBuffer() const noexcept override;

  void step(LinAlg::EigenRef<LinAlg::EigenVectorXd>
                target_weights_buffer) noexcept override;

  void reset() noexcept override;
  void load() noexcept override;
};

END_FASTTEST_NAMESPACE