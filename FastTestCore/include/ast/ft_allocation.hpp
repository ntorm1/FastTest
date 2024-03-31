#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_ast_enums.hpp"
#include "ft_buffer_node.hpp"
#include "standard/ft_linalg.hpp"
#include "ft_observer_base.hpp"
#include "standard/ft_types.hpp"

BEGIN_AST_NAMESPACE

struct AllocationNodeImpl;

//============================================================================
class AllocationNode final : public BufferOpNode {
private:
  UniquePtr<AllocationNodeImpl> m_impl;

  void buildAllocation(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept;


public:
  AllocationNode(SharedPtr<BufferOpNode> parent,NonNullPtr<Tracer>, AllocationType type, 
                 double epsilon, Option<double> alloc_param) noexcept;
  ~AllocationNode() noexcept;

  void reset() noexcept override;
  bool isSame(NonNullPtr<BufferOpNode const> other) const noexcept override;
  void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;

  [[nodiscard]] AllocationType getType() const noexcept;
  [[nodiscard]] double getAllocEpsilon() const noexcept;
};

END_AST_NAMESPACE