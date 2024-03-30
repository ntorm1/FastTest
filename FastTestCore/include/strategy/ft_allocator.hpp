#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_declare.hpp"
#include "ft_linalg.hpp"
#include "ft_types.hpp"

BEGIN_FASTTEST_NAMESPACE

struct StrategyAllocatorImpl;

//============================================================================
struct StrategyAllocatorConfig {
  bool can_short = true;
  double allocation = 1.0f;
  bool disable_on_breach = true;
  Option<double> vol_target = std::nullopt;
  Option<double> weight_clip = std::nullopt;
  Option<double> vol_limit = std::nullopt;
  Option<double> risk_contrib_limit = std::nullopt;
  StrategyAllocatorConfig() noexcept = default;
  ~StrategyAllocatorConfig() noexcept = default;
};

//============================================================================
class StrategyAllocator {
  friend class Exchange;
  friend class MetaStrategy;
  friend class FTManager;

private:
  UniquePtr<StrategyAllocatorImpl> m_impl;
  String m_name;
  size_t m_id = 0;

  /// <summary>
  /// Set the unique integer id of the strategy, create by the manager and used
  /// to index into strategy vector
  /// </summary>
  /// <param name="id"></param>
  void setID(size_t id) noexcept { m_id = id; }

  /// <summary>
  /// Function called by manager on sim reset, zeros out weights and common
  /// strategy functionality, then calls concrete class reset function
  /// </summary>
  void resetBase() noexcept;

protected:
  /// <summary>
  /// Strategy's never throw, instead if error occurs (like risk limit breach)
  /// and exception is stored and the strategy is disabled. The strategy will
  /// not run again untill the exception is taken out.
  /// </summary>
  /// <param name="exception"></param>
  void disable(String const &exception) noexcept;

  /// <summary>
  /// Get asset count of the underlying exchange, useful for derived classes
  /// sizing matrixx buffers
  /// </summary>
  /// <returns></returns>
  [[nodiscard]] size_t getAssetCount() const noexcept;

  /// <summary>
  /// Given strategy config, validate the current weights match certain limits
  /// </summary>
  /// <param name="target_weights_buffer"></param>
  void validate(
      LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept;

  /// <summary>
  /// When not action is taken by a strategy and the weights are not explicitly
  /// set, then have to adjsut them due to underlying asset returns leading to
  /// deviation from orginal target.
  /// </summary>
  /// <param name="target_weights_buffer"></param>
  void lateRebalance(
      LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept;

  /// <summary>
  /// Given a set of portfolio weights, evaluate the strategy
  /// </summary>
  /// <param name="target_weights_buffer"></param>
  void evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> const
                    &target_weights_buffer) noexcept;

  /// <summary>
  /// If a strategy has an exception, take it and push into passed vector.
  /// </summary>
  /// <param name="exceptions"></param>
  virtual void takeException(Vector<FastTestException> &exceptions) noexcept;

  /// <summary>
  /// Enable the meta class for the strategy informing base to search through
  /// child strategies when needed
  /// </summary>
  void enableMetaClass() noexcept;

  /// <summary>
  /// Get mutable reference to tracer instance for the class
  /// </summary>
  /// <returns></returns>
  Tracer &getTracer() noexcept;

public:
  virtual ~StrategyAllocator() noexcept;
  StrategyAllocator(String name, Exchange &exchange,
                    StrategyAllocatorConfig config,
                    Option<SharedPtr<StrategyAllocator>> parent) noexcept;

  /// <summary>
  /// Check current strategy state, perform validation, then call virtual step
  /// function
  /// </summary>
  /// <param name="target_weights_buffer"></param>
  void stepBase(
      LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept;

  /// <summary>
  /// Main execution logic of the strategy to be defined by concrete class
  /// </summary>
  /// <param name="target_weights_buffer"></param>
  virtual void step(LinAlg::EigenRef<LinAlg::EigenVectorXd>
                        target_weights_buffer) noexcept = 0;

  /// <summary>
  /// If warmup is N, strategy's step function is called on Nth index
  /// </summary>
  /// <returns></returns>
  [[nodiscard]] virtual size_t getWarmup() const noexcept = 0;

  /// <summary>
  /// Get read only view into the current target weights of the strategy
  /// </summary>
  /// <returns></returns>
  [[nodiscard]] virtual const LinAlg::EigenRef<const LinAlg::EigenVectorXd>
  getAllocationBuffer() const noexcept = 0;

  /// <summary>
  /// Function call every time manager resets
  /// </summary>
  virtual void reset() noexcept = 0;

  /// <summary>
  /// Function called once on strategy creation
  /// </summary>
  virtual void load() = 0;

  /// <summary>
  /// Peek at the exception if one exists
  /// </summary>
  /// <returns></returns>
  Option<FastTestException> getException() const noexcept;

  [[nodiscard]] String const &getName() const noexcept { return m_name; }
  [[nodiscard]] double getAllocation() const noexcept;
  [[nodiscard]] bool getIsMetaClass() const noexcept;
  [[nodiscard]] Option<SharedPtr<StrategyAllocator>> getParent() const noexcept;
};

END_FASTTEST_NAMESPACE