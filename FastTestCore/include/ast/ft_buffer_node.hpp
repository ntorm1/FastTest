#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_base_node.hpp"
#include "standard/ft_linalg.hpp"
#include "standard/ft_types.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
class BufferOpNode
    : public OpperationNode<void, LinAlg::EigenRef<LinAlg::EigenVectorXd>> {
  friend class FastTest::Exchange;

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

  /// <summary>
  /// Deep search if two nodes are the same. I.e. they are the same type, have
  /// the same parents, and the same parameters
  /// </summary>
  /// <param name="other"></param>
  /// <returns></returns>
  [[nodiscard]] virtual bool
  isSame(NonNullPtr<BufferOpNode const> other) const noexcept = 0;

  /// <summary>
  /// Build cache history for node. It is M x N matrix where M is the number of
  /// assets and N is the number of timesteps for the parent exchange. After the
  /// first run if the node has not been changed subsequent runs will load in
  /// from the cache
  /// </summary>
  /// <param name="v"></param>
  void enableCache(bool v) noexcept;

  /// <summary>
  /// Test of observer as full cache history enabled
  /// </summary>
  /// <returns></returns>
  [[nodiscard]] bool hasCache() const noexcept { return m_cache.cols() > 1; }

  /// <summary>
  /// Get internal address of this object, used for debugging
  /// </summary>
  /// <returns></returns>
  [[nodiscard]] FASTTEST_API uintptr_t address() const noexcept {
    return reinterpret_cast<uintptr_t>(this);
  }

  /// <summary>
  /// Get view into the buffer's cache at a specific column index, if no column
  /// it defaults to the exchange's current row index
  /// </summary>
  /// <param name="col"></param>
  /// <returns></returns>
  [[nodiscard]] LinAlg::EigenRef<LinAlg::EigenVectorXd>
  cacheColumn(Option<size_t> col = std::nullopt) noexcept;
};

END_AST_NAMESPACE
