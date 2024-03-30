#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_base_node.hpp"
#include "ft_linalg.hpp"
#include "ft_types.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
class BufferOpNode
    : public OpperationNode<void, LinAlg::EigenRef<LinAlg::EigenVectorXd>> {
  friend class Exchange;

private:
  LinAlg::EigenMatrixXd m_cache;

protected:
  [[nodiscard]] size_t getAssetCount() const noexcept;

public:
  BufferOpNode(Exchange &exchange, NodeType t, size_t warmup,
               Option<NonNullPtr<ASTNode>> parent) noexcept;
  BufferOpNode(Exchange &exchange, NodeType t, size_t warmup,
               Vector<NonNullPtr<ASTNode>> parent) noexcept;
  ~BufferOpNode() noexcept;
};

END_AST_NAMESPACE
