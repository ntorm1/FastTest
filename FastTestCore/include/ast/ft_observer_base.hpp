#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_ast_enums.hpp"
#include "ft_buffer_node.hpp"
#include "ft_linalg.hpp"
#include "ft_types.hpp"
#include <atomic>

BEGIN_AST_NAMESPACE

//============================================================================
class ObserverNode : public BufferOpNode {
  friend class FastTest::Exchange;

private:
  /// <summary>
  ///	Defines the first index in which to call the child's cacheObserver
  /// method
  /// </summary>
  size_t m_observer_warmup = 0;

  /// <summary>
  /// Rolling window size of observation
  /// </summary>
  size_t m_window = 0;

  /// <summary>
  /// Option string id of the observer to be used when strategy wants to
  /// search for an exsisting observer
  /// </summary>
  Option<String> m_name;

  /// <summary>
  /// Buffer matrix holding previous observations of the observer.
  /// </summary>
  LinAlg::EigenMatrixXd m_buffer_matrix;

  /// <summary>
  /// Vector holding child observers that depend on this observer to run before
  /// they do
  /// </summary>
  Vector<SharedPtr<ObserverNode>> m_children;

  /// <summary>
  /// Base parent for the observer used for evaluation
  /// </summary>
  SharedPtr<BufferOpNode> m_parent;

  /// <summary>
  ///	Current index into the buffer to write to. On cacheObserver, the
  /// m_buffer_idx - 1
  /// column is overwritten with the new observation. Then onOutOfRange is
  /// called on m_buffer_idx to update the observer with the column that is
  /// about to be overwritten in the next step
  /// </summary>
  size_t m_buffer_idx = 0;

  /// <summary>
  /// Concrete implementation type of the observer
  /// </summary>
  ObserverType m_observer_type;

  /// <summary>
  /// Zero out the buffer matrix and call reset on derived implementations
  /// </summary>
  void resetBase() noexcept;

  /// <summary>
  /// On exchange step method is called to update the observer with the new
  /// observation
  /// </summary>
  void cacheBase() noexcept;

  /// <summary>
  /// Atomic counter used to signal when the observer should update given all
  /// parent observers have stepped
  /// </summary>
  std::atomic<int> m_parent_observer_count = 0;

  /// <summary>
  /// Max number of parent observers, used to signal when the atomic counter has
  /// reached it's max
  /// </summary>
  int m_parent_observer_max = 0;

protected:
  LinAlg::EigenVectorXd m_signal;
  LinAlg::EigenVectorXd m_signal_copy;

  /// <summary>
  /// Get const ref to the signal copy
  /// </summary>
  /// <returns></returns>
  auto const &getSignalCopy() const noexcept { return m_signal_copy; }

  /// <summary>
  /// Get a mutable ref into the observers buffer matrix at the current index
  /// </summary>
  /// <returns></returns>
  LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer() noexcept;

public:
  ObserverNode(SharedPtr<BufferOpNode> parent, ObserverType observer_type,
               size_t window, Option<String> name) noexcept;

  virtual ~ObserverNode() noexcept = default;

  /// <summary>
  /// Called when column is out of range from the rolling window
  /// </summary>
  /// <param name="buffer_old"></param>
  virtual void
  onOutOfRange(LinAlg::EigenRef<LinAlg::EigenVectorXd> buffer_old) noexcept = 0;

  /// <summary>
  /// Pure virtual method defining the observer's evaluation method
  /// </summary>
  virtual void cacheObserver() noexcept = 0;

  /// <summary>
  /// Pure virtual method defining the observer's reset method called on
  /// exchange reset
  /// </summary>
  virtual void reset() noexcept override = 0;

  /// <summary>
  /// Method for determining if two observers are the same. In the event they
  /// are, when I copy is attempted to be added to exchange observers, it will
  /// return a copy of the exsisting shared pointer
  /// </summary>
  /// <param name="other"></param>
  /// <returns></returns>
  bool
  isSame(NonNullPtr<BufferOpNode const> other) const noexcept final override;

  /// <summary>
  /// Register new child observer to parent
  /// </summary>
  /// <param name="child"></param>
  void insertChild(SharedPtr<AST::ObserverNode> child) noexcept {
    m_children.push_back(std::move(child));
  }

  /// <summary>
  /// Actual evaluation of the observer, copies the signal buffer into the
  /// target. Optionally allows override if observer is a managed view of
  /// another observer.
  /// </summary>
  /// <param name="target"></param>
  FASTTEST_API void evaluate(
      LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept final override;

  [[nodiscard]] ObserverType getObserverType() const noexcept {
    return m_observer_type;
  }
  [[nodiscard]] size_t getBufferIdx() const noexcept { return m_buffer_idx; }
  [[nodiscard]] size_t getWindow() const noexcept { return m_window; }
  [[nodiscard]] FASTTEST_API size_t getParentObserverCount() const noexcept {
    return m_parent_observer_max;
  }
};

END_AST_NAMESPACE