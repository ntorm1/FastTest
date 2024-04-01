#include "strategy/ft_allocator.hpp"
#include "strategy/ft_tracer.hpp"

#include "exchange/exchange.hpp"

BEGIN_FASTTEST_NAMESPACE

//============================================================================
struct StrategyAllocatorImpl {
  bool step_call = false;
  StrategyType strategy_type;
  StrategyAllocatorConfig m_config;
  Tracer m_tracer;
  Option<FastTestException> exception = std::nullopt;
  Option<SharedPtr<StrategyAllocator>> m_parent = std::nullopt;
  Exchange &exchange;

  StrategyAllocatorImpl(StrategyType type, StrategyAllocator const &allocator,
                        Exchange &exchange, StrategyAllocatorConfig config,
                        Option<SharedPtr<StrategyAllocator>> parent) noexcept
      : exchange(exchange), m_config(std::move(config)), strategy_type(type),
        m_parent(std::move(parent)), m_tracer(exchange, allocator) {}
};

//============================================================================
StrategyAllocator ::~StrategyAllocator() noexcept {}

//============================================================================
StrategyAllocator::StrategyAllocator(
    StrategyType type, String name, Exchange &exchange,
    StrategyAllocatorConfig config,
    Option<SharedPtr<StrategyAllocator>> parent) noexcept
    : m_name(std::move(name)) {
  m_impl = std::make_unique<StrategyAllocatorImpl>(
      type, *this, exchange, std::move(config), std::move(parent));
  exchange.registerAllocator(this);
}

//============================================================================
void StrategyAllocator::resetBase() noexcept {
  m_impl->step_call = false;
  m_impl->m_tracer.reset();
  reset();
}

//============================================================================
void StrategyAllocator::setStepCall(bool step_call) noexcept {
  m_impl->step_call = step_call;
}

//============================================================================
void StrategyAllocator::stepBase(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept {
  if (m_impl->exception) {
    return;
  }
  if (!m_impl->step_call) {
    validate(target_weights_buffer);
    return;
  }
  step(target_weights_buffer);
  validate(target_weights_buffer);
  evaluate(target_weights_buffer);
  m_impl->step_call = false;
}

//============================================================================
void StrategyAllocator::disable(String const &exception) noexcept {
  m_impl->exception = FastTestException(exception);
}

//============================================================================
size_t StrategyAllocator::getAssetCount() const noexcept {
  return m_impl->exchange.getAssetCount();
}

//============================================================================
Exchange &StrategyAllocator::getExchange() const noexcept {
  return m_impl->exchange;
}

StrategyType StrategyAllocator::getType() const noexcept {
  return m_impl->strategy_type;
}

//============================================================================
Option<double>
StrategyAllocator::getAssetAllocation(size_t index) const noexcept {
  auto const &buffer = getAllocationBuffer();
  if (index >= static_cast<size_t>(buffer.size())) {
    return std::nullopt;
  }
  return buffer[index];
}

//============================================================================
Tracer &StrategyAllocator::getMutTracer() noexcept { return m_impl->m_tracer; }

//============================================================================
Tracer const &StrategyAllocator::getTracer() const noexcept { return m_impl->m_tracer; }

//============================================================================
double StrategyAllocator::getAllocation() const noexcept {
  return m_impl->m_config.allocation;
}

//============================================================================
bool StrategyAllocator::getIsMetaClass() const noexcept {
  return m_impl->strategy_type == StrategyType::META_STRATEGY;
}

//============================================================================
Option<FastTestException> StrategyAllocator::getException() const noexcept {
  return m_impl->exception;
}

//============================================================================
Option<SharedPtr<StrategyAllocator>>
StrategyAllocator::getParent() const noexcept {
  return m_impl->m_parent;
}

//============================================================================
void StrategyAllocator::validate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target_weights_buffer) noexcept {
  if (!m_impl->m_config.can_short && target_weights_buffer.minCoeff() < 0) {
    return disable("Shorting is not allowed");
  }
  if (m_impl->m_config.weight_clip) {
    if (m_impl->m_config.disable_on_breach &&
        target_weights_buffer.maxCoeff() > *(m_impl->m_config.weight_clip)) {
      return disable("Weight clip breach");
    }
    target_weights_buffer =
        target_weights_buffer.cwiseMax(*(m_impl->m_config.weight_clip));
  }
}

//============================================================================
void StrategyAllocator::takeException(
    Vector<FastTestException> &exceptions) noexcept {
  if (m_impl->exception) {
    exceptions.push_back(*(m_impl->exception));
    m_impl->exception = std::nullopt;
  }
}

//============================================================================
void StrategyAllocator::evaluate(LinAlg::EigenRef<LinAlg::EigenVectorXd> const
                                     &target_weights_buffer) noexcept {
  if (m_impl->exception) {
    return;
  }
  // get the current market returns
  auto market_returns = m_impl->exchange.getMarketReturns(0);

  // get the portfolio return by calculating the sum product of the market
  // returns and the portfolio weights
  assert(market_returns.rows() == target_weights_buffer.rows());
  assert(!market_returns.array().isNaN().any());

  // print out target weights buffer and market returns
  double portfolio_return = market_returns.dot(target_weights_buffer);

  // update the tracer nlv
  double nlv = m_impl->m_tracer.getNLV();
  m_impl->m_tracer.setNLV(nlv * (1.0 + portfolio_return));
  m_impl->m_tracer.evaluate();
}

//============================================================================
void StrategyAllocator::lateRebalance(
    Eigen::Ref<Eigen::VectorXd> target_weights_buffer) noexcept {
  // if the strategy does not override the target weights buffer at the end of
  // a time step, then we need to rebalance the portfolio to the target
  // weights buffer according to the market returns update the target weights
  // buffer according to the indivual asset returns
  target_weights_buffer =
      m_impl->exchange.getReturnsScalar().cwiseProduct(target_weights_buffer);
  assert(!target_weights_buffer.array().isNaN().any());
}

END_FASTTEST_NAMESPACE