#pragma once
#ifdef FASTTEST_EXPORTS
#define FASTTEST_API __declspec(dllexport)
#else
#define FASTTEST_API __declspec(dllimport)
#endif
#include "ft_macros.hpp"

BEGIN_AST_NAMESPACE

//============================================================================
enum class BinOpType { ADD, SUB, MUL, DIV };

//============================================================================
enum class ObserverType {
  SUM = 0,
  MEAN = 1,
  ATR = 2,
  MAX = 3,
  TS_ARGMAX = 4,
  VARIANCE = 5,
  COVARIANCE = 6,
  CORRELATION = 7,
  LINEAR_DECAY = 8,
  SKEWNESS = 9,
};

END_AST_NAMESPACE
