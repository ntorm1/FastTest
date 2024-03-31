#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_ast_enums.hpp"
#include "ft_buffer_node.hpp"
#include "standard/ft_types.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
class ReduceOpNode final : public BufferOpNode {
private:
  Vector<std::pair<ReduceOpType, double>> m_reduce_ops;

public:
  ReduceOpNode(SharedPtr<BufferOpNode> parent,
               Vector<std::pair<ReduceOpType, double>> filters) noexcept;
  FASTTEST_API ~ReduceOpNode() noexcept;

  [[nodiscard]] auto const &getOps() const noexcept { return m_reduce_ops; }
  [[nodiscard]] NonNullPtr<BufferOpNode const> getParent() const noexcept;

  void reset() noexcept override { resetBase(); }
  bool isSame(NonNullPtr<BufferOpNode const> other) const noexcept override;
  FASTTEST_API void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
};

END_AST_NAMESPACE