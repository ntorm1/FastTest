#include "ast/ft_reduce.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
ReduceOpNode::ReduceOpNode(SharedPtr<BufferOpNode> parent,
                           Vector<std::pair<ReduceOpType, double>> ops) noexcept
    : BufferOpNode(parent->getExchange(), NodeType::REDUCE_OP,
                   parent->getWarmup(), parent.get()),
      m_reduce_ops(ops) {}

//============================================================================
ReduceOpNode::~ReduceOpNode() noexcept {}

NonNullPtr<BufferOpNode const> ReduceOpNode::getParent() const noexcept {
  auto& parents = getParents();
  assert(parents.size() == 1);
  return static_cast<BufferOpNode const*>(parents[0].get());
}

//============================================================================
bool ReduceOpNode::isSame(NonNullPtr<BufferOpNode const> other) const noexcept {
  if (other->getType() != NodeType::REDUCE_OP) {
		return false;
  }
  auto other_reduce = static_cast<ReduceOpNode const *>(other.get());
  return other_reduce->getOps() == m_reduce_ops &&
         getParent()->isSame(other_reduce->getParent());
}

//============================================================================
void ReduceOpNode::evaluate(
    LinAlg::EigenRef<LinAlg::EigenVectorXd> target) noexcept {
  for (auto const &filter : m_reduce_ops) {
    double c = filter.second;
    switch (filter.first) {
    case ReduceOpType::GREATER_THAN:
      target = (target.array() > c)
                   .select(target, std::numeric_limits<double>::quiet_NaN());
      break;
    case ReduceOpType::LESS_THAN:
      target = (target.array() < c)
                   .select(target, std::numeric_limits<double>::quiet_NaN());
      break;
    case ReduceOpType::EQUAL:
      target = (target.array().abs() == c)
                 .select(target, std::numeric_limits<double>::quiet_NaN());
      break;
    }
  }
}

END_AST_NAMESPACE