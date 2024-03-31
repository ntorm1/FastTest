#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_allocator.hpp"
#include "standard/ft_declare.hpp"
#include "standard/ft_types.hpp"

BEGIN_FASTTEST_NAMESPACE

struct ASTStrategyImpl;

//============================================================================
class ASTStrategy final : public StrategyAllocator {
private:
  UniquePtr<ASTStrategyImpl> m_impl;

public:
  FASTTEST_API
  ASTStrategy(String name, SharedPtr<StrategyAllocator> parent,
              StrategyAllocatorConfig config,
              SharedPtr<AST::AllocationNode> allocation) noexcept;
  FASTTEST_API ~ASTStrategy() noexcept;

  [[nodiscard]] size_t getWarmup() const noexcept override;
  void step(LinAlg::EigenRef<LinAlg::EigenVectorXd>
                target_weights_buffer) noexcept override;
  [[nodiscard]] const LinAlg::EigenRef<const LinAlg::EigenVectorXd>
  getAllocationBuffer() const noexcept override;

  void reset() noexcept override;
  void load() noexcept override;
};

END_FASTTEST_NAMESPACE