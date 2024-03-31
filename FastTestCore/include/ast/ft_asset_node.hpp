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
class ReadOpNode final : public BufferOpNode {
private:
  size_t m_column;
  int m_row_offset;
  size_t m_warmup;

public:
  ReadOpNode(Exchange &exchange, size_t column, int row_offset) noexcept;
  ~ReadOpNode() noexcept;

  [[nodiscard]] int getRowOffset() const noexcept { return m_row_offset; }
  [[nodiscard]] size_t getColumn() const noexcept { return m_column; }

  void reset() noexcept override{};
  bool isSame(NonNullPtr<BufferOpNode const> other) const noexcept override;

  FASTTEST_API void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
};

//============================================================================
class BinOpNode final : public BufferOpNode {
private:
  SharedPtr<BufferOpNode> m_asset_op_left;
  SharedPtr<BufferOpNode> m_asset_op_right;
  LinAlg::EigenVectorXd m_right_buffer;
  BinOpType m_op_type;

public:
  BinOpNode(Exchange &exchange, SharedPtr<BufferOpNode> left, BinOpType op_type,
            SharedPtr<BufferOpNode> right) noexcept;
  ~BinOpNode() noexcept;

  [[nodiscard]] NonNullPtr<BufferOpNode const> left() const noexcept {
    return m_asset_op_left.get();
  }
  [[nodiscard]] NonNullPtr<BufferOpNode const> right() const noexcept {
    return m_asset_op_right.get();
  }

  void reset() noexcept override;
  bool isSame(NonNullPtr<BufferOpNode const> other) const noexcept override;

  FASTTEST_API void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
};

//============================================================================
class UnaryOpNode final : public BufferOpNode {
private:
  UnaryOpType m_op_type;
  SharedPtr<BufferOpNode> m_parent;
  Option<double> m_func_param;

public:
  UnaryOpNode(Exchange &exchange, SharedPtr<BufferOpNode> parent,
              UnaryOpType op_type, Option<double> func_param) noexcept;
  ~UnaryOpNode() noexcept;

  void reset() noexcept override;
  bool isSame(NonNullPtr<BufferOpNode const> other) const noexcept override;
  [[nodiscard]] NonNullPtr<BufferOpNode const> getParent() const noexcept {
    return m_parent.get();
  }
  [[nodiscard]] UnaryOpType getOpType() const noexcept { return m_op_type; }

  FASTTEST_API void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
};

END_AST_NAMESPACE