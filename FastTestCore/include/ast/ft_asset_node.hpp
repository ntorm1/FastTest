#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_buffer_node.hpp"
#include "ft_types.hpp"

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
  FASTTEST_API void
  evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept override;
};

END_AST_NAMESPACE