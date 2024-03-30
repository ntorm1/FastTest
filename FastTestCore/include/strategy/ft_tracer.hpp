#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_declare.hpp"
#include "ft_types.hpp"

BEGIN_FASTTEST_NAMESPACE

//============================================================================
class Tracer {
  friend class StrategyAllocator;
  friend class MetaStrategy;

private:
  Exchange const &m_exchange;
  StrategyAllocator const &m_allocator;
  Vector<SharedPtr<Measure>> m_measures;
  size_t m_idx = 0;
  double m_nlv = 0.0;
  double m_cash = 0.0;
  double m_starting_cash = 0.0;

private:
  void setNLV(double nlv) noexcept { m_nlv = nlv; }
  void setStartingCash(double cash) noexcept;
  void evaluate() noexcept;
  void reset() noexcept;

public:
  Tracer(Exchange const &exchange, StrategyAllocator const &allocator) noexcept;
  ~Tracer() noexcept;

  [[nodiscard]] auto const &getAllocator() const noexcept {
    return m_allocator;
  }
  [[nodiscard]] auto const &getExchange() const noexcept { return m_exchange; }
  [[nodiscard]] auto const &getMeasures() const noexcept { return m_measures; }
  [[nodiscard]] auto const &getIdx() const noexcept { return m_idx; }
  [[nodiscard]] double getNLV() const noexcept { return m_nlv; }
  [[nodiscard]] double getCash() const noexcept { return m_cash; }
};

END_FASTTEST_NAMESPACE