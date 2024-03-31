#include "strategy/ft_tracer.hpp"
#include "exchange/exchange.hpp"
#include "ast/ft_allocation.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
struct AllocationNodeImpl {
  AllocationType type;
  double epsilon;
  Option<double> alloc_param;
  bool copy_weights_buffer = false;
  NonNullPtr<Tracer> tracer;
  SharedPtr<BufferOpNode> parent;

  AllocationNodeImpl(SharedPtr<BufferOpNode> _parent,
                     NonNullPtr<Tracer> _tracer, AllocationType _type,
                     double _epsilon, Option<double> _alloc_param) noexcept
      : type(_type), epsilon(_epsilon), alloc_param(_alloc_param),
        tracer(_tracer), parent(_parent) {}
};

//============================================================================
AllocationNode::AllocationNode(SharedPtr<BufferOpNode> parent,
                               NonNullPtr<Tracer> tracer, AllocationType type,
                               double epsilon,
                               Option<double> alloc_param) noexcept
    : BufferOpNode(parent->getExchange(), NodeType::ALLOCATION,
                   parent->getWarmup(), parent.get()) {
  m_impl = std::make_unique<AllocationNodeImpl>(parent, std::move(tracer), type,
                                                epsilon, alloc_param);
  if (epsilon != 0) {
		m_impl->copy_weights_buffer = true;
	}
}

//============================================================================
AllocationNode::~AllocationNode() noexcept {}

//============================================================================
void AllocationNode::reset() noexcept {}

//============================================================================
bool AllocationNode::isSame(
    NonNullPtr<BufferOpNode const> other) const noexcept {
  return false;
}

//============================================================================
void AllocationNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  // if we have a commission manager or trade watcher we need to copy the
  // current weights buffer into the commission manager buffer before it gets
  // overwritten by the ast.
  //
  // Additionally if weight epsilon is set we need to copy the current weights
  // so that any adjustments made are reverted back if the absolute value of the
  // difference is less than epsilon
  if (m_impl->copy_weights_buffer) {
    m_impl->tracer->m_weights_buffer = target;
  }

  // generate new target weights using derived class implementation
  buildAllocation(target);

  // if epsilon is set we need to check if the difference between the current
  // weights and the new weights is less than epsilon. If it is we need to
  // revert the weights back to the original weights before calculating any
  // commissions
  if (m_impl->epsilon > 0) {
    target = ((target - m_impl->tracer->m_weights_buffer).cwiseAbs().array() <
              m_impl->epsilon)
                 .select(m_impl->tracer->m_weights_buffer, target);
  }
  // if epsilons is less than 0, revert weights back to original if the sign is
  // the same or if it closed. If the trade switched sides then allow the trade
  // to go through
  else if (m_impl->epsilon < 0) {
    target = ((target.array() * m_impl->tracer->m_weights_buffer.array()) > 0)
                 .select(m_impl->tracer->m_weights_buffer, target);
  }
}

//============================================================================
void AllocationNode::buildAllocation(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  // evaluate the exchange view to calculate the signal
  m_impl->parent->evaluate(target);

  // apply exchange mask to the signal, prevents allocation to assets that have
  // non data at the current time step. Only call when the mask is required (i.e. has element with 0)
  // this will flip all values in target to nan where mask is overriding.
  if (getExchange().maskRequired()) {
    auto mask = getExchange().getMaskSlice(0);
    target = target.cwiseProduct(mask);
  }

  // calculate the number of non-NaN elements in the signal
  size_t nonNanCount =
      target.unaryExpr([](double x) { return !std::isnan(x) ? 1 : 0; }).sum();
  double c;
  if (nonNanCount > 0) {
    c = (1.0) / static_cast<double>(nonNanCount);
  } else {
    c = 0.0;
  }

  switch (m_impl->type) {
  case AllocationType::NLARGEST:
  case AllocationType::NSMALLEST:
  case AllocationType::UNIFORM: {
    target = target.unaryExpr([c](double x) { return x == x ? c : 0.0; });
    break;
  }
  case AllocationType::CONDITIONAL_SPLIT: {
    // conditional split takes the target array and sets all elemetns that
    // are less than the alloc param to -c and all elements greater than the
    // alloc param to c. All other elements are set to 0.0
    target =
        (target.array() < *m_impl->alloc_param)
            .select(-c,
                    (target.array() > *m_impl->alloc_param).select(c, target));
    target = target.unaryExpr([](double x) { return x == x ? x : 0.0; });
    break;
  }
  case AllocationType::NEXTREME: {
    assert(false);
  }
  }
}

//============================================================================
AllocationType AllocationNode::getType() const noexcept { return m_impl->type; }

//============================================================================
double AllocationNode::getAllocEpsilon() const noexcept {
  return m_impl->epsilon;
}

END_AST_NAMESPACE
