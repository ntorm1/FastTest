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

struct MetaStrategyImpl;

class MetaStrategy final : public StrategyAllocator {
private:
  UniquePtr<MetaStrategyImpl> m_impl;

  void allocate() noexcept;

public:
  FASTTEST_API
  MetaStrategy(String name, SharedPtr<Exchange> exchange,
               StrategyAllocatorConfig config, double starting_cash,
               Option<SharedPtr<StrategyAllocator>> parent) noexcept;
  ~MetaStrategy() noexcept;

  /// <summary>
  /// Call child strategy step functions to generate weights, then excute meta
  /// strategy logic. after child weight matrix has been populated
  /// </summary>
  void step() noexcept;

  /// <summary>
  /// Execute meta strategy logic by allocating weights across all child
  /// strategies.
  /// </summary>
  /// <param name="target_weights_buffer"></param>
  void step(LinAlg::EigenRef<LinAlg::EigenVectorXd>
                target_weights_buffer) noexcept override;

  /// <summary>
  /// Get the meta strategy's target weights buffer.
  /// </summary>
  /// <returns></returns>
  [[nodiscard]] const LinAlg::EigenRef<const LinAlg::EigenVectorXd>
  getAllocationBuffer() const noexcept override;

  [[nodiscard]] const Eigen::Ref<const Eigen::VectorXd> getAllocationBuffer(
      NonNullPtr<StrategyAllocator const> strategy) const noexcept;

  /// <summary>
  /// Add a new strategy into the meta strategy.
  /// </summary>
  /// <param name="allocator">New strategy to insert into the tree</param>
  /// <param name="replace_if_exists">replace a strategy with the same name
  /// if it exsits. If a strategy exsists with the same name and this
  /// parameter is false, the strategy will disabel and store an exception
  /// </param>
  /// <returns>Strategy if it was added successfully, else none</returns>
  [[nodiscard]] Option<SharedPtr<StrategyAllocator>>
  addStrategy(SharedPtr<StrategyAllocator> allocator,
              bool replace_if_exists) noexcept;
  void reset() noexcept override;
  void load() noexcept override;
};

END_FASTTEST_NAMESPACE