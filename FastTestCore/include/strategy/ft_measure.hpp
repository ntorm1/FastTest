#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_declare.hpp"
#include "ft_linalg.hpp"
#include "ft_types.hpp"
#include "ft_enums.hpp"

BEGIN_FASTTEST_NAMESPACE

//============================================================================
class Measure {
  friend class Tracer;

private:
  virtual void measure(size_t m_idx) noexcept = 0;
  void reset() noexcept;

protected:
  Exchange const &m_exchange;
  NonNullPtr<Tracer const> m_tracer;
  TracerType m_type;
  LinAlg::EigenMatrixXd m_values;

public:
  Measure(TracerType type, Exchange const &exchange,
          NonNullPtr<Tracer const> tracer) noexcept;
  virtual ~Measure() noexcept;
  auto const &getValues() const noexcept { return m_values; }
  TracerType getType() const noexcept { return m_type; }
};

//============================================================================
class NLVMeasure final : public Measure {
private:
  void measure(size_t m_idx) noexcept override;

public:
  NLVMeasure(Exchange const &exchange,
             NonNullPtr<Tracer const> tracer) noexcept;
  ~NLVMeasure() noexcept;
};

//============================================================================
class WeightMeasure final : public Measure {
private:
  void measure(size_t m_idx) noexcept override;

public:
  WeightMeasure(Exchange const &exchange,
                NonNullPtr<Tracer const> tracer) noexcept;
  ~WeightMeasure() noexcept;
};

//============================================================================
class VolatilityMeasure final : public Measure {
private:
  size_t m_days_per_year = 252;
  void measure(size_t m_idx) noexcept override;

public:
  VolatilityMeasure(Exchange const &exchange, NonNullPtr<Tracer const> tracer,
                    size_t days_per_year) noexcept;
  ~VolatilityMeasure() noexcept;
};

END_FASTTEST_NAMESPACE