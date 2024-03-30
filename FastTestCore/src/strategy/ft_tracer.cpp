#include "strategy/ft_tracer.hpp"
#include "strategy/ft_measure.hpp"


BEGIN_FASTTEST_NAMESPACE

//============================================================================
Tracer::Tracer(Exchange const &exchange,
               StrategyAllocator const &allocator) noexcept
    : m_exchange(exchange), m_allocator(allocator) {}

//============================================================================
Tracer::~Tracer() noexcept {}

//============================================================================
void Tracer::setStartingCash(double cash) noexcept {
  m_starting_cash = cash;
	m_cash = cash;
}

//============================================================================
void Tracer::evaluate() noexcept {
  for (auto &m : m_measures) {
    m->measure(m_idx);
  }
  m_idx++;
}

//============================================================================
void Tracer::reset() noexcept {
  m_idx = 0;
  for (auto& m : m_measures) {
		m->reset();
	}
  m_cash = m_starting_cash;
  m_nlv = m_starting_cash;
}

END_FASTTEST_NAMESPACE