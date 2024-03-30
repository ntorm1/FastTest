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

struct BenchMarkStrategyImpl;

//============================================================================
class BenchMarkStrategy final : public StrategyAllocator {
private:
  UniquePtr<BenchMarkStrategyImpl> m_impl;

public:
  FASTTEST_API
  BenchMarkStrategy(String name, SharedPtr<Exchange> exchange,
                    SharedPtr<StrategyAllocator> parent,
                    StrategyAllocatorConfig config,
                    Vector<std::pair<String, double>> const &allocations,
                    bool rebalance = false) noexcept;
  FASTTEST_API ~BenchMarkStrategy() noexcept;

  [[nodiscard]] size_t getWarmup() const noexcept override { return 0; }
  void step(LinAlg::EigenRef<LinAlg::EigenVectorXd>
                target_weights_buffer) noexcept override;
  [[nodiscard]] const LinAlg::EigenRef<const LinAlg::EigenVectorXd>
  getAllocationBuffer() const noexcept override;

  void reset() noexcept override;
  void load() noexcept override;
};

END_FASTTEST_NAMESPACE